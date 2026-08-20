// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQMLABSTRACTGRPCCHANNEL_H
#define QQMLABSTRACTGRPCCHANNEL_H

#include <QtGrpcQuick/qtgrpcquickexports.h>

#include <QtGrpc/qabstractgrpcchannel.h>

#include <QtCore/qobject.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QQmlAbstractGrpcChannelPrivate;
class Q_GRPCQUICK_EXPORT QQmlAbstractGrpcChannel : public QObject
{
    Q_OBJECT
public:
    explicit QQmlAbstractGrpcChannel(QObject *parent = nullptr) : QObject(parent) { }
    ~QQmlAbstractGrpcChannel() override;

    virtual std::shared_ptr<QAbstractGrpcChannel> channel() const = 0;

protected:
    explicit QQmlAbstractGrpcChannel(QQmlAbstractGrpcChannelPrivate &dd, QObject *parent = nullptr);

private:
    Q_DECLARE_PRIVATE(QQmlAbstractGrpcChannel)
};

QT_END_NAMESPACE

#endif // QQMLABSTRACTGRPCCHANNEL_H
