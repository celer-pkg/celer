// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBVIEWLOADINGINFO_H
#define QWEBVIEWLOADINGINFO_H

#include <QtWebView/qwebview_global.h>

#include <QtCore/qmetatype.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstringfwd.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QDebug;
class QUrl;

class QWebViewLoadingInfoPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QWebViewLoadingInfoPrivate)

namespace QWebViewFactory {
struct LoadingInfo;
}

class QWebViewLoadingInfo
{
    Q_GADGET_EXPORT(Q_WEBVIEW_EXPORT)
    Q_PROPERTY(QUrl url READ url CONSTANT)
    Q_PROPERTY(LoadStatus status READ status CONSTANT)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

public:
    enum class LoadStatus {
        Started,
        Stopped,
        Succeeded,
        Failed
    };
    Q_ENUM(LoadStatus)

    Q_WEBVIEW_EXPORT QWebViewLoadingInfo();
    Q_WEBVIEW_EXPORT QWebViewLoadingInfo(const QWebViewLoadingInfo &other);
    Q_WEBVIEW_EXPORT QWebViewLoadingInfo &operator=(const QWebViewLoadingInfo &other);
    QWebViewLoadingInfo(QWebViewLoadingInfo &&other) = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QWebViewLoadingInfo)
    Q_WEBVIEW_EXPORT ~QWebViewLoadingInfo();

    void swap(QWebViewLoadingInfo &other) noexcept
    {
        d.swap(other.d);
        std::swap(m_status, other.m_status);
    }

    Q_WEBVIEW_EXPORT QUrl url() const;
    LoadStatus status() const { return m_status; }
    Q_WEBVIEW_EXPORT QString errorString() const;

private:
    // disabled, so users don't get the idea to write them themselves, causing ODR violations
    friend void qHash(const QWebViewLoadingInfo &) = delete;
    friend void qHash(const QWebViewLoadingInfo &, size_t) = delete;
    friend void operator==(const QWebViewLoadingInfo &, const QWebViewLoadingInfo &) = delete;
    friend void operator<<(QDataStream &, const QWebViewLoadingInfo &) = delete;
    friend void operator>>(QDataStream &, QWebViewLoadingInfo &) = delete;
    // end disabled
#ifndef QT_NO_DEBUG_STREAM
    friend Q_WEBVIEW_EXPORT QDebug operator<<(QDebug ds, const QWebViewLoadingInfo &info);
#endif
    Q_WEBVIEW_EXPORT Q_IMPLICIT QWebViewLoadingInfo(const QUrl &url, LoadStatus status,
                                                    const QString &errorString);

    QExplicitlySharedDataPointer<QWebViewLoadingInfoPrivate> d;
    LoadStatus m_status;

    friend struct QWebViewFactory::LoadingInfo;
};

Q_DECLARE_SHARED(QWebViewLoadingInfo)

QT_END_NAMESPACE

#endif // QWEBVIEWLOADINGINFO_H
