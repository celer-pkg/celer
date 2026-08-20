// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTGRPCCOMMONOPTIONS_P_H
#define QTGRPCCOMMONOPTIONS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qshareddata.h>

#include <chrono>
#include <optional>

QT_BEGIN_NAMESPACE

class QGrpcCommonOptions : public QSharedData
{
public:
    QGrpcCommonOptions() = default;
    virtual ~QGrpcCommonOptions() = default;

    [[nodiscard]] std::optional<std::chrono::milliseconds> deadlineTimeout() const noexcept;
    void setDeadlineTimeout(std::chrono::milliseconds t);

#if QT_DEPRECATED_SINCE(6, 13)
    const QHash<QByteArray, QByteArray> &metadata() const & noexcept;
    QHash<QByteArray, QByteArray> metadata() &&;
    void setMetadata(const QHash<QByteArray, QByteArray> &md);
    void setMetadata(QHash<QByteArray, QByteArray> &&md);
#endif
    const QMultiHash<QByteArray, QByteArray> &
        metadata(QtGrpc::MultiValue_t /*tag*/) const & noexcept;
    QMultiHash<QByteArray, QByteArray> metadata(QtGrpc::MultiValue_t /*tag*/) &&;
    void setMetadata(const QMultiHash<QByteArray, QByteArray> &md);
    void setMetadata(QMultiHash<QByteArray, QByteArray> &&md);
    void addMetadata(QByteArray &&key, QByteArray &&value);
    bool containsMetadata(QByteArrayView key, QByteArrayView value) const;

    std::optional<bool> filterServerMetadata() const noexcept;
    void setFilterServerMetadata(bool value);

private:
    std::optional<std::chrono::milliseconds> m_timeout;
    QMultiHash<QByteArray, QByteArray> m_metadata;
#if QT_DEPRECATED_SINCE(6, 13)
    QHash<QByteArray, QByteArray> m_deprecatedMetadata;
#endif
    std::optional<bool> m_filterServerMetadata;
};

namespace QtGrpcPrivate {

#if QT_DEPRECATED_SINCE(6, 13)
QHash<QByteArray, QByteArray> toHash(const QMultiHash<QByteArray, QByteArray> &multiHash);
#endif

}

QT_END_NAMESPACE

#endif
