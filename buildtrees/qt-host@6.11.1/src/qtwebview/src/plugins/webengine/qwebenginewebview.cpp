// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebenginewebview_p.h"
#include <QtWebView/qwebview.h>
#include <QtWebView/private/qwebviewfactory_p.h>
#include <QtWebViewQuick/private/qquickwebview_p.h>

#include <QtCore/qmap.h>
#include <QtGui/qguiapplication.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qurl.h>

#include <QtQml/qqml.h>

#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquickitem_p.h>

#include <QtWebEngineQuick/private/qquickwebengineview_p.h>
#include <QtWebEngineCore/qwebengineloadinginfo.h>

#include <QWebEngineCookieStore>
#include <QNetworkCookie>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QWebEngineWebViewPrivate::QWebEngineWebViewPrivate(QWebView *p) : QWebViewPrivate(p)
{
    m_settings = new QWebEngineWebViewSettingsPrivate(this);
}

QWebEngineWebViewPrivate::~QWebEngineWebViewPrivate() = default;

QString QWebEngineWebViewPrivate::httpUserAgent() const
{
    return m_httpUserAgent;
}

void QWebEngineWebViewPrivate::setHttpUserAgent(const QString &userAgent)
{
    m_httpUserAgent = userAgent;
    if (m_profile) {
        m_profile->setHttpUserAgent(userAgent);
        emit q_ptr->httpUserAgentStringChanged(userAgent);
    }
}

QUrl QWebEngineWebViewPrivate::url() const
{
    return view()->url();
}

void QWebEngineWebViewPrivate::setUrl(const QUrl &url)
{
    view()->setUrl(url);
}

void QWebEngineWebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    view()->loadHtml(html, baseUrl);
}

bool QWebEngineWebViewPrivate::canGoBack() const
{
    return view()->canGoBack();
}

void QWebEngineWebViewPrivate::goBack()
{
    view()->goBack();
}

bool QWebEngineWebViewPrivate::canGoForward() const
{
    return view()->canGoForward();
}

void QWebEngineWebViewPrivate::goForward()
{
    view()->goForward();
}

void QWebEngineWebViewPrivate::reload()
{
    view()->reload();
}

QString QWebEngineWebViewPrivate::title() const
{
    return view()->title();
}

void QWebEngineWebViewPrivate::runJavaScript(
        const QString &script, const std::function<void(const QVariant &)> &resultCallback)
{
    view()->runJavaScript(script, resultCallback);
}

void QWebEngineWebViewPrivate::setCookie(const QString &domain, const QString &name,
                                         const QString &value)
{
    QNetworkCookie cookie;
    cookie.setDomain(domain);
    cookie.setName(QByteArray(name.toUtf8()));
    cookie.setValue(QByteArray(value.toUtf8()));
    cookie.setPath("/");

    m_cookieStore->setCookie(cookie);
}

void QWebEngineWebViewPrivate::deleteCookie(const QString &domain, const QString &name)
{
    QNetworkCookie cookie;
    cookie.setDomain(domain);
    cookie.setName(QByteArray(name.toUtf8()));
    cookie.setPath("/");

    m_cookieStore->deleteCookie(cookie);
}

void QWebEngineWebViewPrivate::deleteAllCookies()
{
    m_cookieStore->deleteAllCookies();
}

QWebViewSettingsPrivate *QWebEngineWebViewPrivate::settings() const
{
    return m_settings;
}

int QWebEngineWebViewPrivate::loadProgress() const
{
    return view()->loadProgress();
}

bool QWebEngineWebViewPrivate::isLoading() const
{
    return view()->isLoading();
}

void QWebEngineWebViewPrivate::stop()
{
    view()->stop();
}

void QWebEngineWebViewPrivate::q_urlChanged()
{
    emit q_ptr->urlChanged(view()->url());
}

void QWebEngineWebViewPrivate::q_loadProgressChanged()
{
    emit q_ptr->loadProgressChanged(view()->loadProgress());
}

void QWebEngineWebViewPrivate::q_titleChanged()
{
    emit q_ptr->titleChanged(view()->title());
}

void QWebEngineWebViewPrivate::q_loadingChanged(const QWebEngineLoadingInfo &loadRequest)
{
    QWebViewLoadingInfo lr(QWebViewFactory::LoadingInfo::create(
            loadRequest.url(),
            static_cast<QWebViewLoadingInfo::LoadStatus>(
                    loadRequest.status()), // These "should" match...
            loadRequest.errorString()));
    emit q_ptr->loadingChanged(lr);
}

