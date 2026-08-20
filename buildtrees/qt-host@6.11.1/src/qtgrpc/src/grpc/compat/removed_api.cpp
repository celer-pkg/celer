// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#define QT_GRPC_BUILD_REMOVED_API

#include <QtGrpc/qtgrpcglobal.h>

QT_USE_NAMESPACE

#if QT_GRPC_REMOVED_SINCE(6, 11)

#include "qgrpccallreply.h"

QGrpcCallReply::QGrpcCallReply(std::shared_ptr<QGrpcOperationContext> operationContext)
    : QGrpcOperation(std::move(operationContext))
{
}

#include "qgrpcoperation.h"

QGrpcOperation::QGrpcOperation(std::shared_ptr<QGrpcOperationContext>, QObject *)
{
    // Users could only have passed a nullptr when reaching this!
    qFatal("Removed constructor %s called", Q_FUNC_INFO);
}

#include "qgrpcstream.h"

QGrpcServerStream::QGrpcServerStream(std::shared_ptr<QGrpcOperationContext> operationContext,
                                     QObject *parent)
    : QGrpcOperation(std::move(operationContext), parent)
{
    QObject::connect(&QGrpcOperation::context(), &QGrpcOperationContext::messageReceived, this,
                     &QGrpcServerStream::messageReceived);
}

QGrpcClientStream::QGrpcClientStream(std::shared_ptr<QGrpcOperationContext> operationContext,
                                     QObject *parent)
    : QGrpcOperation(std::move(operationContext), parent)
{
}

QGrpcBidiStream::QGrpcBidiStream(std::shared_ptr<QGrpcOperationContext> operationContext,
                                 QObject *parent)
    : QGrpcOperation(std::move(operationContext), parent)
{
    QObject::connect(&QGrpcOperation::context(), &QGrpcOperationContext::messageReceived,
                     this, &QGrpcBidiStream::messageReceived);
}

// #include "qotherheader.h"
// // implement removed functions from qotherheader.h
// order sections alphabetically to reduce chances of merge conflicts

#endif // QT_GRPC_REMOVED_SINCE(6, 11)
