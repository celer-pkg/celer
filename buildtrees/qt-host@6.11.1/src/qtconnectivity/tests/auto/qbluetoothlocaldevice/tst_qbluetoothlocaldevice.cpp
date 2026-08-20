// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QDebug>
#include <QVariant>
#include "../../shared/bttestutil_p.h"

#include <private/qtbluetoothglobal_p.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>

QT_USE_NAMESPACE

/*
 * Running the manual tests requires another Bluetooth device in the vincinity.
 * The remote device's address must be passed via the BT_TEST_DEVICE env variable.
 * Every pairing request must be accepted within a 10s interval of appearing.
 * If BT_TEST_DEVICE is not set manual tests will be skipped.
  **/

class tst_QBluetoothLocalDevice : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothLocalDevice();
    ~tst_QBluetoothLocalDevice();

private slots:
    void initTestCase();
    void tst_powerOn();
    void tst_powerOff();
    void tst_hostModes();
    void tst_hostModes_data();
    void tst_address();
    void tst_name();
    void tst_isValid();
    void tst_allDevices();
    void tst_construction();
    void tst_pairingStatus_data();
    void tst_pairingStatus();
    void tst_pairDevice_data();
    void tst_pairDevice();
    void tst_connectedDevices();

private:
    QBluetoothAddress remoteDevice;
    qsizetype numDevices = 0;
    bool expectRemoteDevice = false;
};

tst_QBluetoothLocalDevice::tst_QBluetoothLocalDevice()
{
    if (androidBluetoothEmulator())
        return;
    numDevices = QBluetoothLocalDevice::allDevices().size();
    const QString remote = qgetenv("BT_TEST_DEVICE");
    if (!remote.isEmpty()) {
        remoteDevice = QBluetoothAddress(remote);
        expectRemoteDevice = true;
        qWarning() << "Using remote device " << remote << " for testing. Ensure that the device is discoverable for pairing requests";
    } else {
        qWarning() << "Not using any remote device for testing. Set BT_TEST_DEVICE env to run manual tests involving a remote device";
    }
}

tst_QBluetoothLocalDevice::~tst_QBluetoothLocalDevice()
{
}

void tst_QBluetoothLocalDevice::initTestCase()
{
    if (expectRemoteDevice) {
        //test passed Bt address here since we cannot do that in the ctor
        QVERIFY2(!remoteDevice.isNull(), "BT_TEST_DEVICE is not a valid Bluetooth address" );
    }
}

