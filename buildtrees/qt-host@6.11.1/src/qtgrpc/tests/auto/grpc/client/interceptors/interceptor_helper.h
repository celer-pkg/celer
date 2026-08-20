// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtGrpc/qgrpcinterceptor.h>
#include <QtGrpc/qtgrpcnamespace.h>
#include <QtGrpc/qgrpccalloptions.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

#include <utility>
#include <functional>
#include <memory>

using Capability = QtGrpc::InterceptorCapability;

namespace {

struct InterceptorCall
{
    QByteArray name;
    Capability capability;

    bool operator==(const InterceptorCall &other) const
    {
        return other.name == name && other.capability == capability;
    }

    friend QDebug operator<<(QDebug debug, const InterceptorCall &call)
    {
        const QDebugStateSaver save(debug);
        debug.nospace() << "{ name: " << call.name
                        << ", capability: " << qToUnderlying(call.capability) << " }";
        return debug;
    }
};

inline QList<InterceptorCall> CallLog = {};

class LoggingInterceptor : public QGrpcStartInterceptor,
                           public QGrpcInitialMetadataInterceptor,
                           public QGrpcMessageReceivedInterceptor,
                           public QGrpcWriteMessageInterceptor,
                           public QGrpcWritesDoneInterceptor,
                           public QGrpcTrailingMetadataInterceptor,
                           public QGrpcFinishedInterceptor,
                           public QGrpcCancelInterceptor
{
public:
    explicit LoggingInterceptor(QByteArray name) : m_name(std::move(name)) { }

    Continuation onStart(QGrpcInterceptionContext &, QProtobufMessage &,
                         QGrpcCallOptions &) override
    {
        CallLog.push_back({ m_name, Capability::Start });
        return Continuation::Proceed;
    }

    void onInitialMetadata(QGrpcInterceptionContext &,
                           QMultiHash<QByteArray, QByteArray> &) override
    {
        CallLog.push_back({ m_name, Capability::InitialMetadata });
    }

    void onMessageReceived(QGrpcInterceptionContext &, QByteArray &) override
    {
        CallLog.push_back({ m_name, Capability::MessageReceived });
    }

    void onWriteMessage(QGrpcInterceptionContext &, QProtobufMessage &) override
    {
        CallLog.push_back({ m_name, Capability::WriteMessage });
    }

    void onWritesDone(QGrpcInterceptionContext &) override
    {
        CallLog.push_back({ m_name, Capability::WritesDone });
    }

    void onTrailingMetadata(QGrpcInterceptionContext &,
                            QMultiHash<QByteArray, QByteArray> &) override
    {
        CallLog.push_back({ m_name, Capability::TrailingMetadata });
    }

    void onFinished(QGrpcInterceptionContext &, QGrpcStatus &) override
    {
        CallLog.push_back({ m_name, Capability::Finished });
    }

    void onCancel(QGrpcInterceptionContext &) override
    {
        CallLog.push_back({ m_name, Capability::Cancel });
    }

protected:
    QByteArray m_name;
};

class PartialInterceptor : public QGrpcStartInterceptor, public QGrpcFinishedInterceptor
{
public:
    explicit PartialInterceptor(QByteArray name) : m_name(std::move(name)) { }

    Continuation onStart(QGrpcInterceptionContext &, QProtobufMessage &,
                         QGrpcCallOptions &) override
    {
        CallLog.push_back({ m_name, Capability::Start });
        return Continuation::Proceed;
    }

    void onFinished(QGrpcInterceptionContext &, QGrpcStatus &) override
    {
        CallLog.push_back({ m_name, Capability::Finished });
    }

private:
    QByteArray m_name;
};

class DroppingInterceptor : public QGrpcStartInterceptor
{
public:
    using Predicate = std::function<Continuation(QtGrpc::RpcDescriptor)>;

    explicit DroppingInterceptor(QByteArray name, Predicate shouldDrop)
        : m_name(std::move(name)), m_shouldDrop(std::move(shouldDrop))
    {
    }

    Continuation onStart(QGrpcInterceptionContext &context,
                         QProtobufMessage &, QGrpcCallOptions &) override
    {
        CallLog.push_back({ m_name, Capability::Start });
        return m_shouldDrop(context.descriptor());
    }

private:
    QByteArray m_name;
    Predicate m_shouldDrop;
};

class ContextVerifyingInterceptor : public QGrpcStartInterceptor,
                                    public QGrpcInitialMetadataInterceptor,
                                    public QGrpcFinishedInterceptor
{
public:
    explicit ContextVerifyingInterceptor(QByteArray name) : m_name(std::move(name)) { }

    Continuation onStart(QGrpcInterceptionContext &context,
                         QProtobufMessage &, QGrpcCallOptions &) override
    {
        CallLog.push_back({ m_name, Capability::Start });
        capturedDescriptor1 = std::make_unique<QtGrpc::RpcDescriptor>(context.descriptor());
        return Continuation::Proceed;
    }

    void onInitialMetadata(QGrpcInterceptionContext &context,
                           QMultiHash<QByteArray, QByteArray> &) override
    {
        CallLog.push_back({ m_name, Capability::InitialMetadata });
        capturedDescriptor2 = std::make_unique<QtGrpc::RpcDescriptor>(context.descriptor());
    }

    void onFinished(QGrpcInterceptionContext &context, QGrpcStatus &) override
    {
        capturedCallOptions = context.callOptions();
        capturedDescriptor3 = std::make_unique<QtGrpc::RpcDescriptor>(context.descriptor());
        CallLog.push_back({ m_name, Capability::Finished });
    }

    std::unique_ptr<QtGrpc::RpcDescriptor> capturedDescriptor1;
    std::unique_ptr<QtGrpc::RpcDescriptor> capturedDescriptor2;
    std::unique_ptr<QtGrpc::RpcDescriptor> capturedDescriptor3;
    QGrpcCallOptions capturedCallOptions;

private:
    QByteArray m_name;
};

} // namespace
