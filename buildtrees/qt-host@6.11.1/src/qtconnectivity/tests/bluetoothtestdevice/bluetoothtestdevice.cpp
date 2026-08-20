// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QBluetoothLocalDevice>
#include <QLatin1String>
#include <QLoggingCategory>
#include <QLowEnergyAdvertisingData>
#include <QLowEnergyAdvertisingParameters>
#include <QLowEnergyCharacteristicData>
#include <QLowEnergyController>
#include <QLowEnergyDescriptorData>
#include <QLowEnergyServiceData>
#include <QLowEnergyDescriptorData>
#include <QTimer>

#if defined(Q_OS_ANDROID) || defined(Q_OS_DARWIN)
#include <QGuiApplication>
#else
#include <QCoreApplication>
#endif // Q_OS_ANDROID || Q_OS_IOS

#if QT_CONFIG(permissions)
#include <QtCore/qpermissions.h>
#endif

#include <thread>

static const QLatin1String largeAttributeServiceUuid("1f85e37c-ac16-11eb-ae5c-93d3a763feed");
static const QLatin1String largeAttributeCharUuid("40e4f68e-ac16-11eb-9956-cfe55a8c370c");
static const QLatin1String largeAttributeDescUuid("44e4f68e-ac16-11eb-9956-cfe55a8c370c");
static constexpr qsizetype largeAttributeSize{508}; // Size for char and desc values

static const QLatin1String platformIdentifierServiceUuid("4a92cb7f-5031-4a09-8304-3e89413f458d");
static const QLatin1String platformIdentifierCharUuid("6b0ecf7c-5f09-4c87-aaab-bb49d5d383aa");

static const QLatin1String
        notificationIndicationTestServiceUuid("bb137ac5-5716-4b80-873b-e2d11d29efe2");
static const QLatin1String
        notificationIndicationTestChar1Uuid("6da4d652-0248-478a-a5a8-1e2f076158cc");
static const QLatin1String
        notificationIndicationTestChar2Uuid("990930f0-b9cc-4c27-8c1b-ebc2bcae5c95");
static const QLatin1String
        notificationIndicationTestChar3Uuid("9a60486b-de5b-4e03-b914-4e158c0bd388");
static const QLatin1String
        notificationIndicationTestChar4Uuid("d92435d4-6c2e-43f8-a6be-bbb66b5a3e28");

static const QLatin1String connectionCountServiceUuid("78c61a07-a0f9-4b92-be2d-2570d8dbf010");
static const QLatin1String connectionCountCharUuid("9414ec2d-792f-46a2-a19e-186d0fb38a08");

static const QLatin1String mtuServiceUuid("9a9483eb-cf4f-4c32-9a6b-794238d5b483");
static const QLatin1String mtuCharUuid("960d7e2a-a850-4a70-8064-cd74e9ccb6ff");

static const QLatin1String repeatedWriteServiceUuid("72b12a31-98ea-406d-a89d-2c932d11ff67");
static const QLatin1String repeatedWriteTargetCharUuid("2192ee43-6d17-4e78-b286-db2c3b696833");
static const QLatin1String repeatedWriteNotifyCharUuid("b3f9d1a2-3d55-49c9-8b29-e09cec77ff86");

static void establishNotifyOnWriteConnection(QLowEnergyService *svc)
{
    // Make sure that the value from the repeatedWriteTargetCharUuid
    // characteristic is writted to the repeatedWriteNotifyCharUuid
    // characteristic
    Q_ASSERT(svc->serviceUuid() == QBluetoothUuid(repeatedWriteServiceUuid));
    QObject::connect(svc, &QLowEnergyService::characteristicChanged, svc,
                     [svc](const QLowEnergyCharacteristic &characteristic,
                           const QByteArray &newValue)
    {
        if (characteristic.uuid() == QBluetoothUuid(repeatedWriteTargetCharUuid)) {
            auto notifyChar = svc->characteristic(QBluetoothUuid(repeatedWriteNotifyCharUuid));
            svc->writeCharacteristic(notifyChar, newValue);
        }
    });
}

