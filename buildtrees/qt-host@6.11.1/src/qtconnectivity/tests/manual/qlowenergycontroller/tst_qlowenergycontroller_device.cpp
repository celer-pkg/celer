// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QObject>
#include <QtGlobal>
#include <QTest>
#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QSignalSpy>
#include <QLowEnergyController>
#include <QLoggingCategory>
#include <QScopeGuard>
#include <QBluetoothLocalDevice>

#if QT_CONFIG(permissions)
#include <QtCore/qcoreapplication.h>
#include <QtCore/qpermissions.h>
#include <QtCore/qnamespace.h>
#endif

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

static const QLatin1String repeatedWriteServiceUuid("72b12a31-98ea-406d-a89d-2c932d11ff67");
static const QLatin1String repeatedWriteTargetCharUuid("2192ee43-6d17-4e78-b286-db2c3b696833");
static const QLatin1String repeatedWriteNotifyCharUuid("b3f9d1a2-3d55-49c9-8b29-e09cec77ff86");


// With the defines below some test cases are picked on at compile-time as opposed to runtime skips
// to avoid the lengthy init() and clean() executions
#if defined(QT_ANDROID_BLUETOOTH) || defined(QT_WINRT_BLUETOOTH) || defined(Q_OS_DARWIN)
#define QT_BLUETOOTH_MTU_SUPPORTED
#endif

#if defined(QT_ANDROID_BLUETOOTH) || defined(Q_OS_DARWIN)
#define QT_BLUETOOTH_RSSI_SUPPORTED
#endif

#if defined(QT_BLUETOOTH_MTU_SUPPORTED)
static const QLatin1String mtuServiceUuid("9a9483eb-cf4f-4c32-9a6b-794238d5b483");
static const QLatin1String mtuCharUuid("960d7e2a-a850-4a70-8064-cd74e9ccb6ff");
#endif

/*
 * This class contains all unit tests for QLowEnergyController
 * which require a remote device and can thus not run completely automated.
 *
 * This testcase acts as a bluetooth LE *client*.
 * The counterpart *server* device is the "bluetoothtestdevice" in the same
 * repository and it needs to be ran when this testcase is ran.
 *
 * The name of the server device needs to be adjusted in this code so that
 * this LE client can find it. For example on macOS and Linux it is a generic
 * "BluetoothTestDevice" where as on Android it is the device's bluetooth name
 * set by the user. Please see the BTLE_SERVER_DEVICE_NAME below.
 */
class tst_qlowenergycontroller_device : public QObject
{
    Q_OBJECT

public:
    tst_qlowenergycontroller_device();
    ~tst_qlowenergycontroller_device();
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    // Keep readServerPlatform as the first test, later tests
    // may depend on its results.
    void readServerPlatform();

#if defined(QT_BLUETOOTH_MTU_SUPPORTED)
    void checkMtuNegotiation();
#endif
#if defined(QT_BLUETOOTH_RSSI_SUPPORTED)
    void rssiRead();
#endif
    void readWriteLargeCharacteristic();
    void readWriteLargeDescriptor();
    void readDuringServiceDiscovery();
    void readNotificationAndIndicationProperty();
    void testNotificationAndIndication();
    void testRepeatedCharacteristicsWrite();

public:
    void checkconnectionCounter(std::unique_ptr<QLowEnergyController> &control);
    static int connectionCounter;

private:
    void discoverTestServer();

    std::unique_ptr<QBluetoothDeviceDiscoveryAgent> mDevAgent;
    std::unique_ptr<QLowEnergyController> mController;
    QBluetoothDeviceInfo mRemoteDeviceInfo;
    QString mServerDeviceName;
    QByteArray mServerPlatform;
#if QT_CONFIG(permissions)
    Qt::PermissionStatus permissionStatus = Qt::PermissionStatus::Undetermined;
#endif
};

// connectionCounter is used to check that the server-side connect events
// occur as expected. On the first time when the value is the initial "-1"
// we read the current connection count from a service/characteristic providing it.
// This way we don't need to always restart the server-side for testing
int tst_qlowenergycontroller_device::connectionCounter = -1;

