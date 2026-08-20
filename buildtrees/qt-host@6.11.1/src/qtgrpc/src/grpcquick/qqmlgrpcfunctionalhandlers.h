// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQMLGRPCFUNCTIONALHANDLERS_H
#define QQMLGRPCFUNCTIONALHANDLERS_H

#include <QtGrpcQuick/qtgrpcquickexports.h>

#include <QtGrpc/qgrpccallreply.h>
#include <QtGrpc/qgrpcstream.h>

#include <QtQml/qqmlengine.h>

#include <QtCore/qobject.h>
#include <QtCore/qtconfigmacros.h>

QT_BEGIN_NAMESPACE

namespace QtGrpcQuickFunctional {

Q_ALWAYS_INLINE void validateEngineAndOperation(QJSEngine *jsEngine, QGrpcOperation *operation)
{
    Q_ASSERT(jsEngine != nullptr);
    Q_ASSERT(operation != nullptr);
}

template <typename Ret>
std::optional<QJSValue> readMessageToJSValue(QJSEngine *jsEngine, QGrpcOperation *operation)
{
    QtGrpcQuickFunctional::validateEngineAndOperation(jsEngine, operation);

    if (std::optional<Ret> result = operation->read<Ret>())
        return jsEngine->toScriptValue(*result);
    return std::nullopt;
}

Q_GRPCQUICK_EXPORT
void handleReceivedMessageImpl(QJSEngine *jsEngine, std::optional<QJSValue> message,
                               const QJSValue &successCallback, const QJSValue &errorCallback);

template <typename Ret>
void handleReceivedMessage(QJSEngine *jsEngine, QGrpcOperation *operation,
                           const QJSValue &successCallback, const QJSValue &errorCallback)
{
    auto message = QtGrpcQuickFunctional::readMessageToJSValue<Ret>(jsEngine, operation);
    QtGrpcQuickFunctional::handleReceivedMessageImpl(jsEngine, std::move(message), successCallback,
                                                     errorCallback);
}

Q_GRPCQUICK_EXPORT
bool checkReceivedStatus(QJSEngine *jsEngine, const QGrpcStatus &status,
                         const QJSValue &errorCallback);

Q_GRPCQUICK_EXPORT
void connectMultipleReceiveOperationFinished(QJSEngine *jsEngine,
                                             std::unique_ptr<QGrpcOperation> &&operation,
                                             const QJSValue &successCallback,
                                             const QJSValue &errorCallback);

namespace Private {
using HandleReceivedMessageImpl = void (*)(QJSEngine *jsEngine, QGrpcOperation *operation,
                                           const QJSValue &successCallback,
                                           const QJSValue &errorCallback);
Q_GRPCQUICK_EXPORT
void connectSingleReceiveOperationFinishedImpl(QJSEngine *jsEngine,
                                               HandleReceivedMessageImpl impl,
                                               std::unique_ptr<QGrpcOperation> &&operation,
                                               const QJSValue &successCallback,
                                               const QJSValue &errorCallback);
Q_GRPCQUICK_EXPORT
void makeServerStreamConnectionsImpl(QJSEngine *jsEngine,
                                     HandleReceivedMessageImpl impl,
                                     std::unique_ptr<QGrpcServerStream> &&stream,
                                     const QJSValue &messageCallback,
                                     const QJSValue &finishCallback,
                                     const QJSValue &errorCallback);
} // namespace Private

template <typename Ret>
void connectSingleReceiveOperationFinished(QJSEngine *jsEngine,
                                           std::unique_ptr<QGrpcOperation> &&operation,
                                           const QJSValue &successCallback,
                                           const QJSValue &errorCallback)
{
    Private::connectSingleReceiveOperationFinishedImpl(jsEngine, handleReceivedMessage<Ret>,
                                                       std::move(operation), successCallback,
                                                       errorCallback);
}

template <typename Ret>
void makeCallConnections(QJSEngine *jsEngine, std::unique_ptr<QGrpcCallReply> &&reply,
                         const QJSValue &finishCallback, const QJSValue &errorCallback)
{
    QtGrpcQuickFunctional::connectSingleReceiveOperationFinished<Ret>(jsEngine, std::move(reply),
                                                                      finishCallback,
                                                                      errorCallback);
}

template <typename Ret>
void makeServerStreamConnections(QJSEngine *jsEngine, std::unique_ptr<QGrpcServerStream> &&stream,
                                 const QJSValue &messageCallback, const QJSValue &finishCallback,
                                 const QJSValue &errorCallback)
{
    Private::makeServerStreamConnectionsImpl(jsEngine, &handleReceivedMessage<Ret>,
                                             std::move(stream), messageCallback, finishCallback,
                                             errorCallback);
}

template <typename Ret>
void makeClientStreamConnections(QJSEngine *jsEngine, std::unique_ptr<QGrpcClientStream> &&stream,
                                 const QJSValue &finishCallback, const QJSValue &errorCallback)
{
    QtGrpcQuickFunctional::connectSingleReceiveOperationFinished<Ret>(jsEngine, std::move(stream),
                                                                      finishCallback,
                                                                      errorCallback);
}

template <typename Ret>
void makeBidiStreamConnections(QJSEngine *jsEngine, std::unique_ptr<QGrpcBidiStream> &&stream,
                               const QJSValue &messageCallback, const QJSValue &finishCallback,
                               const QJSValue &errorCallback)
{
    QObject::connect(stream.get(), &QGrpcBidiStream::messageReceived, jsEngine,
                     [streamPtr = stream.get(), messageCallback, jsEngine, errorCallback] {
                         QtGrpcQuickFunctional::handleReceivedMessage<Ret>(jsEngine, streamPtr,
                                                                           messageCallback,
                                                                           errorCallback);
                     });
    QtGrpcQuickFunctional::connectMultipleReceiveOperationFinished(jsEngine, std::move(stream),
                                                                   finishCallback, errorCallback);
}

} // namespace QtGrpcQuickFunctional

QT_END_NAMESPACE

#endif // QQMLGRPCFUNCTIONALHANDLERS_H
