// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QModbusTcpClient>

#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

//! [custom_client]
class ModbusClient : public QModbusTcpClient
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ModbusClient)

public:
    ModbusClient(QObject *parent = nullptr);

    static constexpr QModbusPdu::FunctionCode CustomRead {QModbusPdu::FunctionCode(0x41)};
    static constexpr QModbusPdu::FunctionCode CustomWrite {QModbusPdu::FunctionCode(0x42)};

private:
    bool processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data) override;
};
//! [custom_client]

#endif // MODBUSCLIENT_H
