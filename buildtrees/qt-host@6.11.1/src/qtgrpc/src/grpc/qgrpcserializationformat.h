// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCSERIALIZATIONFORMAT_H
#define QGRPCSERIALIZATIONFORMAT_H

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qbytearrayview.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qtclasshelpermacros.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QAbstractProtobufSerializer;
class QDebug;
class QVariant;

class QGrpcSerializationFormatPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QGrpcSerializationFormatPrivate)

class QGrpcSerializationFormat final
{
public:
    Q_GRPC_EXPORT Q_IMPLICIT QGrpcSerializationFormat(QtGrpc::SerializationFormat format = {});
    Q_GRPC_EXPORT explicit QGrpcSerializationFormat(QByteArrayView suffix,
                                                    std::shared_ptr<QAbstractProtobufSerializer>
                                                        serializer);
    Q_GRPC_EXPORT ~QGrpcSerializationFormat();

    Q_GRPC_EXPORT QGrpcSerializationFormat(const QGrpcSerializationFormat &);
    Q_GRPC_EXPORT QGrpcSerializationFormat &operator=(const QGrpcSerializationFormat &);

    QGrpcSerializationFormat(QGrpcSerializationFormat &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QGrpcSerializationFormat)

    Q_GRPC_EXPORT Q_IMPLICIT operator QVariant() const;

    void swap(QGrpcSerializationFormat &other) noexcept { d_ptr.swap(other.d_ptr); }

    [[nodiscard]] Q_GRPC_EXPORT QByteArrayView suffix() const noexcept;

    [[nodiscard]] Q_GRPC_EXPORT std::shared_ptr<QAbstractProtobufSerializer> serializer() const;

private:
    QExplicitlySharedDataPointer<QGrpcSerializationFormatPrivate> d_ptr;

    Q_GRPC_EXPORT friend bool comparesEqual(const QGrpcSerializationFormat &lhs,
                                            const QGrpcSerializationFormat &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QGrpcSerializationFormat)

    Q_GRPC_EXPORT friend size_t qHash(const QGrpcSerializationFormat &key, size_t seed) noexcept;
    friend size_t qHash(const QGrpcSerializationFormat &key) noexcept { return qHash(key, 0); }

#ifndef QT_NO_DEBUG_STREAM
    friend Q_GRPC_EXPORT QDebug operator<<(QDebug debug, const QGrpcSerializationFormat &sfmt);
#endif

    Q_DECLARE_PRIVATE(QGrpcSerializationFormat)
};

Q_DECLARE_SHARED(QGrpcSerializationFormat)

QT_END_NAMESPACE

#endif // QGRPCSERIALIZATIONFORMAT_H
