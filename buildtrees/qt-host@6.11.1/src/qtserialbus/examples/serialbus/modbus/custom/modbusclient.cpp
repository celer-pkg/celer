// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "modbusclient.h"

// The read response must contain the start address (quint16), the byte count
// (quint8) and at least one register value (quint16).
static constexpr int ReadHeaderSize {3};
static constexpr int MinimumReadResponseSize {ReadHeaderSize + 2}; // 2 payload bytes

// The response must contain the start address (quint16) and the number of
// registers written (quint16).
static constexpr int WriteResponseSize {4};

ModbusClient::ModbusClient(QObject *parent)
    : QModbusTcpClient(parent)
{
    QModbusResponse::registerDataSizeCalculator(CustomRead, [](const QModbusResponse &response) {
        if (!response.isValid())
            return -1;

        if (response.dataSize() < MinimumReadResponseSize)
            return -1;
        return ReadHeaderSize + quint8(response.data().at(ReadHeaderSize - 1));
    });

    QModbusResponse::registerDataSizeCalculator(CustomWrite, [](const QModbusResponse &response) {
        if (!response.isValid())
            return -1;
        return (response.dataSize() != WriteResponseSize ? -1 : WriteResponseSize);
    });
}

//! [process_custom_read]
static bool collateBytes(const QModbusPdu &response, QModbusDataUnit *data)
{
    if (response.dataSize() < MinimumReadResponseSize)
        return false;

    quint16 address; quint8 byteCount;
    response.decodeData(&address, &byteCount);

    if (byteCount % 2 != 0)
        return false;

    if (data) {
        QDataStream stream(response.data().remove(0, 3));

        QList<quint16> values;
        const quint8 itemCount = byteCount / 2;
        for (int i = 0; i < itemCount; i++) {
            quint16 tmp;
            stream >> tmp;
            values.append(tmp);
        }
        *data = {QModbusDataUnit::HoldingRegisters, address, values};
    }
    return true;
}
//! [process_custom_read]

//! [process_custom_write]
static bool collateMultipleValues(const QModbusPdu &response, QModbusDataUnit *data)
{
    if (response.dataSize() != WriteResponseSize)
        return false;

    quint16 address, count;
    response.decodeData(&address, &count);

    if (count < 1 || count > 10)
        return false;

    if (data)
        *data = {QModbusDataUnit::HoldingRegisters, address, count};
    return true;
}
//! [process_custom_write]

//! [private_response]
bool ModbusClient::processPrivateResponse(const QModbusResponse &response, QModbusDataUnit *data)
{
    if (!response.isValid())
        return QModbusClient::processPrivateResponse(response, data);

    if (CustomRead == response.functionCode())
        return collateBytes(response, data);

    if (CustomWrite == response.functionCode())
        return collateMultipleValues(response, data);
    return QModbusClient::processPrivateResponse(response, data);
}
//! [private_response]
