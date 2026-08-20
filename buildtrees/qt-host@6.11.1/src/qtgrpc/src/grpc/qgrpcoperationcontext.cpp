// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/private/qgrpccommonoptions_p.h>
#include <QtGrpc/private/qgrpcoperation_p.h>
#include <QtGrpc/private/qgrpcoperationcontext_p.h>
#include <QtGrpc/qgrpcoperation.h>
#include <QtGrpc/qgrpcoperationcontext.h>
#include <QtGrpc/qgrpcstatus.h>

#include <QtProtobuf/qprotobufserializer.h>

#include <QtCore/private/qobject_p.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qbytearrayview.h>
#include <QtCore/qlatin1stringview.h>

#include <atomic>
#include <utility>

QT_BEGIN_NAMESPACE

namespace {

quint64 nextOperationId()
{
    static std::atomic<quint64> counter{ 0 };
    return counter.fetch_add(1, std::memory_order_relaxed);
}

}

/*!
    \class QGrpcOperationContext
    \inmodule QtGrpc
    \since 6.7
    \brief The QGrpcOperationContext class provides context for communication
    between an individual \gRPC operation and a channel.

    QGrpcOperationContext is constructed internally when a remote procedure call
    (RPC) is requested, mediating interaction between the client's operation
    request and the channel's operation outcome.

    RPCs requested on the client interface return specializations of
    QGrpcOperation, such as QGrpcCallReply, QGrpcServerStream,
    QGrpcClientStream, or QGrpcBidiStream. These classes are implicitly
    integrated with the underlying QGrpcOperationContext counterpart.

    \note Don't use this class directly unless implementing a custom channel.

    The signals contained within this class are used to build the communication
    between the client-facing QGrpcOperation and the QAbstractGrpcChannel
    implementation. These operations are asynchronous in nature, and multiple
    RPCs can operate on the same channel concurrently. Channels typically treat
    all operations the same, and it is the channel's responsibility to
    correctly support and restrict the subset of features its RPC type
    supports.

    Signals which should only be emitted by the channel's implementation are:
    \list
        \li finished()
        \li messageReceived()
    \endlist

    Signals which will be emitted by QGrpcOperation and its specializations are:
    \list
        \li cancelRequested()
        \li writeMessageRequested()
        \li writesDoneRequested()
    \endlist

    Custom implementations of QAbstractGrpcChannel must handle the client's
    signals, as no default signal handling is provided, and emit their own
    signals accordingly.
*/

/*!
    \fn void QGrpcOperationContext::finished(const QGrpcStatus &status)

    This signal should be emitted by the channel when an RPC finishes.

    It usually means that the server sent a \a status for the requested RPC and
    closed the connection. Implementations of QAbstractGrpcChannel should
    detect this situation and emit the signal.

    After receiving this signal, no further communication should occur through
    this operation context. The client side may then safely delete the
    corresponding RPC object.

    \note This signal is implicitly connected to the QGrpcOperation counterpart.

    \sa QGrpcOperation::finished
*/

/*!
    \fn void QGrpcOperationContext::messageReceived(const QByteArray &data)

    This signal should be emitted by the channel when a new chunk of \a data is
    received from the server.

    For client streams and unary calls, this means that the single and final
    response has arrived and communication is about to finish.

    For server and bidirectional streams, this signal should be continuously
    emitted by the channel upon receiving each new messages.

    \note This signal is implicitly connected to the QGrpcOperation
    counterpart.

    \sa QGrpcServerStream::messageReceived
    \sa QGrpcBidiStream::messageReceived
*/

/*!
    \fn void QGrpcOperationContext::cancelRequested()

    This signal is emitted by QGrpcOperation when requesting cancellation of the
    communication.

    The channel is expected to connect its cancellation logic to this signal
    and attempt to cancel the RPC and finish it with a
    \l{QtGrpc::StatusCode::}{Cancelled} status code. Successful cancellation
    cannot be guaranteed. Further processing of the data received from a
    channel is not required and should be avoided.

    \sa QGrpcOperation::cancel
*/

/*!
    \fn void QGrpcOperationContext::writeMessageRequested(const QByteArray &data)

    This signal is emitted by QGrpcClientStream or QGrpcBidiStream when it has
    serialized \a data ready for a channel to deliver.

    The channel is expected to connect its writing logic to this signal and wrap
    the serialized data in channel-related headers before writing it to the
    wire.

    \sa QGrpcClientStream::writeMessage
    \sa QGrpcBidiStream::writeMessage
*/

