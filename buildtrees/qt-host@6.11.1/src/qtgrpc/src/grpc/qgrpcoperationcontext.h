// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCOPERATIONCONTEXT_H
#define QGRPCOPERATIONCONTEXT_H

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtProtobuf/qabstractprotobufserializer.h>

#include <QtCore/qhash.h>
#include <QtCore/qobject.h>
#include <QtCore/qstringfwd.h>
#include <QtCore/qtdeprecationdefinitions.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QGrpcCallOptions;
class QGrpcOperationContextPrivate;
class QGrpcStatus;
class QGrpcOperation;

class Q_GRPC_EXPORT QGrpcOperationContext final : public QObject
{
    Q_OBJECT
    QT_DEFINE_TAG_STRUCT(PrivateConstructor);

public:
    explicit QGrpcOperationContext(QtGrpc::RpcDescriptor description,
                                   const QGrpcCallOptions &options,
                                   std::shared_ptr<QAbstractProtobufSerializer> serializer,
                                   QGrpcOperation *parent, PrivateConstructor);
    ~QGrpcOperationContext() override;

    [[nodiscard]] QLatin1StringView method() const noexcept;
    [[nodiscard]] QLatin1StringView service() const noexcept;
    [[nodiscard]] QtGrpc::RpcDescriptor descriptor() const noexcept;

#if 0 // Use new QAbstractGrpcChannel virtual RPC methods
    [[nodiscard]] QByteArrayView argument() const noexcept;
#endif

    void callOptions() const && = delete;
    [[nodiscard]] const QGrpcCallOptions &callOptions() const & noexcept;

#if QT_DEPRECATED_SINCE(6, 13)
    void serverMetadata() const && = delete;
    QT_DEPRECATED_VERSION_X_6_13("Use serverInitialMetadata()")
    [[nodiscard]] const QHash<QByteArray, QByteArray> &serverMetadata() const & noexcept;
    QT_DEPRECATED_VERSION_X_6_13("Use setServerInitialMetadata(QMultiHash&&)")
    void setServerMetadata(const QHash<QByteArray, QByteArray> &metadata);
    QT_DEPRECATED_VERSION_X_6_13("Use setServerInitialMetadata(QMultiHash&&)")
    void setServerMetadata(QHash<QByteArray, QByteArray> &&metadata);
#endif // QT_DEPRECATED_SINCE(6, 13)

    void serverInitialMetadata() const && = delete;
    [[nodiscard]] const QMultiHash<QByteArray, QByteArray> &
    serverInitialMetadata() const & noexcept;
    void setServerInitialMetadata(QMultiHash<QByteArray, QByteArray> &&metadata);

    void serverTrailingMetadata() const && = delete;
    [[nodiscard]] const QMultiHash<QByteArray, QByteArray> &
    serverTrailingMetadata() const & noexcept;
    void setServerTrailingMetadata(QMultiHash<QByteArray, QByteArray> &&metadata);

    [[nodiscard]] QMetaType responseMetaType() const;
    void setResponseMetaType(QMetaType metaType);

    [[nodiscard]] std::shared_ptr<const QAbstractProtobufSerializer> serializer() const;

    [[nodiscard]] quint64 operationId() const noexcept;

Q_SIGNALS:
    // Outgoing signals of the channel.
    void finished(const QGrpcStatus &status);
    void messageReceived(const QByteArray &data);
    // Icoming signals from the client.
    void cancelRequested();
    void writeMessageRequested(const QByteArray &data);
    void writesDoneRequested();
    // Internal signals of this class.
    void serverInitialMetadataReceived(const QMultiHash<QByteArray, QByteArray> &metadata,
                                       QPrivateSignal);

private:
    Q_DISABLE_COPY_MOVE(QGrpcOperationContext)
    Q_DECLARE_PRIVATE(QGrpcOperationContext)

    void operation() const && = delete;
    [[nodiscard]] const QGrpcOperation &operation() const &;
    [[nodiscard]] QGrpcOperation &operation() &
    {
        return const_cast<QGrpcOperation &>(std::as_const(*this).operation());
    }

    friend class QGrpcOperation;
    friend class QGrpcOperationPrivate;

public:
    bool event(QEvent *event) override;
};

QT_END_NAMESPACE

#endif // QGRPCOPERATIONCONTEXT_H
