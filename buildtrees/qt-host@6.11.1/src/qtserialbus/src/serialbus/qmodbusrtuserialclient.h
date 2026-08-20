// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSRTUSERIALCLIENT_H
#define QMODBUSRTUSERIALCLIENT_H

// The pragma is required to generate proper headers for source compatibility.
#if 0
#pragma qt_deprecates(QModbusRtuSerialMaster, 7.0)
#endif

#include <QtSerialBus/qmodbusclient.h>

QT_BEGIN_NAMESPACE

class QModbusRtuSerialClientPrivate;

class Q_SERIALBUS_EXPORT QModbusRtuSerialClient : public QModbusClient
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QModbusRtuSerialClient)

public:
    explicit QModbusRtuSerialClient(QObject *parent = nullptr);
    ~QModbusRtuSerialClient();

    int interFrameDelay() const;
    void setInterFrameDelay(int microseconds);

    int turnaroundDelay() const;
    void setTurnaroundDelay(int turnaroundDelay);

protected:
    QModbusRtuSerialClient(QModbusRtuSerialClientPrivate &dd, QObject *parent = nullptr);

    void close() override;
    bool open() override;
};

#if QT_DEPRECATED_SINCE(6, 2)
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")
using QModbusRtuSerialMaster
    Q_DECL_DEPRECATED_X("Please port your application to QModbusRtuSerialClient.") = QModbusRtuSerialClient;
#endif
QT_WARNING_POP
QT_END_NAMESPACE

#endif // QMODBUSRTUSERIALCLIENT_H
