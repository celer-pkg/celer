// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCSTATUS_H
#define QGRPCSTATUS_H

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qanystringview.h>
#include <QtCore/qcompare.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qtclasshelpermacros.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QDebug;
class QVariant;

class QGrpcStatus final
{
    Q_GADGET_EXPORT(Q_GRPC_EXPORT)
    Q_PROPERTY(QtGrpc::StatusCode code READ code CONSTANT)
    Q_PROPERTY(QString message READ message CONSTANT)

public:
    Q_GRPC_EXPORT Q_IMPLICIT QGrpcStatus(QtGrpc::StatusCode code = {}, QAnyStringView message = {});
    Q_GRPC_EXPORT ~QGrpcStatus();
    Q_GRPC_EXPORT QGrpcStatus(const QGrpcStatus &other);
    Q_GRPC_EXPORT QGrpcStatus &operator=(const QGrpcStatus &other);
    QGrpcStatus(QGrpcStatus &&other) noexcept = default;
    QGrpcStatus &operator=(QGrpcStatus &&other) noexcept = default;

    Q_GRPC_EXPORT Q_IMPLICIT operator QVariant() const;

    void swap(QGrpcStatus &other) noexcept
    {
        std::swap(m_code, other.m_code);
        m_message.swap(other.m_message);
    }

    [[nodiscard]] QtGrpc::StatusCode code() const noexcept { return m_code; }
    [[nodiscard]] bool isOk() const noexcept { return code() == QtGrpc::StatusCode::Ok; }

    [[nodiscard]] const QString &message() const & noexcept { return m_message; }
    [[nodiscard]] QString message() && noexcept { return std::move(m_message); }

private:
    QtGrpc::StatusCode m_code;
    QString m_message;

    friend bool comparesEqual(const QGrpcStatus &lhs, QtGrpc::StatusCode rhs) noexcept
    {
        return lhs.code() == rhs;
    }
    friend bool comparesEqual(const QGrpcStatus &lhs, const QGrpcStatus &rhs) noexcept
    {
        return lhs.code() == rhs.code();
    }
    Q_DECLARE_EQUALITY_COMPARABLE(QGrpcStatus, QtGrpc::StatusCode)
    Q_DECLARE_EQUALITY_COMPARABLE(QGrpcStatus)

    friend size_t qHash(const QGrpcStatus &key, size_t seed = 0) noexcept
    {
        return qHash(key.code(), seed);
    }

#ifndef QT_NO_DEBUG_STREAM
    friend Q_GRPC_EXPORT QDebug operator<<(QDebug debug, const QGrpcStatus &status);
#endif
#ifndef QT_NO_DATASTREAM
    friend Q_GRPC_EXPORT QDataStream &operator<<(QDataStream &out, const QGrpcStatus &status);
    friend Q_GRPC_EXPORT QDataStream &operator>>(QDataStream &in, QGrpcStatus &status);
#endif
};

Q_DECLARE_SHARED(QGrpcStatus)

QT_END_NAMESPACE

#endif // QGRPCSTATUS_H