void tst_QBluetoothLocalDevice::tst_powerOn()
{
    if (androidBluetoothEmulator())
        QSKIP("Skipping test on Android 12+ emulator, CI can timeout waiting for user input");
#ifdef Q_OS_MACOS
    QSKIP("Not possible on OS X");
#endif
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    if (localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff) {
        // Ensure device is OFF so we can test switching it ON
        localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
        // On Android user may need to authorize the transition, hence a longer timeout
        QTRY_VERIFY_WITH_TIMEOUT(localDevice.hostMode()
                                 == QBluetoothLocalDevice::HostPoweredOff, 15000);
        // Allow possible mode-change signal(s) to arrive (QTRY_COMPARE polls the
        // host mode in a loop, and thus may return before the host mode change signal)
        QTest::qWait(1000);
    }

    QSignalSpy hostModeSpy(&localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    // there should be no changes yet
    QVERIFY(hostModeSpy.isValid());
    QVERIFY(hostModeSpy.isEmpty());

    localDevice.powerOn();
    // On Android user may need to authorize the transition => longer timeout.
    QTRY_VERIFY_WITH_TIMEOUT(!hostModeSpy.isEmpty(), 15000);
    QVERIFY(localDevice.hostMode()
                             != QBluetoothLocalDevice::HostPoweredOff);
}

void tst_QBluetoothLocalDevice::tst_powerOff()
{
    if (androidBluetoothEmulator())
        QSKIP("Skipping test on Android 12+ emulator, CI can timeout waiting for user input");
#ifdef Q_OS_MACOS
    QSKIP("Not possible on OS X");
#endif
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        // Ensure device is ON so we can test switching it OFF
        localDevice.powerOn();
        // On Android user may need to authorize the transition => longer timeout.
        QTRY_VERIFY_WITH_TIMEOUT(localDevice.hostMode()
                                 != QBluetoothLocalDevice::HostPoweredOff, 15000);
        // Allow possible mode-change signal(s) to arrive (QTRY_COMPARE polls the
        // host mode in a loop, and thus may return before the host mode change signal)
        QTest::qWait(1000);
    }

    QSignalSpy hostModeSpy(&localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    // there should be no changes yet
    QVERIFY(hostModeSpy.isValid());
    QVERIFY(hostModeSpy.isEmpty());

    localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    // On Android user may need to authorize the transition => longer timeout.
    QTRY_VERIFY_WITH_TIMEOUT(!hostModeSpy.isEmpty(), 15000);
    QVERIFY(localDevice.hostMode()
                             == QBluetoothLocalDevice::HostPoweredOff);
}

void tst_QBluetoothLocalDevice::tst_hostModes_data()
{
    QTest::addColumn<QBluetoothLocalDevice::HostMode>("hostModeExpected");
    QTest::addColumn<bool>("expectSignal");
#if defined(Q_OS_WIN)
    // On Windows local device does not support HostDiscoverable as a separate mode
    QTest::newRow("HostPoweredOff1") << QBluetoothLocalDevice::HostPoweredOff << false;
    QTest::newRow("HostConnectable1") << QBluetoothLocalDevice::HostConnectable << true;
    QTest::newRow("HostConnectable2") << QBluetoothLocalDevice::HostConnectable << false;
    QTest::newRow("HostPoweredOff3") << QBluetoothLocalDevice::HostPoweredOff << true;
    QTest::newRow("HostPoweredOff3") << QBluetoothLocalDevice::HostPoweredOff << false;
    return;
#elif defined(Q_OS_ANDROID)
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 31) {
        // On Android-12 (API Level 31+) it seems the device's bluetooth visibility setting
        // defines if we enter "HostDiscoverable" (visible true) or "HostConnectable"
        // (visible false). Here we assume that the visibility setting is true. For lower
        // Android versions the default testdata rows are fine
        qDebug() << "On this Android version the bluetooth visibility setting is assumed true";
        QTest::newRow("HostDiscoverable1") << QBluetoothLocalDevice::HostDiscoverable << true;
        QTest::newRow("HostPoweredOff1") << QBluetoothLocalDevice::HostPoweredOff << true;
        QTest::newRow("HostPoweredOff2") << QBluetoothLocalDevice::HostPoweredOff << false;
        QTest::newRow("HostDiscoverable2") << QBluetoothLocalDevice::HostDiscoverable << true;
        QTest::newRow("HostPoweredOff3") << QBluetoothLocalDevice::HostPoweredOff << true;
        QTest::newRow("HostDiscoverable3") << QBluetoothLocalDevice::HostDiscoverable << true;
        QTest::newRow("HostDiscoverable4") << QBluetoothLocalDevice::HostDiscoverable << false;
        return;
    }
#endif
    QTest::newRow("HostDiscoverable1") << QBluetoothLocalDevice::HostDiscoverable << true;
    QTest::newRow("HostPoweredOff1") << QBluetoothLocalDevice::HostPoweredOff << true;
    QTest::newRow("HostPoweredOff2") << QBluetoothLocalDevice::HostPoweredOff << false;
    QTest::newRow("HostConnectable1") << QBluetoothLocalDevice::HostConnectable << true;
    QTest::newRow("HostConnectable2") << QBluetoothLocalDevice::HostConnectable << false;
    QTest::newRow("HostDiscoverable2") << QBluetoothLocalDevice::HostDiscoverable << true;
    QTest::newRow("HostConnectable3") << QBluetoothLocalDevice::HostConnectable << true;
    QTest::newRow("HostPoweredOff3") << QBluetoothLocalDevice::HostPoweredOff << true;
    QTest::newRow("HostDiscoverable3") << QBluetoothLocalDevice::HostDiscoverable << true;
    QTest::newRow("HostDiscoverable4") << QBluetoothLocalDevice::HostDiscoverable << false;
    QTest::newRow("HostConnectable4") << QBluetoothLocalDevice::HostConnectable << true;
}

