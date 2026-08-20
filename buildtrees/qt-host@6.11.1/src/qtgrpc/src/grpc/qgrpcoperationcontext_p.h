// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGRPCOPERATIONCONTEXT_P_H
#define QGRPCOPERATIONCONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGrpc/qabstractgrpcchannel.h>
#include <QtGrpc/qgrpccalloptions.h>
#include <QtGrpc/qgrpcoperationcontext.h>

#include <QtCore/private/qobject_p.h>
#include <QtCore/qhash.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qtconfigmacros.h>

QT_BEGIN_NAMESPACE

class QGrpcOperationContextPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGrpcOperationContext)
public:
    QGrpcOperationContextPrivate(QtGrpc::RpcDescriptor &&descriptor_, QGrpcCallOptions options_,
                                 std::shared_ptr<QAbstractProtobufSerializer> &&serializer_,
                                 quint64 operationId_)
        : descriptor(std::move(descriptor_)), options(std::move(options_)),
          serializer(std::move(serializer_)), operationId(operationId_)
    {
    }

    QtGrpc::RpcDescriptor descriptor;
    QGrpcCallOptions options;
    std::shared_ptr<QAbstractProtobufSerializer> serializer;
    QMetaType responseMetaType;
    QMultiHash<QByteArray, QByteArray> serverInitialMetadata;
#if QT_DEPRECATED_SINCE(6, 13)
    QHash<QByteArray, QByteArray> deprServerInitialMetadata;
#endif
    QMultiHash<QByteArray, QByteArray> serverTrailingMetadata;
    const quint64 operationId;
};

QT_END_NAMESPACE

#endif // QGRPCOPERATIONCONTEXT_P_H
