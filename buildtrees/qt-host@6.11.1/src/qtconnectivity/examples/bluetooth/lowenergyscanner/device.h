// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICE_H
#define DEVICE_H

#include "characteristicinfo.h"
#include "deviceinfo.h"
#include "serviceinfo.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QLowEnergyService>

#include <QList>
#include <QObject>
#include <QVariant>

#include <QQmlEngine>

QT_BEGIN_NAMESPACE
class QBluetoothDeviceInfo;
class QBluetoothUuid;
QT_END_NAMESPACE

class Device: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant devicesList READ getDevices NOTIFY devicesUpdated)
    Q_PROPERTY(QVariant servicesList READ getServices NOTIFY servicesUpdated)
    Q_PROPERTY(QVariant characteristicList READ getCharacteristics NOTIFY characteristicsUpdated)
    Q_PROPERTY(QString update READ getUpdate WRITE setUpdate NOTIFY updateChanged)
    Q_PROPERTY(bool useRandomAddress READ isRandomAddress WRITE setRandomAddress
               NOTIFY randomAddressChanged)
    Q_PROPERTY(bool state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool controllerError READ hasControllerError)

    QML_ELEMENT
    QML_SINGLETON

public:
    Device();
    ~Device();
    QVariant getDevices();
    QVariant getServices();
    QVariant getCharacteristics();
    QString getUpdate();
    bool state();
    bool hasControllerError() const;

    bool isRandomAddress() const;
    void setRandomAddress(bool newValue);

public slots:
    void startDeviceDiscovery();
    void stopDeviceDiscovery();
    void scanServices(const QString &address);

    void connectToService(const QString &uuid);
    void disconnectFromDevice();

private slots:
    // QBluetoothDeviceDiscoveryAgent related
    void addDevice(const QBluetoothDeviceInfo&);
    void deviceScanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);

    // QLowEnergyController realted
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void deviceConnected();
    void errorReceived(QLowEnergyController::Error);
    void serviceScanDone();
    void deviceDisconnected();

    // QLowEnergyService related
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

Q_SIGNALS:
    void devicesUpdated();
    void servicesUpdated();
    void characteristicsUpdated();
    void updateChanged();
    void stateChanged();
    void disconnected();
    void randomAddressChanged();

private:
    void setUpdate(const QString &message);
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    DeviceInfo currentDevice;
    QList<DeviceInfo *> devices;
    QList<ServiceInfo *> m_services;
    QList<CharacteristicInfo *> m_characteristics;
    QString m_previousAddress;
    QString m_message;
    bool connected = false;
    QLowEnergyController *controller = nullptr;
    bool m_deviceScanState = false;
    bool randomAddress = false;
};

#endif // DEVICE_H
