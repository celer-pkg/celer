// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qandroidwebview_p.h"
#include <QtWebView/qwebview.h>
#include <QtWebView/qwebviewloadinginfo.h>
#include <QtWebView/private/qwebviewfactory_p.h>

#include <QtCore/private/qjnihelpers_p.h>
#include <QtCore/qjniobject.h>

#include <QtCore/qset.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qdebug.h>
#include <QtCore/qthread.h>
#include <QtCore/qabstracteventdispatcher.h>

#include <QtGui/qguiapplication.h>

#include <android/bitmap.h>
#include <android/log.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_JNI_CLASS(Bitmap, "android/graphics/Bitmap");

using namespace QtJniTypes;
using namespace Qt::StringLiterals;

QAndroidWebViewSettingsPrivate::QAndroidWebViewSettingsPrivate(
        const WebViewController &viewController)
    : m_viewController(viewController)
{
}

bool QAndroidWebViewSettingsPrivate::doTestAttribute(WebAttribute attribute) const
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

void QAndroidWebViewSettingsPrivate::doSetAttribute(WebAttribute attribute, bool value)
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

bool QAndroidWebViewSettingsPrivate::localStorageEnabled() const
{
     return m_viewController.callMethod<jboolean>("isLocalStorageEnabled");
}

bool QAndroidWebViewSettingsPrivate::javaScriptEnabled() const
{
    return m_viewController.callMethod<jboolean>("isJavaScriptEnabled");
}

bool QAndroidWebViewSettingsPrivate::localContentCanAccessFileUrls() const
{
    return m_viewController.callMethod<jboolean>("isAllowFileAccessFromFileURLsEnabled");
}

bool QAndroidWebViewSettingsPrivate::allowFileAccess() const
{
    return m_viewController.callMethod<jboolean>("isAllowFileAccessEnabled");
}

void QAndroidWebViewSettingsPrivate::setLocalContentCanAccessFileUrls(bool enabled)
{
    m_viewController.callMethod<void>("setAllowFileAccessFromFileURLs", enabled);
}

void QAndroidWebViewSettingsPrivate::setJavaScriptEnabled(bool enabled)
{
    m_viewController.callMethod<void>("setJavaScriptEnabled", enabled);
}

void QAndroidWebViewSettingsPrivate::setLocalStorageEnabled(bool enabled)
{
    m_viewController.callMethod<void>("setLocalStorageEnabled", enabled);
}

void QAndroidWebViewSettingsPrivate::setAllowFileAccess(bool enabled)
{
    m_viewController.callMethod<void>("setAllowFileAccess", enabled);
}

typedef QSet<QAndroidWebViewPrivate *> WebViews;
Q_GLOBAL_STATIC(WebViews, g_webViews)

QAndroidWebViewPrivate::QAndroidWebViewPrivate(QWebView *view)
    : QWebViewPrivate(view),
      m_callbackId(0),
      m_window(nullptr),
      m_viewController(nullptr),
      m_webView(nullptr)
{
    // QtAndroidWebViewController constructor blocks a qGuiThread until
    // the WebView is created and configured in UI thread.
    // That is why we cannot proceed until AndroidDeadlockProtector is locked
    while (!QtAndroidPrivate::acquireAndroidDeadlockProtector()) {
        auto eventDispatcher = QThread::currentThread()->eventDispatcher();
        if (eventDispatcher)
            eventDispatcher->processEvents(
                    QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers);
    }
    m_viewController = WebViewController(QtAndroidPrivate::activity(), reinterpret_cast<jlong>(this));

    QtAndroidPrivate::releaseAndroidDeadlockProtector();

    m_webView = m_viewController.callMethod<WebView>("getWebView");
    m_settings = new QAndroidWebViewSettingsPrivate(m_viewController);

    m_window = QWindow::fromWinId(reinterpret_cast<WId>(m_webView.object()));
    if (m_window) {
        m_window->setParent(view);
        connect(view, &QWindow::widthChanged, m_window, &QWindow::setWidth);
        connect(view, &QWindow::heightChanged, m_window, &QWindow::setHeight);
        connect(view, &QWindow::visibleChanged, m_window, &QWindow::setVisible);
    }
    g_webViews->insert(this);
    connect(qApp, &QGuiApplication::applicationStateChanged,
            this, &QAndroidWebViewPrivate::onApplicationStateChanged);
}

QAndroidWebViewPrivate::~QAndroidWebViewPrivate()
{
    g_webViews->remove(this);
    if (m_window != 0) {
        m_window->setVisible(false);
        m_window->setParent(0);
        delete m_window;
    }

    m_viewController.callMethod<void>("destroy");
}

QString QAndroidWebViewPrivate::httpUserAgent() const
{
    return m_viewController.callMethod<QString>("getUserAgent");
}

