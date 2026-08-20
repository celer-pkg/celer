// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "logginginterceptor.h"

#include <QtGrpc/QGrpcStatus>

using namespace Qt::Literals::StringLiterals;

LoggingInterceptor::LoggingInterceptor(std::shared_ptr<LogModel> logModel)
    : m_log(std::move(logModel))
{
    Q_ASSERT(m_log);
}

LoggingInterceptor::~LoggingInterceptor() = default;

//! [interceptor-3]
LoggingInterceptor::Continuation LoggingInterceptor::onStart(QGrpcInterceptionContext &context,
                                                             QProtobufMessage &, QGrpcCallOptions &)
{
    const auto id = context.operationId();
    m_activeRPCs.insert(id, Clock::now());
    m_log->add(LogModel::Level::Debug, id, context.descriptor(), u"Starting"_s);
    return Continuation::Proceed;
}
//! [interceptor-3]

void LoggingInterceptor::onInitialMetadata(QGrpcInterceptionContext &context,
                                           QMultiHash<QByteArray, QByteArray> &metadata)
{
    if (metadata.isEmpty())
        return;
    m_log->add(LogModel::Level::Info, context.operationId(), context.descriptor(),
               u"Initial metadata: %1"_s.arg(QDebug::toString(metadata)));
}

void LoggingInterceptor::onMessageReceived(QGrpcInterceptionContext &context,
                                           QByteArray &messageData)
{
    m_log->add(LogModel::Level::Debug, context.operationId(), context.descriptor(),
               u"Received %1 bytes"_s.arg(messageData.size()));
}

void LoggingInterceptor::onWriteMessage(QGrpcInterceptionContext &context, QProtobufMessage &)
{
    m_log->add(LogModel::Level::Debug, context.operationId(), context.descriptor(),
               u"About to write message"_s);
}

void LoggingInterceptor::onWritesDone(QGrpcInterceptionContext &context)
{
    m_log->add(LogModel::Level::Info, context.operationId(), context.descriptor(),
               u"About to finish communication"_s);
}

void LoggingInterceptor::onTrailingMetadata(QGrpcInterceptionContext &context,
                                            QMultiHash<QByteArray, QByteArray> &metadata)
{
    if (metadata.isEmpty())
        return;
    m_log->add(LogModel::Level::Info, context.operationId(), context.descriptor(),
               u"Trailing metadata: %1"_s.arg(QDebug::toString(metadata)));
}

void LoggingInterceptor::onCancel(QGrpcInterceptionContext &context)
{
    m_log->add(LogModel::Level::Info, context.operationId(), context.descriptor(),
               u"About to cancel the RPC"_s);
}

//! [interceptor-4]
void LoggingInterceptor::onFinished(QGrpcInterceptionContext &context, QGrpcStatus &status)
{
    const auto it = m_activeRPCs.find(context.operationId());
    Q_ASSERT(it != m_activeRPCs.cend());
    const auto duration = Ms(Clock::now() - it.value()).count();
    m_activeRPCs.erase(it);

    const auto codeStr = QDebug::toString(status.code()).section("::", -1);
    auto msg = u"Finished in %1 ms. StatusCode: %2"_s.arg(duration).arg(codeStr);
    if (!status.message().isEmpty())
        msg += u", Message: "_s + status.message();

    const auto level = [&] {
        switch (status.code()) {
        case QtGrpc::StatusCode::Ok:
            return LogModel::Level::Info;
        case QtGrpc::StatusCode::NotFound:
        case QtGrpc::StatusCode::Unauthenticated:
            return LogModel::Level::Warning;
        default:
            return LogModel::Level::Error;
        }
    }();

    m_log->add(level, context.operationId(), context.descriptor(), msg);
}
//! [interceptor-4]
