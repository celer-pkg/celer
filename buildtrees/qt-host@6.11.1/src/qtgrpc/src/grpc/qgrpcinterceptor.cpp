// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtGrpc/qgrpcinterceptor.h>
#include <QtGrpc/qgrpcoperationcontext.h>

using namespace QtGrpc;
using QtGrpcPrivate::InterceptorTypes;

static_assert(InterceptorTypes::size <= std::numeric_limits<
                  std::underlying_type_t<QtGrpc::InterceptorCapability>>::digits,
              "Interceptors exceed the capability limit!");

QT_BEGIN_NAMESPACE

/*!
    \since 6.11
    \inmodule QtGrpc
    \headerfile <QGrpcInterceptor>
    \title Interceptor types
    \brief The <QGrpcInterceptor> header file provides the basic interceptor
    types.

    Qt GRPC provides various types that are commonly grouped in the
    \c{<QtGrpcInterceptor>} header file.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    Qt GRPC provides the following interceptor interfaces as hook points in the
    RPC lifecycle:

    \include qtgrpc-interceptors-overview.qdoc interface-table

    \section1 Interceptor mechanism

    The following classes and pages form the Qt GRPC interceptor mechanism:

    \generatelist qtgrpc-interceptors
*/

/*!
    \group qtgrpc-interceptors
    \title Qt GRPC Interceptors

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    Qt GRPC provides the following interceptor interfaces as hook points in the
    RPC lifecycle:

    \include qtgrpc-interceptors-overview.qdoc interface-table

    The following classes and pages form the Qt GRPC interceptor mechanism:
*/

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcInterceptionContext
    \brief Provides contextual information to interceptor callbacks.

    QGrpcInterceptionContext is passed to interceptor callbacks and provides
    access to information about the intercepted RPC, such as the channel, the
    RPC descriptor, or call options.

    A QGrpcInterceptionContext instance is created by Qt GRPC for each callback
    invocation. The context object is only valid for the duration of the
    callback and must not be stored.

    \note QGrpcInterceptionContext is not copyable or movable.

    \sa QGrpcStartInterceptor, QGrpcInitialMetadataInterceptor,
        QGrpcMessageReceivedInterceptor, QGrpcWriteMessageInterceptor,
        QGrpcWritesDoneInterceptor, QGrpcTrailingMetadataInterceptor,
        QGrpcFinishedInterceptor, QGrpcCancelInterceptor
*/

/*!
    \fn quint32 QGrpcInterceptionContext::version() const noexcept

    Returns the version of the interception context.

    Future versions of Qt may extend QGrpcInterceptionContext. This function
    can be used to identify the supported revision of the context.
*/

/*!
    \fn const QAbstractGrpcChannel &QGrpcInterceptionContext::channel() const & noexcept

    Returns the channel used for the intercepted RPC.
*/

/*!
    Returns the call options that are in effect for the intercepted RPC.
*/
const QGrpcCallOptions &QGrpcInterceptionContext::callOptions() const & noexcept
{
    return m_operationContext->callOptions();
}

/*!
    Returns the descriptor identifying the intercepted RPC.
*/
RpcDescriptor QGrpcInterceptionContext::descriptor() const noexcept
{
    return m_operationContext->descriptor();
}

/*!
    Returns the unique identifier of the intercepted call.

    Each ID is unique across all channels for the application lifetime.

    This identifier can be used to correlate interception callbacks that belong
    to the same RPC operation across multiple hook points.
*/
quint64 QGrpcInterceptionContext::operationId() const noexcept
{
    return m_operationContext->operationId();
}

/*!
    \since 6.11
    \ingroup qtgrpc-interceptors
    \relates <QGrpcInterceptor>
    \enum QtGrpc::InterceptorCapability
    \brief Specifies interception hook points supported by an interceptor.

    Qt GRPC uses InterceptorCapability values to describe which interception
    hook points an interceptor provides.

    A single interceptor class can implement multiple interceptor interfaces
    (for example, QGrpcStartInterceptor and QGrpcFinishedInterceptor) and
    therefore support multiple capabilities.

    \value Start            The interceptor implements QGrpcStartInterceptor.
    \value InitialMetadata  The interceptor implements QGrpcInitialMetadataInterceptor.
    \value MessageReceived  The interceptor implements QGrpcMessageReceivedInterceptor.
    \value WriteMessage     The interceptor implements QGrpcWriteMessageInterceptor.
    \value WritesDone       The interceptor implements QGrpcWritesDoneInterceptor.
    \value TrailingMetadata The interceptor implements QGrpcTrailingMetadataInterceptor.
    \value Finished         The interceptor implements QGrpcFinishedInterceptor.
    \value Cancel           The interceptor implements QGrpcCancelInterceptor.
*/

