// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTGRPCOPERATION_P_H
#define QTGRPCOPERATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGrpc/private/qabstractgrpcchannel_p.h>
#include <QtGrpc/qabstractgrpcchannel.h>
#include <QtGrpc/qgrpcoperation.h>
#include <QtGrpc/qgrpcoperationcontext.h>

#include <QtCore/private/qobject_p.h>
#include <QtCore/qatomic.h>
#include <QtCore/qbytearray.h>

#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

class QGrpcOperationPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGrpcOperation)
public:
    enum class State : quint8 {
        Invalid,
        Valid,
        Finished,
    };

    ~QGrpcOperationPrivate() override;

    QByteArray data;
    std::weak_ptr<QAbstractGrpcChannel> channel;
    QGrpcOperationContext *operationContext = nullptr;
    State state = State::Invalid;

    static const QGrpcOperationPrivate *get(const QGrpcOperation *op) { return op->d_func(); }
    static QGrpcOperationPrivate *get(QGrpcOperation *op) { return op->d_func(); }

    void asyncFinishInvalid(QGrpcStatus &&status);
    std::optional<QByteArray> serializeInitialMessage(const QProtobufMessage &message);
};

QT_END_NAMESPACE

#endif // QTGRPCOPERATION_P_H
