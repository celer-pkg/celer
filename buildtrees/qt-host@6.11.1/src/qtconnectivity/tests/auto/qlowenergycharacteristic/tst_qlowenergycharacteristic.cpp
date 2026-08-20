// Copyright (C) 2016 BlackBerry Limited all rights reserved
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QUuid>

#include <QDebug>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QBluetoothLocalDevice>

QT_USE_NAMESPACE

// This define must be set if the platform provides access to GATT handles
// otherwise it must not be defined. As of now the two supported platforms
// (Android and Bluez/Linux) provide access or some notion of it.
#define HANDLES_PROVIDED_BY_PLATFORM

#ifdef HANDLES_PROVIDED_BY_PLATFORM
#define HANDLE_VERIFY(stmt) QVERIFY(stmt)
#else
#define HANDLE_VERIFY(stmt)
#endif

class tst_QLowEnergyCharacteristic : public QObject
{
    Q_OBJECT

public:
    tst_QLowEnergyCharacteristic();
    ~tst_QLowEnergyCharacteristic();

protected slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void tst_constructionDefault();
    void tst_assignCompare();

private:
    QList<QBluetoothDeviceInfo> remoteLeDevices;
    QLowEnergyController *globalControl;
    QLowEnergyService *globalService;
};

tst_QLowEnergyCharacteristic::tst_QLowEnergyCharacteristic() :
    globalControl(0), globalService(0)
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    qRegisterMetaType<QBluetoothDeviceDiscoveryAgent::Error>();
}

tst_QLowEnergyCharacteristic::~tst_QLowEnergyCharacteristic()
{
}

void tst_QLowEnergyCharacteristic::initTestCase()
{
#if defined(Q_OS_MACOS)
    QSKIP("The low energy characteristic tests fail on macOS");
#endif
    if (QBluetoothLocalDevice::allDevices().isEmpty()) {
        qWarning("No local adapter, not discovering remote devices");
        return;
    }

    QBluetoothLocalDevice device;
    if (device.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        // Attempt to switch Bluetooth ON
        device.powerOn();
        QTest::qWait(1000);
        if (device.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
            qWarning("Bluetooth couldn't be switched ON, not discovering remote devices");
            return;
        }
    }

    // find an arbitrary low energy device in vicinity
    // find an arbitrary service with characteristic
    QBluetoothDeviceDiscoveryAgent *devAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(devAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

    QSignalSpy errorSpy(devAgent, SIGNAL(errorOccurred(QBluetoothDeviceDiscoveryAgent::Error)));
    QVERIFY(errorSpy.isValid());
    QVERIFY(errorSpy.isEmpty());

    QSignalSpy spy(devAgent, SIGNAL(finished()));
    QVERIFY(spy.isValid());
    QVERIFY(spy.isEmpty());

    devAgent->start();
    QTRY_VERIFY_WITH_TIMEOUT(!spy.isEmpty(), 100000);

    // find first service with descriptor
    QLowEnergyController *controller = 0;
    for (const QBluetoothDeviceInfo &remoteDevice : std::as_const(remoteLeDevices)) {
        controller = QLowEnergyController::createCentral(remoteDevice, this);
        qDebug() << "Connecting to" << remoteDevice.name()
                 << remoteDevice.address() << remoteDevice.deviceUuid();
        controller->connectToDevice();
        QTRY_IMPL(controller->state() != QLowEnergyController::ConnectingState,
                  50000);
        if (controller->state() != QLowEnergyController::ConnectedState) {
            // any error and we skip
            delete controller;
            qDebug() << "Skipping device";
            continue;
        }

        QSignalSpy discoveryFinishedSpy(controller, SIGNAL(discoveryFinished()));
        QSignalSpy stateSpy(controller, SIGNAL(stateChanged(QLowEnergyController::ControllerState)));
        controller->discoverServices();
        QTRY_VERIFY_WITH_TIMEOUT(discoveryFinishedSpy.size() == 1, 10000);
        QCOMPARE(stateSpy.size(), 2);
        QCOMPARE(stateSpy.at(0).at(0).value<QLowEnergyController::ControllerState>(),
                 QLowEnergyController::DiscoveringState);
        QCOMPARE(stateSpy.at(1).at(0).value<QLowEnergyController::ControllerState>(),
                 QLowEnergyController::DiscoveredState);

        const QList<QBluetoothUuid> leServiceUuids = controller->services();
        for (const QBluetoothUuid &leServiceUuid : leServiceUuids) {
            QLowEnergyService *leService = controller->createServiceObject(leServiceUuid, this);
            if (!leService)
                continue;

            leService->discoverDetails();
            QTRY_VERIFY_WITH_TIMEOUT(
                        leService->state() == QLowEnergyService::RemoteServiceDiscovered, 10000);

            const QList<QLowEnergyCharacteristic> chars = leService->characteristics();
            for (const QLowEnergyCharacteristic &ch : chars) {
                if (!ch.descriptors().isEmpty()) {
                    globalService = leService;
                    globalControl = controller;
                    qWarning() << "Found service with descriptor" << remoteDevice.address()
                               << globalService->serviceName() << globalService->serviceUuid();
                    break;
                }
            }

            if (globalControl)
                break;
            else
                delete leService;
        }

        if (globalControl)
            break;

        delete controller;
    }

    if (!globalControl) {
        qWarning() << "Test limited due to missing remote QLowEnergyDescriptor."
                   << "Please ensure the Bluetooth Low Energy device is advertising its services.";
    }
}

void tst_QLowEnergyCharacteristic::cleanupTestCase()
{
    if (globalControl)
        globalControl->disconnectFromDevice();
}

void tst_QLowEnergyCharacteristic::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
        remoteLeDevices.append(info);
}

