// Copyright (C) 2016 BlackBerry Limited all rights reserved
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QUuid>

#include <QDebug>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyDescriptor>
#include <QLowEnergyController>
#include <QBluetoothLocalDevice>

#if QT_CONFIG(permissions)
#include <QCoreApplication>
#include <QPermissions>
#include <QtCore/qnamespace.h>
#endif

QT_USE_NAMESPACE

class tst_QLowEnergyDescriptor : public QObject
{
    Q_OBJECT

public:
    tst_QLowEnergyDescriptor();
    ~tst_QLowEnergyDescriptor();

protected slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void tst_constructionDefault();
    void tst_assignCompare();

private:
    QList<QBluetoothDeviceInfo> remoteLeDeviceInfos;
    QLowEnergyController *globalControl;
    QLowEnergyService *globalService;
#if QT_CONFIG(permissions)
    Qt::PermissionStatus permissionStatus = Qt::PermissionStatus::Undetermined;
#endif
};

tst_QLowEnergyDescriptor::tst_QLowEnergyDescriptor() :
    globalControl(nullptr), globalService(nullptr)
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#if QT_CONFIG(permissions)
    permissionStatus = qApp->checkPermission(QBluetoothPermission{});
    const bool ciRun = qEnvironmentVariable("QTEST_ENVIRONMENT").split(' ').contains("ci");
    if (!ciRun && permissionStatus == Qt::PermissionStatus::Undetermined) {
        QTestEventLoop loop;
        qApp->requestPermission(QBluetoothPermission{},
                                &loop, [this, &loop](const QPermission &permission) {
            permissionStatus = permission.status();
            loop.exitLoop();
        });
        if (permissionStatus == Qt::PermissionStatus::Undetermined)
            loop.enterLoopMSecs(30000);
    }
#endif // QT_CONFIG(permissions)
}

tst_QLowEnergyDescriptor::~tst_QLowEnergyDescriptor()
{
}