/*!
    \fn void QGrpcOperationContext::writesDoneRequested()

    This signal is emitted by QGrpcClientStream or QGrpcBidiStream to indicate
    that it's done writing messages. The channel should respond to this by
    half-closing the stream.

    \note After receiving this signal no more write operations are permitted
    for the streaming RPCs. The server can still send messages, which should be
    forwarded with messageReceived().

    \sa QGrpcClientStream::writesDone
    \sa QGrpcBidiStream::writesDone
*/

/*!
    \fn void QGrpcOperationContext::serverInitialMetadataReceived(const QMultiHash<QByteArray, QByteArray> &metadata)
    \since 6.11

    \include qgrpcoperation.cpp serverInitialMetadataReceived

    \sa QGrpcOperation::serverInitialMetadataReceived
    \sa QGrpcBidiStream::writesDone
*/

/*!
    \internal

    Constructs an operation context with the given \a descriptor, \a options,
    and \a serializer. The \a parent is the QGrpcOperation that owns this
    context.

    \note This class can only be constructed by its parent QGrpcOperation.
*/
QGrpcOperationContext::QGrpcOperationContext(QtGrpc::RpcDescriptor descriptor,
                                             const QGrpcCallOptions &options,
                                             std::shared_ptr<QAbstractProtobufSerializer>
                                                 serializer,
                                             QGrpcOperation *parent, PrivateConstructor /*unused*/)
    : QObject(*new QGrpcOperationContextPrivate(std::move(descriptor), options,
                                                std::move(serializer), nextOperationId()),
              parent)
{
    using namespace QtGrpc;
    Q_D(const QGrpcOperationContext);
    const auto type = d->descriptor.type;
    if (type == RpcType::UnaryCall || type == RpcType::ServerStreaming) {
        connect(this, &QGrpcOperationContext::writeMessageRequested, this, []() {
            Q_ASSERT_X(false, "QGrpcOperationContext::writeMessageRequested",
                       "This signal is disallowed for unary and server-streaming operations");
        });
        connect(this, &QGrpcOperationContext::writesDoneRequested, this, []() {
            Q_ASSERT_X(false, "QGrpcOperationContext::writesDoneRequested",
                       "This signal is disallowed for unary and server-streaming operations");
        });
    }
}

/*!
    Destroys the operation-context.
*/
QGrpcOperationContext::~QGrpcOperationContext() = default;

/*!
    Returns the method name of the service associated with this
    operation-context.
*/
QLatin1StringView QGrpcOperationContext::method() const noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->descriptor.method;
}

/*!
    Returns the service name associated with this operation-context.
*/
QLatin1StringView QGrpcOperationContext::service() const noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->descriptor.service;
}

/*!
    \since 6.11
    Returns the QtGrpc::RpcDescriptor for this operation-context.
*/
QtGrpc::RpcDescriptor QGrpcOperationContext::descriptor() const noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->descriptor;
}

/*!
    Returns the call options that is utilized by this operation-context.

    For channel-wide options, see QGrpcChannelOptions.
*/
const QGrpcCallOptions &QGrpcOperationContext::callOptions() const & noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->options;
}

/*!
    Returns the serializer of this operation-context
*/
std::shared_ptr<const QAbstractProtobufSerializer>
QGrpcOperationContext::serializer() const
{
    Q_D(const QGrpcOperationContext);
    return d->serializer;
}

#if QT_DEPRECATED_SINCE(6, 13)

/*!
    \deprecated [6.13] Use serverInitialMetadata() and serverTrailingMetadata() instead.

    \include qgrpcoperation.cpp serverInitialMetadata-shared
    \note This method is used implicitly by QGrpcOperation.

    \sa serverInitialMetadata() QGrpcOperation::serverInitialMetadata()
*/
const QHash<QByteArray, QByteArray> &QGrpcOperationContext::serverMetadata() const & noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->deprServerInitialMetadata;
}

