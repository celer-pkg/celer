// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpcQuick/private/qqmlabstractgrpcchannel_p.h>
#include <QtGrpcQuick/qqmlabstractgrpcchannel.h>

QT_BEGIN_NAMESPACE

QQmlAbstractGrpcChannel::QQmlAbstractGrpcChannel(QQmlAbstractGrpcChannelPrivate &dd,
                                                 QObject *parent)
    : QObject(dd, parent)
{
}

QQmlAbstractGrpcChannel::~QQmlAbstractGrpcChannel()
    = default;


QT_END_NAMESPACE

#include "moc_qqmlabstractgrpcchannel.cpp"
