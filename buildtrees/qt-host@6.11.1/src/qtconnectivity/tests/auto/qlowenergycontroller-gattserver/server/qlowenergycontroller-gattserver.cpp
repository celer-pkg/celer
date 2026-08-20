// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergyconnectionparameters.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qendian.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qsharedpointer.h>

static QByteArray deviceName() { return "Qt GATT server"; }

static QScopedPointer<QLowEnergyController> leController;
typedef QSharedPointer<QLowEnergyService> ServicePtr;
static QHash<QBluetoothUuid, ServicePtr> services;
static int descriptorWriteCount = 0;
static int disconnectCount = 0;
static QBluetoothAddress remoteDevice;

void addService(const QLowEnergyServiceData &serviceData)
{
    const ServicePtr service(leController->addService(serviceData));
    Q_ASSERT(service);
    services.insert(service->serviceUuid(), service);
}

void addRunningSpeedService()
{
    QLowEnergyServiceData serviceData;
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyDescriptorData desc;
    desc.setUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    desc.setValue(QByteArray(2, 0)); // Default: No indication, no notification.
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::RSCMeasurement);
    charData.addDescriptor(desc);
    charData.setProperties(QLowEnergyCharacteristic::Notify);
    QByteArray value(4, 0);
    value[0] = 1 << 2; // "Running", no optional fields.
    charData.setValue(value);
    serviceData.addCharacteristic(charData);

    charData = QLowEnergyCharacteristicData();
    charData.setUuid(QBluetoothUuid::CharacteristicType::RSCFeature);
    charData.setProperties(QLowEnergyCharacteristic::Read);
    value = QByteArray(2, 0);
    qToLittleEndian<quint16>(1 << 2, reinterpret_cast<uchar *>(value.data()));
    charData.setValue(value);
    serviceData.addCharacteristic(charData);
    addService(serviceData);
}

void addGenericAccessService()
{
    QLowEnergyServiceData serviceData;
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::GenericAccess);
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::DeviceName);
    charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Write);
    charData.setValue(deviceName());
    serviceData.addCharacteristic(charData);

    charData = QLowEnergyCharacteristicData();
    charData.setUuid(QBluetoothUuid::CharacteristicType::Appearance);
    charData.setProperties(QLowEnergyCharacteristic::Read);
    QByteArray value(2, 0);
    qToLittleEndian<quint16>(128, reinterpret_cast<uchar *>(value.data())); // Generic computer.
    charData.setValue(value);
    serviceData.addCharacteristic(charData);

    serviceData.addIncludedService(services.value(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence).data());
    addService(serviceData);
}

void addCustomService()
{
    QLowEnergyServiceData serviceData;
    serviceData.setUuid(QBluetoothUuid(quint16(0x2000)));
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid(quint16(0x5000))); // Made up.
    charData.setProperties(QLowEnergyCharacteristic::Read);
    charData.setValue(QByteArray(1024, 'x')); // Long value to test "Read Blob".
    serviceData.addCharacteristic(charData);

    charData.setUuid(QBluetoothUuid(quint16(0x5001)));
    charData.setProperties(QLowEnergyCharacteristic::Read);
    charData.setReadConstraints(
            QBluetooth::AttAccessConstraint::AttAuthorizationRequired); // To test read failure.
    serviceData.addCharacteristic(charData);
    charData.setValue("something");

    charData.setUuid(QBluetoothUuid(quint16(0x5002)));
    charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Indicate);
    charData.setReadConstraints(QBluetooth::AttAccessConstraints());
    const QLowEnergyDescriptorData desc(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                        QByteArray(2, 0));
    charData.addDescriptor(desc);
    serviceData.addCharacteristic(charData);

    charData.setUuid(QBluetoothUuid(quint16(0x5003)));
    charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
    serviceData.addCharacteristic(charData);

    charData.setUuid(QBluetoothUuid(quint16(0x5004)));
    charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::WriteSigned);
    charData.setDescriptors(QList<QLowEnergyDescriptorData>());
    charData.setValue("initial");
    serviceData.addCharacteristic(charData);

    addService(serviceData);

    // service with full 128 bit custom uuids
    QLowEnergyServiceData serviceData128;
    serviceData128.setUuid(QBluetoothUuid(QString("c47774c7-f237-4523-8968-e4ae75431daf")));
    serviceData128.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyCharacteristicData charData128;
    charData128.setUuid(QBluetoothUuid(QString("c0ad61b1-79e7-42f9-ace0-0a9aa0d0a4f8")));
    charData128.setProperties(QLowEnergyCharacteristic::Read);
    charData128.setValue(QByteArray(15, 'a'));
    serviceData128.addCharacteristic(charData128);

    addService(serviceData128);
}

