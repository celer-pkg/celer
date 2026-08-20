// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCOPERATION_H
#define QGRPCOPERATION_H

#include <QtGrpc/qgrpcstatus.h>
#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qhash.h>
#include <QtCore/qobject.h>
#include <QtCore/qstringfwd.h>
#include <QtCore/qtdeprecationdefinitions.h>

#include <optional>

QT_BEGIN_NAMESPACE

class QGrpcOperationContext;
class QGrpcOperationPrivate;
class QAbstractGrpcChannel;
class QGrpcCallOptions;

class Q_GRPC_EXPORT QGrpcOperation : public QObject
{
    Q_OBJECT
public:
    ~QGrpcOperation() override;

    template <typename T, QtProtobuf::if_protobuf_message<T> = true>
    std::optional<T> read() const
    {
        std::optional<T> r(std::in_place);
        if (!read(&*r))
            r.reset();
        return r;
    }
    bool read(QProtobufMessage *message) const;

#if QT_DEPRECATED_SINCE(6, 13)
    QT_DEPRECATED_VERSION_X_6_13("Use serverInitialMetadata()")
    [[nodiscard]] const QHash<QByteArray, QByteArray> &metadata() const & noexcept;
    void metadata() const && = delete;
#endif

    [[nodiscard]] const QMultiHash<QByteArray, QByteArray> &
    serverInitialMetadata() const & noexcept;
    void serverInitialMetadata() const && = delete;
    [[nodiscard]] const QMultiHash<QByteArray, QByteArray> &
    serverTrailingMetadata() const & noexcept;
    void serverTrailingMetadata() const && = delete;

    [[nodiscard]] QLatin1StringView method() const noexcept;

    [[nodiscard]] bool isFinished() const noexcept;

    [[nodiscard]] QMetaType responseMetaType() const;

Q_SIGNALS:
    void finished(const QGrpcStatus &status);
    void serverInitialMetadataReceived(const QMultiHash<QByteArray, QByteArray> &metadata);

public Q_SLOTS:
    void cancel();

protected:
#if QT_GRPC_REMOVED_SINCE(6, 11)
    explicit QGrpcOperation(std::shared_ptr<QGrpcOperationContext> operationContext,
                            QObject *parent = nullptr);
#endif
    explicit QGrpcOperation(QtGrpc::RpcDescriptor descriptor, const QGrpcCallOptions &options,
                            std::weak_ptr<QAbstractGrpcChannel> &&channel);

    [[nodiscard]] const QGrpcOperationContext &context() const & noexcept;
    [[nodiscard]] QGrpcOperationContext &context() & noexcept
    {
        return const_cast<QGrpcOperationContext &>(std::as_const(*this).context());
    }
    void context() const && = delete;
    void writeMessage(const QProtobufMessage &message);
    void writesDone();

private:
    Q_DISABLE_COPY_MOVE(QGrpcOperation)
    Q_DECLARE_PRIVATE(QGrpcOperation)
    friend class QGrpcOperationContext;

public:
    bool event(QEvent *event) override;
};

QT_END_NAMESPACE

#endif // QGRPCOPERATION_H