int main(int argc, char *argv[])
{
    qDebug() << "build:" << __DATE__ << __TIME__;
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#if defined(Q_OS_ANDROID) || defined(Q_OS_DARWIN)
    QGuiApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif // Q_OS_ANDROID || Q_OS_IOS

#if QT_CONFIG(permissions)
    // Check Bluetooth permission and request it if the app doesn't have it
    auto permissionStatus = app.checkPermission(QBluetoothPermission{});
    if (permissionStatus == Qt::PermissionStatus::Undetermined) {
        app.requestPermission(QBluetoothPermission{},
                              &app, [&permissionStatus](const QPermission &permission) {
            qApp->exit(); // Exit the permission request processing started below
            permissionStatus = permission.status();
        });
        // Process permission request
        app.exec();
    }
    if (permissionStatus == Qt::PermissionStatus::Denied) {
        qWarning("Bluetooth permission denied, exiting");
        return -1;
    }
#endif

    // prepare list of services
    QList<QLowEnergyServiceData> serviceDefinitions;

    int connectioncount = 0;
    {
        // connection count service
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(connectionCountServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(connectionCountCharUuid));
        size_t size = sizeof(int);
        QByteArray initialValue(size, 0);
        charData.setValue(initialValue);
        charData.setValueLength(size, size);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }

    {
        // mtu size service
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(mtuServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(mtuCharUuid));
        size_t size = sizeof(int);
        int mtu = 23;
        QByteArray initialValue((const char *)&mtu, sizeof(int));
        charData.setValue(initialValue);
        charData.setValueLength(size, size);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                               | QLowEnergyCharacteristic::PropertyType::Notify);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }
    {
        // large attribute service
        //
        // This is a service offering a large characteristic and descriptor which can
        // be read and written to
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(largeAttributeServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(largeAttributeCharUuid));
        QByteArray initialValue(largeAttributeSize, 0);
        initialValue[0] = 0x0b;
        charData.setValue(initialValue);
        charData.setValueLength(largeAttributeSize, largeAttributeSize);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                               | QLowEnergyCharacteristic::PropertyType::Write);


        QByteArray descInitialValue(largeAttributeSize, 0);
        descInitialValue[0] = 0xdd;
        QLowEnergyDescriptorData descData(
                    QBluetoothUuid{largeAttributeDescUuid},
                    descInitialValue
        );
        descData.setWritePermissions(true);
        descData.setReadPermissions(true);

        charData.addDescriptor(descData);

        serviceData.addCharacteristic(charData);
        serviceDefinitions << serviceData;
    }

    {

        // notification service
        //
        // This is a service which offers:
        //   - one characteristic which does not support notification or indication
        //   - one characteristic which does only support notification
        //   - one characteristic which does only support indication
        //   - one characteristic which supports both notification or indication
        // to test their discovery
        QLowEnergyServiceData serviceData;

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(notificationIndicationTestServiceUuid));

        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar1Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read);

            serviceData.addCharacteristic(charData);
        }
        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar2Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Notify);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }
        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar3Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Indicate);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }
        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar4Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Notify
                                   | QLowEnergyCharacteristic::PropertyType::Indicate);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }

        serviceDefinitions << serviceData;
    }

    {
        // server's platform identifier service
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(platformIdentifierServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(platformIdentifierCharUuid));
#if defined(Q_OS_ANDROID)
        QByteArray platformIdentifier("android");
#elif defined(QT_OS_WIN)
        QByteArray platformIdentifier("windows");
#elif defined(Q_OS_DARWIN)
        QByteArray platformIdentifier("darwin");
#elif defined(Q_OS_LINUX)
        QByteArray platformIdentifier("linux");
#else
        QByteArray platformIdentifier("unspecified");
#endif
        qDebug() << "Server will report it is running on: " << platformIdentifier;
        charData.setValue(platformIdentifier);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }

    {
        // repeated characteristic write service
        //
        // This service offers an 8 bytes large characteristic which can
        // be read and written. Once the value is updated, it writes the
        // same value to the other characteristic, which notifies the client
        // about its change. This way we can make sure that all write were
        // successful and happened in the right order.
        // We can't use one characteristics for writing and notification,
        // because on most backends when the characteristics was written
        // by the client, there will be no notification about it.
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(repeatedWriteServiceUuid));

        {
            // The characteristics to be written by the client.
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(repeatedWriteTargetCharUuid));
            QByteArray initialValue(8, 0);
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Write);

            serviceData.addCharacteristic(charData);
        }
        {
            // The characteristics written by the server,
            // it will send notifications to the client.
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(repeatedWriteNotifyCharUuid));
            QByteArray initialValue(8, 0);
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Write
                                   | QLowEnergyCharacteristic::PropertyType::Notify);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }

        serviceDefinitions << serviceData;
    }

#ifndef Q_OS_IOS
    auto localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.isEmpty()) {
        qWarning() << "Bluetoothtestdevice did not find a local adapter";
        return EXIT_FAILURE;
    }
    QBluetoothLocalDevice adapter(localAdapters.back().address());
    adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
