// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSTCPSERVER_H
#define QMODBUSTCPSERVER_H

#include <QtSerialBus/qmodbuspdu.h>
#include <QtSerialBus/qmodbusserver.h>

QT_BEGIN_NAMESPACE

class QModbusTcpServerPrivate;
class QTcpSocket;

class Q_SERIALBUS_EXPORT QModbusTcpConnectionObserver
{
public:
    virtual ~QModbusTcpConnectionObserver();

    virtual bool acceptNewConnection(QTcpSocket *newClient) = 0;
};

class Q_SERIALBUS_EXPORT QModbusTcpServer : public QModbusServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusTcpServer)

public:
    explicit QModbusTcpServer(QObject *parent = nullptr);
    ~QModbusTcpServer();

    void installConnectionObserver(QModbusTcpConnectionObserver *observer);

Q_SIGNALS:
    void modbusClientDisconnected(QTcpSocket *modbusClient);

protected:
    QModbusTcpServer(QModbusTcpServerPrivate &dd, QObject *parent = nullptr);

    bool open() override;
    void close() override;

    QModbusResponse processRequest(const QModbusPdu &request) override;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPSERVER_H
