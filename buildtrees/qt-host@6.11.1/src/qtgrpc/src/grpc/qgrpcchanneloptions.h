// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCHANNELOPTIONS_H
#define QGRPCHANNELOPTIONS_H

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#if QT_CONFIG(ssl)
#  include <QtNetwork/qsslconfiguration.h>
#endif

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
class QGrpcSerializationFormat;

class QGrpcChannelOptionsPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QGrpcChannelOptionsPrivate)

class QGrpcChannelOptions final
{
public:
    Q_GRPC_EXPORT QGrpcChannelOptions();
    Q_GRPC_EXPORT ~QGrpcChannelOptions();

    Q_GRPC_EXPORT QGrpcChannelOptions(const QGrpcChannelOptions &other);
    Q_GRPC_EXPORT QGrpcChannelOptions &operator=(const QGrpcChannelOptions &other);

    QGrpcChannelOptions(QGrpcChannelOptions &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QGrpcChannelOptions)

    Q_GRPC_EXPORT Q_IMPLICIT operator QVariant() const;

    void swap(QGrpcChannelOptions &other) noexcept { d_ptr.swap(other.d_ptr); }

    [[nodiscard]] Q_GRPC_EXPORT std::optional<std::chrono::milliseconds>
    deadlineTimeout() const noexcept;
    Q_GRPC_EXPORT QGrpcChannelOptions &setDeadlineTimeout(std::chrono::milliseconds timeout);

#if QT_DEPRECATED_SINCE(6, 13)
    QT_DEPRECATED_VERSION_X_6_13("Use metadata(QtGrpc::MultiValue) for QMultiHash")
    [[nodiscard]] Q_GRPC_EXPORT const QHash<QByteArray, QByteArray> &metadata() const & noexcept;
    QT_DEPRECATED_VERSION_X_6_13("Use metadata(QtGrpc::MultiValue) for QMultiHash")
    [[nodiscard]] Q_GRPC_EXPORT QHash<QByteArray, QByteArray> metadata() &&;
    QT_DEPRECATED_VERSION_X_6_13("Use the QMultiHash overload")
    Q_GRPC_EXPORT QGrpcChannelOptions &setMetadata(const QHash<QByteArray, QByteArray> &metadata);
    QT_DEPRECATED_VERSION_X_6_13("Use the QMultiHash overload")
    Q_GRPC_EXPORT QGrpcChannelOptions &setMetadata(QHash<QByteArray, QByteArray> &&metadata);
#endif
    [[nodiscard]] Q_GRPC_EXPORT const QMultiHash<QByteArray, QByteArray> &
        metadata(QtGrpc::MultiValue_t) const & noexcept;
    [[nodiscard]] Q_GRPC_EXPORT QMultiHash<QByteArray, QByteArray>
    metadata(QtGrpc::MultiValue_t) &&;
    Q_GRPC_EXPORT QGrpcChannelOptions &
    setMetadata(const QMultiHash<QByteArray, QByteArray> &metadata);
    Q_GRPC_EXPORT QGrpcChannelOptions &setMetadata(QMultiHash<QByteArray, QByteArray> &&metadata);
    Q_GRPC_EXPORT QGrpcChannelOptions &
    setMetadata(std::initializer_list<std::pair<QByteArray, QByteArray>> list);
    Q_GRPC_EXPORT QGrpcChannelOptions &addMetadata(QByteArrayView key, QByteArrayView value);

    [[nodiscard]] Q_GRPC_EXPORT std::optional<bool> filterServerMetadata() const noexcept;
    Q_GRPC_EXPORT QGrpcChannelOptions &setFilterServerMetadata(bool value);

    [[nodiscard]] Q_GRPC_EXPORT QGrpcSerializationFormat serializationFormat() const;
    Q_GRPC_EXPORT QGrpcChannelOptions &
    setSerializationFormat(const QGrpcSerializationFormat &format);

#if QT_CONFIG(ssl)
    [[nodiscard]] Q_GRPC_EXPORT std::optional<QSslConfiguration> sslConfiguration() const;
    Q_GRPC_EXPORT QGrpcChannelOptions &
    setSslConfiguration(const QSslConfiguration &sslConfiguration);
#endif

private:
    QExplicitlySharedDataPointer<QGrpcChannelOptionsPrivate> d_ptr;

    friend Q_GRPC_EXPORT bool comparesEqual(const QGrpcChannelOptions &lhs,
                                            const QGrpcChannelOptions &rhs);
    Q_DECLARE_EQUALITY_COMPARABLE_NON_NOEXCEPT(QGrpcChannelOptions)

    friend size_t qHash(const QGrpcChannelOptions &) = delete;
    friend size_t qHash(const QGrpcChannelOptions &, size_t) = delete;

#ifndef QT_NO_DEBUG_STREAM
    friend Q_GRPC_EXPORT QDebug operator<<(QDebug debug, const QGrpcChannelOptions &chOpts);
#endif
#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QGrpcChannelOptions &) = delete;
    friend QDataStream &operator>>(QDataStream &, QGrpcChannelOptions &) = delete;
#endif

    Q_DECLARE_PRIVATE(QGrpcChannelOptions)
};

Q_DECLARE_SHARED(QGrpcChannelOptions)

QT_END_NAMESPACE

#endif // QGRPCHANNELOPTIONS_H