void tst_QLowEnergyCharacteristic::tst_constructionDefault()
{
    QLowEnergyCharacteristic characteristic;
    QVERIFY(!characteristic.isValid());
    QCOMPARE(characteristic.value(), QByteArray());
    QVERIFY(characteristic.uuid().isNull());
    QCOMPARE(characteristic.name(), QString());
    QCOMPARE(characteristic.descriptors().size(), 0);
    QCOMPARE(characteristic.descriptor(QBluetoothUuid()),
             QLowEnergyDescriptor());
    QCOMPARE(characteristic.descriptor(QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)),
             QLowEnergyDescriptor());
    QCOMPARE(characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration),
             QLowEnergyDescriptor());
    QCOMPARE(characteristic.properties(), QLowEnergyCharacteristic::Unknown);

    QLowEnergyCharacteristic copyConstructed(characteristic);
    QVERIFY(!copyConstructed.isValid());
    QCOMPARE(copyConstructed.value(), QByteArray());
    QVERIFY(copyConstructed.uuid().isNull());
    QCOMPARE(copyConstructed.name(), QString());
    QCOMPARE(copyConstructed.descriptors().size(), 0);
    QCOMPARE(copyConstructed.properties(), QLowEnergyCharacteristic::Unknown);

    QVERIFY(copyConstructed == characteristic);
    QVERIFY(characteristic == copyConstructed);
    QVERIFY(!(copyConstructed != characteristic));
    QVERIFY(!(characteristic != copyConstructed));

    QLowEnergyCharacteristic assigned;

    QVERIFY(assigned == characteristic);
    QVERIFY(characteristic == assigned);
    QVERIFY(!(assigned != characteristic));
    QVERIFY(!(characteristic != assigned));

    assigned = characteristic;
    QVERIFY(!assigned.isValid());
    QCOMPARE(assigned.value(), QByteArray());
    QVERIFY(assigned.uuid().isNull());
    QCOMPARE(assigned.name(), QString());
    QCOMPARE(assigned.descriptors().size(), 0);
    QCOMPARE(assigned.properties(), QLowEnergyCharacteristic::Unknown);

    QVERIFY(assigned == characteristic);
    QVERIFY(characteristic == assigned);
    QVERIFY(!(assigned != characteristic));
    QVERIFY(!(characteristic != assigned));
}