void QWebEngineWebViewPrivate::q_profileChanged()
{
    auto profile = view()->profile();
    if (profile == m_profile)
        return;

    m_profile = profile;
    m_profile->setStorageName(QCoreApplication::applicationName());
    auto userAgent = m_profile->httpUserAgent();
    if (m_httpUserAgent == userAgent)
        return;
    m_httpUserAgent = userAgent;
    QObject::connect(m_profile, &QQuickWebEngineProfile::httpUserAgentChanged, this, &QWebEngineWebViewPrivate::q_httpUserAgentChanged);
    emit q_ptr->httpUserAgentStringChanged(userAgent);
}

void QWebEngineWebViewPrivate::q_httpUserAgentChanged()
{
    QString httpUserAgent = m_profile->httpUserAgent();
    if (m_httpUserAgent == httpUserAgent)
        return;
     m_httpUserAgent = httpUserAgent;
     emit q_ptr->httpUserAgentStringChanged(m_httpUserAgent);
}

void QWebEngineWebViewPrivate::q_cookieAdded(const QNetworkCookie &cookie)
{
    emit q_ptr->cookieAdded(cookie.domain(), cookie.name());
}

void QWebEngineWebViewPrivate::q_cookieRemoved(const QNetworkCookie &cookie)
{
    emit q_ptr->cookieRemoved(cookie.domain(), cookie.name());
}

void QWebEngineWebViewPrivate::initialize(QObject *context)
{
    Q_UNUSED(context);
    QQuickWebEngineView *webEngineView = view();
    Q_ASSERT(webEngineView);
    QQuickWebEngineProfile *profile = webEngineView->profile();
    Q_ASSERT(profile);
    QQuickWebEngineSettings *settings = webEngineView->settings();
    m_profile = profile;
    m_profile->setStorageName(QCoreApplication::applicationName());
    Q_ASSERT(m_settings);
    m_settings->init(settings);
    webEngineView->settings()->setErrorPageEnabled(false);
    webEngineView->settings()->setPluginsEnabled(true);
    // When the httpUserAgent is set as a property then it will be set before
    // init() is called
    if (m_httpUserAgent.isEmpty())
        m_httpUserAgent = profile->httpUserAgent();
    else
        profile->setHttpUserAgent(m_httpUserAgent);
    QObject::connect(webEngineView, &QQuickWebEngineView::urlChanged, this,
                     &QWebEngineWebViewPrivate::q_urlChanged);
    QObject::connect(webEngineView, &QQuickWebEngineView::loadProgressChanged, this,
                     &QWebEngineWebViewPrivate::q_loadProgressChanged);
    QObject::connect(webEngineView, &QQuickWebEngineView::loadingChanged, this,
                     &QWebEngineWebViewPrivate::q_loadingChanged);
    QObject::connect(webEngineView, &QQuickWebEngineView::titleChanged, this,
                     &QWebEngineWebViewPrivate::q_titleChanged);
    QObject::connect(webEngineView, &QQuickWebEngineView::profileChanged, this,
                     &QWebEngineWebViewPrivate::q_profileChanged);
    QObject::connect(profile, &QQuickWebEngineProfile::httpUserAgentChanged, this,
                     &QWebEngineWebViewPrivate::q_httpUserAgentChanged);

    m_cookieStore = m_profile->cookieStore();

    QObject::connect(m_cookieStore, &QWebEngineCookieStore::cookieAdded, this,
                     &QWebEngineWebViewPrivate::q_cookieAdded);
    QObject::connect(m_cookieStore, &QWebEngineCookieStore::cookieRemoved, this,
                     &QWebEngineWebViewPrivate::q_cookieRemoved);
}

// QQuickViewWebEngineWebViewPrivate

QQuickViewWebEngineWebViewPrivate::QQuickViewWebEngineWebViewPrivate(QWebView *p)
    : QWebEngineWebViewPrivate(p), m_view(new QQuickView(p)), m_webEngineView(nullptr)
{
    connect(p, &QWindow::visibleChanged, m_view.get(), &QWindow::setVisible);
    connect(p, &QWindow::widthChanged, m_view.get(), &QWindow::setWidth);
    connect(p, &QWindow::heightChanged, m_view.get(), &QWindow::setHeight);
}

QQuickViewWebEngineWebViewPrivate::~QQuickViewWebEngineWebViewPrivate() = default;

