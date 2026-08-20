// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSCLIENT_H
#define QMODBUSCLIENT_H

#include <QtCore/qobject.h>
#include <QtSerialBus/qmodbusdataunit.h>
#include <QtSerialBus/qmodbusdevice.h>
#include <QtSerialBus/qmodbuspdu.h>
#include <QtSerialBus/qmodbusreply.h>

QT_BEGIN_NAMESPACE

class QModbusClientPrivate;

class Q_SERIALBUS_EXPORT QModbusClient : public QModbusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusClient)

public:
    explicit QModbusClient(QObject *parent = nullptr);
    ~QModbusClient();

    QModbusReply *sendReadRequest(const QModbusDataUnit &read, int serverAddress);
    QModbusReply *sendWriteRequest(const QModbusDataUnit &write, int serverAddress);
    QModbusReply *sendReadWriteRequest(const QModbusDataUnit &read, const QModbusDataUnit &write,
                                       int serverAddress);
    QModbusReply *sendRawRequest(const QModbusRequest &request, int serverAddress);

    int timeout() const;
    void setTimeout(int newTimeout);

    int numberOfRetries() const;
    void setNumberOfRetries(int number);

Q_SIGNALS:
    void timeoutChanged(int newTimeout);

protected:
    QModbusClient(QModbusClientPrivate &dd, QObject *parent = nullptr);

    virtual bool processResponse(const QModbusResponse &response, QModbusDataUnit *data);
    virtual bool processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data);
};

QT_END_NAMESPACE

#endif // QMODBUSCLIENT_H
