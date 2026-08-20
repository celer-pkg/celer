// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwasmwebview_p.h"
#include <QtWebView/qwebview.h>
#include <QtWebView/qwebviewloadinginfo.h>

#include <QtCore/qmap.h>
#include <QtGui/qguiapplication.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qdebug.h>

#include <QAbstractEventDispatcher>
#include <QThread>

#include <emscripten/emscripten.h>
#include <iostream>

QT_BEGIN_NAMESPACE

QWasmWebViewSettingsPrivate::QWasmWebViewSettingsPrivate() { }

bool QWasmWebViewSettingsPrivate::doTestAttribute(WebAttribute attribute) const
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

void QWasmWebViewSettingsPrivate::doSetAttribute(WebAttribute attribute, bool value)
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

bool QWasmWebViewSettingsPrivate::localStorageEnabled() const
{
    qWarning("localStorageEnabled() not supported on this platform");
    return false;
}

bool QWasmWebViewSettingsPrivate::javaScriptEnabled() const
{
    qWarning("javaScriptEnabled() not supported on this platform");
    return false;
}

bool QWasmWebViewSettingsPrivate::localContentCanAccessFileUrls() const
{
    qWarning("localContentCanAccessFileUrls() not supported on this platform");
    return false;
}

bool QWasmWebViewSettingsPrivate::allowFileAccess() const
{
    qWarning("allowFileAccess() not supported on this platform");
    return false;
}

void QWasmWebViewSettingsPrivate::setLocalContentCanAccessFileUrls(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setLocalContentCanAccessFileUrls() not supported on this platform");
}

void QWasmWebViewSettingsPrivate::setJavaScriptEnabled(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setJavaScriptEnabled() not supported on this platform");
}

void QWasmWebViewSettingsPrivate::setLocalStorageEnabled(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setLocalStorageEnabled() not supported on this platform");
}

void QWasmWebViewSettingsPrivate::setAllowFileAccess(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setAllowFileAccess() not supported on this platform");
}

QWasmWebViewPrivate::QWasmWebViewPrivate(QWebView *view) : QWebViewPrivate(view), m_window(view)
{
    m_settings = new QWasmWebViewSettingsPrivate();
    QMetaObject::invokeMethod(this, &QWasmWebViewPrivate::initializeIFrame, Qt::QueuedConnection);
}

QWasmWebViewPrivate::~QWasmWebViewPrivate() { }

QString QWasmWebViewPrivate::httpUserAgent() const
{
    if (m_iframe)
        return QString::fromStdString(
                (*m_iframe)["contentWindow"]["navigator"]["userAgent"].as<std::string>());
    return QString();
}

void QWasmWebViewPrivate::setHttpUserAgent(const QString &userAgent)
{
    Q_UNUSED(userAgent);
    qWarning("setHttpUserAgent() not supported on this platform");
}

QUrl QWasmWebViewPrivate::url() const
{
    return m_currentUrl;
}

void QWasmWebViewPrivate::setUrl(const QUrl &url)
{
    m_currentUrl = url;
    if (m_iframe) {
        (*m_iframe).call<void>("removeAttribute", emscripten::val("srcdoc"));
        (*m_iframe).set("src", m_currentUrl.toString().toStdString());
    }
}

void QWasmWebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    if (!baseUrl.isEmpty())
        qWarning("Base URLs for loadHtml() are not supported on this platform.");

    if (m_iframe)
        (*m_iframe).set("srcdoc", html.toStdString());
}

bool QWasmWebViewPrivate::canGoBack() const
{
    qWarning("canGoBack() not supported on this platform");
    return false;
}

void QWasmWebViewPrivate::goBack()
{
    qWarning("goBack() not supported on this platform");
}

bool QWasmWebViewPrivate::canGoForward() const
{
    qWarning("canGoForward() not supported on this platform");
    return false;
}

void QWasmWebViewPrivate::goForward()
{
    qWarning("goForward() not supported on this platform");
}

void QWasmWebViewPrivate::reload()
{
    if (m_iframe) {
        (*m_iframe)["contentWindow"]["location"].call<void>("reload");
        setUrl(m_currentUrl);
        // this order of operation is important, setting URL before reload()
        // did not work, HTTP reload request was being cancelled by browser
    }
}

void QWasmWebViewPrivate::stop()
{
    qWarning("stop() not supported on this platform");
}

QString QWasmWebViewPrivate::title() const
{
    if (m_iframe)
        return QString::fromStdString(
                (*m_iframe)["contentWindow"]["document"]["title"].as<std::string>());
    return QString();
}

int QWasmWebViewPrivate::loadProgress() const
{
    qWarning("loadProgress() not supported on this platform");
    return 100;
}

bool QWasmWebViewPrivate::isLoading() const
{
    qWarning("isLoading() not supported on this platform");
    return false;
}

void QWasmWebViewPrivate::setCookie(const QString &domain, const QString &name,
                                    const QString &value)
{
    Q_UNUSED(domain);
    Q_UNUSED(name);
    Q_UNUSED(value);
    qWarning("setCookie() not supported on this platform");
}

void QWasmWebViewPrivate::deleteCookie(const QString &domain, const QString &name)
{
    Q_UNUSED(domain);
    Q_UNUSED(name);
    qWarning("deleteCookie() not supported on this platform");
}

void QWasmWebViewPrivate::deleteAllCookies()
{
    qWarning("deleteAllCookies() not supported on this platform");
}

void QWasmWebViewPrivate::runJavaScript(const QString &script,
                                        const std::function<void(const QVariant &)> &resultCallback)
{
    Q_UNUSED(script);
    Q_UNUSED(resultCallback);
    qWarning("runJavaScript() not supported on this platform");
}

QWebViewSettingsPrivate *QWasmWebViewPrivate::settings() const
{
    return m_settings;
}

// clang-format off
EM_JS(void, addResizeObservers, (emscripten::EM_VAL clientAreaHandle, emscripten::EM_VAL frameHandle), {
    var clientArea = Emval.toValue(clientAreaHandle);
    var frame = Emval.toValue(frameHandle);
    const resizeObserver = new ResizeObserver((entries) => {
        for (const entry of entries) {
            frame.width = entry.contentBoxSize[0].inlineSize;
            frame.height = entry.contentBoxSize[0].blockSize;
        }
    });
    resizeObserver.observe(clientArea);
});
// clang-format on

void QWasmWebViewPrivate::initializeIFrame()
{
    auto wasmWindow = dynamic_cast<QNativeInterface::Private::QWasmWindow *>(m_window->handle());
    Q_ASSERT(wasmWindow);

    auto document = wasmWindow->document();
    auto clientArea = wasmWindow->clientArea();

    m_iframe = document.call<emscripten::val>("createElement", emscripten::val("iframe"));
    clientArea.call<void>("appendChild", *m_iframe);
    (*m_iframe)["style"].set("position", "absolute");
    (*m_iframe)["style"].set("border", "none");
    (*m_iframe)["style"].set("top", "0px");
    (*m_iframe)["style"].set("left", "0px");
    addResizeObservers(clientArea.as_handle(), m_iframe.value().as_handle());

    // NOTE: Make sure any pending url is set now.
    setUrl(m_currentUrl);
}

QT_END_NAMESPACE
