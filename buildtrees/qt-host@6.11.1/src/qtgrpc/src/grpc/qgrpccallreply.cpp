// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qgrpccallreply.h>
#include <QtGrpc/qgrpcoperationcontext.h>

#include <QtCore/qeventloop.h>
#include <QtCore/qthread.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QGrpcCallReply
    \inmodule QtGrpc
    \brief The QGrpcCallReply class provides access in handling unary RPCs.

    The QGrpcCallReply class provides the interface for handling unary remote
    procedure calls (RPCs), which is one of the four \gRPC \l{Service
    Methods}{service methods}.

    For a high-level overview, refer to the \l{Unary Calls} {Qt GRPC
    Client Guide}.

    \include qtgrpc-shared.qdocinc rpc-lifetime-note
*/

/*!
    \internal

    Constructs a new QGrpcCallReply from the QGrpcClientBase.

    This is indirectly called by the generated client interface.

    \sa QGrpcClientBase::call QAbstractGrpcChannel::call
*/
QGrpcCallReply::QGrpcCallReply(QLatin1StringView service, QLatin1StringView method,
                               const QGrpcCallOptions &options,
                               std::weak_ptr<QAbstractGrpcChannel> channel,
                               PrivateConstructor /* unused */)
    : QGrpcOperation({ service, method, QtGrpc::RpcType::UnaryCall }, options, std::move(channel))
{
}

/*!
    Destroys the QGrpcCallReply.
*/
QGrpcCallReply::~QGrpcCallReply() = default;

bool QGrpcCallReply::event(QEvent *event)
{
    return QGrpcOperation::event(event);
}

QT_END_NAMESPACE

#include "moc_qgrpccallreply.cpp"
