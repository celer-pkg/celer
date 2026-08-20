// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickwebviewsettings_p.h"

#include <QtWebView/qwebview.h>
#include <QtWebView/qwebviewsettings.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebViewSettings
    //! \nativetype QQuickWebViewSettings
    \inqmlmodule QtWebView
    \since QtWebView 6.5
    \brief Allows configuration of browser properties and attributes.

    The WebViewSettings type can be used to configure browser properties and generic
    attributes, such as JavaScript support, file access and local storage features.
    This type is uncreatable, but the default settings for all web views can be accessed by using
    the \l [QML] {WebView::settings}{WebView.settings} property.

    The default values are left as set by the different platforms.
*/

QQuickWebViewSettings::QQuickWebViewSettings(QWebViewSettings *webviewsettings) : d(webviewsettings)
{
    Q_ASSERT(d);
}

QQuickWebViewSettings::~QQuickWebViewSettings()
{

}

/*!
    \qmlproperty bool WebViewSettings::localStorageEnabled

    Enables support for the HTML 5 local storage feature.
*/
bool QQuickWebViewSettings::localStorageEnabled() const
{
    return d->testAttribute(QWebViewSettings::WebAttribute::LocalStorageEnabled);
}

void QQuickWebViewSettings::setLocalStorageEnabled(bool enabled)
{
    return d->setAttribute(QWebViewSettings::WebAttribute::LocalStorageEnabled, enabled);
}

/*!
    \qmlproperty bool WebViewSettings::javaScriptEnabled

    Enables the running of JavaScript programs.
*/
bool QQuickWebViewSettings::javaScriptEnabled() const
{
    return d->testAttribute(QWebViewSettings::WebAttribute::JavaScriptEnabled);
}

void QQuickWebViewSettings::setJavaScriptEnabled(bool enabled)
{
    return d->setAttribute(QWebViewSettings::WebAttribute::JavaScriptEnabled, enabled);
}

/*!
    \qmlproperty bool WebViewSettings::localContentCanAccessFileUrls

    Allows locally loaded documents to access other local URLs.
*/
bool QQuickWebViewSettings::localContentCanAccessFileUrls() const
{
    return d->testAttribute(QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls);
}

void QQuickWebViewSettings::setLocalContentCanAccessFileUrls(bool enabled)
{
    return d->setAttribute(QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls, enabled);
}

/*!
    \qmlproperty bool WebViewSettings::allowFileAccess

    Enables the WebView to load file URLs.
*/
bool QQuickWebViewSettings::allowFileAccess() const
{
    return d->testAttribute(QWebViewSettings::WebAttribute::AllowFileAccess);
}

void QQuickWebViewSettings::setAllowFileAccess(bool enabled)
{
    return d->setAttribute(QWebViewSettings::WebAttribute::AllowFileAccess, enabled);
}

QT_END_NAMESPACE
