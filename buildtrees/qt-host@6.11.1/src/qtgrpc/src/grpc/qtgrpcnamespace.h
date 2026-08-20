// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTGRPCNAMESPACE_H
#define QTGRPCNAMESPACE_H

#if 0
#pragma qt_class(QtGrpcNamespace)
#endif

#include <QtGrpc/qtgrpcexports.h>

#include <QtCore/qcompare.h>
#include <QtCore/qhashfunctions.h>
#include <QtCore/qlatin1stringview.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qtmetamacros.h>

QT_BEGIN_NAMESPACE

struct QMetaObject;
class QDataStream;
class QDebug;

namespace QtGrpc {
Q_NAMESPACE_EXPORT(Q_GRPC_EXPORT)

enum class SerializationFormat : quint8 {
    Default,
    Protobuf,
    Json,
};
Q_ENUM_NS(SerializationFormat)

enum class StatusCode : quint8 {
    Ok = 0,
    Cancelled = 1,
    Unknown = 2,
    InvalidArgument = 3,
    DeadlineExceeded = 4,
    NotFound = 5,
    AlreadyExists = 6,
    PermissionDenied = 7,
    ResourceExhausted = 8,
    FailedPrecondition = 9,
    Aborted = 10,
    OutOfRange = 11,
    Unimplemented = 12,
    Internal = 13,
    Unavailable = 14,
    DataLoss = 15,
    Unauthenticated = 16,
};
Q_ENUM_NS(StatusCode)

enum class RpcType : quint8 {
    UnaryCall,
    ServerStreaming,
    ClientStreaming,
    BidiStreaming,
};
Q_ENUM_NS(RpcType)

struct RpcDescriptor
{
    QLatin1StringView service;
    QLatin1StringView method;
    RpcType type;

private:
    friend bool comparesEqual(const RpcDescriptor &lhs, const RpcDescriptor &rhs) noexcept
    {
        return lhs.service == rhs.service && lhs.method == rhs.method && lhs.type == rhs.type;
    }

    friend Qt::strong_ordering compareThreeWay(const RpcDescriptor &lhs,
                                               const RpcDescriptor &rhs) noexcept
    {
        if (auto cmp = compareThreeWay(lhs.service, rhs.service); is_neq(cmp))
            return cmp;
        if (auto cmp = compareThreeWay(lhs.method, rhs.method); is_neq(cmp))
            return cmp;
        return Qt::compareThreeWay(lhs.type, rhs.type);
    }
    Q_DECLARE_STRONGLY_ORDERED(RpcDescriptor)

    constexpr friend size_t qHash(const RpcDescriptor &key, size_t seed = 0) noexcept
    {
        return qHashMulti(seed, key.service, key.method, key.type);
    }

#ifndef QT_NO_DEBUG_STREAM
    friend Q_GRPC_EXPORT QDebug operator<<(QDebug debug, const RpcDescriptor &description);
#endif
#ifndef QT_NO_DATASTREAM
    // prevent users from implementing their own, as the data-members don't qualify for it.
    friend QDataStream &operator<<(QDataStream &, const RpcDescriptor &) = delete;
    friend QDataStream &operator>>(QDataStream &, RpcDescriptor &) = delete;
#endif
};

// ### Qt7: remove QHash metadata interfaces.
inline QT_DEFINE_TAG(MultiValue);

Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
} // namespace QtGrpc

QT_END_NAMESPACE

#endif // QTGRPCNAMESPACE_H