void tst_QLowEnergyCharacteristic::tst_assignCompare()
{
    if (!globalService)
        QSKIP("No characteristic found.");

    QLowEnergyCharacteristic target;
    QVERIFY(!target.isValid());
    QCOMPARE(target.value(), QByteArray());
    QVERIFY(target.uuid().isNull());
    QCOMPARE(target.name(), QString());
    QCOMPARE(target.descriptors().size(), 0);
    QCOMPARE(target.properties(), QLowEnergyCharacteristic::Unknown);

    qsizetype indexWithDescriptor = -1;
    const QList<QLowEnergyCharacteristic> chars = globalService->characteristics();
    QVERIFY(!chars.isEmpty());
    for (qsizetype i = 0; i < chars.size(); ++i) {
        const QLowEnergyCharacteristic specific =
                globalService->characteristic(chars[i].uuid());
        QVERIFY(specific.isValid());
        QCOMPARE(specific, chars[i]);
        if (!chars[i].descriptors().isEmpty()) {
            indexWithDescriptor = i;
            break;
        }
    }

    if (chars.isEmpty())
        QSKIP("No suitable characteristic found despite prior indication.");

    bool noDescriptors = (indexWithDescriptor == -1);
    if (noDescriptors)
        indexWithDescriptor = 0; // just choose one

    // test assignment operator
    target = chars[indexWithDescriptor];
    QVERIFY(target.isValid());
    // Only standard 16-bit Bluetooth SIG UUIDs are guaranteed to have a name
    if (target.uuid().minimumSize() == 2)
        QVERIFY(!target.name().isEmpty());
    QVERIFY(!target.uuid().isNull());
    QVERIFY(target.properties() != QLowEnergyCharacteristic::Unknown);
    if (target.properties() & QLowEnergyCharacteristic::Read)
        QVERIFY(!target.value().isEmpty());
    if (!noDescriptors)
        QVERIFY(!target.descriptors().isEmpty());

    QVERIFY(target == chars[indexWithDescriptor]);
    QVERIFY(chars[indexWithDescriptor] == target);
    QVERIFY(!(target != chars[indexWithDescriptor]));
    QVERIFY(!(chars[indexWithDescriptor] != target));

    QCOMPARE(target.isValid(), chars[indexWithDescriptor].isValid());
    QCOMPARE(target.name(), chars[indexWithDescriptor].name());
    QCOMPARE(target.uuid(), chars[indexWithDescriptor].uuid());
    QCOMPARE(target.value(), chars[indexWithDescriptor].value());
    QCOMPARE(target.properties(), chars[indexWithDescriptor].properties());
    {
        const auto targetDescriptors = target.descriptors();
        const auto referenceDescriptors = chars[indexWithDescriptor].descriptors();
        QCOMPARE(targetDescriptors.size(), referenceDescriptors.size());
        for (qsizetype i = 0; i < targetDescriptors.size(); ++i) {
            const QLowEnergyDescriptor ref = referenceDescriptors[i];
            QCOMPARE(targetDescriptors[i].name(), ref.name());
            QCOMPARE(targetDescriptors[i].isValid(), ref.isValid());
            QCOMPARE(targetDescriptors[i].type(), ref.type());
            QCOMPARE(targetDescriptors[i].uuid(), ref.uuid());
            QCOMPARE(targetDescriptors[i].value(), ref.value());

            const QLowEnergyDescriptor ref2 = chars[indexWithDescriptor].descriptor(ref.uuid());
            QCOMPARE(ref, ref2);
        }
    }

    // test copy constructor
    QLowEnergyCharacteristic copyConstructed(target);
    QCOMPARE(copyConstructed.isValid(), chars[indexWithDescriptor].isValid());
    QCOMPARE(copyConstructed.name(), chars[indexWithDescriptor].name());
    QCOMPARE(copyConstructed.uuid(), chars[indexWithDescriptor].uuid());
    QCOMPARE(copyConstructed.value(), chars[indexWithDescriptor].value());
    QCOMPARE(copyConstructed.properties(), chars[indexWithDescriptor].properties());
    QCOMPARE(copyConstructed.descriptors().size(),
             chars[indexWithDescriptor].descriptors().size());

    QVERIFY(copyConstructed == target);
    QVERIFY(target == copyConstructed);
    QVERIFY(!(copyConstructed != target));
    QVERIFY(!(target != copyConstructed));

    // test invalidation
    QLowEnergyCharacteristic invalid;
    target = invalid;
    QVERIFY(!target.isValid());
    QCOMPARE(target.value(), QByteArray());
    QVERIFY(target.uuid().isNull());
    QCOMPARE(target.name(), QString());
    QCOMPARE(target.descriptors().size(), 0);
    QCOMPARE(target.properties(), QLowEnergyCharacteristic::Unknown);

    QVERIFY(invalid == target);
    QVERIFY(target == invalid);
    QVERIFY(!(invalid != target));
    QVERIFY(!(target != invalid));

    QVERIFY(!(chars[indexWithDescriptor] == target));
    QVERIFY(!(target == chars[indexWithDescriptor]));
    QVERIFY(chars[indexWithDescriptor] != target);
    QVERIFY(target != chars[indexWithDescriptor]);

    if (chars.size() >= 2) {
        // at least two characteristics
        QVERIFY(!(chars[0] == chars[1]));
        QVERIFY(!(chars[1] == chars[0]));
        QVERIFY(chars[0] != chars[1]);
        QVERIFY(chars[1] != chars[0]);
    }
}

QTEST_MAIN(tst_QLowEnergyCharacteristic)

#include "tst_qlowenergycharacteristic.moc"
