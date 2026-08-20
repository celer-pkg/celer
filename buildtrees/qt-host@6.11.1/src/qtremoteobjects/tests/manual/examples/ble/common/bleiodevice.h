// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyCharacteristic>
#include <QtBluetooth/QLowEnergyService>

#include <QtCore/QIODevice>
#include <QtCore/QPointer>

class BLEIoDevice : public QIODevice
{
    Q_OBJECT
public:
    BLEIoDevice(QLowEnergyService *service, const QBluetoothUuid &rx,
                const QBluetoothUuid &tx, QObject *parent = nullptr);

    static const QBluetoothUuid SERVICE_UUID;
    static const QBluetoothUuid CLIENT_TX_SERVER_RX_CHAR_UUID;
    static const QBluetoothUuid CLIENT_RX_SERVER_TX_CHAR_UUID;
    static const QByteArray CLIENT_READY_SIGNAL;

signals:
    void disconnected();
    void closing();

    // QIODevice interface
public:
    qint64 bytesAvailable() const override;
    bool isSequential() const override;
    void close() override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    QPointer<QLowEnergyService> m_service;
    QLowEnergyCharacteristic m_txCharacteristic;
    QBluetoothUuid m_rxCharacteristicUuid;
    QByteArray m_rxBuffer;
};