tst_qlowenergycontroller_device::tst_qlowenergycontroller_device()
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#if QT_CONFIG(permissions)
    // FIXME: Android has more specific BT permissions, fix when appropriate
    // change is done in qtbase
    permissionStatus = qApp->checkPermission(QBluetoothPermission{});
    if (qApp->checkPermission(QBluetoothPermission{}) == Qt::PermissionStatus::Undetermined) {
        QTestEventLoop loop;
        qApp->requestPermission(QBluetoothPermission{},
                                 &loop, [this, &loop](const QPermission &permission) {
            permissionStatus = permission.status();
            loop.exitLoop();
        });
        if (permissionStatus == Qt::PermissionStatus::Undetermined)
            loop.enterLoopMSecs(30000);
    }
#endif
}

tst_qlowenergycontroller_device::~tst_qlowenergycontroller_device() { }

void tst_qlowenergycontroller_device::initTestCase()
{
#if QT_CONFIG(permissions)
    if (permissionStatus != Qt::PermissionStatus::Granted)
        QSKIP("This manual test requires Blutooth permissions granted.");
#endif
    qDebug() << "Testcase build time: " << __TIME__;
    mDevAgent.reset(new QBluetoothDeviceDiscoveryAgent(this));
    mDevAgent->setLowEnergyDiscoveryTimeout(75000);
    mServerDeviceName = qEnvironmentVariable("BTLE_SERVER_DEVICE_NAME");
    if (mServerDeviceName.isEmpty())
        mServerDeviceName = QStringLiteral("BluetoothTestDevice");
    qDebug() << "Using server device name for testing: " << mServerDeviceName;
    qDebug() << "To change this set BTLE_SERVER_DEVICE_NAME environment variable";
}

void tst_qlowenergycontroller_device::discoverTestServer()
{
    mRemoteDeviceInfo = QBluetoothDeviceInfo(); // Invalidate whatever we had before
    QSignalSpy finishedSpy(mDevAgent.get(), SIGNAL(finished()));
    QSignalSpy canceledSpy(mDevAgent.get(), SIGNAL(canceled()));
    // there should be no changes yet
    QVERIFY(finishedSpy.isValid() && finishedSpy.isEmpty());
    QVERIFY(canceledSpy.isValid() && canceledSpy.isEmpty());

    QObject forLifeTime;
    QObject::connect(mDevAgent.get(), &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                     &forLifeTime, [&](const QBluetoothDeviceInfo& info) {
        if (info.name() == mServerDeviceName) {
            qDebug() << "Matching server device discovered, stopping device discovery agent";
            mDevAgent->stop();
        }
    });

    // Start device discovery
    mDevAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    QTRY_VERIFY_WITH_TIMEOUT(!finishedSpy.isEmpty() || !canceledSpy.isEmpty(), 80000);

    // Verify that we have found a matching server device
    bool deviceFound = false;
    const QList<QBluetoothDeviceInfo> infos = mDevAgent->discoveredDevices();
    for (const QBluetoothDeviceInfo &info : infos) {
        if (info.name() == mServerDeviceName) {
            mRemoteDeviceInfo = info;
            deviceFound = true;
            qDebug() << "Found a matching server device" << info.name() << info.address();
            break;
        }
        qDebug() << "Ignoring a non-matching device:" << info.name() << info.address();
    }
    QVERIFY2(deviceFound, "Cannot find remote device.");
}


void tst_qlowenergycontroller_device::init()
{
    // Connect to the test server. If unsuccessful, rediscover and retry.
    // Rediscovery may be needed as the address of the remote can change and
    // also at least Bluez backend recovers from its own internal errors
    // more likely making the test less flaky
    while (!mController || mController->state() != QLowEnergyController::ConnectedState) {
        discoverTestServer();
        QVERIFY(mRemoteDeviceInfo.isValid());
        mController.reset(QLowEnergyController::createCentral(mRemoteDeviceInfo));
        mController->connectToDevice();
        QTRY_VERIFY_WITH_TIMEOUT(mController->state() != QLowEnergyController::ConnectingState, 45000);
        if (mController->state() != QLowEnergyController::ConnectedState) {
            mController.reset();
            qDebug() << "Retrying connecting to the server in 5 seconds";
            QTest::qWait(5000);
        }
    }
    QCOMPARE(mController->state(), QLowEnergyController::ConnectedState);
    QCOMPARE(mController->error(), QLowEnergyController::NoError);
}

