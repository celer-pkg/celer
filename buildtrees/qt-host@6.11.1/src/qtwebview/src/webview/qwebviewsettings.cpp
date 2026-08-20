// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebviewsettings.h"

QT_BEGIN_NAMESPACE

/*!
    \enum QWebViewSettings::WebAttribute

    This enum type specifies settings for web pages:

    \value LocalStorageEnabled
           Enables support for the HTML 5 local storage feature.
    \value JavaScriptEnabled
           Enables the running of JavaScript programs.
    \value AllowFileAccess
           Allows locally loaded documents to access other local URLs.
    \value LocalContentCanAccessFileUrls
           Enables the WebView to load file URLs.
*/

/*!
    \class QWebViewSettings
    \brief Allows configuration of browser properties and attributes.
    \since 6.11
    \ingroup webview
    \inmodule QtWebView

    The QWebViewSettings object can be used to configure browser properties and generic
    attributes, such as JavaScript support, file access and local storage features.

    The default values are left as set by the different platforms.
*/

QWebViewSettings::QWebViewSettings()
{
}

QWebViewSettings::~QWebViewSettings() { }


/*!
    \fn bool QWebViewSettings::testAttribute(WebAttribute attribute) const

    Returns \c true if \a attribute is enabled; otherwise returns \c false.
*/
bool QWebViewSettings::testAttribute(WebAttribute attribute) const
{
    return doTestAttribute(attribute);
}

/*!
    \fn void QWebViewSettings::setAttribute(WebAttribute attribute, bool on)

    Enables or disables the specified \a attribute feature depending on the
    value of \a on.
*/
void QWebViewSettings::setAttribute(WebAttribute attribute, bool value)
{
    doSetAttribute(attribute, value);
}

QT_END_NAMESPACE

#include "moc_qwebviewsettings.cpp"
