// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdarwinwebview_p.h"
#include <QtWebView/qwebview.h>
#include <QtWebView/qwebviewloadinginfo.h>
#include <QtWebView/private/qwebviewfactory_p.h>
#include "qtwebviewfunctions.h"

#include <QtCore/private/qglobal_p.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <QtCore/qvariant.h>

#include <QtQuick/qquickrendercontrol.h>
#include <QtQuick/qquickwindow.h>

#include <CoreFoundation/CoreFoundation.h>
#include <WebKit/WebKit.h>

#include <QtCore/qjsondocument.h>
#include <QtCore/qfile.h>

#ifdef Q_OS_IOS
#import <UIKit/UIKit.h>
#endif

#ifdef Q_OS_MACOS
#include <AppKit/AppKit.h>

typedef NSView UIView;
#endif

@interface QtWKWebViewDelegate : NSObject<WKNavigationDelegate> {
    QPointer<QDarwinWebViewPrivate> qDarwinWebViewPrivate;
}
- (QtWKWebViewDelegate *)initWithQWebViewPrivate:(QDarwinWebViewPrivate *)webViewPrivate;
- (void)pageDone;
- (void)handleError:(NSError *)error;

// protocol:
- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation;
- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation;
- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation
      withError:(NSError *)error;
- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation
      withError:(NSError *)error;

@end

@implementation QtWKWebViewDelegate
- (QtWKWebViewDelegate *)initWithQWebViewPrivate:(QDarwinWebViewPrivate *)webViewPrivate
{
    if ((self = [super init])) {
        Q_ASSERT(webViewPrivate);
        qDarwinWebViewPrivate = webViewPrivate;
    }
    return self;
}

- (void)pageDone
{
    if (!qDarwinWebViewPrivate)
        return;
    emit qDarwinWebViewPrivate->q_ptr->loadProgressChanged(qDarwinWebViewPrivate->loadProgress());
}

