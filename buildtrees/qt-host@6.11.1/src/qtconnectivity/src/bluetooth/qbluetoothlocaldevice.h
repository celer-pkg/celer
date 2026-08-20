// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHLOCALDEVICE_H
#define QBLUETOOTHLOCALDEVICE_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

#include <QtBluetooth/QBluetoothHostInfo>

QT_BEGIN_NAMESPACE

class QBluetoothLocalDevicePrivate;

class Q_BLUETOOTH_EXPORT QBluetoothLocalDevice : public QObject
{
    Q_OBJECT

public:
    enum Pairing {
        Unpaired,
        Paired,
        AuthorizedPaired
    };
    Q_ENUM(Pairing)

    enum HostMode {
        HostPoweredOff,
        HostConnectable,
        HostDiscoverable,
        HostDiscoverableLimitedInquiry
    };
    Q_ENUM(HostMode)

    enum Error {
        NoError,
        PairingError,
        MissingPermissionsError,
        UnknownError = 100
    };
    Q_ENUM(Error)

    explicit QBluetoothLocalDevice(QObject *parent = nullptr);
    explicit QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent = nullptr);
    virtual ~QBluetoothLocalDevice();

    bool isValid() const;

    void requestPairing(const QBluetoothAddress &address, Pairing pairing);
    Pairing pairingStatus(const QBluetoothAddress &address) const;

    void setHostMode(QBluetoothLocalDevice::HostMode mode);
    HostMode hostMode() const;
    QList<QBluetoothAddress> connectedDevices() const;

    void powerOn();

    QString name() const;
    QBluetoothAddress address() const;

    static QList<QBluetoothHostInfo> allDevices();

Q_SIGNALS:
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode state);
    void deviceConnected(const QBluetoothAddress &address);
    void deviceDisconnected(const QBluetoothAddress &address);
    void pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

    void errorOccurred(QBluetoothLocalDevice::Error error);

private:
    Q_DECLARE_PRIVATE(QBluetoothLocalDevice)
    QBluetoothLocalDevicePrivate *d_ptr;
};

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QBluetoothLocalDevice::Pairing, QBluetoothLocalDevice__Pairing,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QBluetoothLocalDevice::HostMode, QBluetoothLocalDevice__HostMode,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QBluetoothLocalDevice::Error, QBluetoothLocalDevice__Error,
                               Q_BLUETOOTH_EXPORT)

#endif // QBLUETOOTHLOCALDEVICE_H