void QAndroidWebViewPrivate::setHttpUserAgent(const QString &userAgent)
{
    m_viewController.callMethod<void>("setUserAgent", userAgent);
    emit q_ptr->httpUserAgentStringChanged(userAgent);
}

QUrl QAndroidWebViewPrivate::url() const
{
    return QUrl(m_viewController.callMethod<QString>("getUrl"));
}

void QAndroidWebViewPrivate::setUrl(const QUrl &url)
{
    m_viewController.callMethod<void>("loadUrl", url.toString());
}

void QAndroidWebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    const QString mimeTypeString = u"text/html;charset=UTF-8"_s;
    if (baseUrl.isEmpty() || baseUrl.scheme() == "data"_L1) {
        const QString encoded = QUrl::toPercentEncoding(html);
        m_viewController.callMethod<void>("loadData", encoded, mimeTypeString, jstring(nullptr));
    } else {
        // andorid webview in case of non data baseURL scheme will loaded 'html' into the WebView as
        // a plain string meaning any url encoded entities in the string will not be decoded.
        m_viewController.callMethod<void>("loadDataWithBaseURL", baseUrl.toString(), html,
                                          mimeTypeString, jstring(nullptr),
                                          baseUrl.isValid() ? baseUrl.toString() : QString());
    }
}

bool QAndroidWebViewPrivate::canGoBack() const
{
    return m_viewController.callMethod<bool>("canGoBack");
}

void QAndroidWebViewPrivate::goBack()
{
    m_viewController.callMethod<void>("goBack");
}

bool QAndroidWebViewPrivate::canGoForward() const
{
    return m_viewController.callMethod<bool>("canGoForward");
}

void QAndroidWebViewPrivate::goForward()
{
    m_viewController.callMethod<void>("goForward");
}

void QAndroidWebViewPrivate::reload()
{
    m_viewController.callMethod<void>("reload");
}

QString QAndroidWebViewPrivate::title() const
{
    return m_viewController.callMethod<QString>("getTitle");
}

void QAndroidWebViewPrivate::runJavaScript(
        const QString &script, const std::function<void(const QVariant &)> &resultCallback)
{
    if (QtAndroidPrivate::androidSdkVersion() < 19) {
        qWarning("runJavaScript() requires API level 19 or higher.");
        if (resultCallback)
            resultCallback(QVariant());
        return;
    }

    m_callbacks.insert(m_callbackId, resultCallback);
    m_viewController.callMethod<void>("runJavaScript", script, jlong(m_callbackId));
    ++m_callbackId;
}

QWebViewSettingsPrivate *QAndroidWebViewPrivate::settings() const
{
    return m_settings;
}

void QAndroidWebViewPrivate::setCookie(const QString &domain, const QString &name, const QString &value)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        const QString cookie = name + u'=' + value;
        WebViewController::callStaticMethod<void>("setCookie", jlong(this), domain, cookie);
    });
}

void QAndroidWebViewPrivate::deleteCookie(const QString &domain, const QString &name)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        const QString cookie = name.split(u'=').at(0) + u'=';
        WebViewController::callStaticMethod<void>("removeCookie", jlong(this), domain, cookie);
    });
}

void QAndroidWebViewPrivate::deleteAllCookies()
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        WebViewController::callStaticMethod<void>("removeCookies");
    });
}

int QAndroidWebViewPrivate::loadProgress() const
{
    return m_viewController.callMethod<int>("getProgress");
}

bool QAndroidWebViewPrivate::isLoading() const
{
    return m_viewController.callMethod<bool>("isLoading");
}

void QAndroidWebViewPrivate::stop()
{
    m_viewController.callMethod<void>("stopLoading");
}

void QAndroidWebViewPrivate::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (QtAndroidPrivate::androidSdkVersion() < 11)
        return;

    if (state == Qt::ApplicationActive)
        m_viewController.callMethod<void>("onResume");
    else
        m_viewController.callMethod<void>("onPause");
}

void QAndroidWebViewPrivate::javaScriptResult(int id, const QVariant &result)
{
    auto it = m_callbacks.find(id);
    if (it != m_callbacks.end()) {
        auto callback = it.value();
        if (callback)
            callback(result);
        m_callbacks.erase(it);
    }
}

static void c_onRunJavaScriptResult(JNIEnv *env,
                                    jobject thiz,
                                    jlong id,
                                    jlong callbackId,
                                    jstring result)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    const QString &resultString = QJniObject(result).toString();

    // The result string is in JSON format, lets parse it to see what we got.
    QJsonValue jsonValue;
    const QByteArray &jsonData = "{ \"data\": " + resultString.toUtf8() + " }";
    QJsonParseError error;
    const QJsonDocument &jsonDoc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error == QJsonParseError::NoError && jsonDoc.isObject()) {
        const QJsonObject &object = jsonDoc.object();
        jsonValue = object.value(QStringLiteral("data"));
    }

    wc->javaScriptResult(int(callbackId),
                         jsonValue.isNull() ? resultString : jsonValue.toVariant());
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onRunJavaScriptResult)

