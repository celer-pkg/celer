// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "rep_heater_source.h"
#include "bleiodevice.h"

#include <QtRemoteObjects/QRemoteObjectNode>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/QLowEnergyServiceData>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QLoggingCategory>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>

using namespace Qt::Literals::StringLiterals;

class Heater : public HeaterSimpleSource
{
    Q_OBJECT
public:
    explicit Heater(QObject* parent = nullptr) : HeaterSimpleSource(parent)
    {
        m_changeTimer.setInterval(std::chrono::seconds{2});
        m_changeTimer.setSingleShot(false);

        QObject::connect(&m_changeTimer, &QTimer::timeout, this, [this]() {
            if (heaterPoweredOn())
                setCurrentTemperature(currentTemperature() + 1);
            else
                setCurrentTemperature(currentTemperature() - 1);
        });

        m_changeTimer.start();
    }

private:
    QTimer m_changeTimer;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption verboseOption("verbose", "Verbose mode");
    parser.addOption(verboseOption);
    parser.process(app);
    if (parser.isSet(verboseOption)) {
        QLoggingCategory::setFilterRules("qt.remoteobjects* = true"_L1);
        QLoggingCategory::setFilterRules("qt.bluetooth* = true"_L1);
    }

    // The model that will be used as the source object
    std::unique_ptr<Heater> heater = std::make_unique<Heater>();
    std::unique_ptr<QLowEnergyController> bleController;
    std::unique_ptr<QRemoteObjectHost> hostNode;
    std::unique_ptr<BLEIoDevice> ioDevice;

    // Setup BLE server. The two-way communication consists of a GATT service
    // which has two characteristics. The characteristics form the two-way communication
    // between client/server. The characteristic notifications are enabled so that each
    // side (client/server) gets notified if the other has written data.
    QLowEnergyCharacteristicData rxCharData;
    rxCharData.setUuid(BLEIoDevice::CLIENT_TX_SERVER_RX_CHAR_UUID);
    // Allow the remote end (client) to write to this characteristic
    rxCharData.setProperties(QLowEnergyCharacteristic::Write);

    QLowEnergyCharacteristicData txCharData;
    txCharData.setUuid(BLEIoDevice::CLIENT_RX_SERVER_TX_CHAR_UUID);
    // Allow the remote end (client) to read characteristic's data and subscribe to value changes
    const QLowEnergyDescriptorData clientConfig(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                QLowEnergyCharacteristic::CCCDDisable);
    txCharData.addDescriptor(clientConfig);
    txCharData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);

    // Set up the service, its characteristics, and advertisement data
    QLowEnergyServiceData sppServiceData;
    sppServiceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    sppServiceData.setUuid(BLEIoDevice::SERVICE_UUID);
    sppServiceData.addCharacteristic(txCharData);
    sppServiceData.addCharacteristic(rxCharData);

    QLowEnergyAdvertisingData sppAdvertisingData;
    sppAdvertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    sppAdvertisingData.setIncludePowerLevel(true);
    // The device name that is broadcasted in the advertisement. Note that this is not supported
    // on Android where the device's name is used instead
    sppAdvertisingData.setLocalName("QtRO peripheral"_L1);

    bleController.reset(QLowEnergyController::createPeripheral());

    // Add the service to the BT LE controller and start advertising so that clients
    // can discover this server. When the client disconnects the advertisement is
    // resumed so that next clients can find the server
    std::unique_ptr<QLowEnergyService> sppService;
    auto startAdvertising = [&bleController, sppAdvertisingData, sppServiceData, &sppService]
    {
        sppService.reset(bleController->addService(sppServiceData));
        if (sppService) {
            bleController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                           sppAdvertisingData, sppAdvertisingData);
        }
    };

    QObject::connect(bleController.get(), &QLowEnergyController::disconnected, bleController.get(),
                     [&]() {
        // Upon disconnection the underlying BLE service used by the ioDevice becomes
        // obsolete. Free up the resources, resume advertising and wait for a new client
        hostNode.reset(nullptr);
        ioDevice.reset(nullptr);
        startAdvertising();
    });

    auto errorHandler = [&](QLowEnergyController::Error errorCode)
    {
        // Exit server in cases we won't try to recover
        qWarning().noquote().nospace() << errorCode << " error: " << bleController->errorString();
        if (errorCode != QLowEnergyController::RemoteHostClosedError) {
            qWarning("BLE server quitting due to the error.");
            QCoreApplication::quit();
        }
    };
    // Use queued connection in case the advertising fails before we even enter
    // the event loop / exec(), in which case the quit() does nothing
    QObject::connect(bleController.get(), &QLowEnergyController::errorOccurred,
                     &app, errorHandler, Qt::QueuedConnection);

    // Start initial advertising
    startAdvertising();

    // Now we are advertising, next:
    // 1) Wait for a client to discover the service and get connected
    // 2) Wait for client to indicate it is fully ready
    // 3) Create RO sourcenode and BLE IO device
    QMetaObject::Connection readConnection;
    QObject::connect(bleController.get(), &QLowEnergyController::connected, bleController.get(),
                     [&]() {
        Q_ASSERT(!hostNode);
        Q_ASSERT(!ioDevice);
        readConnection = QObject::connect(sppService.get(),
                                          &QLowEnergyService::characteristicChanged,
                                          bleController.get(),
                                          [&](const QLowEnergyCharacteristic &info,
                                              const QByteArray &value) {
            // Wait for the client to signal it is ready
            if (info.uuid() != BLEIoDevice::CLIENT_TX_SERVER_RX_CHAR_UUID
                    || value != BLEIoDevice::CLIENT_READY_SIGNAL) {
                return;
            }

            QObject::disconnect(readConnection);
            readConnection = {};

            // We have a client that is connected and has indicated it is ready;
            // Create the hostnode and the associated BLE IO device and enable
            // remote access
            hostNode = std::make_unique<QRemoteObjectHost>();
            hostNode->setHostUrl(u"ble://qt_ro_ble"_s,
                                 QRemoteObjectHost::AllowExternalRegistration);
            hostNode->enableRemoting(heater.get());

            ioDevice = std::make_unique<BLEIoDevice>(sppService.get(),
                           BLEIoDevice::CLIENT_TX_SERVER_RX_CHAR_UUID,
                           BLEIoDevice::CLIENT_RX_SERVER_TX_CHAR_UUID);
            QObject::connect(ioDevice.get(), &BLEIoDevice::closing, bleController.get(),
                             &QLowEnergyController::disconnectFromDevice);
            hostNode->addHostSideConnection(ioDevice.get());
        });
    });

    return QCoreApplication::exec();
}

#include "main.moc"
