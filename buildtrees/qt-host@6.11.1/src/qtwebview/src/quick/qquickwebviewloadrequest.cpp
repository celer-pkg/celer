// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickwebviewloadrequest_p.h"
#include <QtWebView/qwebviewloadinginfo.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebViewLoadRequest
    //! \nativetype QQuickWebViewLoadRequest
    \inqmlmodule QtWebView

    \brief A utility type for \l {WebView}'s \l {WebView::}{loadingChanged()} signal.

    The WebViewLoadRequest type contains load status information for the requested URL.

    \sa {WebView::loadingChanged()}{WebView.loadingChanged()}
*/
QQuickWebViewLoadRequest::QQuickWebViewLoadRequest(const QWebViewLoadingInfo &d)
    : d_ptr(new QWebViewLoadingInfo(d))
{
}

QQuickWebViewLoadRequest::~QQuickWebViewLoadRequest() { }

/*!
    \qmlproperty url QtWebView::WebViewLoadRequest::url
    \readonly

    The URL of the load request.
 */
QUrl QQuickWebViewLoadRequest::url() const
{
    return d_ptr->url();
}

/*!
    \qmlproperty enumeration WebViewLoadRequest::status
    \readonly

    This enumeration represents the load status of a web page load request.

    \value WebView.LoadStartedStatus The page is currently loading.
    \value WebView.LoadSucceededStatus The page was loaded successfully.
    \value WebView.LoadFailedStatus The page could not be loaded.

    \sa {WebView::loadingChanged()}{WebView.loadingChanged}
*/
QQuickWebView::LoadStatus QQuickWebViewLoadRequest::status() const
{
    return QQuickWebView::LoadStatus(d_ptr->status());
}

/*!
    \qmlproperty string QtWebView::WebViewLoadRequest::errorString
    \readonly

    Holds the error message if the load request failed.
*/
QString QQuickWebViewLoadRequest::errorString() const
{
    return d_ptr->errorString();
}

QT_END_NAMESPACE