#endif // Q_OS_IOS

    // Advertising data
    QLowEnergyAdvertisingData advertisingData;
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("BluetoothTestDevice");
    QList<QBluetoothUuid> serviceUuids;
    for (const auto &serviceData : serviceDefinitions) {
        serviceUuids << serviceData.uuid();
    }
    // leads to too large advertising data
    // advertisingData.setServices(serviceUuids);

    // start advertising
#ifndef Q_OS_IOS
    auto devices = QBluetoothLocalDevice::allDevices();
    qDebug() << "advertising on" << devices.back().address();
    const QScopedPointer<QLowEnergyController> leController(
            QLowEnergyController::createPeripheral(devices.back().address()));
#else
    const QScopedPointer<QLowEnergyController> leController(
            QLowEnergyController::createPeripheral());
#endif // Q_OS_IOS

    QObject::connect(leController.data(), &QLowEnergyController::errorOccurred,
                     &app, [](QLowEnergyController::Error error) {
        qDebug() << "Bluetoothtestdevice QLowEnergyController errorOccurred:" << error;
    });

    QObject::connect(leController.data(), &QLowEnergyController::stateChanged,
                     &app, [](QLowEnergyController::ControllerState state) {
        qDebug() << "Bluetoothtestdevice QLowEnergyController stateChanged:" << state;
    });


    QList<QSharedPointer<QLowEnergyService>> services;

    for (const auto &serviceData : serviceDefinitions) {
        services.emplaceBack(leController->addService(serviceData));
    }

    establishNotifyOnWriteConnection(services[5].get());

    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);


    auto reconnect = [&connectioncount, &leController, advertisingData, &services, serviceDefinitions]() {
        connectioncount++;
        for (qsizetype i = 0; i < services.size(); ++i) {
            services[i].reset(leController->addService(serviceDefinitions[i]));
        }

        establishNotifyOnWriteConnection(services[5].get());

        {
            // set connection counter
            Q_ASSERT(services[0]->serviceUuid()
                     == QBluetoothUuid(connectionCountServiceUuid));
            QByteArray value((const char *)&connectioncount, sizeof(int));
            QLowEnergyCharacteristic characteristic = services[0]->characteristic(
                    QBluetoothUuid(connectionCountCharUuid));
            Q_ASSERT(characteristic.isValid());
            services[0]->writeCharacteristic(characteristic, value);
        }

        bool allValid = true;
        for (const auto &service : services) {
            allValid = allValid & !service.isNull();
        }

        if (allValid){
            leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                           advertisingData);
            qDebug() << "starting advertising for " << connectioncount << "th time.";
        }
        else
            qDebug() << "Cannot start advertising: Service not valid.";
    };
    QObject::connect(leController.data(), &QLowEnergyController::disconnected, &app, reconnect);

    QObject::connect(leController.data(), &QLowEnergyController::mtuChanged,
                     &app, [&services](int mtu) {
        qDebug() << "MTU changed, callback called.";
        Q_ASSERT(services[1]->serviceUuid() == QBluetoothUuid(mtuServiceUuid));
        QByteArray value((const char *)&mtu, sizeof(int));
        QLowEnergyCharacteristic characteristic =
                services[1]->characteristic(QBluetoothUuid(mtuCharUuid));
        Q_ASSERT(characteristic.isValid());
        services[1]->writeCharacteristic(characteristic, value);
    });

    QTimer notificationTestTimer;
    quint64 currentCharacteristicValue = 0;
    const auto characteristicChanger = [&services, &currentCharacteristicValue]() {
        QByteArray value((const char *)&currentCharacteristicValue, 8);

        Q_ASSERT(services[3]->serviceUuid()
                 == QBluetoothUuid(notificationIndicationTestServiceUuid));
        {
            QLowEnergyCharacteristic characteristic = services[3]->characteristic(
                    QBluetoothUuid(notificationIndicationTestChar2Uuid));
            Q_ASSERT(characteristic.isValid());
            services[3]->writeCharacteristic(characteristic,
                                             value); // Potentially causes notification.
        }
        {
            QLowEnergyCharacteristic characteristic = services[3]->characteristic(
                    QBluetoothUuid(notificationIndicationTestChar3Uuid));
            Q_ASSERT(characteristic.isValid());
            services[3]->writeCharacteristic(characteristic,
                                             value); // Potentially causes notification.
        }
        {
            QLowEnergyCharacteristic characteristic = services[3]->characteristic(
                    QBluetoothUuid(notificationIndicationTestChar4Uuid));
            Q_ASSERT(characteristic.isValid());
            services[3]->writeCharacteristic(characteristic,
                                             value); // Potentially causes notification.
        }

        ++currentCharacteristicValue;
    };
    QObject::connect(&notificationTestTimer, &QTimer::timeout, &app, characteristicChanger);
    notificationTestTimer.start(100);

    return app.exec();
}