void QQuickViewWebEngineWebViewPrivate::initialize(QObject *context)
{
    Q_ASSERT(!m_webEngineView);
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);
    m_view->loadFromModule("QtWebEngine"_L1, "WebEngineView"_L1);
    m_webEngineView = qobject_cast<QQuickWebEngineView *>(m_view->rootObject());
    Q_ASSERT(m_webEngineView);
    QWebEngineWebViewPrivate::initialize(context);
}

QQuickWebEngineView *QQuickViewWebEngineWebViewPrivate::view() const
{
    return m_webEngineView;
}

// QQuickItemWebEngineWebViewPrivate

QQuickItemWebEngineWebViewPrivate::QQuickItemWebEngineWebViewPrivate(QWebView *p)
    : QWebEngineWebViewPrivate(p), m_parentItem(nullptr)
{
}

QQuickItemWebEngineWebViewPrivate::~QQuickItemWebEngineWebViewPrivate() = default;

void QQuickItemWebEngineWebViewPrivate::initialize(QObject *context)
{
    m_parentItem = qobject_cast<QQuickItem *>(context);
    if (!m_parentItem)
        return;
    Q_ASSERT(!m_webEngineView);
    QQmlEngine *engine = qmlEngine(m_parentItem);
    Q_ASSERT(engine);
    QQmlComponent component(engine);
    component.loadFromModule("QtWebEngine"_L1, "WebEngineView"_L1);
    m_webEngineView.reset(qobject_cast<QQuickWebEngineView *>(component.create()));
    Q_ASSERT(m_webEngineView);
    m_webEngineView->setParentItem(m_parentItem);
    QQuickItemPrivate::get(m_webEngineView.get())->anchors()->setFill(m_parentItem);
    QWebEngineWebViewPrivate::initialize(context);
}

QQuickWebEngineView *QQuickItemWebEngineWebViewPrivate::view() const
{
    return m_webEngineView.get();
}

// QWebEngineWebViewSettingsPrivate

QWebEngineWebViewSettingsPrivate::QWebEngineWebViewSettingsPrivate(QWebEngineWebViewPrivate *p)
    : parent(p)
{

}

bool QWebEngineWebViewSettingsPrivate::doTestAttribute(WebAttribute attribute) const
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

void QWebEngineWebViewSettingsPrivate::doSetAttribute(WebAttribute attribute, bool value)
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

bool QWebEngineWebViewSettingsPrivate::localStorageEnabled() const
{
    return m_settings ? m_settings->localStorageEnabled() : m_localStorageEnabled;
}
bool QWebEngineWebViewSettingsPrivate::javaScriptEnabled() const
{
    return m_settings ? m_settings->javascriptEnabled() : m_javaScriptEnabled;
}
bool QWebEngineWebViewSettingsPrivate::localContentCanAccessFileUrls() const
{
    return m_settings ? m_settings->localContentCanAccessFileUrls() : m_localContentCanAccessFileUrlsEnabled;
}
bool QWebEngineWebViewSettingsPrivate::allowFileAccess() const
{
    return m_allowFileAccessEnabled;
}
void QWebEngineWebViewSettingsPrivate::setLocalContentCanAccessFileUrls(bool enabled)
{
    if (m_settings)
        m_settings->setLocalContentCanAccessFileUrls(enabled);

    m_localContentCanAccessFileUrlsEnabled  = enabled;
}
void QWebEngineWebViewSettingsPrivate::setJavaScriptEnabled(bool enabled)
{
    if (m_settings)
        m_settings->setJavascriptEnabled(enabled);

    m_javaScriptEnabled = enabled;
}
void QWebEngineWebViewSettingsPrivate::setLocalStorageEnabled(bool enabled)
{
    // Interpret this property to also affect the "off the record" profile setting.
    if (parent) {
        if (parent->m_profile)
            parent->m_profile->setOffTheRecord(!enabled);
    }

    if (m_settings)
        m_settings->setLocalStorageEnabled(enabled);

    m_localStorageEnabled = enabled;
}
void QWebEngineWebViewSettingsPrivate::setAllowFileAccess(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setAllowFileAccess() not supported with the WebEngine backend");
}

void QWebEngineWebViewSettingsPrivate::init(QQuickWebEngineSettings *settings)
{
    m_settings = settings;

    if (m_settings) {
        // Sync any values already set.
        setLocalContentCanAccessFileUrls(m_localContentCanAccessFileUrlsEnabled);
        setJavaScriptEnabled(m_javaScriptEnabled);
        setLocalStorageEnabled(m_localStorageEnabled);
    }
}

QT_END_NAMESPACE