static void c_onPageFinished(JNIEnv *env,
                             jobject thiz,
                             jlong id,
                             jstring url)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    QWebViewLoadingInfo loadingInfo(QWebViewFactory::LoadingInfo::create(
        QUrl(QJniObject(url).toString()),
        QWebViewLoadingInfo::LoadStatus::Succeeded,
        QString()));
    emit wc->q_ptr->loadingChanged(loadingInfo);
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onPageFinished)

static void c_onPageStarted(JNIEnv *env,
                            jobject thiz,
                            jlong id,
                            jstring url,
                            Bitmap icon)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    Q_UNUSED(icon);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    QUrl qurl(QJniObject(url).toString());
    QWebViewLoadingInfo loadingInfo(QWebViewFactory::LoadingInfo::create(
        qurl,
        QWebViewLoadingInfo::LoadStatus::Started,
        QString()));
    emit wc->q_ptr->urlChanged(qurl);
    emit wc->q_ptr->loadingChanged(loadingInfo);

//    if (!icon)
//        return;

//    QImage image;
//    if (favIcon(env, icon, &image))
//        Q_EMIT wc->iconChanged(image);
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onPageStarted)

static void c_onProgressChanged(JNIEnv *env,
                                jobject thiz,
                                jlong id,
                                jint newProgress)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    emit wc->q_ptr->loadProgressChanged(newProgress);
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onProgressChanged)

static void c_onReceivedIcon(JNIEnv *env,
                             jobject thiz,
                             jlong id,
                             Bitmap icon)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    Q_UNUSED(icon);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    if (!icon.isValid())
        return;

//    QImage image;
//    if (favIcon(env, icon, &image))
//        Q_EMIT wc->iconChanged(image);
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onReceivedIcon)

static void c_onReceivedTitle(JNIEnv *env,
                              jobject thiz,
                              jlong id,
                              jstring title)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;
    const QString &qTitle = QJniObject(title).toString();
    emit wc->q_ptr->titleChanged(qTitle);
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onReceivedTitle)

static void c_onReceivedError(JNIEnv *env,
                              jobject thiz,
                              jlong id,
                              jint errorCode,
                              jstring description,
                              jstring url)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);
    Q_UNUSED(errorCode);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    QWebViewLoadingInfo loadingInfo(QWebViewFactory::LoadingInfo::create(
        QUrl(QJniObject(url).toString()),
        QWebViewLoadingInfo::LoadStatus::Failed,
        QJniObject(description).toString()));
    emit wc->q_ptr->loadingChanged(loadingInfo);
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onReceivedError)

static void c_onCookieAdded(JNIEnv *env,
                            jclass thiz,
                            jlong id,
                            jboolean result,
                            jstring domain,
                            jstring name)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    if (result)
        emit wc->q_ptr->cookieAdded(QJniObject(domain).toString(), QJniObject(name).toString());
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onCookieAdded)

static void c_onCookieRemoved(JNIEnv *env,
                              jclass thiz,
                              jlong id,
                              jboolean result,
                              jstring domain,
                              jstring name)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    Q_ASSERT(id);
    QAndroidWebViewPrivate *wc = reinterpret_cast<QAndroidWebViewPrivate *>(id);
    if (!g_webViews->contains(wc))
        return;

    if (result)
        emit wc->q_ptr->cookieRemoved(QJniObject(domain).toString(), QJniObject(name).toString());
}
Q_DECLARE_JNI_NATIVE_METHOD(c_onCookieRemoved)
QT_END_NAMESPACE

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* /* vm */, void* /*reserved*/)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    if (!WebViewController::registerNativeMethods({
        Q_JNI_NATIVE_METHOD(c_onRunJavaScriptResult),
        Q_JNI_NATIVE_METHOD(c_onPageFinished),
        Q_JNI_NATIVE_METHOD(c_onPageStarted),
        Q_JNI_NATIVE_METHOD(c_onProgressChanged),
        Q_JNI_NATIVE_METHOD(c_onReceivedIcon),
        Q_JNI_NATIVE_METHOD(c_onReceivedTitle),
        Q_JNI_NATIVE_METHOD(c_onReceivedError),
        Q_JNI_NATIVE_METHOD(c_onCookieAdded),
        Q_JNI_NATIVE_METHOD(c_onCookieRemoved),
    })) {
        qCritical("Failed to register native methods for WebViewController");
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