void tst_QLowEnergyDescriptor::initTestCase()
{
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

#if QT_CONFIG(permissions)
    if (permissionStatus != Qt::PermissionStatus::Granted) {
        qWarning("Use of BLuetooth LE requires the Bluetooth permission granted");
        return;
    }
#endif

    // find an arbitrary low energy device in vicinity
    // find an arbitrary service with descriptor

    QBluetoothDeviceDiscoveryAgent *devAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(devAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

    QSignalSpy errorSpy(devAgent, SIGNAL(errorOccurred(QBluetoothDeviceDiscoveryAgent::Error)));
    QVERIFY(errorSpy.isValid());
    QVERIFY(errorSpy.isEmpty());

    QSignalSpy spy(devAgent, SIGNAL(finished()));
    // there should be no changes yet
    QVERIFY(spy.isValid());
    QVERIFY(spy.isEmpty());

    devAgent->start();
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() > 0, 100000);

    // find first service with descriptor
    QLowEnergyController *controller = nullptr;
    for (const QBluetoothDeviceInfo& remoteDeviceInfo : std::as_const(remoteLeDeviceInfos)) {
        controller = QLowEnergyController::createCentral(remoteDeviceInfo, this);
        qDebug() << "Connecting to" << remoteDeviceInfo.address();
        controller->connectToDevice();
        QTRY_IMPL(controller->state() != QLowEnergyController::ConnectingState,
                  50000)
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
                const QList<QLowEnergyDescriptor> descriptors = ch.descriptors();
                for (const QLowEnergyDescriptor &d : descriptors) {
                    if (!d.value().isEmpty()) {
                        globalService = leService;
                        globalControl = controller;
                        qWarning() << "Found service with descriptor" << remoteDeviceInfo.address()
                                   << globalService->serviceName() << globalService->serviceUuid();
                        break;
                    }
                }
                if (globalControl)
                    break;
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

void tst_QLowEnergyDescriptor::cleanupTestCase()
{
    if (globalControl)
        globalControl->disconnectFromDevice();
}

void tst_QLowEnergyDescriptor::deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
        remoteLeDeviceInfos.append(info);
}

void tst_QLowEnergyDescriptor::tst_constructionDefault()
{
    QLowEnergyDescriptor descriptor;
    QVERIFY(!descriptor.isValid());
    QCOMPARE(descriptor.value(), QByteArray());
    QVERIFY(descriptor.uuid().isNull());
    QCOMPARE(descriptor.name(), QString());
    QCOMPARE(descriptor.type(), QBluetoothUuid::DescriptorType::UnknownDescriptorType);

    QLowEnergyDescriptor copyConstructed(descriptor);
    QVERIFY(!copyConstructed.isValid());
    QCOMPARE(copyConstructed.value(), QByteArray());
    QVERIFY(copyConstructed.uuid().isNull());
    QCOMPARE(copyConstructed.name(), QString());
    QCOMPARE(copyConstructed.type(), QBluetoothUuid::DescriptorType::UnknownDescriptorType);

    QVERIFY(copyConstructed == descriptor);
    QVERIFY(descriptor == copyConstructed);
    QVERIFY(!(copyConstructed != descriptor));
    QVERIFY(!(descriptor != copyConstructed));

    QLowEnergyDescriptor assigned;

    QVERIFY(assigned == descriptor);
    QVERIFY(descriptor == assigned);
    QVERIFY(!(assigned != descriptor));
    QVERIFY(!(descriptor != assigned));

    assigned = descriptor;
    QVERIFY(!assigned.isValid());
    QCOMPARE(assigned.value(), QByteArray());
    QVERIFY(assigned.uuid().isNull());
    QCOMPARE(assigned.name(), QString());
    QCOMPARE(assigned.type(), QBluetoothUuid::DescriptorType::UnknownDescriptorType);

    QVERIFY(assigned == descriptor);
    QVERIFY(descriptor == assigned);
    QVERIFY(!(assigned != descriptor));
    QVERIFY(!(descriptor != assigned));
}


void tst_QLowEnergyDescriptor::tst_assignCompare()
{
    //find the descriptor
    if (!globalService)
        QSKIP("No descriptor found.");

    QLowEnergyDescriptor target;
    QVERIFY(!target.isValid());
    QCOMPARE(target.type(), QBluetoothUuid::DescriptorType::UnknownDescriptorType);
    QCOMPARE(target.name(), QString());
    QCOMPARE(target.uuid(), QBluetoothUuid());
    QCOMPARE(target.value(), QByteArray());

    qsizetype index = 0;
    bool valueFound = false;
    QList<QLowEnergyDescriptor> targets;
    const QList<QLowEnergyCharacteristic> chars = globalService->characteristics();
    for (const QLowEnergyCharacteristic &ch : chars) {
        if (!ch.descriptors().isEmpty()) {
           targets = ch.descriptors();
           for (qsizetype i = 0; i < targets.size(); ++i) {
               // try to get a descriptor we can read
               if (targets[i].type() == QBluetoothUuid::DescriptorType::CharacteristicUserDescription) {
                   index = i;
                   valueFound = true;
                   break;
               }
           }
           break;
        }
    }

    if (targets.isEmpty())
        QSKIP("No descriptor found despite prior indication.");

    // test assignment operator
    target = targets[index];
    QVERIFY(target.isValid());
    QVERIFY(target.type() != QBluetoothUuid::DescriptorType::UnknownDescriptorType);
    QVERIFY(!target.name().isEmpty());
    QVERIFY(!target.uuid().isNull());
    QVERIFY(!valueFound || !target.value().isEmpty());

    QVERIFY(target == targets[index]);
    QVERIFY(targets[index] == target);
    QVERIFY(!(target != targets[index]));
    QVERIFY(!(targets[index] != target));

    QCOMPARE(target.isValid(), targets[index].isValid());
    QCOMPARE(target.type(), targets[index].type());
    QCOMPARE(target.name(), targets[index].name());
    QCOMPARE(target.uuid(), targets[index].uuid());
    QCOMPARE(target.value(), targets[index].value());

    // test copy constructor
    QLowEnergyDescriptor copyConstructed(target);
    QCOMPARE(copyConstructed.isValid(), targets[index].isValid());
    QCOMPARE(copyConstructed.type(), targets[index].type());
    QCOMPARE(copyConstructed.name(), targets[index].name());
    QCOMPARE(copyConstructed.uuid(), targets[index].uuid());
    QCOMPARE(copyConstructed.value(), targets[index].value());

    QVERIFY(copyConstructed == target);
    QVERIFY(target == copyConstructed);
    QVERIFY(!(copyConstructed != target));
    QVERIFY(!(target != copyConstructed));

    // test invalidation
    QLowEnergyDescriptor invalid;
    target = invalid;
    QVERIFY(!target.isValid());
    QCOMPARE(target.value(), QByteArray());
    QVERIFY(target.uuid().isNull());
    QCOMPARE(target.name(), QString());
    QCOMPARE(target.type(), QBluetoothUuid::DescriptorType::UnknownDescriptorType);

    QVERIFY(invalid == target);
    QVERIFY(target == invalid);
    QVERIFY(!(invalid != target));
    QVERIFY(!(target != invalid));

    QVERIFY(!(targets[index] == target));
    QVERIFY(!(target == targets[index]));
    QVERIFY(targets[index] != target);
    QVERIFY(target != targets[index]);

    if (targets.size() >= 2) {
        QLowEnergyDescriptor second = targets[(index+1)%2];
        // at least two descriptors
        QVERIFY(!(targets[index] == second));
        QVERIFY(!(second == targets[index]));
        QVERIFY(targets[index] != second);
        QVERIFY(second != targets[index]);
    }
}

QTEST_MAIN(tst_QLowEnergyDescriptor)

#include "tst_qlowenergydescriptor.moc"