- (void)handleError:(NSError *)error
{
    if (!qDarwinWebViewPrivate)
        return;
    [self pageDone];
    NSString *errorString = [error localizedDescription];
    NSURL *failingURL = error.userInfo[@"NSErrorFailingURLKey"];
    const QUrl url = [failingURL isKindOfClass:[NSURL class]]
                        ? QUrl::fromNSURL(failingURL) : qDarwinWebViewPrivate->url();
    emit qDarwinWebViewPrivate->q_ptr->loadingChanged(
            QWebViewFactory::LoadingInfo::create(url, QWebViewLoadingInfo::LoadStatus::Failed,
                                QString::fromNSString(errorString)));
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation
{
    Q_UNUSED(webView);
    if (!qDarwinWebViewPrivate)
        return;
    // WKNavigationDelegate gives us per-frame notifications while the QWebView API
    // should provide per-page notifications. Therefore we keep track of the last frame
    // to be started, if that finishes or fails then we indicate that it has loaded.
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        qDarwinWebViewPrivate->wkNavigation = navigation;
    else
        return;

    QUrl url = qDarwinWebViewPrivate->url();
    emit qDarwinWebViewPrivate->q_ptr->urlChanged(url);
    emit qDarwinWebViewPrivate->q_ptr->loadingChanged(
            QWebViewFactory::LoadingInfo::create(url,
                                QWebViewLoadingInfo::LoadStatus::Started, QString()));
    emit qDarwinWebViewPrivate->q_ptr->loadProgressChanged(qDarwinWebViewPrivate->loadProgress());
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    Q_UNUSED(webView);
    if (!qDarwinWebViewPrivate)
        return;
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        return;

    [self pageDone];
    emit qDarwinWebViewPrivate->q_ptr->loadingChanged(
            QWebViewFactory::LoadingInfo::create(qDarwinWebViewPrivate->url(),
                                QWebViewLoadingInfo::LoadStatus::Succeeded, QString()));
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation
      withError:(NSError *)error
{
    Q_UNUSED(webView);
    if (!qDarwinWebViewPrivate)
        return;
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        return;
    [self handleError:error];
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation
      withError:(NSError *)error
{
    Q_UNUSED(webView);
    if (!qDarwinWebViewPrivate)
        return;
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        return;
    [self handleError:error];
}

- (void)webView:(WKWebView *)webView
decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction
                decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
                __attribute__((availability(ios_app_extension,unavailable)))
{
    Q_UNUSED(webView);
    NSURL *url = navigationAction.request.URL;
    const BOOL handled = (^{
        // For links with target="_blank", open externally
        if (!navigationAction.targetFrame)
            return NO;

#if QT_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(101300, 110000)
        if (__builtin_available(macOS 10.13, iOS 11.0, *)) {
            return [WKWebView handlesURLScheme:url.scheme];
        } else
#endif
        {
            // +[WKWebView handlesURLScheme:] is a stub that calls
            // WebCore::SchemeRegistry::isBuiltinScheme();
            // replicate that as closely as possible
            return [@[
                @"about", @"applewebdata", @"blob", @"data",
                @"file", @"http", @"https", @"javascript",
#ifdef Q_OS_MACOS
                @"safari-extension",
#endif
                @"webkit-fake-url", @"wss", @"x-apple-content-filter",
#ifdef Q_OS_MACOS
                @"x-apple-ql-id"
#endif
                ] containsObject:url.scheme];
        }
    })();
    if (!handled) {
#ifdef Q_OS_MACOS
        [[NSWorkspace sharedWorkspace] openURL:url];
#elif defined(Q_OS_IOS)
        // Check if it can be opened first, if it is a file scheme then it can't
        // be opened, therefore if it is a _blank target in that case we need to open
        // inside the current webview
        if ([[UIApplication sharedApplication] canOpenURL:url])
            [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        else if (!navigationAction.targetFrame)
            [webView loadRequest:navigationAction.request];
#endif
    }
    decisionHandler(handled ? WKNavigationActionPolicyAllow : WKNavigationActionPolicyCancel);
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change
                       context:(void *)context {
    Q_UNUSED(object);
    Q_UNUSED(change);
    Q_UNUSED(context);
    if ([keyPath isEqualToString:@"estimatedProgress"]) {
        emit qDarwinWebViewPrivate->q_ptr->loadProgressChanged(
                qDarwinWebViewPrivate->loadProgress());
    } else if ([keyPath isEqualToString:@"title"]) {
        emit qDarwinWebViewPrivate->q_ptr->titleChanged(qDarwinWebViewPrivate->title());
    }
}

@end

QT_BEGIN_NAMESPACE

QDarwinWebViewSettingsPrivate::QDarwinWebViewSettingsPrivate(WKWebViewConfiguration *conf)
    :  m_conf(conf)
{

}

bool QDarwinWebViewSettingsPrivate::doTestAttribute(WebAttribute attribute) const
{
    switch (attribute) {
    case QWebViewSettings::WebAttribute::LocalStorageEnabled:
        return localStorageEnabled();
    case QWebViewSettings::WebAttribute::JavaScriptEnabled:
        return javaScriptEnabled();
    case QWebViewSettings::WebAttribute::AllowFileAccess:
        return allowFileAccess();
    case QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls:
        return localContentCanAccessFileUrls();
    }
    return false;
}

void QDarwinWebViewSettingsPrivate::doSetAttribute(WebAttribute attribute, bool value)
{
    switch (attribute) {
    case QWebViewSettings::WebAttribute::LocalStorageEnabled:
        setLocalStorageEnabled(value);
        break;
    case QWebViewSettings::WebAttribute::JavaScriptEnabled:
        setJavaScriptEnabled(value);
        break;
    case QWebViewSettings::WebAttribute::AllowFileAccess:
        setAllowFileAccess(value);
        break;
    case QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls:
        setLocalContentCanAccessFileUrls(value);
        break;
    }
}

bool QDarwinWebViewSettingsPrivate::localStorageEnabled() const
{
    return m_conf.websiteDataStore.persistent;
}

bool QDarwinWebViewSettingsPrivate::javaScriptEnabled() const
{
    // Deprecated
    bool isJsEnabled = false;
#if QT_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(110000, 140000)
    if (__builtin_available(macOS 11.0, iOS 14.0, *))
        isJsEnabled = m_conf.defaultWebpagePreferences.allowsContentJavaScript;
#else
    isJsEnabled = m_conf.preferences.javaScriptEnabled;
#endif
    return isJsEnabled;
}

bool QDarwinWebViewSettingsPrivate::localContentCanAccessFileUrls() const
{
    return m_localContentCanAccessFileUrls;
}

bool QDarwinWebViewSettingsPrivate::allowFileAccess() const
{
    return m_allowFileAccess;
}

void QDarwinWebViewSettingsPrivate::setLocalContentCanAccessFileUrls(bool enabled)
{
    // This will be checked in QDarwinWebViewPrivate::setUrl()
    m_localContentCanAccessFileUrls = enabled;
}

void QDarwinWebViewSettingsPrivate::setJavaScriptEnabled(bool enabled)
{
#if QT_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(110000, 140000)
    if (__builtin_available(macOS 11.0, iOS 14.0, *))
        m_conf.defaultWebpagePreferences.allowsContentJavaScript = enabled;
#else
    m_conf.preferences.javaScriptEnabled = enabled;
#endif
}

void QDarwinWebViewSettingsPrivate::setLocalStorageEnabled(bool enabled)
{
    if (enabled == localStorageEnabled())
        return;

    if (enabled)
        m_conf.websiteDataStore = [WKWebsiteDataStore defaultDataStore];
    else
        m_conf.websiteDataStore = [WKWebsiteDataStore nonPersistentDataStore];
}

void QDarwinWebViewSettingsPrivate::setAllowFileAccess(bool enabled)
{
    // This will be checked in QDarwinWebViewPrivate::setUrl()
    m_allowFileAccess = enabled;
}

QDarwinWebViewPrivate::QDarwinWebViewPrivate(QWebView *view) : QWebViewPrivate(view), wkWebView(nil)
{
    CGRect frame = CGRectMake(0.0, 0.0, 400, 400);
    wkWebView = [[WKWebView alloc] initWithFrame:frame];
    wkWebView.navigationDelegate = [[QtWKWebViewDelegate alloc] initWithQWebViewPrivate:this];
    [wkWebView addObserver:wkWebView.navigationDelegate forKeyPath:@"estimatedProgress"
                   options:NSKeyValueObservingOptions(NSKeyValueObservingOptionNew)
                   context:nil];
    [wkWebView addObserver:wkWebView.navigationDelegate forKeyPath:@"title"
                   options:NSKeyValueObservingOptions(NSKeyValueObservingOptionNew)
                   context:nil];

    m_window = QWindow::fromWinId(reinterpret_cast<WId>(wkWebView));
    if (m_window) {
        m_window->setParent(view);
        connect(view, &QWindow::widthChanged, m_window, &QWindow::setWidth);
        connect(view, &QWindow::heightChanged, m_window, &QWindow::setHeight);
        connect(view, &QWindow::visibleChanged, m_window, &QWindow::setVisible);
    }
    m_settings = new QDarwinWebViewSettingsPrivate(wkWebView.configuration);
}

QDarwinWebViewPrivate::~QDarwinWebViewPrivate()
{
    [wkWebView stopLoading];
    [wkWebView removeObserver:wkWebView.navigationDelegate forKeyPath:@"estimatedProgress"
                      context:nil];
    [wkWebView removeObserver:wkWebView.navigationDelegate forKeyPath:@"title"
                      context:nil];
    [wkWebView.navigationDelegate release];
    wkWebView.navigationDelegate = nil;
    [wkWebView release];
    if (m_window)
        delete m_window;
}

QUrl QDarwinWebViewPrivate::url() const
{
    return QUrl::fromNSURL(wkWebView.URL);
}

void QDarwinWebViewPrivate::setUrl(const QUrl &url)
{
    if (url.isValid()) {
        if (url.isLocalFile()) {
            // NOTE: Check if the file exists before attempting to load it, we follow the same
            // asynchronous pattern as expected to not break the tests (Started + Failed).
            if (!QFile::exists(url.toLocalFile())) {
                QMetaObject::invokeMethod(
                        q_ptr, &QWebView::loadingChanged, Qt::QueuedConnection,
                        QWebViewFactory::LoadingInfo::create(url, QWebViewLoadingInfo::LoadStatus::Started,
                                            {}));
                QMetaObject::invokeMethod(
                        q_ptr, &QWebView::loadingChanged, Qt::QueuedConnection,
                        QWebViewFactory::LoadingInfo::create(url, QWebViewLoadingInfo::LoadStatus::Failed,
                                            QStringLiteral("File does not exist")));
                return;
            }
            // We need to pass local files via loadFileURL and the read access should cover
            // the directory that the file is in, to facilitate loading referenced images etc
            if (m_settings->allowFileAccess()) {
                if (m_settings->localContentCanAccessFileUrls())
                    [wkWebView loadFileURL:url.toNSURL() allowingReadAccessToURL:QUrl(url.toString(QUrl::RemoveFilename)).toNSURL()];
                else
                    [wkWebView loadRequest:[NSURLRequest requestWithURL:url.toNSURL()]];
            }
        } else {
            [wkWebView loadRequest:[NSURLRequest requestWithURL:url.toNSURL()]];
        }
    } else {
        QMetaObject::invokeMethod(
                q_ptr, &QWebView::loadingChanged, Qt::QueuedConnection,
                QWebViewFactory::LoadingInfo::create(url, QWebViewLoadingInfo::LoadStatus::Failed,
                                    QStringLiteral("Invalid URL")));
    }
}

void QDarwinWebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    [wkWebView loadHTMLString:html.toNSString() baseURL:baseUrl.toNSURL()];
}

bool QDarwinWebViewPrivate::canGoBack() const
{
    return wkWebView.canGoBack;
}

bool QDarwinWebViewPrivate::canGoForward() const
{
    return wkWebView.canGoForward;
}

QString QDarwinWebViewPrivate::title() const
{
    return QString::fromNSString(wkWebView.title);
}

int QDarwinWebViewPrivate::loadProgress() const
{
    return int(wkWebView.estimatedProgress * 100);
}

bool QDarwinWebViewPrivate::isLoading() const
{
    return wkWebView.loading;
}

void QDarwinWebViewPrivate::goBack()
{
    [wkWebView goBack];
}

void QDarwinWebViewPrivate::goForward()
{
    [wkWebView goForward];
}

void QDarwinWebViewPrivate::stop()
{
    [wkWebView stopLoading];
}

void QDarwinWebViewPrivate::reload()
{
    [wkWebView reload];
}

QVariant fromNSNumber(const NSNumber *number)
{
    if (!number)
        return QVariant();
    if (strcmp([number objCType], @encode(BOOL)) == 0) {
        return QVariant::fromValue(!![number boolValue]);
    } else if (strcmp([number objCType], @encode(signed char)) == 0) {
        return QVariant::fromValue([number charValue]);
    } else if (strcmp([number objCType], @encode(unsigned char)) == 0) {
        return QVariant::fromValue([number unsignedCharValue]);
    } else if (strcmp([number objCType], @encode(signed short)) == 0) {
        return QVariant::fromValue([number shortValue]);
    } else if (strcmp([number objCType], @encode(unsigned short)) == 0) {
        return QVariant::fromValue([number unsignedShortValue]);
    } else if (strcmp([number objCType], @encode(signed int)) == 0) {
        return QVariant::fromValue([number intValue]);
    } else if (strcmp([number objCType], @encode(unsigned int)) == 0) {
        return QVariant::fromValue([number unsignedIntValue]);
    } else if (strcmp([number objCType], @encode(signed long long)) == 0) {
        return QVariant::fromValue([number longLongValue]);
    } else if (strcmp([number objCType], @encode(unsigned long long)) == 0) {
        return QVariant::fromValue([number unsignedLongLongValue]);
    } else if (strcmp([number objCType], @encode(float)) == 0) {
        return QVariant::fromValue([number floatValue]);
    } else if (strcmp([number objCType], @encode(double)) == 0) {
        return QVariant::fromValue([number doubleValue]);
    }
    return QVariant();
}

QVariant fromJSValue(id result)
{
    if ([result isKindOfClass:[NSString class]])
        return QString::fromNSString(static_cast<NSString *>(result));
    if ([result isKindOfClass:[NSNumber class]])
        return fromNSNumber(static_cast<NSNumber *>(result));
    if ([result isKindOfClass:[NSDate class]])
        return QDateTime::fromNSDate(static_cast<NSDate *>(result));

    if ([result isKindOfClass:[NSArray class]]
     || [result isKindOfClass:[NSDictionary class]]) {
        @try {
            // Round-trip via JSON, so we don't have to implement conversion
            // from NSArray and NSDictionary manually.

            // FIXME: NSJSONSerialization requires that any nested object
            // is NSString, NSNumber, NSArray, NSDictionary, or NSNull, so
            // nested NSDates are not supported -- meaning we support plain
            // NSDate (above), but not in an array or dict. To handle this
            // use-case we'd need a manual conversion.
            auto jsonData = QByteArray::fromNSData(
                [NSJSONSerialization dataWithJSONObject:result options:0 error:nil]);

            QJsonParseError parseError;
            auto jsonDocument = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError)
                return jsonDocument.toVariant();
        } @catch (NSException *) {
            return QVariant();
        }
    }

    return QVariant();
}

void QDarwinWebViewPrivate::runJavaScript(
        const QString &script, const std::function<void(const QVariant &)> &resultCallback)
{
    std::function<void(const QVariant &)> callbackCopy = resultCallback;
    QPointer<QDarwinWebViewPrivate> observer(this);
    [wkWebView evaluateJavaScript:script.toNSString()
                completionHandler:^(id result, NSError *error) {
                    QVariant r = error ? QVariant() : fromJSValue(result);
                    if (callbackCopy && observer)
                        callbackCopy(r);
                }];
}

void QDarwinWebViewPrivate::setCookie(const QString &domain, const QString &name, const QString &value)
{
    NSString *cookieDomain = domain.toNSString();
    NSString *cookieName = name.toNSString();
    NSString *cookieValue = value.toNSString();

    WKHTTPCookieStore *cookieStore = wkWebView.configuration.websiteDataStore.httpCookieStore;

    if (cookieStore == nullptr) {
        return;
    }

    NSMutableDictionary *cookieProperties = [NSMutableDictionary dictionary];
    [cookieProperties setObject:cookieName forKey:NSHTTPCookieName];
    [cookieProperties setObject:cookieValue forKey:NSHTTPCookieValue];
    [cookieProperties setObject:cookieDomain forKey:NSHTTPCookieDomain];
    [cookieProperties setObject:cookieDomain forKey:NSHTTPCookieOriginURL];
    [cookieProperties setObject:@"/" forKey:NSHTTPCookiePath];
    [cookieProperties setObject:@"0" forKey:NSHTTPCookieVersion];

    NSHTTPCookie *cookie = [NSHTTPCookie cookieWithProperties:cookieProperties];

    if (cookie == nullptr) {
        return;
    }
    QPointer<QDarwinWebViewPrivate> observer(this);
    [cookieStore setCookie:cookie
            completionHandler:^{
                if (observer)
                    emit q_ptr->cookieAdded(QString::fromNSString(cookie.domain),
                                            QString::fromNSString(cookie.name));
            }];
}

void QDarwinWebViewPrivate::deleteCookie(const QString &domain, const QString &name)
{
    NSString *cookieDomain = domain.toNSString();
    NSString *cookieName = name.toNSString();

    WKHTTPCookieStore *cookieStore = wkWebView.configuration.websiteDataStore.httpCookieStore;

    if (cookieStore == nullptr) {
        return;
    }

    QPointer<QDarwinWebViewPrivate> observer(this);
    [cookieStore getAllCookies:^(NSArray *cookies) {
        NSHTTPCookie *cookie;
        for (cookie in cookies) {
            if ([cookie.domain isEqualToString:cookieDomain] && [cookie.name isEqualToString:cookieName]) {
                [cookieStore deleteCookie:cookie
                        completionHandler:^{
                            if (observer)
                                emit q_ptr->cookieRemoved(QString::fromNSString(cookie.domain),
                                                          QString::fromNSString(cookie.name));
                        }];
            }
        }
    }];
}

void QDarwinWebViewPrivate::deleteAllCookies()
{
    WKHTTPCookieStore *cookieStore = wkWebView.configuration.websiteDataStore.httpCookieStore;

    QPointer<QDarwinWebViewPrivate> observer(this);
    [cookieStore getAllCookies:^(NSArray *cookies) {
        NSHTTPCookie *cookie;
        for (cookie in cookies) {
            [cookieStore deleteCookie:cookie
                    completionHandler:^{
                        if (observer)
                            emit q_ptr->cookieRemoved(QString::fromNSString(cookie.domain),
                                                      QString::fromNSString(cookie.name));
                    }];
        }
    }];
}

QString QDarwinWebViewPrivate::httpUserAgent() const
{
    return QString::fromNSString([wkWebView valueForKey:@"userAgent"]);
}

void QDarwinWebViewPrivate::setHttpUserAgent(const QString &userAgent)
{
    if (!userAgent.isEmpty()) {
        wkWebView.customUserAgent = userAgent.toNSString();
    }
    emit q_ptr->httpUserAgentStringChanged(userAgent);
}

QWebViewSettingsPrivate *QDarwinWebViewPrivate::settings() const
{
    return m_settings;
}

QT_END_NAMESPACE
