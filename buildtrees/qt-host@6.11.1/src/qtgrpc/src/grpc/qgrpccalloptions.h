// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCALLOPTIONS_H
#define QGRPCALLOPTIONS_H

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qcompare.h>
#include <QtCore/qhash.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstringfwd.h>
#include <QtCore/qtclasshelpermacros.h>
#include <QtCore/qtdeprecationdefinitions.h>

#include <chrono>
#include <optional>

QT_BEGIN_NAMESPACE

class QDataStream;
class QDebug;
class QVariant;

class QGrpcCallOptionsPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QGrpcCallOptionsPrivate)

class QGrpcCallOptions final
{
public:
    Q_GRPC_EXPORT QGrpcCallOptions();
    Q_GRPC_EXPORT ~QGrpcCallOptions();

    Q_GRPC_EXPORT QGrpcCallOptions(const QGrpcCallOptions &other);
    Q_GRPC_EXPORT QGrpcCallOptions &operator=(const QGrpcCallOptions &other);

    QGrpcCallOptions(QGrpcCallOptions &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QGrpcCallOptions)

    Q_GRPC_EXPORT Q_IMPLICIT operator QVariant() const;

    void swap(QGrpcCallOptions &other) noexcept { d_ptr.swap(other.d_ptr); }

    [[nodiscard]] Q_GRPC_EXPORT std::optional<std::chrono::milliseconds>
    deadlineTimeout() const noexcept;
    Q_GRPC_EXPORT QGrpcCallOptions &setDeadlineTimeout(std::chrono::milliseconds timeout);

#if QT_DEPRECATED_SINCE(6, 13)
    QT_DEPRECATED_VERSION_X_6_13("Use metadata(QtGrpc::MultiValue) for QMultiHash")
    [[nodiscard]] Q_GRPC_EXPORT const QHash<QByteArray, QByteArray> &metadata() const & noexcept;
    QT_DEPRECATED_VERSION_X_6_13("Use metadata(QtGrpc::MultiValue) for QMultiHash")
    [[nodiscard]] Q_GRPC_EXPORT QHash<QByteArray, QByteArray> metadata() &&;
    QT_DEPRECATED_VERSION_X_6_13("Use the QMultiHash overload")
    Q_GRPC_EXPORT QGrpcCallOptions &setMetadata(const QHash<QByteArray, QByteArray> &metadata);
    QT_DEPRECATED_VERSION_X_6_13("Use the QMultiHash overload")
    Q_GRPC_EXPORT QGrpcCallOptions &setMetadata(QHash<QByteArray, QByteArray> &&metadata);
#endif
    [[nodiscard]] Q_GRPC_EXPORT const QMultiHash<QByteArray, QByteArray> &
        metadata(QtGrpc::MultiValue_t) const & noexcept;
    [[nodiscard]] Q_GRPC_EXPORT QMultiHash<QByteArray, QByteArray>
    metadata(QtGrpc::MultiValue_t) &&;
    Q_GRPC_EXPORT QGrpcCallOptions &setMetadata(const QMultiHash<QByteArray, QByteArray> &metadata);
    Q_GRPC_EXPORT QGrpcCallOptions &setMetadata(QMultiHash<QByteArray, QByteArray> &&metadata);
    Q_GRPC_EXPORT QGrpcCallOptions &
    setMetadata(std::initializer_list<std::pair<QByteArray, QByteArray>> list);
    Q_GRPC_EXPORT QGrpcCallOptions &addMetadata(QByteArrayView key, QByteArrayView value);

    [[nodiscard]] Q_GRPC_EXPORT std::optional<bool> filterServerMetadata() const noexcept;
    Q_GRPC_EXPORT QGrpcCallOptions &setFilterServerMetadata(bool value);

private:
    QExplicitlySharedDataPointer<QGrpcCallOptionsPrivate> d_ptr;

    friend Q_GRPC_EXPORT bool comparesEqual(const QGrpcCallOptions &lhs,
                                            const QGrpcCallOptions &rhs);
    Q_DECLARE_EQUALITY_COMPARABLE_NON_NOEXCEPT(QGrpcCallOptions)

    friend size_t qHash(const QGrpcCallOptions &) = delete;
    friend size_t qHash(const QGrpcCallOptions &, size_t) = delete;

#ifndef QT_NO_DEBUG_STREAM
    friend Q_GRPC_EXPORT QDebug operator<<(QDebug debug, const QGrpcCallOptions &callOpts);
#endif
#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QGrpcCallOptions &) = delete;
    friend QDataStream &operator>>(QDataStream &, QGrpcCallOptions &) = delete;
#endif

    Q_DECLARE_PRIVATE(QGrpcCallOptions)
};

Q_DECLARE_SHARED(QGrpcCallOptions)

QT_END_NAMESPACE

#endif // QGRPCALLOPTIONS_H