void tst_qlowenergycontroller_device::cleanup()
{
    mController->disconnectFromDevice();

    // Attempt a graceful close. If the test has already failed, the QTRY would exit immediately
    if (!QTest::currentTestFailed())
        QTRY_VERIFY_WITH_TIMEOUT(mController->state() ==
                                 QLowEnergyController::UnconnectedState, 30000);
    else
        QTest::qWait(2000);

    QCOMPARE(mController->state(), QLowEnergyController::UnconnectedState);
    qDebug() << "Disconnected from remote device, waiting 5s before deleting controller.";
    QTest::qWait(5000);
    mController.reset();
}

void tst_qlowenergycontroller_device::cleanupTestCase() { }

// The readServerPlatform should always be the first actual test function to execute.
// It reads a characteristic where the server reports on which platform it runs on.
// This information can then be used to adjust the test case's behavior accordingly;
// different platforms support slightly different things.
void tst_qlowenergycontroller_device::readServerPlatform()
{
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);
    QCOMPARE(mController->error(), QLowEnergyController::NoError);

    QSharedPointer<QLowEnergyService> service(
            mController->createServiceObject(QBluetoothUuid(platformIdentifierServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    auto characteristic = service->characteristic(QBluetoothUuid(platformIdentifierCharUuid));
    mServerPlatform = characteristic.value();
    qDebug() << "Server reported its running on: " << mServerPlatform;
    QVERIFY(!mServerPlatform.isEmpty());
}


#if defined(QT_BLUETOOTH_MTU_SUPPORTED)
void tst_qlowenergycontroller_device::checkMtuNegotiation()
{
    // service discovery, including MTU negotiation
    qDebug() << "Client-side MTU after connect" << mController->mtu();
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);
    QCOMPARE(mController->error(), QLowEnergyController::NoError);

    checkconnectionCounter(mController);

    // now a larger MTU should have been negotiated
    QTRY_VERIFY(mController->mtu() > 23);
    qDebug() << "MTU after service discovery" << mController->mtu();

    // check that central and peripheral agree on negotiated mtu
    QSharedPointer<QLowEnergyService> service(
            mController->createServiceObject(QBluetoothUuid(mtuServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    auto characteristic = service->characteristic(QBluetoothUuid(mtuCharUuid));
    int mtu;
    memcpy(&mtu, characteristic.value().constData(), sizeof(int));
    qDebug() << "MTU reported by server-side:" << mtu;
    // Server-side mtu is not supported on all platforms
    if (mServerPlatform != "linux" && mServerPlatform != "darwin")
        QCOMPARE(mtu, mController->mtu());
}
#endif
#undef QT_BLUETOOTH_MTU_SUPPORTED

#if defined(QT_BLUETOOTH_RSSI_SUPPORTED)

#define READ_AND_VERIFY_RSSI_VALUE \
        errorSpy.clear(); \
        rssiSpy.clear();  \
        mController->readRssi(); \
        QTRY_VERIFY(!rssiSpy.isEmpty()); \
        QVERIFY(errorSpy.isEmpty()); \
        rssi = rssiSpy.takeFirst().at(0).toInt(); \
        QCOMPARE_GE(rssi, -127); \
        QCOMPARE_LE(rssi, 127);  \

void tst_qlowenergycontroller_device::rssiRead()
{
    QSignalSpy errorSpy(mController.get(), &QLowEnergyController::errorOccurred);
    QSignalSpy rssiSpy(mController.get(), &QLowEnergyController::rssiRead);
    int rssi = 1000; // valid values are -127..127

    // 1) Read RSSI in connected state
    QCOMPARE(mController->state(), QLowEnergyController::ConnectedState);
    READ_AND_VERIFY_RSSI_VALUE;
    // Check that the value changes. This might seem flaky with stationary bluetooth
    // devices but in practice the RSSI constantly fluctuates
    const int initialRssi = rssi;
    for (int attempt = 1; attempt < 20; attempt++) {
        READ_AND_VERIFY_RSSI_VALUE;
        QTest::qWait(200); // Provide a bit time for the RSSI to change
        if (rssi != initialRssi)
            break;
    }
    QVERIFY(rssi != initialRssi);

    // 2) Read RSSI while discovering services
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QCOMPARE(mController->state(), QLowEnergyController::DiscoveringState);
    READ_AND_VERIFY_RSSI_VALUE;

    // 3) Read RSSI after services have been discovered
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);
    READ_AND_VERIFY_RSSI_VALUE;

    // 4) Read RSSI while discovering service details
    QSharedPointer<QLowEnergyService> service(mController->createServiceObject(
            QBluetoothUuid(repeatedWriteServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QCOMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovering);
    READ_AND_VERIFY_RSSI_VALUE;

    // 5) Read RSSI after service detail discovery
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);
    READ_AND_VERIFY_RSSI_VALUE;

    checkconnectionCounter(mController);

    // 6) Read RSSI amidst characteristic reads and writes
    QSignalSpy writeSpy(service.get(), &QLowEnergyService::characteristicWritten);
    QSignalSpy readSpy(service.get(), &QLowEnergyService::characteristicRead);
    auto characteristic = service->characteristic(QBluetoothUuid(repeatedWriteTargetCharUuid));
    QByteArray value(8, 'a');
    service->writeCharacteristic(characteristic, value);
    READ_AND_VERIFY_RSSI_VALUE;
    QTRY_VERIFY(!writeSpy.isEmpty());

    service->readCharacteristic(characteristic);
    READ_AND_VERIFY_RSSI_VALUE;
    QTRY_VERIFY(!readSpy.isEmpty());

    // A bit of stress-testing to check that read/write and
    // RSSI reading don't interfere with one another
    writeSpy.clear();
    readSpy.clear();
    rssiSpy.clear();
    errorSpy.clear();
    const int TIMES = 5;
    for (int i = 0; i < TIMES; i++) {
        mController->readRssi();
        service->writeCharacteristic(characteristic, value);
        mController->readRssi();
        service->readCharacteristic(characteristic);
    }
    QTRY_COMPARE(writeSpy.size(), TIMES);
    QTRY_COMPARE(readSpy.size(), TIMES);
#if defined(Q_OS_ANDROID)
    QTRY_COMPARE(rssiSpy.size(), TIMES * 2);
#else
    // On darwin several pending requests will get one callback
    QTRY_COMPARE_GE(rssiSpy.size(), 1);
#endif
    QVERIFY(errorSpy.isEmpty());

    // 7) Disconnect the device and verify we get error for RSSI read
    errorSpy.clear();
    rssiSpy.clear();
    mController->disconnectFromDevice();
    // First right after requesting disconnect
    mController->readRssi();
    QTRY_COMPARE(errorSpy.size(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).value<QLowEnergyController::Error>(),
             QLowEnergyController::Error::RssiReadError);
    QCOMPARE(mController->error(), QLowEnergyController::Error::RssiReadError);
    QVERIFY(rssiSpy.isEmpty());

    // Then once the disconnection is complete
    QTRY_COMPARE(mController->state(), QLowEnergyController::UnconnectedState);
    errorSpy.clear();
    mController->readRssi();
    QTRY_COMPARE(errorSpy.size(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).value<QLowEnergyController::Error>(),
             QLowEnergyController::Error::RssiReadError);
    QCOMPARE(mController->error(), QLowEnergyController::Error::RssiReadError);
    QVERIFY(rssiSpy.isEmpty());
}
#endif
#undef QT_BLUETOOTH_RSSI_SUPPORTED

void tst_qlowenergycontroller_device::checkconnectionCounter(
        std::unique_ptr<QLowEnergyController> &mController)
{
    // on darwin the server-side connect/disconnect events are not reported reliably
    if (mServerPlatform == "darwin")
        return;
    QSharedPointer<QLowEnergyService> service(
            mController->createServiceObject(QBluetoothUuid(connectionCountServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    auto counterCharacteristic = service->characteristic(QBluetoothUuid(connectionCountCharUuid));
    // If we have just started the test, read the current connection counter from the server.
    // The connection counter is essentially the number of "connected" events the server
    // has had.
    if (connectionCounter == -1) {
        memcpy(&connectionCounter, counterCharacteristic.value().constData(), sizeof(int));
        qDebug() << "Connection counter initialized from the server to:" << connectionCounter;
    } else {
        connectionCounter++;
        QByteArray value((const char *)&connectionCounter, sizeof(int));
        QCOMPARE(counterCharacteristic.value(), value);
    }
}

void tst_qlowenergycontroller_device::readWriteLargeCharacteristic()
{
    // This tests reading and writing a large characteristic value
    //
    // Most modern platforms cope with up-to 512 bytes. One exception is Bluez DBus peripheral,
    // which may reject a write from client when going above 508 due to the way it internally
    // checks the payload size (tested with Bluez 5.64).
    //
    // This test can also be used for testing long (aka prepared) writes & reads by setting
    // the MTU to for example 50 bytes. Asking a specific MTU is platform-specific and
    // there is no Qt API for it
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);

    checkconnectionCounter(mController);

    QSharedPointer<QLowEnergyService> service(
            mController->createServiceObject(QBluetoothUuid(largeAttributeServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::SkipValueDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    QSignalSpy readSpy(service.get(), &QLowEnergyService::characteristicRead);
    QSignalSpy writtenSpy(service.get(), &QLowEnergyService::characteristicWritten);

    // The service discovery skipped the values => check that the default values are all zeroes
    auto characteristic = service->characteristic(QBluetoothUuid(largeAttributeCharUuid));
    QVERIFY(characteristic.isValid());
    QByteArray testArray(0);
    qDebug() << "Initial large characteristic value:" << characteristic.value();
    QCOMPARE(characteristic.value(), testArray);

    // Read the characteristic value and verify it is the one the server-side sets (0x0b 0x00 ..)
    service->readCharacteristic(characteristic);
    QTRY_COMPARE(readSpy.size(), 1);
    qDebug() << "Large characteristic value after read:" << characteristic.value();
    testArray = QByteArray(largeAttributeSize, 0);
    testArray[0] = 0x0b;
    QCOMPARE(characteristic.value(), testArray);

    // Write a new value to characteristic and read it back
    for (int i = 0; i < largeAttributeSize; ++i) {
        testArray[i] = i % 5;
    }
    service->writeCharacteristic(characteristic, testArray);
    QCOMPARE(service->error(), QLowEnergyService::ServiceError::NoError);
    QTRY_COMPARE(writtenSpy.size(), 1);

    service->readCharacteristic(characteristic);
    QTRY_COMPARE(readSpy.size(), 2);
    qDebug() << "Large characteristic value after write/read:" << characteristic.value();
    QCOMPARE(characteristic.value(), testArray);
}

void tst_qlowenergycontroller_device::readWriteLargeDescriptor()
{
    // This tests reading and writing a large descriptor value
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);

    checkconnectionCounter(mController);

    QSharedPointer<QLowEnergyService> service(
            mController->createServiceObject(QBluetoothUuid(largeAttributeServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    QSignalSpy readSpy(service.get(), &QLowEnergyService::descriptorRead);
    QSignalSpy writtenSpy(service.get(), &QLowEnergyService::descriptorWritten);

    auto characteristic = service->characteristic(QBluetoothUuid(largeAttributeCharUuid));
    QVERIFY(characteristic.isValid());
    auto descriptor = characteristic.descriptor(QBluetoothUuid(largeAttributeDescUuid));
    QVERIFY(descriptor.isValid());

    QByteArray testArray = QByteArray(largeAttributeSize, 0);
    testArray[0] = 0xdd;

    // Read descriptor value and verify it is what the server set (0xdd 0x00 ..)
    QVERIFY(readSpy.isEmpty());
    QVERIFY(writtenSpy.isEmpty());
    service->readDescriptor(descriptor);
    QTRY_COMPARE(readSpy.size(), 1);
    qDebug() << "Large descriptor value after read:" << descriptor.value();
    QCOMPARE(descriptor.value(), testArray);

    // Write a new value to descriptor and read it back
    for (int i = 0; i < largeAttributeSize; ++i) {
        testArray[i] = i % 5;
    }
    service->writeDescriptor(descriptor, testArray);
    QCOMPARE(service->error(), QLowEnergyService::ServiceError::NoError);
    QTRY_COMPARE(writtenSpy.size(), 1);

    service->readDescriptor(descriptor);
    QTRY_COMPARE(readSpy.size(), 2);
    qDebug() << "Large descriptor value after write/read:" << descriptor.value();
    QCOMPARE(descriptor.value(), testArray);
}

void tst_qlowenergycontroller_device::readDuringServiceDiscovery()
{
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);

    checkconnectionCounter(mController);

    QSharedPointer<QLowEnergyService> service(
            mController->createServiceObject(QBluetoothUuid(largeAttributeServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    QSignalSpy readSpy(service.get(), &QLowEnergyService::characteristicRead);
    QSignalSpy writtenSpy(service.get(), &QLowEnergyService::characteristicWritten);
    QCOMPARE(readSpy.size(), 0);
    QCOMPARE(writtenSpy.size(), 0);

    // Value that is initially set on the characteristic at the server-side (0x0b 0x00 ..)
    QByteArray testArray(largeAttributeSize, 0);
    testArray[0] = 0x0b;

    // We did a full service discovery and should have an initial value to compare
    auto characteristic = service->characteristic(QBluetoothUuid(largeAttributeCharUuid));
    QByteArray valueFromServiceDiscovery = characteristic.value();
    qDebug() << "Large characteristic value from service discovery:" << valueFromServiceDiscovery;
    // On darwin the server does not restart (disconnect) in-between the case runs
    // and the initial value may be from a previous test function
    if (mServerPlatform != "darwin")
        QCOMPARE(characteristic.value(), testArray);

    // Check that the value from service discovery and explicit characteristic read match
    service->readCharacteristic(characteristic);
    QTRY_COMPARE(readSpy.size(), 1);
    qDebug() << "Large characteristic value after read:" << characteristic.value();
    QCOMPARE(characteristic.value(), valueFromServiceDiscovery);

    // Write a new value to the characteristic and read it back
    for (int i = 0; i < largeAttributeSize; ++i) {
        testArray[i] = i % 5;
    }
    service->writeCharacteristic(characteristic, testArray);
    QCOMPARE(service->error(), QLowEnergyService::ServiceError::NoError);
    QTRY_COMPARE(writtenSpy.size(), 1);

    service->readCharacteristic(characteristic);
    QTRY_COMPARE(readSpy.size(), 2);
    qDebug() << "Large characteristic value after write/read:" << characteristic.value();
    QCOMPARE(characteristic.value(), testArray);
}

void tst_qlowenergycontroller_device::readNotificationAndIndicationProperty()
{
    // discover services
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);

    checkconnectionCounter(mController);

    // check test service is available
    QVERIFY(mController->services().contains(QBluetoothUuid(notificationIndicationTestServiceUuid)));

    // get service object
    QSharedPointer<QLowEnergyService> service(mController->createServiceObject(
            QBluetoothUuid(notificationIndicationTestServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    // check that all four characteristics are found
    QCOMPARE(service->characteristics().size(), 4);

    // check that properties are correctly set
    auto notifyOrIndicate = QLowEnergyCharacteristic::PropertyType::Notify
            | QLowEnergyCharacteristic::PropertyType::Indicate;
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar1Uuid));
        QCOMPARE(characteristic.properties() & notifyOrIndicate, 0);
    }
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar2Uuid));
        QCOMPARE(characteristic.properties() & notifyOrIndicate,
                 QLowEnergyCharacteristic::PropertyType::Notify);
    }
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar3Uuid));
        QCOMPARE(characteristic.properties() & notifyOrIndicate,
                 QLowEnergyCharacteristic::PropertyType::Indicate);
    }
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar4Uuid));
        QCOMPARE(characteristic.properties() & notifyOrIndicate, notifyOrIndicate);
    }
}

void tst_qlowenergycontroller_device::testNotificationAndIndication()
{
    // discover services
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);

    checkconnectionCounter(mController);

    // get service object
    QSharedPointer<QLowEnergyService> service(mController->createServiceObject(
            QBluetoothUuid(notificationIndicationTestServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    // Verify that notification works
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar2Uuid));
        auto notifyOrIndicate = QLowEnergyCharacteristic::PropertyType::Notify
                | QLowEnergyCharacteristic::PropertyType::Indicate;
        QCOMPARE(characteristic.properties() & notifyOrIndicate,
                 QLowEnergyCharacteristic::PropertyType::Notify);

        // getting cccd
        QLowEnergyDescriptor cccd = characteristic.clientCharacteristicConfiguration();
        QVERIFY(cccd.isValid());

        // write to cccd
        bool cccdWritten = false;
        QObject dummy; // for lifetime management
        QObject::connect(
                service.get(), &QLowEnergyService::descriptorWritten, &dummy,
                [&cccdWritten](const QLowEnergyDescriptor &info, const QByteArray &) {
                    if (info.uuid()
                        == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration) {
                        cccdWritten = true;
                    }
                });
        service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);
        QTRY_VERIFY(cccdWritten);

        // notification should be enabled
        auto oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);
        // wait again
        oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);

        // disable notification
        cccdWritten = false;
        service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDDisable);
        QTRY_VERIFY(cccdWritten);

        // wait for a moment in case there is a value change just happening,
        // then check that there are no more notifications:
        QTest::qWait(200);
        oldvalue = characteristic.value();
        for (int i = 0; i < 3; ++i) {
            QTest::qWait(100);
            QCOMPARE(characteristic.value(), oldvalue);
        }
    }

    // Verify that indication works
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar3Uuid));
        auto notifyOrIndicate = QLowEnergyCharacteristic::PropertyType::Notify
                | QLowEnergyCharacteristic::PropertyType::Indicate;
        QCOMPARE(characteristic.properties() & notifyOrIndicate,
                 QLowEnergyCharacteristic::PropertyType::Indicate);

        // getting cccd
        QLowEnergyDescriptor cccd = characteristic.clientCharacteristicConfiguration();
        QVERIFY(cccd.isValid());

        // write to cccd
        bool cccdWritten = false;
        QObject dummy; // for lifetime management
        QObject::connect(
                service.get(), &QLowEnergyService::descriptorWritten, &dummy,
                [&cccdWritten](const QLowEnergyDescriptor &info, const QByteArray &) {
                    if (info.uuid()
                        == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration) {
                        cccdWritten = true;
                    }
                });
        QByteArray newValue = QByteArray::fromHex("0200");
        service->writeDescriptor(cccd, newValue);
        QTRY_VERIFY(cccdWritten);

        // indication should be enabled
        auto oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);
        // wait again
        oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);

        // disable indication
        cccdWritten = false;
        newValue = QByteArray::fromHex("0000");
        service->writeDescriptor(cccd, newValue);
        QTRY_VERIFY(cccdWritten);

        // wait for a moment in case there is a value change just happening,
        // then check that there are no more notifications:
        QTest::qWait(200);
        oldvalue = characteristic.value();
        for (int i = 0; i < 3; ++i) {
            QTest::qWait(100);
            QCOMPARE(characteristic.value(), oldvalue);
        }
    }

    // indication and notification works
