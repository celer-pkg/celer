// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpcQuick/qqmlgrpcfunctionalhandlers.h>

QT_BEGIN_NAMESPACE

namespace QtGrpcQuickFunctional {

void handleDeserializationError(QJSEngine *jsEngine, const QJSValue &errorCallback)
{
    if (!errorCallback.isCallable())
        return;
    using StatusCode = QtGrpc::StatusCode;
    const auto status = QGrpcStatus{ StatusCode::InvalidArgument,
                                     "Unable to deserialize return value" };
    errorCallback.call(QJSValueList{ jsEngine->toScriptValue(status) });
}

bool checkReceivedStatus(QJSEngine *jsEngine, const QGrpcStatus &status,
                         const QJSValue &errorCallback)
{
    Q_ASSERT(jsEngine != nullptr);

    if (status.isOk())
        return true;

    if (errorCallback.isCallable())
        errorCallback.call(QJSValueList{ jsEngine->toScriptValue(status) });
    return false;
}

void connectMultipleReceiveOperationFinished(QJSEngine *jsEngine,
                                             std::unique_ptr<QGrpcOperation> &&operation,
                                             const QJSValue &successCallback,
                                             const QJSValue &errorCallback)
{
    auto *operationPtr = operation.get();
    QtGrpcQuickFunctional::validateEngineAndOperation(jsEngine, operationPtr);

    QObject::connect(
        operationPtr, &QGrpcOperation::finished, jsEngine,
        [successCallback, errorCallback, jsEngine,
         operation = std::move(operation)](const QGrpcStatus &status) {
            // We take 'operation' by copy so that its lifetime
            // is extended until this lambda is destroyed.
            if (QtGrpcQuickFunctional::checkReceivedStatus(jsEngine, status, errorCallback)
                && successCallback.isCallable()) {
                successCallback.call();
            }
        },
        Qt::SingleShotConnection);
}

void handleReceivedMessageImpl(QJSEngine *jsEngine, std::optional<QJSValue> message,
                               const QJSValue &successCallback, const QJSValue &errorCallback)
{
    if (!successCallback.isCallable())
        return;

    if (message)
        successCallback.call(QJSValueList{ *message });
    else
        QtGrpcQuickFunctional::handleDeserializationError(jsEngine, errorCallback);
}

void Private::connectSingleReceiveOperationFinishedImpl(QJSEngine *jsEngine,
                                                        HandleReceivedMessageImpl impl,
                                                        std::unique_ptr<QGrpcOperation> &&operation,
                                                        const QJSValue &successCallback,
                                                        const QJSValue &errorCallback)
{
    auto *operationPtr = operation.get();
    QtGrpcQuickFunctional::validateEngineAndOperation(jsEngine, operationPtr);

    QObject::connect(
        operationPtr, &QGrpcCallReply::finished, jsEngine,
        [jsEngine, successCallback, errorCallback, impl,
         operation = std::move(operation)](const QGrpcStatus &status) {
            // We take 'operation' by copy so that its lifetime
            // is extended until this lambda is destroyed.
            if (QtGrpcQuickFunctional::checkReceivedStatus(jsEngine, status, errorCallback))
                impl(jsEngine, operation.get(), successCallback, errorCallback);
        },
        Qt::SingleShotConnection);
}

void Private::makeServerStreamConnectionsImpl(QJSEngine *jsEngine,
                                              HandleReceivedMessageImpl impl,
                                              std::unique_ptr<QGrpcServerStream> &&stream,
                                              const QJSValue &messageCallback,
                                              const QJSValue &finishCallback,
                                              const QJSValue &errorCallback)
{
    QObject::connect(stream.get(), &QGrpcServerStream::messageReceived, jsEngine,
                     [streamPtr = stream.get(), impl, messageCallback, jsEngine, errorCallback]() {
        impl(jsEngine, streamPtr, messageCallback, errorCallback);
    });
    QtGrpcQuickFunctional::connectMultipleReceiveOperationFinished(jsEngine, std::move(stream),
                                                                   finishCallback, errorCallback);
}

} // namespace QtGrpcQuickFunctional

QT_END_NAMESPACE