void startAdvertising()
{
    QLowEnergyAdvertisingParameters params;
    params.setMode(QLowEnergyAdvertisingParameters::AdvInd);
    QLowEnergyAdvertisingData data;
    data.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityLimited);
    data.setServices(services.keys());
    data.setIncludePowerLevel(true);
    data.setLocalName(deviceName());
    leController->startAdvertising(params, data);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    leController.reset(QLowEnergyController::createPeripheral());
    addRunningSpeedService();
    addGenericAccessService();
    addCustomService();
    startAdvertising();

    const ServicePtr customService = services.value(QBluetoothUuid(quint16(0x2000)));
    Q_ASSERT(customService);

    const auto stateChangedHandler = [customService]() {
        switch (leController->state()) {
        case QLowEnergyController::ConnectedState:
            remoteDevice = leController->remoteAddress();
            break;
        case QLowEnergyController::UnconnectedState: {
            if (++disconnectCount == 2) {
                qApp->quit();
                break;
            }
            Q_ASSERT(disconnectCount == 1);
            const QLowEnergyCharacteristic indicatableChar
                    = customService->characteristic(QBluetoothUuid(quint16(0x5002)));
            Q_ASSERT(indicatableChar.isValid());
            customService->writeCharacteristic(indicatableChar, "indicated2");
            Q_ASSERT(indicatableChar.value() == "indicated2");
            const QLowEnergyCharacteristic notifiableChar
                    = customService->characteristic(QBluetoothUuid(quint16(0x5003)));
            Q_ASSERT(notifiableChar.isValid());
            customService->writeCharacteristic(notifiableChar, "notified2");
            Q_ASSERT(notifiableChar.value() == "notified2");
            startAdvertising();
            break;
        }
        default:
            break;
        }
    };

    QObject::connect(leController.data(), &QLowEnergyController::stateChanged,
                     &app, stateChangedHandler);
    const auto descriptorWriteHandler = [customService]() {
        if (++descriptorWriteCount != 2)
            return;
        const QLowEnergyCharacteristic indicatableChar
                = customService->characteristic(QBluetoothUuid(quint16(0x5002)));
        Q_ASSERT(indicatableChar.isValid());
        customService->writeCharacteristic(indicatableChar, "indicated");
        Q_ASSERT(indicatableChar.value() == "indicated");
        const QLowEnergyCharacteristic notifiableChar
                = customService->characteristic(QBluetoothUuid(quint16(0x5003)));
        Q_ASSERT(notifiableChar.isValid());
        customService->writeCharacteristic(notifiableChar, "notified");
        Q_ASSERT(notifiableChar.value() == "notified");
        QLowEnergyConnectionParameters connParams;
        connParams.setIntervalRange(30, 62.5);
        connParams.setLatency(5);
        connParams.setSupervisionTimeout(5500);
        leController->requestConnectionUpdate(connParams);
    };
    QObject::connect(customService.data(), &QLowEnergyService::descriptorWritten,
                     &app, descriptorWriteHandler);

    return app.exec();
}
