// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bleiodevice.h"

#include <QtBluetooth/QLowEnergyController>

using namespace Qt::Literals::StringLiterals;

// The BT LE service consists of two characteristics which are used to establish
// a two-way ommunication channel. The Service which contains the characteristics:
const QBluetoothUuid BLEIoDevice::SERVICE_UUID =
        QBluetoothUuid{"11223344-ac16-11eb-ae5c-93d3a763feed"_L1};
// Characteristic for data Client => Server
const QBluetoothUuid BLEIoDevice::CLIENT_TX_SERVER_RX_CHAR_UUID =
      QBluetoothUuid{"889930f0-b9cc-4c27-8c1b-ebc2bcae5c95"_L1};
// Characteristic for data Server => Client
const QBluetoothUuid BLEIoDevice::CLIENT_RX_SERVER_TX_CHAR_UUID =
      QBluetoothUuid{"667730f0-b9cc-4c27-8c1b-ebc2bcae5c95"_L1};

const QByteArray BLEIoDevice::CLIENT_READY_SIGNAL = "clientready"_ba;

BLEIoDevice::BLEIoDevice(QLowEnergyService *service, const QBluetoothUuid &rx,
                         const QBluetoothUuid &tx, QObject *parent)
    : QIODevice(parent)
    , m_service(service)
    , m_txCharacteristic(service->characteristic(tx))
    , m_rxCharacteristicUuid(rx)
{
    Q_ASSERT(service);

    open(QIODevice::ReadWrite);

    QObject::connect(service, &QLowEnergyService::characteristicChanged, this, [this](
                     const QLowEnergyCharacteristic &info, const QByteArray &value) {
        // If the characteristic where we are the receiving end has new value (ie. the remote end
        // has written), store the data and indicate that we have it
        if (info.uuid() != m_rxCharacteristicUuid)
            return;
        m_rxBuffer.append(value);
        emit readyRead();
    });

    QObject::connect(service, &QLowEnergyService::errorOccurred, this,
                     [this](QLowEnergyService::ServiceError error) {
        qWarning() << "A BT LE Service error occurred:" << error;
        emit disconnected();
    });
}

qint64 BLEIoDevice::bytesAvailable() const
{
    return QIODevice::bytesAvailable() + m_rxBuffer.size();
}

bool BLEIoDevice::isSequential() const
{
    return true;
}

void BLEIoDevice::close()
{
    emit closing();
}

qint64 BLEIoDevice::readData(char *data, qint64 maxlen)
{
    auto sz = (std::min)(qsizetype(maxlen), m_rxBuffer.size());
    if (sz <= 0)
        return sz;
    memcpy(data, m_rxBuffer.constData(), size_t(sz));
    m_rxBuffer.remove(0, sz);
    return sz;
}

qint64 BLEIoDevice::writeData(const char *data, qint64 len)
{
    // Write data by writing to the characteristic where we are the transmitting end.
    // As there may be potentially a lot of data, we need to decide in how big chunks
    // we send. Maximum is 512 - 3 = 509 bytes but it depends on the bluetooth stacks.
    // We take a very conservative choice which should work with all bluetooth stacks, as this
    // doesn't require support for long / prepared writes (ie. writes larger than MTU,
    // whose default is 23) and also there is no risk of exceeding the notification mechanism
    // payload limit (MTU - 3).
    static const int MAX_PAYLOAD = 20;
    if (m_service) {
        do {
            auto sz = (std::min)(qint64(MAX_PAYLOAD) , len);
            m_service->writeCharacteristic(m_txCharacteristic, QByteArray{data, qsizetype(sz)});
            len -= sz;
            data += sz;
            // Here we consider bytes now written to the underlying 'channel'. BT LE below
            // does any necessary queueing. In principle we could also wait for
            // characteristicWritten() signal but it would only be available on BT LE
            // Client/Central side, whereas this class is used on both client/server side.
            emit bytesWritten(sz);
        } while (len > 0);
        return len;
    }
    return -1;
}
