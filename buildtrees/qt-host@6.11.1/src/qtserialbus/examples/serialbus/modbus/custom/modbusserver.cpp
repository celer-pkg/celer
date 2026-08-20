// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "modbusserver.h"
#include "modbusclient.h"

// The request must contain the start address (quint16) and the number of
// registers to read (quint16).
static constexpr int ReadRequestSize {4};

// The request must contain the start address (quint16), the number of
// registers to write (quint16), the byte count (quint8) and at least one
// register value (quint16).
static constexpr int WriteHeaderSize {5};
static constexpr int MinimumWriteRequestSize {WriteHeaderSize + 2}; // 2 payload bytes

ModbusServer::ModbusServer(QObject *parent)
    : QModbusTcpServer(parent)
{
    setMap({{QModbusDataUnit::HoldingRegisters, {QModbusDataUnit::HoldingRegisters, 0, 10}}});

    QModbusRequest::registerDataSizeCalculator(ModbusClient::CustomRead, [](const QModbusRequest &request) {
        if (!request.isValid())
            return -1;
        return (request.dataSize() != ReadRequestSize ? -1 : ReadRequestSize);
    });

    QModbusRequest::registerDataSizeCalculator(ModbusClient::CustomWrite, [](const QModbusRequest &request) {
        if (!request.isValid())
            return -1;

        if (request.dataSize() < MinimumWriteRequestSize)
            return -1;
        return WriteHeaderSize + quint8(request.data().at(WriteHeaderSize - 1));
    });
}

QModbusResponse ModbusServer::processPrivateRequest(const QModbusPdu &request)
{
    if (!request.isValid())
        return QModbusServer::processPrivateRequest(request);

//! [handle_custom_read]
    if (ModbusClient::CustomRead == request.functionCode()) {
        quint16 startAddress, count;
        request.decodeData(&startAddress, &count);

        QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, startAddress, count);
        if (!data(&unit)) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::IllegalDataAddress);
        }
        return QModbusResponse(request.functionCode(), startAddress, quint8(count * 2), unit.values());
    }
//! [handle_custom_read]

//! [handle_custom_write]
    if (ModbusClient::CustomWrite == request.functionCode()) {
        quint8 byteCount;
        quint16 startAddress, numberOfRegisters;
        request.decodeData(&startAddress, &numberOfRegisters, &byteCount);

        if (byteCount % 2 != 0) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::IllegalDataValue);
        }

        const QByteArray pduData = request.data().remove(0, WriteHeaderSize);
        QDataStream stream(pduData);

        QList<quint16> values;
        for (int i = 0; i < numberOfRegisters; i++) {
            quint16 tmp;
            stream >> tmp;
            values.append(tmp);
        }

        if (!writeData({QModbusDataUnit::HoldingRegisters, startAddress, values})) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::ServerDeviceFailure);
        }

        return QModbusResponse(request.functionCode(), startAddress, numberOfRegisters);
    }
//! [handle_custom_write]

    return QModbusServer::processPrivateRequest(request);
}
