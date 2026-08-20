// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <QBluetoothLocalDevice>
#include <QLowEnergyController>
#include <QLowEnergyServiceData>
#include <QLowEnergyCharacteristicData>
#include <QLowEnergyDescriptorData>

#if QT_CONFIG(permissions)
#include <QtTest/qtesteventloop.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qpermissions.h>
#include <QtCore/qnamespace.h>
#endif // permissions

using namespace Qt::Literals::StringLiterals;

static constexpr auto leServiceUuid{"10f5e37c-ac16-11eb-ae5c-93d3a763feed"_L1};
static constexpr auto leCharUuid1{"11f4f68e-ac16-11eb-9956-cfe55a8ccafe"_L1};
static const qsizetype leCharacteristicSize = 4; // Set to 1...512 bytes
static QByteArray leCharacteristicValue = QByteArray{leCharacteristicSize, 1};
static QByteArray leDescriptorValue = "a descriptor value"_ba;

class tst_qlowenergycontroller_peripheral : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();

    void localCharacteristicReadAfterUpdate();
    void localDescriptorReadAfterUpdate();

private:
    QBluetoothHostInfo mDevice;
    std::unique_ptr<QLowEnergyController> mController;
    QList<QSharedPointer<QLowEnergyService>> mServices;
};

void tst_qlowenergycontroller_peripheral::initTestCase()
{
#ifndef Q_OS_IOS
    auto devices = QBluetoothLocalDevice::allDevices();
    if (devices.isEmpty())
        QSKIP("Failed to find local adapter");
    else
        mDevice = devices.back();
#endif // Q_OS_IOS

#if QT_CONFIG(permissions)
    Qt::PermissionStatus permissionStatus = qApp->checkPermission(QBluetoothPermission{});
    // FIXME: Android will add more specific BT permissions, fix when appropriate
    // change is in qtbase.
    if (qApp->checkPermission(QBluetoothPermission{}) == Qt::PermissionStatus::Undetermined) {
        QTestEventLoop loop;
        qApp->requestPermission(QBluetoothPermission{},
                                &loop, [&permissionStatus, &loop](const QPermission &permission){
            permissionStatus = permission.status();
            loop.exitLoop();
        });
        if (permissionStatus == Qt::PermissionStatus::Undetermined)
            loop.enterLoopMSecs(30000);
    }
    if (permissionStatus != Qt::PermissionStatus::Granted)
        QSKIP("This manual test requires Blutooth permissions granted.");
#endif // permissions
}

void tst_qlowenergycontroller_peripheral::init()
{
    QList<QLowEnergyServiceData> serviceDefinitions;

    QLowEnergyServiceData sd;
    sd.setType(QLowEnergyServiceData::ServiceTypePrimary);
    sd.setUuid(QBluetoothUuid(leServiceUuid));

    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid(leCharUuid1));
    charData.setValue(leCharacteristicValue);
    charData.setValueLength(leCharacteristicSize, leCharacteristicSize);
    charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                           | QLowEnergyCharacteristic::PropertyType::Write
                           | QLowEnergyCharacteristic::PropertyType::Notify
                           | QLowEnergyCharacteristic::ExtendedProperty);

    const QLowEnergyDescriptorData clientConfig(
            QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
            QLowEnergyCharacteristic::CCCDDisable);
    charData.addDescriptor(clientConfig);

    const QLowEnergyDescriptorData userDescription(
                QBluetoothUuid::DescriptorType::CharacteristicUserDescription,
                leDescriptorValue);
    charData.addDescriptor(userDescription);

    const QLowEnergyDescriptorData extendedProperties(
                QBluetoothUuid::DescriptorType::CharacteristicExtendedProperties,
                // From bluetooth specs: length 2 bytes
                // bit 0: reliable write, bit 1: writable auxiliaries
                QByteArray::fromHex("0300"));
    charData.addDescriptor(extendedProperties);

    sd.addCharacteristic(charData);

    serviceDefinitions << sd;


#ifndef Q_OS_IOS
    mController.reset(QLowEnergyController::createPeripheral(mDevice.address()));
#else
    mController.reset(QLowEnergyController::createPeripheral());
#endif // Q_OS_IOS
    QVERIFY(mController);

    for (const auto &serviceData : serviceDefinitions) {
        mServices.emplaceBack(mController->addService(serviceData));
    }
}

void tst_qlowenergycontroller_peripheral::cleanup()
{
    if (mController)
        mController->stopAdvertising();

    mController.reset();
    mServices.clear();
}

void tst_qlowenergycontroller_peripheral::localCharacteristicReadAfterUpdate()
{
#ifdef Q_OS_WINDOWS
    QSKIP("Windows does not support peripheral");
#endif
    auto characteristic = mServices[0]->characteristic(QBluetoothUuid(leCharUuid1));
    QVERIFY(characteristic.isValid());
    const auto initialValue = characteristic.value();
    auto newValue = initialValue;
    newValue[0] = newValue[0] + 1;
    mServices[0]->writeCharacteristic(characteristic, newValue);
    QTRY_COMPARE(characteristic.value(), newValue);
}

void tst_qlowenergycontroller_peripheral::localDescriptorReadAfterUpdate()
{
#ifdef Q_OS_WINDOWS
    QSKIP("Windows does not support peripheral");
#elif defined Q_OS_DARWIN
    QSKIP("Apple devices do not support descriptor value update");
#endif
    auto characteristic = mServices[0]->characteristic(QBluetoothUuid(leCharUuid1));
    QVERIFY(characteristic.isValid());
    auto descriptor = characteristic.descriptor(
                QBluetoothUuid::DescriptorType::CharacteristicUserDescription);
    QVERIFY(descriptor.isValid());
    const auto initialValue = descriptor.value();
    auto newValue = initialValue;
    newValue[0] = newValue[0] + 1;
    mServices[0]->writeDescriptor(descriptor, newValue);
    QCOMPARE(descriptor.value(), newValue);
}

QTEST_MAIN(tst_qlowenergycontroller_peripheral)

#include "tst_qlowenergycontroller_peripheral.moc"