/*!
    \since 6.11
    \ingroup qtgrpc-interceptors
    \relates <QGrpcInterceptor>
    \typedef QtGrpc::InterceptorCapabilities
    \brief A QFlags type that stores a combination of InterceptorCapability values.

    \sa QtGrpc::InterceptorCapability
*/

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcStartInterceptor
    \brief Intercepts the start of an RPC.
    \reentrant

    Implement QGrpcStartInterceptor to observe or adjust parameters that are
    used when an RPC is started.

    The interceptor is invoked once per RPC, before the call proceeds.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {outbound}

    \sa QtGrpc::InterceptorCapability::Start, QGrpcInterceptionContext,
        QGrpcInterceptionContext
*/

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \relates QGrpcStartInterceptor
    \enum QGrpcStartInterceptor::Continuation
    \brief Controls whether an intercepted RPC should proceed.

    \value Proceed Continue the call.
    \value Drop Abort the call.
*/

/*!
    \fn QGrpcStartInterceptor::Continuation QGrpcStartInterceptor::onStart(QGrpcInterceptionContext &context, QProtobufMessage &message, QGrpcCallOptions &callOptions)

    Called when an RPC is about to start.

    The \a context provides information about the intercepted RPC.

    The \a message contains the initial request message and can be modified in place.

    The \a callOptions can be modified to adjust per-call settings.

    Return Continuation::Proceed to continue the RPC, or
    Continuation::Drop to prevent it from proceeding.

    If Continuation::Drop is returned, the RPC is aborted and the operation
    transitions to \l{QGrpcOperation::}{finished()}. The resulting status code
    is \l{QtGrpc::StatusCode::}{Aborted}. Interceptors implementing
    QGrpcFinishedInterceptor are still invoked.

    \note The mutable \a callOptions refers to the same call options
    instance returned by \c{context.callOptions()}.
*/

/*!
    Destroys the interceptor.
*/
QGrpcStartInterceptor::~QGrpcStartInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcInitialMetadataInterceptor
    \brief Intercepts initial metadata for an RPC.
    \reentrant

    Implement QGrpcInitialMetadataInterceptor to observe or modify initial
    metadata associated with an RPC.

    Initial metadata is sent by the server immediately after the call is
    established and before any response messages are delivered.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {outbound}

    \sa QtGrpc::InterceptorCapability::InitialMetadata,
        QGrpcOperation::serverInitialMetadata(),
        QGrpcOperation::serverInitialMetadataReceived(),
        QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcInitialMetadataInterceptor::onInitialMetadata(QGrpcInterceptionContext &context, QMultiHash<QByteArray, QByteArray> &metadata)

    Called when the initial \a metadata becomes available for the RPC.

    The \a context provides information about the intercepted RPC.
    The \a metadata can be modified in place before it is forwarded to the RPC.

    \sa QGrpcOperation::serverInitialMetadata(),
        QGrpcOperation::serverInitialMetadataReceived()
*/

/*!
    Destroys the interceptor.
*/
QGrpcInitialMetadataInterceptor::~QGrpcInitialMetadataInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcMessageReceivedInterceptor
    \brief Intercepts incoming message payloads for an RPC.
    \reentrant

    Implement QGrpcMessageReceivedInterceptor to observe or modify the
    serialized message payload of messages received from an RPC.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {outbound}

    \sa QtGrpc::InterceptorCapability::MessageReceived, QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcMessageReceivedInterceptor::onMessageReceived(QGrpcInterceptionContext &context, QByteArray &messageData)

    Called when an incoming message payload \a messageData becomes available.

    The \a context provides information about the intercepted RPC.
    The \a messageData contains serialized message bytes and can be modified in place.

    \sa QGrpcServerStream::messageReceived(), QGrpcBidiStream::messageReceived()
*/

