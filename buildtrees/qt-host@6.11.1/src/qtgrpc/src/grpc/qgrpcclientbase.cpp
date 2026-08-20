// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/private/qabstractgrpcchannel_p.h>
#include <QtGrpc/private/qgrpcoperation_p.h>
#include <QtGrpc/private/qtgrpclogging_p.h>
#include <QtGrpc/qgrpcclientbase.h>

#include <QtCore/private/qminimalflatset_p.h>
#include <QtCore/private/qobject_p.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlatin1stringview.h>

QT_BEGIN_NAMESPACE

/*!
    \class QGrpcClientBase
    \inmodule QtGrpc
    \brief The QGrpcClientBase class serves as base for generated client
    interfaces.

    The QGrpcClientBase class provides a common set of functionalities for the
    generated client interface of the \gRPC service definition.

    The RPC methods of this class should not be called directly.

    \note Thread safety is enforced for the non-const member functions. These
    functions must be called from the same \l{QObject::} {thread} in which the
    object was created.
*/

/*!
    \fn void QGrpcClientBase::channelChanged()
    \since 6.7

    Indicates that a new channel got attached to the client.
*/

/*!
    \property QGrpcClientBase::channel
    \since 6.7

    This property holds the channel attached to this client. The channel is used
    as the transport layer for gRPC operations.

    \sa attachChannel()
*/

class QGrpcClientBasePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGrpcClientBase)
public:
    explicit QGrpcClientBasePrivate(QLatin1StringView service) : service(service) { }

    void addStream(QGrpcOperation *stream);

    template <typename T>
    [[nodiscard]] std::unique_ptr<T> initOperation(QLatin1StringView method,
                                                   const QProtobufMessage &arg,
                                                   const QGrpcCallOptions &options)
    {
        typename T::PrivateConstructor tag;
        auto operation = std::make_unique<T>(service, method, options, channel, tag);
        auto *operation_p = QGrpcOperationPrivate::get(operation.get());

        if (operation_p->state != QGrpcOperationPrivate::State::Valid)
            return operation;

        auto argData = operation_p->serializeInitialMessage(arg);
        if (!argData)
            return operation;
        Q_ASSERT(operation_p->state == QGrpcOperationPrivate::State::Valid);

        using ChannelFn = void (QAbstractGrpcChannel::*)(QGrpcOperationContext *, QByteArray &&);
        constexpr ChannelFn startRpcFunction = [&]() -> ChannelFn {
            if constexpr (std::is_same_v<T, QGrpcServerStream>) {
                return &QAbstractGrpcChannel::serverStream;
            } else if constexpr (std::is_same_v<T, QGrpcClientStream>) {
                return &QAbstractGrpcChannel::clientStream;
            } else if constexpr (std::is_same_v<T, QGrpcBidiStream>) {
                return &QAbstractGrpcChannel::bidiStream;
            } else if constexpr (std::is_same_v<T, QGrpcCallReply>) {
                return &QAbstractGrpcChannel::call;
            } else {
                Q_UNREACHABLE_RETURN(nullptr);
            }
        }();

        ((*channel).*(startRpcFunction))(operation_p->operationContext, std::move(*argData));
        addStream(operation.get());

        return operation;
    }

    std::shared_ptr<QAbstractGrpcChannel> channel;
    const QLatin1StringView service;
    QMinimalFlatSet<QGrpcOperation *> activeStreams;
};

void QGrpcClientBasePrivate::addStream(QGrpcOperation *grpcStream)
{
    Q_ASSERT(grpcStream);

    Q_Q(QGrpcClientBase);
    // Remove the operation pointer upon QObject destruction if it hasn't
    // already been gracefully removed by receiving finished()
    QObject::connect(grpcStream, &QObject::destroyed, q, [this, grpcStream](QObject *obj) {
        Q_ASSERT(obj == grpcStream);
        activeStreams.remove(grpcStream);
    });

    QObject::connect(
        grpcStream, &QGrpcOperation::finished, q,
        [this, grpcStream] {
            Q_ASSERT(activeStreams.contains(grpcStream));
            activeStreams.remove(grpcStream);
        },
        Qt::SingleShotConnection);
    const auto it = activeStreams.insert(grpcStream);
    Q_ASSERT(it.second);
}

