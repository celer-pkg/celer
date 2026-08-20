// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QLowEnergyAdvertisingData>
#include <QLowEnergyAdvertisingParameters>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyCharacteristicData>
#include <QLowEnergyDescriptorData>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyServiceData>

#include <QByteArray>

#if defined(Q_OS_ANDROID) || defined(Q_OS_DARWIN)
#include <QGuiApplication>
#else
#include <QCoreApplication>
#endif

#include <QList>
#include <QLoggingCategory>
#include <QTimer>

#if QT_CONFIG(permissions)
#include <QPermissions>
#endif

#include <memory>

int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#if defined(Q_OS_ANDROID) || defined(Q_OS_DARWIN)
    QGuiApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif

#if QT_CONFIG(permissions)
    //! [Check Bluetooth Permission]
    auto permissionStatus = app.checkPermission(QBluetoothPermission{});
    //! [Check Bluetooth Permission]

    //! [Request Bluetooth Permission]
    if (permissionStatus == Qt::PermissionStatus::Undetermined) {
        qInfo("Requesting Bluetooth permission ...");
        app.requestPermission(QBluetoothPermission{}, [&permissionStatus](const QPermission &permission){
            qApp->exit();
            permissionStatus = permission.status();
        });
        // Now, wait for permission request to resolve.
        app.exec();
    }
    //! [Request Bluetooth Permission]

    if (permissionStatus == Qt::PermissionStatus::Denied) {
        // Either explicitly denied by a user, or Bluetooth is off.
        qWarning("This application cannot use Bluetooth, the permission was denied");
        return -1;
    }

#endif
    //! [Advertising Data]
    QLowEnergyAdvertisingData advertisingData;
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("HeartRateServer");
    advertisingData.setServices(QList<QBluetoothUuid>() << QBluetoothUuid::ServiceClassUuid::HeartRate);
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
    charData.setValue(QByteArray(2, 0));
    charData.setProperties(QLowEnergyCharacteristic::Notify);
    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData.addDescriptor(clientConfig);

    QLowEnergyServiceData serviceData;
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::HeartRate);
    serviceData.addCharacteristic(charData);
    //! [Service Data]

    //! [Start Advertising]
    bool errorOccurred = false;
    const std::unique_ptr<QLowEnergyController> leController(QLowEnergyController::createPeripheral());
    auto errorHandler = [&leController, &errorOccurred](QLowEnergyController::Error errorCode) {
            qWarning().noquote().nospace() << errorCode << " occurred: "
                << leController->errorString();
            if (errorCode != QLowEnergyController::RemoteHostClosedError) {
                qWarning("Heartrate-server quitting due to the error.");
                errorOccurred = true;
                QCoreApplication::quit();
            }
    };
    QObject::connect(leController.get(), &QLowEnergyController::errorOccurred, errorHandler);

    std::unique_ptr<QLowEnergyService> service(leController->addService(serviceData));
    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);
    if (errorOccurred)
        return -1;
    //! [Start Advertising]

    //! [Provide Heartbeat]
    QTimer heartbeatTimer;
    quint8 currentHeartRate = 60;
    enum ValueChange { ValueUp, ValueDown } valueChange = ValueUp;
    const auto heartbeatProvider = [&service, &currentHeartRate, &valueChange]() {
        QByteArray value;
        value.append(char(0)); // Flags that specify the format of the value.
        value.append(char(currentHeartRate)); // Actual value.
        QLowEnergyCharacteristic characteristic
                = service->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
        Q_ASSERT(characteristic.isValid());
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
        if (currentHeartRate == 60)
            valueChange = ValueUp;
        else if (currentHeartRate == 100)
            valueChange = ValueDown;
        if (valueChange == ValueUp)
            ++currentHeartRate;
        else
            --currentHeartRate;
    };
    QObject::connect(&heartbeatTimer, &QTimer::timeout, heartbeatProvider);
    heartbeatTimer.start(1000);
    //! [Provide Heartbeat]

    auto reconnect = [&leController, advertisingData, &service, serviceData]() {
        service.reset(leController->addService(serviceData));
        if (service) {
            leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                           advertisingData, advertisingData);
        }
    };
    QObject::connect(leController.get(), &QLowEnergyController::disconnected, reconnect);

    const int retval = QCoreApplication::exec();
    return errorOccurred ? -1 : retval;
}
