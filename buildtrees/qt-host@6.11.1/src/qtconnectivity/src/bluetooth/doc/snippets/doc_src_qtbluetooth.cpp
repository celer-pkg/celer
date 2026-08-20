// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [include]
#include <QtBluetooth/QBluetoothLocalDevice>
//! [include]
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothServiceDiscoveryAgent>

#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyCharacteristic>

//! [namespace]
QT_USE_NAMESPACE
//! [namespace]

class MyClass : public QObject
{
    Q_OBJECT
public:
    MyClass() : QObject() {}
    void localDevice();
    void startDeviceDiscovery();
    void startServiceDiscovery();
    void objectPush();
    void btleSharedData();
    void enableCharNotifications();

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void characteristicChanged(const QLowEnergyCharacteristic& ,const QByteArray&);
};

void MyClass::localDevice() {
//! [turningon]
QBluetoothLocalDevice localDevice;
QString localDeviceName;

// Check if Bluetooth is available on this device
if (localDevice.isValid()) {

    // Turn Bluetooth on
    localDevice.powerOn();

    // Read local device name
    localDeviceName = localDevice.name();

    // Make it visible to others
    localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    // Get connected devices
    QList<QBluetoothAddress> remotes;
    remotes = localDevice.connectedDevices();
}
//! [turningon]


}

//! [device_discovery]
void MyClass::startDeviceDiscovery()
{

    // Create a discovery agent and connect to its signals
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

    // Start a discovery
    discoveryAgent->start();

    //...
}

// In your local slot, read information about the found devices
void MyClass::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device:" << device.name() << '(' << device.address().toString() << ')';
}
//! [device_discovery]

//! [service_discovery]
void MyClass::startServiceDiscovery()
{

    // Create a discovery agent and connect to its signals
    QBluetoothServiceDiscoveryAgent *discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    connect(discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));

    // Start a discovery
    discoveryAgent->start();

    //...
}

// In your local slot, read information about the found devices
void MyClass::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    qDebug() << "Found new service:" << service.serviceName()
             << '(' << service.device().address().toString() << ')';
}
//! [service_discovery]

void MyClass::characteristicChanged(const QLowEnergyCharacteristic &, const QByteArray &)
{
}

void MyClass::btleSharedData()
{
    QBluetoothDeviceInfo remoteDevice;

//! [data_share_qlowenergyservice]
    QLowEnergyService *first, *second;
    QLowEnergyController control(remoteDevice);
    control.connectToDevice();

    // waiting for connection

    first = control.createServiceObject(QBluetoothUuid::ServiceClassUuid::BatteryService);
    second = control.createServiceObject(QBluetoothUuid::ServiceClassUuid::BatteryService);
    Q_ASSERT(first->state() == QLowEnergyService::RemoteService);
    Q_ASSERT(first->state() == second->state());

    first->discoverDetails();

    Q_ASSERT(first->state() == QLowEnergyService::RemoteServiceDiscovering);
    Q_ASSERT(first->state() == second->state());
//! [data_share_qlowenergyservice]
}

void MyClass::enableCharNotifications()
{
    QBluetoothDeviceInfo remoteDevice;
    QLowEnergyService *service;
    QLowEnergyController *control = QLowEnergyController::createCentral(remoteDevice, this);
    control->connectToDevice();


    service = control->createServiceObject(QBluetoothUuid::ServiceClassUuid::BatteryService, this);
    if (!service)
        return;

    service->discoverDetails();

    //... wait until discovered

//! [enable_btle_notifications]
    //PreCondition: service details already discovered
    QLowEnergyCharacteristic batteryLevel = service->characteristic(
                QBluetoothUuid::CharacteristicType::BatteryLevel);
    if (!batteryLevel.isValid())
        return;

    QLowEnergyDescriptor notification = batteryLevel.descriptor(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (!notification.isValid())
        return;

    // establish hook into notifications
    connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));

    // enable notification
    service->writeDescriptor(notification, QByteArray::fromHex("0100"));

    // disable notification
    //service->writeDescriptor(notification, QByteArray::fromHex("0000"));

    // wait until descriptorWritten() signal is emitted
    // to confirm successful write
//! [enable_btle_notifications]
}



int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    MyClass cl;

    return app.exec();
}

#include "doc_src_qtbluetooth.moc"
