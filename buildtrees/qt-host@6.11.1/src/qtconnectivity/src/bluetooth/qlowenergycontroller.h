// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYCONTROLLER_H
#define QLOWENERGYCONTROLLER_H

#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyConnectionParameters>
#include <QtBluetooth/QLowEnergyService>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingParameters;
class QLowEnergyControllerPrivate;
class QLowEnergyServiceData;

class Q_BLUETOOTH_EXPORT QLowEnergyController : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        UnknownError,
        UnknownRemoteDeviceError,
        NetworkError,
        InvalidBluetoothAdapterError,
        ConnectionError,
        AdvertisingError,
        RemoteHostClosedError,
        AuthorizationError,
        MissingPermissionsError,
        RssiReadError
    };
    Q_ENUM(Error)

    enum ControllerState {
        UnconnectedState = 0,
        ConnectingState,
        ConnectedState,
        DiscoveringState,
        DiscoveredState,
        ClosingState,
        AdvertisingState,
    };
    Q_ENUM(ControllerState)

    enum RemoteAddressType {
        PublicAddress = 0,
        RandomAddress
    };
    Q_ENUM(RemoteAddressType)

    enum Role { CentralRole, PeripheralRole };
    Q_ENUM(Role)

    static QLowEnergyController *createCentral(const QBluetoothDeviceInfo &remoteDevice,
                                               QObject *parent = nullptr);
    static QLowEnergyController *createCentral(const QBluetoothDeviceInfo &remoteDevice,
                                               const QBluetoothAddress &localDevice,
                                               QObject *parent = nullptr);
    static QLowEnergyController *createPeripheral(const QBluetoothAddress &localDevice,
                                                  QObject *parent = nullptr);
    static QLowEnergyController *createPeripheral(QObject *parent = nullptr);

    // TODO: Allow to set connection timeout (disconnect when no data has been exchanged for n seconds).

    ~QLowEnergyController();

    QBluetoothAddress localAddress() const;
    QBluetoothAddress remoteAddress() const;
    QBluetoothUuid remoteDeviceUuid() const;

    QString remoteName() const;

    ControllerState state() const;

    // TODO Qt6 remove this property. It is not longer needed when using Bluez DBus backend
    RemoteAddressType remoteAddressType() const;
    void setRemoteAddressType(RemoteAddressType type);

    void connectToDevice();
    void disconnectFromDevice();

    void discoverServices();
    QList<QBluetoothUuid> services() const;
    QLowEnergyService *createServiceObject(const QBluetoothUuid &service, QObject *parent = nullptr);

    void startAdvertising(const QLowEnergyAdvertisingParameters &parameters,
                          const QLowEnergyAdvertisingData &advertisingData,
                          const QLowEnergyAdvertisingData &scanResponseData = QLowEnergyAdvertisingData());
    void stopAdvertising();

    QLowEnergyService *addService(const QLowEnergyServiceData &service, QObject *parent = nullptr);

    void requestConnectionUpdate(const QLowEnergyConnectionParameters &parameters);

    Error error() const;
    QString errorString() const;

    Role role() const;

    int mtu() const;
    void readRssi();

Q_SIGNALS:
    void connected();
    void disconnected();
    void stateChanged(QLowEnergyController::ControllerState state);
    void errorOccurred(QLowEnergyController::Error newError);
    void mtuChanged(int mtu);
    void rssiRead(qint16 rssi);

    void serviceDiscovered(const QBluetoothUuid &newService);
    void discoveryFinished();
    void connectionUpdated(const QLowEnergyConnectionParameters &parameters);


private:
    // peripheral role ctor
    explicit QLowEnergyController(const QBluetoothAddress &localDevice, QObject *parent = nullptr);

    // central role ctors
    explicit QLowEnergyController(const QBluetoothDeviceInfo &remoteDevice,
                                  const QBluetoothAddress &localDevice,
                                  QObject *parent = nullptr);


    Q_DECLARE_PRIVATE(QLowEnergyController)
    QLowEnergyControllerPrivate *d_ptr;
};

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyController::Error, QLowEnergyController__Error,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyController::ControllerState,
                               QLowEnergyController__ControllerState,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyController::RemoteAddressType,
                               QLowEnergyController__RemoteAddressType,
                               Q_BLUETOOTH_EXPORT)
QT_DECL_METATYPE_EXTERN_TAGGED(QLowEnergyController::Role, QLowEnergyController__Role,
                               Q_BLUETOOTH_EXPORT)

#endif // QLOWENERGYCONTROLLER_H
