// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "connectpage.h"
#include "ui_connectpage.h"
#include "bleiodevice.h"

#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>

using namespace Qt::Literals::StringLiterals;

ConnectPage::ConnectPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConnectPage)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    ui->setupUi(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(20000);
    connect(m_discoveryAgent,  &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, [this]() {
        emit showMessage(m_discoveryAgent->errorString());
        ui->scanButton->setEnabled(true);
        ui->connectButton->setEnabled(false);
    });
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &ConnectPage::refreshDevices);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, [this]() {
        ui->scanButton->setEnabled(true);
        emit showMessage("Scan finished"_L1);
        refreshDevices();
    });
    connect(ui->devicesListWidget, &QListWidget::clicked, this, [this](const QModelIndex &index) {
        ui->connectButton->setEnabled(index.isValid() && index.row() >= 0
                                      && index.row() < m_leDevices.size());
        showMessage("Scan stopped");
        ui->scanButton->setEnabled(true);
        m_discoveryAgent->stop();
    });
    connect(ui->connectButton, &QPushButton::clicked, this, [this]() {
        if (ui->devicesListWidget->currentRow() < 0
                || ui->devicesListWidget->currentRow() >= m_leDevices.size()) {
            return; // Should never happen, but better safe than sorry
        }
        auto device = m_leDevices[ui->devicesListWidget->currentRow()];
        qDebug() << "Connecting to:" << device.name() << device.address() << device.deviceUuid();
        connectToDevice(device);
    });

    connect(ui->scanButton, &QPushButton::clicked, this, &ConnectPage::startScanning);
}

ConnectPage::~ConnectPage()
{
    if (m_controller)
        m_controller->disconnect(); // Avoid possible stray signals
    if (m_discoveryAgent && m_discoveryAgent->isActive())
        m_discoveryAgent->stop();
    delete ui;
}

void ConnectPage::startScanning()
{
    ui->scanButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
    emit showMessage("Scanning for BLE devices, please wait"_L1);
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    ui->devicesListWidget->setEnabled(true);
    ui->devicesListWidget->clear();
    m_leDevices.clear();
}

void ConnectPage::disconnectFromDevice()
{
    if (m_controller)
        m_controller->disconnectFromDevice();
}

void ConnectPage::refreshDevices()
{
    ui->devicesListWidget->clear();
    m_leDevices.clear();
    // The LE device list is used to filter out classic bluetooth devices which the device
    // discovery method may find (even if using low energy device discovery method). This list
    // helps in matching the right device listwidget index the user has selected
    for (const auto &device : m_discoveryAgent->discoveredDevices()) {
        if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
            m_leDevices.append(device);
#ifdef Q_OS_DARWIN
            // On macOS/iOS we don't have address available
            ui->devicesListWidget->addItem(device.name() + " " + device.deviceUuid().toString());
#else
            ui->devicesListWidget->addItem(device.name() + " " + device.address().toString());
#endif
        }
    }
}

void ConnectPage::connectToDevice(const QBluetoothDeviceInfo &device)
{
    ui->scanButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
    ui->devicesListWidget->setEnabled(false);

    QString deviceIdentifier;
    if (!device.name().isEmpty())
        deviceIdentifier = device.name();
    else if (!device.address().isNull())
        deviceIdentifier = device.address().toString();
    else
        deviceIdentifier = device.deviceUuid().toString(QUuid::WithoutBraces);
    emit showMessage("Connecting to %1"_L1.arg(deviceIdentifier));

    // If this is not the first time, release the previous low energy controller
    if (m_controller) {
        m_controller->disconnectFromDevice();
        delete m_controller;
        m_controller = nullptr;
    }

    m_controller = QLowEnergyController::createCentral(device, this);
    connect(m_controller, &QLowEnergyController::connected, this, [this]() {
        m_controller->discoverServices();
    });
    connect(m_controller, &QLowEnergyController::errorOccurred, this, [this]() {
        ui->devicesListWidget->setEnabled(true);
        ui->scanButton->setEnabled(!m_discoveryAgent->isActive());
        m_service = nullptr;
        emit showMessage("Error occurred: "_L1 + m_controller->errorString());
    });

    connect(m_controller, &QLowEnergyController::disconnected, this, [this]() {
        m_service = nullptr;
        ui->devicesListWidget->setEnabled(true);
        emit showMessage("Diconnected from remote"_L1);
        emit disconnected();
    });

    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, [this](const QBluetoothUuid &newService) {
        if (newService != BLEIoDevice::SERVICE_UUID)
            return;
        emit showMessage("Service found"_L1);
        m_service = m_controller->createServiceObject(newService, m_controller);
        if (!m_service) {
            qWarning("BT LE Service couldn't be created");
            return;
        }
        if (m_service->state() == QLowEnergyService::RemoteService) {
            emit showMessage("Service found, scanning for details"_L1);
            connect(m_service, &QLowEnergyService::stateChanged,
                    this, [this](QLowEnergyService::ServiceState state) {

                if (state == QLowEnergyService::ServiceState::RemoteServiceDiscovered)
                    connectNode();
            });
            m_service->discoverDetails();
        } else {
            connectNode();
        }
    });

    connect(m_controller, &QLowEnergyController::discoveryFinished, this, [this]() {
        if (m_service)
            return;
        emit showMessage("Service was not found");
        ui->scanButton->setEnabled(true);
        ui->devicesListWidget->setEnabled(true);
    });

    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);
    m_controller->connectToDevice();
}

void ConnectPage::connectNode()
{
    if (!m_service)
        return;

    // Below we subscribe to characteristic value change notifications.
    // If subscription fails, disconnect as there's no point in continuing
    QObject::connect(m_service.data(), &QLowEnergyService::descriptorWritten, this,
            [this](const QLowEnergyDescriptor &info,
            const QByteArray &value) {

        if (info.uuid() != QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)
            return;

        if (value != QLowEnergyCharacteristic::CCCDEnableNotification) {
            // We failed to subscribe to characteristic change notifications
            m_controller->disconnectFromDevice();
        } else {
            emit showMessage("Connected to QtRO node"_L1);
            auto ioDevice = new BLEIoDevice(m_service,
                                            BLEIoDevice::CLIENT_RX_SERVER_TX_CHAR_UUID,
                                            BLEIoDevice::CLIENT_TX_SERVER_RX_CHAR_UUID,
                                            this);
            connect(this, &ConnectPage::disconnected, ioDevice, &BLEIoDevice::disconnected);
            connect(ioDevice, &BLEIoDevice::disconnected, ioDevice, &BLEIoDevice::deleteLater);
            emit connected(ioDevice);
        }
    });

    QObject::connect(m_service, &QLowEnergyService::errorOccurred, this, [this]
                                (QLowEnergyService::ServiceError error) {
            if (error == QLowEnergyService::ServiceError::DescriptorWriteError)
                m_controller->disconnectFromDevice();
    });

    m_service->writeDescriptor(
                  m_service->characteristic(BLEIoDevice::CLIENT_RX_SERVER_TX_CHAR_UUID).descriptor(
                  QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration),
                  QLowEnergyCharacteristic::CCCDEnableNotification);
}
