// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef BTLOCALDEVICE_H
#define BTLOCALDEVICE_H

#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothServer>
#include <QtBluetooth/QBluetoothServiceDiscoveryAgent>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyServiceData>

class BtLocalDevice : public QObject
{
    Q_OBJECT
public:
    explicit BtLocalDevice(QObject *parent = 0);
    ~BtLocalDevice();
    Q_PROPERTY(QString hostMode READ hostMode NOTIFY hostModeStateChanged)
    Q_PROPERTY(int secFlags READ secFlags WRITE setSecFlags NOTIFY secFlagsChanged)

    Q_PROPERTY(bool centralExists READ centralExists NOTIFY leChanged);
    Q_PROPERTY(bool centralSubscribed READ centralSubscribed NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralState READ centralState NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralError READ centralError NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralServiceState READ centralServiceState NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralServiceError READ centralServiceError NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralRSSI READ centralRSSI NOTIFY leChanged);

    Q_PROPERTY(QByteArray peripheralState READ peripheralState NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralError READ peripheralError NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralServiceState READ peripheralServiceState NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralServiceError READ peripheralServiceError NOTIFY leChanged);
    Q_PROPERTY(bool peripheralExists READ peripheralExists NOTIFY leChanged);

    QBluetooth::SecurityFlags secFlags() const;
    void setSecFlags(int);
    QString hostMode() const;

signals:
    void errorOccurred(QBluetoothLocalDevice::Error error);
    void hostModeStateChanged();
    void socketStateUpdate(int foobar);
    void secFlagsChanged();
    bool leChanged(); // Same signal used for LE changes for simplicity

public slots:
    //QBluetoothLocalDevice
    void dumpInformation();
    void dumpErrors();
    void powerOn();
    void reset();
    void setHostMode(int newMode);
    void requestPairingUpdate(bool isPairing);
    void pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);
    void connected(const QBluetoothAddress &addr);
    void disconnected(const QBluetoothAddress &addr);
    void cycleSecurityFlags();

    //QBluetoothDeviceDiscoveryAgent
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void deviceUpdated(const QBluetoothDeviceInfo &info,
                       QBluetoothDeviceInfo::Fields updateFields);
    void discoveryFinished();
    void discoveryCanceled();
    void discoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void startDiscovery();
    void stopDiscovery();

    //QBluetoothServiceDiscoveryAgent
    void startServiceDiscovery(bool isMinimalDiscovery);
    void startTargettedServiceDiscovery();
    void stopServiceDiscovery();
    void serviceDiscovered(const QBluetoothServiceInfo &info);
    void serviceDiscoveryFinished();
    void serviceDiscoveryCanceled();
    void serviceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error error);
    void dumpServiceDiscovery();

    //QBluetoothSocket
    void connectToService();
    void connectToServiceViaSearch();
    void disconnectToService();
    void closeSocket();
    void abortSocket();
    void socketConnected();
    void socketDisconnected();
    void socketError(QBluetoothSocket::SocketError error);
    void socketStateChanged(QBluetoothSocket::SocketState);
    void dumpSocketInformation();
    void writeData();
    void readData();

    //QBluetoothServer
    void serverError(QBluetoothServer::Error error);
    void serverListenPort();
    void serverListenUuid();
    void serverClose();
    void serverNewConnection();
    void clientSocketDisconnected();
    void clientSocketReadyRead();
    void dumpServerInformation();

    //QLowEnergyController central
    void centralCreate();
    void centralConnect();
    void centralStartServiceDiscovery();
    void centralDiscoverServiceDetails();
    void centralCharacteristicWrite();
    void centralCharacteristicRead();
    void centralDescriptorWrite();
    void centralDescriptorRead();
    void centralSubscribeUnsubscribe();
    void centralDelete();
    void centralDisconnect();
    bool centralExists() const;
    bool centralSubscribed() const;
    QByteArray centralState() const;
    QByteArray centralServiceState() const;
    QByteArray centralError() const;
    QByteArray centralServiceError() const;
    void centralReadRSSI() const;
    QByteArray centralRSSI() const;

    //QLowEnergyController peripheral
    void peripheralCreate();
    void peripheralAddServices();
    void peripheralStartAdvertising();
    void peripheralStopAdvertising();
    void peripheralCharacteristicRead();
    void peripheralCharacteristicWrite();
    void peripheralDescriptorRead();
    void peripheralDescriptorWrite();
    void peripheralDelete();
    void peripheralDisconnect();
    bool peripheralExists() const;
    QByteArray peripheralState() const;
    QByteArray peripheralServiceState() const;
    QByteArray peripheralError() const;
    QByteArray peripheralServiceError() const;

    // QLowEnergyController misc
    void startLeDeviceDiscovery();
    void dumpLeInfo();

private:
    void dumpLocalDevice(QBluetoothLocalDevice *dev);

    QBluetoothLocalDevice *localDevice = nullptr;
    QBluetoothDeviceDiscoveryAgent *deviceAgent = nullptr;
    QBluetoothServiceDiscoveryAgent *serviceAgent = nullptr;
    QBluetoothSocket *socket = nullptr;
    QBluetoothServer *server = nullptr;
    QList<QBluetoothSocket *> serverSockets;
    QBluetoothServiceInfo serviceInfo;
    QList<QBluetoothServiceInfo> foundTestServers;
    QBluetooth::SecurityFlags securityFlags;

    std::unique_ptr<QLowEnergyController> leCentralController;
    std::unique_ptr<QLowEnergyController> lePeripheralController;
    std::unique_ptr<QLowEnergyService> leCentralService;
    QLowEnergyAdvertisingData leAdvertisingData;
    QList<QLowEnergyServiceData> lePeripheralServiceData;
    QList<QSharedPointer<QLowEnergyService>> lePeripheralServices;
    QBluetoothDeviceInfo leRemotePeripheralDevice;
    QByteArray latestRSSI = "N/A";
};

#endif // BTLOCALDEVICE_H
