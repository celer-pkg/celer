// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSTCPCLIENT_H
#define QMODBUSTCPCLIENT_H

#include <QtSerialBus/qmodbusclient.h>

QT_BEGIN_NAMESPACE

class QModbusTcpClientPrivate;

class Q_SERIALBUS_EXPORT QModbusTcpClient : public QModbusClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusTcpClient)

public:
    explicit QModbusTcpClient(QObject *parent = nullptr);
    ~QModbusTcpClient();

protected:
    QModbusTcpClient(QModbusTcpClientPrivate &dd, QObject *parent = nullptr);

    bool open() override;
    void close() override;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPCLIENT_H