/*!
    \internal
    Constructs a QGrpcClientBase using \a service name from the protobuf schema
    and sets \a parent as the owner.
*/
QGrpcClientBase::QGrpcClientBase(QLatin1StringView service, QObject *parent)
    : QObject(*new QGrpcClientBasePrivate(service), parent)
{
}

/*!
    Destroys the QGrpcClientBase.
*/
QGrpcClientBase::~QGrpcClientBase() = default;

/*!
    Attaches \a channel to the client as transport layer for \gRPC operations.
    Returns \c true if the channel successfully attached; otherwise, returns \c
    false.

    Request and response messages will be serialized in a format that the
    channel supports.

    \note \b Warning: Qt GRPC doesn't guarantee thread safety on the channel level.
    You have to invoke the channel-related functions on the same thread as
    QGrpcClientBase.
*/
bool QGrpcClientBase::attachChannel(std::shared_ptr<QAbstractGrpcChannel> channel)
{
    Q_D(QGrpcClientBase);

    if (channel == d->channel) {
        qGrpcWarning("Refusing to attach channel. The same channel is already assigned.");
        return false;
    }

    // channel is not a QObject so we compare against the threadId set on construction.
    if (channel->d_func()->threadId != QThread::currentThreadId()) {
        qGrpcWarning("QtGrpc doesn't allow attaching the channel from a different thread");
        return false;
    }

    for (const auto &stream : d->activeStreams) {
        assert(stream != nullptr);
        stream->cancel();
    }
    Q_ASSERT(d->activeStreams.isEmpty());

    d->channel = std::move(channel);
    emit channelChanged();
    return true;
}

/*!
    \since 6.7
    Returns the channel attached to this client.
*/
std::shared_ptr<QAbstractGrpcChannel> QGrpcClientBase::channel() const
{
    Q_D(const QGrpcClientBase);
    return d->channel;
}

/*!
    \internal
//! [rpc-init-desc]
    Initializes the RPC with \a method name and initial argument \a arg by
    calling the corresponding QAbstractGrpcChannel method. The RPC is
    customized through the provided \a options.
//! [rpc-init-desc]
*/
std::unique_ptr<QGrpcCallReply> QGrpcClientBase::call(QLatin1StringView method,
                                                      const QProtobufMessage &arg,
                                                      const QGrpcCallOptions &options)
{
    Q_D(QGrpcClientBase);
    return d->initOperation<QGrpcCallReply>(method, arg, options);
}

/*!
    \internal
    \include qgrpcclientbase.cpp rpc-init-desc
*/
std::unique_ptr<QGrpcServerStream> QGrpcClientBase::serverStream(QLatin1StringView method,
                                                                 const QProtobufMessage &arg,
                                                                 const QGrpcCallOptions &options)
{
    Q_D(QGrpcClientBase);
    return d->initOperation<QGrpcServerStream>(method, arg, options);
}

/*!
    \internal
    \include qgrpcclientbase.cpp rpc-init-desc
*/
std::unique_ptr<QGrpcClientStream> QGrpcClientBase::clientStream(QLatin1StringView method,
                                                                 const QProtobufMessage &arg,
                                                                 const QGrpcCallOptions &options)
{
    Q_D(QGrpcClientBase);
    return d->initOperation<QGrpcClientStream>(method, arg, options);
}

/*!
    \internal
    \include qgrpcclientbase.cpp rpc-init-desc
*/
std::unique_ptr<QGrpcBidiStream> QGrpcClientBase::bidiStream(QLatin1StringView method,
                                                             const QProtobufMessage &arg,
                                                             const QGrpcCallOptions &options)
{
    Q_D(QGrpcClientBase);
    return d->initOperation<QGrpcBidiStream>(method, arg, options);
}

void QGrpcClientBase::setOperationResponseMetaType(QGrpcOperation *operation,
                                                       QMetaType responseMetaType)
{
    Q_ASSERT(operation);
    QGrpcOperationPrivate::get(operation)->operationContext->setResponseMetaType(responseMetaType);
}

bool QGrpcClientBase::event(QEvent *event)
{
    return QObject::event(event);
}

QT_END_NAMESPACE

#include "moc_qgrpcclientbase.cpp"
