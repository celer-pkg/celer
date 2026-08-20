// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCHTTP2CHANNEL_H
#define QGRPCHTTP2CHANNEL_H

#include <QtGrpc/qabstractgrpcchannel.h>

#include <QtCore/qstringfwd.h>
#include <QtCore/qtclasshelpermacros.h>
#include <QtCore/qurl.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QAbstractProtobufSerializer;
class QGrpcChannelOptions;
class QGrpcOperationContext;

class QGrpcHttp2ChannelPrivate;
class Q_GRPC_EXPORT QGrpcHttp2Channel final : public QAbstractGrpcChannel
{
public:
    explicit QGrpcHttp2Channel(const QUrl &hostUri);
    explicit QGrpcHttp2Channel(const QUrl &hostUri, const QGrpcChannelOptions &options);
    explicit QGrpcHttp2Channel(const QUrl &hostUri, QGrpcInterceptorChain interceptorChain);
    explicit QGrpcHttp2Channel(const QUrl &hostUri, const QGrpcChannelOptions &options,
                               QGrpcInterceptorChain interceptorChain);
    ~QGrpcHttp2Channel() override;

    [[nodiscard]] QUrl hostUri() const;

private:
    void call(QGrpcOperationContext *operationContext, QByteArray &&messageData) override;
    void serverStream(QGrpcOperationContext *operationContext, QByteArray &&messageData) override;
    void clientStream(QGrpcOperationContext *operationContext, QByteArray &&messageData) override;
    void bidiStream(QGrpcOperationContext *operationContext, QByteArray &&messageData) override;

    [[nodiscard]] std::shared_ptr<QAbstractProtobufSerializer> serializer() const override;

    Q_DISABLE_COPY_MOVE(QGrpcHttp2Channel)

    std::unique_ptr<QGrpcHttp2ChannelPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QGRPCHTTP2CHANNEL_H
