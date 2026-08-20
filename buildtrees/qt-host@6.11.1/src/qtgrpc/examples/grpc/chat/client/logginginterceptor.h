// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LOGGINGINTERCEPTOR_H
#define LOGGINGINTERCEPTOR_H

#include "logmodel.h"

#include <QtGrpc/QGrpcInterceptor>

#include <QtCore/QHash>

#include <chrono>
#include <memory>

//! [interceptor-2]
class LoggingInterceptor final : public QGrpcStartInterceptor,
                                 public QGrpcInitialMetadataInterceptor,
                                 public QGrpcMessageReceivedInterceptor,
                                 public QGrpcWriteMessageInterceptor,
                                 public QGrpcWritesDoneInterceptor,
                                 public QGrpcTrailingMetadataInterceptor,
                                 public QGrpcCancelInterceptor,
                                 public QGrpcFinishedInterceptor
{
public:
    explicit LoggingInterceptor(std::shared_ptr<LogModel> logModel);
    ~LoggingInterceptor() override;

    Continuation onStart(QGrpcInterceptionContext &context, QProtobufMessage &message,
                         QGrpcCallOptions &callOptions) override;

    void onInitialMetadata(QGrpcInterceptionContext &context,
                           QMultiHash<QByteArray, QByteArray> &metadata) override;

    void onMessageReceived(QGrpcInterceptionContext &context, QByteArray &messageData) override;

    void onWriteMessage(QGrpcInterceptionContext &context, QProtobufMessage &message) override;

    void onWritesDone(QGrpcInterceptionContext &context) override;

    void onTrailingMetadata(QGrpcInterceptionContext &context,
                            QMultiHash<QByteArray, QByteArray> &metadata) override;

    void onCancel(QGrpcInterceptionContext &context) override;

    void onFinished(QGrpcInterceptionContext &context, QGrpcStatus &status) override;

private:
    std::shared_ptr<LogModel> m_log;
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::duration<double, std::milli>;
    QHash<quint64, Clock::time_point> m_activeRPCs;
};
//! [interceptor-2]

#endif // LOGGINGINTERCEPTOR_H