#if defined(Q_OS_LINUX)
    // If the client (this testcase) is linux and the characteristic has both
    // NTF & IND supported, the Bluez stack will try to enable both NTF & IND
    // at the same time regardless of what we write to CCCD. Darwin server will
    // reject this as an error (ATT error 0xf5, which is a reserved error)
    if (mServerPlatform != "darwin")
#endif
    {
        QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(notificationIndicationTestChar4Uuid));
        auto notifyOrIndicate = QLowEnergyCharacteristic::PropertyType::Notify
                | QLowEnergyCharacteristic::PropertyType::Indicate;
        QCOMPARE(characteristic.properties() & notifyOrIndicate, notifyOrIndicate);

        // getting cccd
        QLowEnergyDescriptor cccd = characteristic.clientCharacteristicConfiguration();
        QVERIFY(cccd.isValid());

        // write to cccd
        bool cccdWritten = false;
        QObject dummy; // for lifetime management
        QObject::connect(
                service.get(), &QLowEnergyService::descriptorWritten, &dummy,
                [&cccdWritten](const QLowEnergyDescriptor &info, const QByteArray &) {
                    if (info.uuid()
                        == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration) {
                        cccdWritten = true;
                    }
                });
        QByteArray newValue = QByteArray::fromHex("0100");
        service->writeDescriptor(cccd, newValue);
        QTRY_VERIFY(cccdWritten);

        // notification should be enabled
        auto oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);
        // wait again
        oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);

        // disable notification
        cccdWritten = false;
        newValue = QByteArray::fromHex("0000");
        service->writeDescriptor(cccd, newValue);
        QTRY_VERIFY(cccdWritten);

        // wait for a moment in case there is a value change just happening,
        // then check that there are no more notifications:
        QTest::qWait(200);
        oldvalue = characteristic.value();
        for (int i = 0; i < 3; ++i) {
            QTest::qWait(100);
            QCOMPARE(characteristic.value(), oldvalue);
        }

        newValue = QByteArray::fromHex("0200");
        service->writeDescriptor(cccd, newValue);
        QTRY_VERIFY(cccdWritten);

        // indication should be enabled
        oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);
        // wait again
        oldvalue = characteristic.value();
        QCOMPARE(characteristic.value(), oldvalue);
        // wait for change
        QTRY_VERIFY(characteristic.value() != oldvalue);

        // disable indication
        cccdWritten = false;
        newValue = QByteArray::fromHex("0000");
        service->writeDescriptor(cccd, newValue);
        QTRY_VERIFY(cccdWritten);

        // wait for a moment in case there is a value change just happening,
        // then check that there are no more indications:
        QTest::qWait(200);
        oldvalue = characteristic.value();
        for (int i = 0; i < 3; ++i) {
            QTest::qWait(100);
            QCOMPARE(characteristic.value(), oldvalue);
        }
    }
}