void tst_QBluetoothLocalDevice::tst_hostModes()
{
    if (androidBluetoothEmulator())
        QSKIP("Skipping test on Android 12+ emulator, CI can timeout waiting for user input");
#ifdef Q_OS_MACOS
    QSKIP("Not possible on OS X");
#endif
    QFETCH(QBluetoothLocalDevice::HostMode, hostModeExpected);
    QFETCH(bool, expectSignal);

    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;

    static bool firstIteration = true;
    if (firstIteration) {
        // On the first iteration establish a known hostmode so that the test
        // function can reliably test changes to it
        firstIteration = false;
        if (localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff) {
            localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
            // On Android user may need to authorize the transition => longer timeout.
            QTRY_VERIFY_WITH_TIMEOUT(localDevice.hostMode()
                                     == QBluetoothLocalDevice::HostPoweredOff, 15000);
            // Allow possible mode-change signal(s) to arrive (QTRY_COMPARE polls the
            // host mode in a loop, and thus may return before the host mode change signal).
            QTest::qWait(1000);
        }
    }

    QSignalSpy hostModeSpy(&localDevice,
                           SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    // there should be no changes yet
    QVERIFY(hostModeSpy.isValid());
    QVERIFY(hostModeSpy.isEmpty());

    // Switch the bluetooth mode and verify it changes
    localDevice.setHostMode(hostModeExpected);
    // Manual interaction may be needed (for example on Android you may
    // need to authorize a permission) => hence a longer timeout.
    // If you see a fail on Android here, please see the comment in _data()
    QTRY_COMPARE_WITH_TIMEOUT(localDevice.hostMode(), hostModeExpected, 15000);
    // Allow possible mode-change signal(s) to arrive (QTRY_COMPARE polls the
    // host mode in a loop, and thus may return before the host mode change signal).
    QTest::qWait(1000);

    // Verify that signals are as expected
    if (expectSignal) {
        QVERIFY(hostModeSpy.size() > 0);
        // Verify that the last signal contained the right mode
        auto arguments = hostModeSpy.takeLast();
        auto hostMode = qvariant_cast<QBluetoothLocalDevice::HostMode>(arguments.at(0));
        QCOMPARE(hostMode, hostModeExpected);
    } else {
        QCOMPARE(hostModeSpy.size(), 0);
    }
}

void tst_QBluetoothLocalDevice::tst_address()
{
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    QVERIFY(!localDevice.address().toString().isEmpty());
    QVERIFY(!localDevice.address().isNull());
}
void tst_QBluetoothLocalDevice::tst_name()
{
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    QVERIFY(!localDevice.name().isEmpty());
}
void tst_QBluetoothLocalDevice::tst_isValid()
{
    if (androidBluetoothEmulator())
        QSKIP("Skipping test on Android 12+ emulator, CI can timeout waiting for user input");
#if defined(Q_OS_MACOS) || QT_CONFIG(winrt_bt)
    // On OS X we can have a valid device (device.isValid() == true),
    // that has neither a name nor a valid address - this happens
    // if a Bluetooth adapter is OFF.
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");
#endif

    QBluetoothLocalDevice localDevice;
    QBluetoothAddress invalidAddress("FF:FF:FF:FF:FF:FF");

    const QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    if (!devices.isEmpty()) {
        QVERIFY(localDevice.isValid());
        bool defaultFound = false;
        for (qsizetype i = 0; i < devices.size(); ++i) {
            QVERIFY(devices.at(i).address() != invalidAddress);
            if (devices.at(i).address() == localDevice.address() ) {
                defaultFound = true;
            } else {
                QBluetoothLocalDevice otherDevice(devices.at(i).address());
                QVERIFY(otherDevice.isValid());
            }
        }
        QVERIFY(defaultFound);
    } else {
        QVERIFY(!localDevice.isValid());
    }

    //ensure common behavior of invalid local device
    QBluetoothLocalDevice invalidLocalDevice(invalidAddress);
    QVERIFY(!invalidLocalDevice.isValid());
    QCOMPARE(invalidLocalDevice.address(), QBluetoothAddress());
    QCOMPARE(invalidLocalDevice.name(), QString());
    QCOMPARE(invalidLocalDevice.pairingStatus(QBluetoothAddress()), QBluetoothLocalDevice::Unpaired );
    QCOMPARE(invalidLocalDevice.hostMode(), QBluetoothLocalDevice::HostPoweredOff);
}

void tst_QBluetoothLocalDevice::tst_allDevices()
{
    //nothing we can really test here
}
void tst_QBluetoothLocalDevice::tst_construction()
{
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    QVERIFY(localDevice.isValid());

    QBluetoothLocalDevice anotherDevice(QBluetoothAddress(000000000000));
    QVERIFY(anotherDevice.isValid());
    QVERIFY(anotherDevice.address().toUInt64() != 0);

}

void tst_QBluetoothLocalDevice::tst_pairDevice_data()
{
    QTest::addColumn<QBluetoothAddress>("deviceAddress");
    QTest::addColumn<QBluetoothLocalDevice::Pairing>("pairingExpected");
    //waiting time larger for manual tests -> requires device interaction
    QTest::addColumn<int>("pairingWaitTime");
    QTest::addColumn<bool>("expectErrorSignal");

    QTest::newRow("UnPaired Device: DUMMY->unpaired") << QBluetoothAddress("11:00:00:00:00:00")
            << QBluetoothLocalDevice::Unpaired << 1000 << false;
    //Bluez5 may have to do a device search which can take up to 20s
    QTest::newRow("UnPaired Device: DUMMY->paired") << QBluetoothAddress("11:00:00:00:00:00")
            << QBluetoothLocalDevice::Paired << 21000 << true;
    QTest::newRow("UnPaired Device: DUMMY") << QBluetoothAddress()
            << QBluetoothLocalDevice::Unpaired << 1000 << true;

    if (!remoteDevice.isNull()) {
        // Unpairing is quick but pairing level upgrade requires manual interaction
        // on both devices. Therefore the timeouts are higher for the changes
        // which require manual interaction.
        QTest::newRow("UnParing Test device 1") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Unpaired << 5000 << false;
        //Bluez5 may have to do a device search which can take up to 20s
        QTest::newRow("Pairing Test Device") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Paired << 30000 << false;
        QTest::newRow("Pairing upgrade for Authorization") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::AuthorizedPaired << 5000 << false;
        QTest::newRow("Unpairing Test device 2") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Unpaired << 5000 << false;
        QTest::newRow("Authorized Pairing") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::AuthorizedPaired << 30000 << false;
        QTest::newRow("Pairing Test Device after Authorization Pairing") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Paired << 5000 << false;

        QTest::newRow("Pairing Test Device after Authorization2") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Paired << 5000 << false; //same again
        QTest::newRow("Unpairing Test device 3") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Unpaired << 5000 << false;
        QTest::newRow("UnParing Test device 4") << QBluetoothAddress(remoteDevice)
                << QBluetoothLocalDevice::Unpaired << 5000 << false;
    }
}

void tst_QBluetoothLocalDevice::tst_pairDevice()
{
    if (androidBluetoothEmulator())
        QSKIP("Skipping test on Android 12+ emulator, CI can timeout waiting for user input");
#if defined(Q_OS_MACOS)
    QSKIP("The pair device test fails on macOS");
#endif
    QFETCH(QBluetoothAddress, deviceAddress);
    QFETCH(QBluetoothLocalDevice::Pairing, pairingExpected);
    QFETCH(int, pairingWaitTime);
    QFETCH(bool, expectErrorSignal);

    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    //powerOn if not already
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        localDevice.powerOn();
        QTRY_VERIFY(localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff);
    }

    QSignalSpy pairingSpy(&localDevice, SIGNAL(pairingFinished(QBluetoothAddress,QBluetoothLocalDevice::Pairing)) );
    QSignalSpy errorSpy(&localDevice, SIGNAL(errorOccurred(QBluetoothLocalDevice::Error)));
    // there should be no signals yet
    QVERIFY(pairingSpy.isValid());
    QVERIFY(pairingSpy.isEmpty());
    QVERIFY(errorSpy.isValid());
    QVERIFY(errorSpy.isEmpty());

    QVERIFY(localDevice.isValid());

    localDevice.requestPairing(deviceAddress, pairingExpected);

    // The above function triggers async interaction with the user on two machines.
    // Responding takes time. Let's adjust the subsequent timeout dyncamically based on
    // the need of the operation
    if (expectErrorSignal) {
        QTRY_VERIFY_WITH_TIMEOUT(!errorSpy.isEmpty(), pairingWaitTime);
        QVERIFY(pairingSpy.isEmpty());
        QList<QVariant> arguments = errorSpy.first();
        QBluetoothLocalDevice::Error e = qvariant_cast<QBluetoothLocalDevice::Error>(arguments.at(0));
        QCOMPARE(e, QBluetoothLocalDevice::PairingError);
    } else {
        QTRY_VERIFY_WITH_TIMEOUT(!pairingSpy.isEmpty(), pairingWaitTime);
        QVERIFY(errorSpy.isEmpty());

        // test the actual signal values.
        QList<QVariant> arguments = pairingSpy.takeFirst();
        QBluetoothAddress address = qvariant_cast<QBluetoothAddress>(arguments.at(0));
        QBluetoothLocalDevice::Pairing pairingResult = qvariant_cast<QBluetoothLocalDevice::Pairing>(arguments.at(1));
        QCOMPARE(deviceAddress, address);
        // Verify that the local device pairing status and the signal value match
        QCOMPARE(pairingResult, localDevice.pairingStatus(deviceAddress));
#ifndef Q_OS_WIN
        // On Windows the resulting pairing mode may differ from test's "expected" as the
        // decision is up to Windows.
#ifdef Q_OS_ANDROID
        // On Android we always use "Paired"
        if (pairingExpected == QBluetoothLocalDevice::AuthorizedPaired)
            pairingExpected = QBluetoothLocalDevice::Paired;
#endif
        QCOMPARE(pairingResult, pairingExpected);
        QCOMPARE(localDevice.pairingStatus(deviceAddress), pairingExpected);
#endif
    }
}

