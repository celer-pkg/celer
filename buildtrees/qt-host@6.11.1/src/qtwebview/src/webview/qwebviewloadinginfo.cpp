// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtWebView/qwebviewloadinginfo.h>

#include <QtCore/qdebug.h>
#include <QtCore/qurl.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QWebViewLoadingInfoPrivate : public QSharedData
{
public:
    QWebViewLoadingInfoPrivate() = default;
    QWebViewLoadingInfoPrivate(const QUrl &url, const QString &errorString)
        : url{url},
          errorString{errorString}
    {}

    QUrl url;
    QString errorString;
};

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QWebViewLoadingInfoPrivate)

/*!
    \class QWebViewLoadingInfo
    \brief The class provides load status information for the \l {QWebView::loadingChanged()} signal.
    \since 6.11
    \ingroup webview
    \inmodule QtWebView

    The QWebViewLoadingInfo type contains load status information for the requested URL.
*/

QWebViewLoadingInfo::QWebViewLoadingInfo()
    : d{new QWebViewLoadingInfoPrivate()},
      m_status{LoadStatus::Started}
{}

QWebViewLoadingInfo::QWebViewLoadingInfo(const QUrl &url, LoadStatus status,
                                         const QString &errorString)
    : d{new QWebViewLoadingInfoPrivate(url, errorString)},
      m_status{status}
{
}

QWebViewLoadingInfo::QWebViewLoadingInfo(const QWebViewLoadingInfo &other)
    = default;

QWebViewLoadingInfo &QWebViewLoadingInfo::operator=(const QWebViewLoadingInfo &other)
    = default;

QWebViewLoadingInfo::~QWebViewLoadingInfo() { }

/*!
    \property QWebViewLoadingInfo::url
    \brief The URL of the load request.
    The URL of the load request.
    \sa {QWebView::url}
*/

/*!
    \fn QWebViewLoadingInfo::swap(QWebViewLoadingInfo &other)
    \memberswap{loading info}
*/

QUrl QWebViewLoadingInfo::url() const
{
    return d->url;
}

/*!
    \property QWebViewLoadingInfo::status
    \brief The status of a web page load request.

    This enumeration represents the load status of a web page load request.
    \sa {QWebView::loadingChanged()}
*/

/*!
    \enum QWebViewLoadingInfo::LoadStatus

    This enumeration represents the load status of a web page load request.

    \value Started The page is currently loading.
    \value Succeeded The page was loaded successfully.
    \value Failed The page could not be loaded.
    \value Stopped The page load was stoped.

    \sa {QWebView::loadingChanged()}
*/

/*!
    \property QWebViewLoadingInfo::errorString
    \brief Holds the error message.

    Holds the error message if the load request failed.
    \sa {QWebView::loadingChanged()}
*/

QString QWebViewLoadingInfo::errorString() const
{
    return d->errorString;
}

#ifndef QT_NO_DEBUG_STREAM
// friend
QDebug operator<<(QDebug ds, const QWebViewLoadingInfo &info)
{
    const QDebugStateSaver saver{ds};
    ds.nospace() << "QWebViewLoadingInfo{";
    if (info.d) {
        ds << info.d->url << ", "
           << info.m_status << ", "
           << info.d->errorString;
    } else {
        ds << "<moved-from-object>";
    }
    return ds << '}';
}
#endif // QT_NO_DEBUG_STREAM

QT_END_NAMESPACE

#include "moc_qwebviewloadinginfo.cpp"