void tst_qlowenergycontroller_device::testRepeatedCharacteristicsWrite()
{
    // This test generates multiple consecutive writes to the same characteristic
    // and waits for the notifications (on other characteristic) with the same
    // values. After that it verifies that the received values are the same (and
    // in the same order) as written values. The server writes each received
    // value to a notifying characteristic, which allows us to perform the check.

    // Discover services
    QVERIFY(mController->services().isEmpty());
    mController->discoverServices();
    QTRY_COMPARE(mController->state(), QLowEnergyController::DiscoveredState);

    checkconnectionCounter(mController);

    // Get service object.
    QSharedPointer<QLowEnergyService> service(mController->createServiceObject(
            QBluetoothUuid(repeatedWriteServiceUuid)));
    QVERIFY(service != nullptr);
    service->discoverDetails(QLowEnergyService::FullDiscovery);
    QTRY_COMPARE(service->state(), QLowEnergyService::ServiceState::RemoteServiceDiscovered);

    // Enable notification.
    QLowEnergyCharacteristic notifyChar =
            service->characteristic(QBluetoothUuid(repeatedWriteNotifyCharUuid));
    const auto notifyOrIndicate = QLowEnergyCharacteristic::PropertyType::Notify
            | QLowEnergyCharacteristic::PropertyType::Indicate;
    QCOMPARE(notifyChar.properties() & notifyOrIndicate,
             QLowEnergyCharacteristic::PropertyType::Notify);

    QLowEnergyDescriptor cccd = notifyChar.clientCharacteristicConfiguration();
    QVERIFY(cccd.isValid());

    QObject dummy; // for lifetime management
    bool cccdWritten = false;
    connect(service.get(), &QLowEnergyService::descriptorWritten, &dummy,
            [&cccdWritten](const QLowEnergyDescriptor &info, const QByteArray &) {
                if (info.uuid()
                    == QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration) {
                    cccdWritten = true;
                }
            });
    service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);
    QTRY_VERIFY(cccdWritten);

    // Track the notifications of value changes.
    QList<QByteArray> receivedValues;
    connect(service.get(), &QLowEnergyService::characteristicChanged, &dummy,
            [&receivedValues](const QLowEnergyCharacteristic &characteristic,
                              const QByteArray &value)
    {
        if (characteristic.uuid() == QBluetoothUuid(repeatedWriteNotifyCharUuid)) {
            receivedValues.push_back(value);
        }
    });

    // Write characteristics multiple times. This shouldn't crash, and all
    // values should be written. We use the notifications to track it.
    receivedValues.clear();
    QList<QByteArray> sentValues;
    QLowEnergyCharacteristic writeChar =
            service->characteristic(QBluetoothUuid(repeatedWriteTargetCharUuid));
    static const int totalWrites = 50;
    QByteArray value(8, 0);
    for (int i = 0; i < totalWrites; ++i) {
        value[0] += 1;
        value[7] += 1;
        service->writeCharacteristic(writeChar, value);
        sentValues.push_back(value);
    }

    // We expect to get notifications about all writes.
    // We set a large timeout to be on a safe side.
    QTRY_COMPARE_WITH_TIMEOUT(receivedValues.size(), totalWrites, 60000);
    QCOMPARE(receivedValues, sentValues);
}

QTEST_MAIN(tst_qlowenergycontroller_device)

#include "tst_qlowenergycontroller_device.moc"
