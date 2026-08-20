// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// clang-format off

#pragma once

#include <QtGrpc/QGrpcInterceptor>
#include <QtGrpc/QGrpcInterceptorChain>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <memory>
#include <chrono>

//! [metrics-0]
class MetricsInterceptor final : public QGrpcStartInterceptor,
                                 public QGrpcMessageReceivedInterceptor,
                                 public QGrpcFinishedInterceptor
{
public:
    ~MetricsInterceptor() override;

    Continuation onStart(QGrpcInterceptionContext &context,
                         QProtobufMessage &message,
                         QGrpcCallOptions &callOptions) override;

    void onMessageReceived(QGrpcInterceptionContext &context,
                           QByteArray &messageData) override;

    void onFinished(QGrpcInterceptionContext &context,
                    QGrpcStatus &status) override;

private:
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::duration<double, std::milli>;

    struct RpcMetrics {
        quint64 bytesReceived = 0;
        quint64 completed = 0;
        quint64 failed = 0;
        QList<double> durations;
    };

    QHash<quint64, Clock::time_point> m_activeRPCs;
    QMap<QtGrpc::RpcDescriptor, RpcMetrics> m_metrics;
};
//! [metrics-0]

//! [auth-0]
class TokenProvider
{
public:
    TokenProvider();
    QByteArray token();
    void refresh();

private:
    QByteArray m_token;
};

class AuthInterceptor final : public QGrpcStartInterceptor,
                              public QGrpcTrailingMetadataInterceptor,
                              public QGrpcFinishedInterceptor
{
public:
    Continuation onStart(QGrpcInterceptionContext &context,
                         QProtobufMessage &message,
                         QGrpcCallOptions &callOptions) override;

    void onTrailingMetadata(QGrpcInterceptionContext &context,
                            QMultiHash<QByteArray, QByteArray> &metadata) override;

    void onFinished(QGrpcInterceptionContext &context,
                    QGrpcStatus &status) override;

private:
    TokenProvider m_tokenProvider;
    QByteArray m_serverAuthHint;
};
//! [auth-0]

//! [sanitizing-0]
class SanitizingInterceptor final : public QGrpcStartInterceptor,
                                    public QGrpcWriteMessageInterceptor,
                                    public QGrpcMessageReceivedInterceptor
{
public:
    Continuation onStart(QGrpcInterceptionContext &context,
                         QProtobufMessage &message,
                         QGrpcCallOptions &callOptions) override;

    void onWriteMessage(QGrpcInterceptionContext &context,
                        QProtobufMessage &message) override;

    void onMessageReceived(QGrpcInterceptionContext &context,
                           QByteArray &messageData) override;
};
//! [sanitizing-0]

//! [interceptorchain-0]
inline QGrpcInterceptorChain createInterceptors()
{
    QGrpcInterceptorChain chain;
    chain.set(
        std::make_unique<MetricsInterceptor>(),
        std::make_unique<AuthInterceptor>(),
        std::make_unique<SanitizingInterceptor>()
    );
    return chain;
}
//! [interceptorchain-0]

inline int64_t now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}
