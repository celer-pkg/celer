/// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "interceptors.h"
#include "clientguide.qpb.h"

#include <QtProtobuf/QAbstractProtobufSerializer>

#include <QtGrpc/QAbstractGrpcChannel>
#include <QtGrpc/QGrpcCallOptions>
#include <QtGrpc/QGrpcStatus>

#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QUuid>
#include <QtCore/QMultiHash>

using namespace Qt::Literals::StringLiterals;

//! [metrics-2]
MetricsInterceptor::~MetricsInterceptor()
{
    Q_ASSERT(m_activeRPCs.isEmpty());
    const auto keys = m_metrics.keys();
    for (const auto &k : keys) {
        qInfo() << "Metrics for:" << k;
        const auto &m = m_metrics[k];
        qInfo() << "  Bytes received    :" << m.bytesReceived;
        qInfo() << "  Failed RPCs       :" << m.failed;
        qInfo() << "  Completed RPCs    :" << m.completed;
        qInfo() << "  Completed Durations (ms):" << m.durations;
    }
}
//! [metrics-2]

//! [metrics-1]
QGrpcStartInterceptor::Continuation
MetricsInterceptor::onStart(QGrpcInterceptionContext &context,
                            QProtobufMessage & /*message*/,
                            QGrpcCallOptions &callOptions)
{
    m_activeRPCs.insert(context.operationId(), std::chrono::steady_clock::now());
    callOptions.addMetadata("x-trace-id"_L1, QUuid::createUuid().toByteArray());

    return Continuation::Proceed;
}

void MetricsInterceptor::onMessageReceived(QGrpcInterceptionContext &context,
                                           QByteArray &messageData)
{
    m_metrics[context.descriptor()].bytesReceived += messageData.size();
}

void MetricsInterceptor::onFinished(QGrpcInterceptionContext &context,
                                    QGrpcStatus &status)
{
    const auto it = m_activeRPCs.find(context.operationId());
    Q_ASSERT(it != m_activeRPCs.cend());

    const auto duration = Ms(Clock::now() - it.value()).count();
    m_activeRPCs.erase(it);

    auto &metrics = m_metrics[context.descriptor()];
    if (!status.isOk()) {
        ++metrics.failed;
        return;
    }
    ++metrics.completed;
    metrics.durations.append(duration);
}
//! [metrics-1]

//! [auth-1]
TokenProvider::TokenProvider() { refresh(); }
QByteArray TokenProvider::token() { return m_token; }
void TokenProvider::refresh()
{
    // NOTE: This is NOT a real JWT or secure token.
    // It only mimics the structure: header.payload.signature
    // The payload is just a timestamp used for expiry checks.
    auto header = R"({"alg":"none","typ":"JWT"})"_ba; // fake header
    auto payload = QByteArray::number(now()); // fake "iat"
    auto signature = "no_signature"_ba; // fake signing

    m_token = header + '.' + payload + '.' + signature;
}
//! [auth-1]

//! [auth-2]
QGrpcStartInterceptor::Continuation
AuthInterceptor::onStart(QGrpcInterceptionContext & /*context*/,
                         QProtobufMessage & /*messasge*/,
                         QGrpcCallOptions &callOptions)
{
    constexpr QByteArrayView AuthKey("authorization");

    if (!callOptions.metadata(QtGrpc::MultiValue).contains(AuthKey))
        callOptions.addMetadata(AuthKey, "Bearer " + m_tokenProvider.token());

    return Continuation::Proceed;
}

void AuthInterceptor::onTrailingMetadata(QGrpcInterceptionContext & /*context*/,
                                         QMultiHash<QByteArray, QByteArray> &metadata)
{
    constexpr QByteArrayView AuthErrorKey("www-authenticate");
    const auto it = metadata.constFind(AuthErrorKey);
    if (it == metadata.cend())
        return;
    m_serverAuthHint = it.value();
}

void AuthInterceptor::onFinished(QGrpcInterceptionContext & /*context*/, QGrpcStatus &status)
{
    // Only handle Unauthenticated status codes.
    if (status.code() != QtGrpc::StatusCode::Unauthenticated)
        return;

    qDebug("[AuthInterceptor] Refreshing token for next call");
    // NOTE: Simplified for example. Real tokens are obtained via proper auth flow.
    m_tokenProvider.refresh();
    if (!m_serverAuthHint.isEmpty()) {
        status = QGrpcStatus{ status.code(), status.message() + u", Hint: "_s + m_serverAuthHint };
        m_serverAuthHint.clear();
    }
}
//! [auth-2]

namespace {

//! [sanitizing-1]
qsizetype getTotalBytes(const QMultiHash<QByteArray, QByteArray> &md)
{
    qsizetype total = 0;
    for (const auto &[k, v] : md.asKeyValueRange())
        total += k.size() + v.size();
    return total;
}

template <typename T>
void sanitizeMessage(T &request)
{
    const auto num = request.num();
    if (num < 0 || num > 5)
        request.setNum(qBound(0, num, 5));

    const auto time = request.time();
    const auto currentTime = now();
    if (time < 0 || time > currentTime)
        request.setTime(qMax(0, currentTime));
}
//! [sanitizing-1]

} // namespace

//! [sanitizing-2]
QGrpcStartInterceptor::Continuation
SanitizingInterceptor::onStart(QGrpcInterceptionContext &context, QProtobufMessage &message,
                               QGrpcCallOptions &callOptions)
{
    constexpr quint16 MaxMetadata = 1024 * 8; // 8 KiB
    auto totalMdBytes = getTotalBytes(callOptions.metadata(QtGrpc::MultiValue));
    if (totalMdBytes > MaxMetadata) {
        qDebug() << "[Sanitizing] Aborting call with metadata size of" << totalMdBytes << "bytes";
        return Continuation::Drop;
    }

    onWriteMessage(context, message);

    return Continuation::Proceed;
}

void SanitizingInterceptor::onWriteMessage(QGrpcInterceptionContext & /*context*/,
                                           QProtobufMessage &message)
{
    // For handling more complex scenarios with various Request message types,
    // it would make sense to branch on the context.descriptor() appropriately.
    if (auto *request = qprotobufmessage_cast<client::guide::Request *>(&message))
        sanitizeMessage(*request);
}

void SanitizingInterceptor::onMessageReceived(QGrpcInterceptionContext &context,
                                              QByteArray &messageData)
{
    // messageData can only be of type client::guide::Response. For more
    // complex handling add appropriate handling with context.descriptor().
    // Note that the deserialization and serialization introduces overhead.
    client::guide::Response response;
    auto serializer = context.channel().serializer();
    if (serializer->deserialize(&response, messageData))
        return;
    sanitizeMessage(response);
    messageData = serializer->serialize(&response);
}
//! [sanitizing-2]
