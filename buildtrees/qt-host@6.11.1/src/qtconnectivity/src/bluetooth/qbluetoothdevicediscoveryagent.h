// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHDEVICEDISCOVERYAGENT_H
#define QBLUETOOTHDEVICEDISCOVERYAGENT_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothAddress>

QT_BEGIN_NAMESPACE

class QBluetoothDeviceDiscoveryAgentPrivate;

class Q_BLUETOOTH_EXPORT QBluetoothDeviceDiscoveryAgent : public QObject
{
    Q_OBJECT

public:
    // FIXME: add more errors
    // FIXME: add bluez error handling
    enum Error {
        NoError,
        InputOutputError,
        PoweredOffError,
        InvalidBluetoothAdapterError,
        UnsupportedPlatformError,
        UnsupportedDiscoveryMethod,
        LocationServiceTurnedOffError,
        MissingPermissionsError,
        UnknownError = 100 // New errors must be added before Unknown error
    };
    Q_ENUM(Error)

    enum DiscoveryMethod
    {
        NoMethod = 0x0,
        ClassicMethod = 0x01,
        LowEnergyMethod = 0x02,
    };
    Q_DECLARE_FLAGS(DiscoveryMethods, DiscoveryMethod)
    Q_FLAG(DiscoveryMethods)

    explicit QBluetoothDeviceDiscoveryAgent(QObject *parent = nullptr);
    explicit QBluetoothDeviceDiscoveryAgent(const QBluetoothAddress &deviceAdapter,
                                            QObject *parent = nullptr);
    ~QBluetoothDeviceDiscoveryAgent();

    bool isActive() const;

    Error error() const;
    QString errorString() const;

    QList<QBluetoothDeviceInfo> discoveredDevices() const;

    void setLowEnergyDiscoveryTimeout(int msTimeout);
    int lowEnergyDiscoveryTimeout() const;

    static DiscoveryMethods supportedDiscoveryMethods();
public Q_SLOTS:
    void start();
    void start(DiscoveryMethods method);
    void stop();

Q_SIGNALS:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceUpdated(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields);
    void finished();
    void errorOccurred(QBluetoothDeviceDiscoveryAgent::Error error);
    void canceled();

private:
    Q_DECLARE_PRIVATE(QBluetoothDeviceDiscoveryAgent)
    QBluetoothDeviceDiscoveryAgentPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods)

QT_END_NAMESPACE

#endif
