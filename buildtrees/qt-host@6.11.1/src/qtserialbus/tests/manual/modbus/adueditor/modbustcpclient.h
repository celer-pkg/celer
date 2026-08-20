// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include <QModbusTcpClient>

QT_USE_NAMESPACE

class ModbusTcpClientPrivate;
class ModbusTcpClient : public QModbusTcpClient
{
    Q_OBJECT
    Q_DISABLE_COPY(ModbusTcpClient)
    Q_DECLARE_PRIVATE(ModbusTcpClient)

public:
    ModbusTcpClient(QObject *parent = nullptr);

public slots:
    void valueChanged(int value);

private:
    ModbusTcpClient(ModbusTcpClientPrivate &dd, QObject *parent = nullptr);
};

#endif // MODBUSTCPCLIENT_H