/*!
    \fn void QGrpcOperationContext::setServerMetadata(const QHash<QByteArray, QByteArray> &metadata)
    \fn void QGrpcOperationContext::setServerMetadata(QHash<QByteArray, QByteArray> &&metadata)
    \deprecated [6.13] Use setServerInitialMetadata() instead.

    Sets the metadata received from the server at the start of the RPC.

    \sa setServerInitialMetadata()
*/
void QGrpcOperationContext::setServerMetadata(const QHash<QByteArray, QByteArray> &metadata)
{
    Q_D(QGrpcOperationContext);
    if (d->deprServerInitialMetadata == metadata)
        return;
    d->deprServerInitialMetadata = metadata;
    d->serverInitialMetadata = QMultiHash<QByteArray, QByteArray>(metadata);
}
void QGrpcOperationContext::setServerMetadata(QHash<QByteArray, QByteArray> &&metadata)
{
    Q_D(QGrpcOperationContext);
    if (d->deprServerInitialMetadata == metadata)
        return;
    d->deprServerInitialMetadata = std::move(metadata);
    d->serverInitialMetadata = QMultiHash<QByteArray, QByteArray>(d->deprServerInitialMetadata);
}

#endif // QT_DEPRECATED_SINCE(6, 13)

/*!
    \since 6.10

    \include qgrpcoperation.cpp serverInitialMetadata-shared
    \note This method is used implicitly by QGrpcOperation.

    \sa QGrpcOperation::serverInitialMetadata() serverTrailingMetadata()
*/
const QMultiHash<QByteArray, QByteArray> &
QGrpcOperationContext::serverInitialMetadata() const & noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->serverInitialMetadata;
}

/*!
    \since 6.10

    Sets the \a metadata received from the server at the start of the RPC.

    \sa serverInitialMetadata()
*/
void QGrpcOperationContext::setServerInitialMetadata(QMultiHash<QByteArray, QByteArray> &&metadata)
{
    Q_D(QGrpcOperationContext);
    if (auto channel = operation().d_func()->channel.lock()) {
        auto &engine = QAbstractGrpcChannelPrivate::get(channel.get())->interceptorEngine;
        if (engine.hasHandlerFor(QtGrpc::InterceptorCapability::InitialMetadata))
            engine.onInitialMetadata(*this, metadata);
    }
    d->serverInitialMetadata = std::move(metadata);
#if QT_DEPRECATED_SINCE(6, 13)
    d->deprServerInitialMetadata = QtGrpcPrivate::toHash(d->serverInitialMetadata);
#endif
    emit serverInitialMetadataReceived(d->serverInitialMetadata, QPrivateSignal{});
}

/*!
    \since 6.10

    \include qgrpcoperation.cpp serverTrailingMetadata
    \note This method is used implicitly by QGrpcOperation.

    \sa QGrpcOperation::serverTrailingMetadata() setServerTrailingMetadata()
*/
const QMultiHash<QByteArray, QByteArray> &
QGrpcOperationContext::serverTrailingMetadata() const & noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->serverTrailingMetadata;
}

/*!
    \since 6.10

    Sets the trailing \a metadata received from the server after all response
    messages.

    \sa serverTrailingMetadata()
*/
void QGrpcOperationContext::setServerTrailingMetadata(QMultiHash<QByteArray, QByteArray> &&metadata)
{
    Q_D(QGrpcOperationContext);
    if (auto channel = operation().d_func()->channel.lock()) {
        auto &engine = QAbstractGrpcChannelPrivate::get(channel.get())->interceptorEngine;
        if (engine.hasHandlerFor(QtGrpc::InterceptorCapability::TrailingMetadata))
            engine.onTrailingMetadata(*this, metadata);
    }
    if (d->serverTrailingMetadata == metadata)
        return;
    d->serverTrailingMetadata = std::move(metadata);
}

/*!
    Returns the meta type of the RPC result message.
 */
QMetaType QGrpcOperationContext::responseMetaType() const
{
    Q_D(const QGrpcOperationContext);
    return d->responseMetaType;
}

/*!
    Stores the \a metaType of the RPC result message.
*/
void QGrpcOperationContext::setResponseMetaType(QMetaType metaType)
{
    Q_D(QGrpcOperationContext);
    d->responseMetaType = metaType;
}

/*!
    \since 6.11

    Returns the unique identifier for this operation context.

    Each ID is unique across all channels for the application lifetime.
*/
quint64 QGrpcOperationContext::operationId() const noexcept
{
    Q_D(const QGrpcOperationContext);
    return d->operationId;
}

const QGrpcOperation &QGrpcOperationContext::operation() const &
{
    // Guaranteed by construction & parent<>child relationship
    const auto *p = parent();
    Q_ASSERT(p);
    return static_cast<const QGrpcOperation &>(*p);
}

// For future extensions
bool QGrpcOperationContext::event(QEvent *event)
{
    return QObject::event(event);
}

QT_END_NAMESPACE

#include "moc_qgrpcoperationcontext.cpp"
