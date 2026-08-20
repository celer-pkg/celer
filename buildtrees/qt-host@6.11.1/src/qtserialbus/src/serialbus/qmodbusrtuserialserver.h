// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSRTUSERIALSERVER_H
#define QMODBUSRTUSERIALSERVER_H

// The pragma is required to generate proper headers for source compatibility.
#if 0
#pragma qt_deprecates(QModbusRtuSerialSlave, 7.0)
#endif

#include <QtSerialBus/qmodbusserver.h>

QT_BEGIN_NAMESPACE

class QModbusRtuSerialServerPrivate;

class Q_SERIALBUS_EXPORT QModbusRtuSerialServer : public QModbusServer
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusRtuSerialServer)

public:
    explicit QModbusRtuSerialServer(QObject *parent = nullptr);
    ~QModbusRtuSerialServer();

    bool processesBroadcast() const override;

    int interFrameDelay() const;
    void setInterFrameDelay(int microseconds);

protected:
    QModbusRtuSerialServer(QModbusRtuSerialServerPrivate &dd, QObject *parent = nullptr);

    bool open() override;
    void close() override;

    QModbusResponse processRequest(const QModbusPdu &request) override;
};

#if QT_DEPRECATED_SINCE(6, 2)
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")
using QModbusRtuSerialSlave
    Q_DECL_DEPRECATED_X("Please port your application to QModbusRtuSerialServer.") = QModbusRtuSerialServer;
#endif
QT_WARNING_POP
QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALSERVER_H