void tst_QBluetoothLocalDevice::tst_connectedDevices()
{
#if defined(Q_OS_MACOS)
    QSKIP("The connectedDevices test fails on macOS");
#endif
    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");
    if (remoteDevice.isNull())
        QSKIP("This test only makes sense with remote device");

    QBluetoothLocalDevice localDevice;
    // powerOn if not already
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        localDevice.powerOn();
        QTRY_VERIFY(localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff);
    }

    QSignalSpy pairingSpy(&localDevice, &QBluetoothLocalDevice::pairingFinished);

    // Make sure that the remote device is not paired
    localDevice.requestPairing(remoteDevice, QBluetoothLocalDevice::Unpaired);
    QTRY_VERIFY(!pairingSpy.isEmpty());

    QList<QBluetoothAddress> connectedDevices = localDevice.connectedDevices();
    QVERIFY(!connectedDevices.contains(remoteDevice));

    QSignalSpy deviceConnectedSpy(&localDevice, &QBluetoothLocalDevice::deviceConnected);
    QSignalSpy deviceDisconnectedSpy(&localDevice, &QBluetoothLocalDevice::deviceDisconnected);

    // Now pair with the device. We should have a deviceConnected signal.
    pairingSpy.clear();
    localDevice.requestPairing(remoteDevice, QBluetoothLocalDevice::Paired);
    // Manual confirmation for pairing might be required
    QTRY_VERIFY_WITH_TIMEOUT(!pairingSpy.isEmpty(), 30000);
    QTRY_VERIFY(!deviceConnectedSpy.isEmpty());
    QList<QVariant> arguments = deviceConnectedSpy.takeFirst();
    auto address = arguments.at(0).value<QBluetoothAddress>();
    QCOMPARE(address, remoteDevice);

    connectedDevices = localDevice.connectedDevices();
    QVERIFY(connectedDevices.contains(remoteDevice));

    // Unpair again. We should have a deviceDisconnected signal.
    pairingSpy.clear();
    localDevice.requestPairing(remoteDevice, QBluetoothLocalDevice::Unpaired);
    QTRY_VERIFY(!pairingSpy.isEmpty());
    QTRY_VERIFY(!deviceDisconnectedSpy.isEmpty());
    arguments = deviceDisconnectedSpy.takeFirst();
    address = arguments.at(0).value<QBluetoothAddress>();
    QCOMPARE(address, remoteDevice);

    connectedDevices = localDevice.connectedDevices();
    QVERIFY(!connectedDevices.contains(remoteDevice));
}

void tst_QBluetoothLocalDevice::tst_pairingStatus_data()
{
    QTest::addColumn<QBluetoothAddress>("deviceAddress");
    QTest::addColumn<QBluetoothLocalDevice::Pairing>("pairingExpected");

    QTest::newRow("UnPaired Device: DUMMY") << QBluetoothAddress("11:00:00:00:00:00")
            << QBluetoothLocalDevice::Unpaired;
    QTest::newRow("Invalid device") << QBluetoothAddress() << QBluetoothLocalDevice::Unpaired;
    //valid devices are already tested by tst_pairDevice()
}

void tst_QBluetoothLocalDevice::tst_pairingStatus()
{
    QFETCH(QBluetoothAddress, deviceAddress);
    QFETCH(QBluetoothLocalDevice::Pairing, pairingExpected);

    if (numDevices == 0)
        QSKIP("Skipping test due to missing Bluetooth device");

    QBluetoothLocalDevice localDevice;
    QCOMPARE(pairingExpected, localDevice.pairingStatus(deviceAddress));
}
QTEST_MAIN(tst_QBluetoothLocalDevice)

#include "tst_qbluetoothlocaldevice.moc"