/*!
    Destroys the interceptor.
*/
QGrpcMessageReceivedInterceptor::~QGrpcMessageReceivedInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcWriteMessageInterceptor
    \brief Intercepts outgoing messages written for an RPC.
    \reentrant

    Implement QGrpcWriteMessageInterceptor to observe or modify messages that are
    written as part of an RPC after it has been started.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {inbound}

    \sa QtGrpc::InterceptorCapability::WriteMessage, QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcWriteMessageInterceptor::onWriteMessage(QGrpcInterceptionContext &context, QProtobufMessage &message)

    Called when an outgoing \a message is about to be written for the RPC.

    The \a context provides information about the intercepted RPC.
    The \a message can be modified in place.

    \sa QGrpcClientStream::writeMessage(), QGrpcBidiStream::writeMessage()
*/

/*!
    Destroys the interceptor.
*/
QGrpcWriteMessageInterceptor::~QGrpcWriteMessageInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcWritesDoneInterceptor
    \brief Intercepts the completion of message writes for an RPC.
    \reentrant

    Implement QGrpcWritesDoneInterceptor to observe when no further outgoing
    messages will be written for an RPC.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {inbound}

    \sa QtGrpc::InterceptorCapability::WritesDone, QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcWritesDoneInterceptor::onWritesDone(QGrpcInterceptionContext &context)

    Called when the local side indicates that no further outgoing messages will
    be written.

    The \a context provides information about the intercepted RPC.

    \sa QGrpcClientStream::writesDone(), QGrpcBidiStream::writesDone()
*/

/*!
    Destroys the interceptor.
*/
QGrpcWritesDoneInterceptor::~QGrpcWritesDoneInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcTrailingMetadataInterceptor
    \brief Intercepts trailing metadata for an RPC.
    \reentrant

    Implement QGrpcTrailingMetadataInterceptor to observe or modify trailing
    metadata associated with an RPC.

    Trailing metadata is sent by the server when the RPC is completing,
    after all response messages have been delivered.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {outbound}

    \sa QtGrpc::InterceptorCapability::TrailingMetadata,
        QGrpcOperation::serverTrailingMetadata(), QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcTrailingMetadataInterceptor::onTrailingMetadata(QGrpcInterceptionContext &context, QMultiHash<QByteArray, QByteArray> &metadata)

    Called when trailing \a metadata becomes available for the RPC.

    The \a context provides information about the intercepted RPC.
    The \a metadata can be modified in place before it is forwarded to the RPC.

    \sa QGrpcOperation::serverTrailingMetadata()
*/

/*!
    Destroys the interceptor.
*/
QGrpcTrailingMetadataInterceptor::~QGrpcTrailingMetadataInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcFinishedInterceptor
    \brief Intercepts the completion status of an RPC.
    \reentrant

    Implement QGrpcFinishedInterceptor to observe or modify the final status of
    an RPC.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {outbound}

    \sa QtGrpc::InterceptorCapability::Finished, QGrpcOperation::finished(),
        QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcFinishedInterceptor::onFinished(QGrpcInterceptionContext &context, QGrpcStatus &status)

    Called when an RPC is completing.

    The \a context provides information about the intercepted RPC.
    The \a status contains the final status and can be modified in place.

    \sa QGrpcOperation::finished()
*/

/*!
    Destroys the interceptor.
*/
QGrpcFinishedInterceptor::~QGrpcFinishedInterceptor() = default;

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \inheaderfile QGrpcInterceptor
    \class QGrpcCancelInterceptor
    \brief Intercepts cancellation requests for an RPC.
    \reentrant

    Implement QGrpcCancelInterceptor to observe when cancellation is requested
    for an RPC.

    \include qtgrpc-shared.qdocinc interceptors_overview_ref

    \interceptorDirection {inbound}

    \sa QtGrpc::InterceptorCapability::Cancel, QGrpcInterceptionContext
*/

/*!
    \fn void QGrpcCancelInterceptor::onCancel(QGrpcInterceptionContext &context)

    Called when cancellation is requested for the RPC.

    The \a context provides information about the intercepted RPC.

    \sa QGrpcOperation::cancel()
*/

/*!
    Destroys the interceptor.
*/
QGrpcCancelInterceptor::~QGrpcCancelInterceptor() = default;

QT_END_NAMESPACE
