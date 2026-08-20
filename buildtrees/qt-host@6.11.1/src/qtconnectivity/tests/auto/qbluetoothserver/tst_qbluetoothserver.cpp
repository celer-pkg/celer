// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QDebug>

#include "../../shared/bttestutil_p.h"
#include <private/qtbluetoothglobal_p.h>
#include <qbluetoothserver.h>
#include <qbluetoothsocket.h>
#include <qbluetoothlocaldevice.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetooth::SecurityFlags)

class tst_QBluetoothServer : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothServer();
    ~tst_QBluetoothServer();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void tst_construction();

    void tst_receive_data();
    void tst_receive();

    void setHostMode(const QBluetoothAddress &localAdapter, QBluetoothLocalDevice::HostMode newHostMode);

private:
    QBluetoothLocalDevice *localDevice = nullptr;
    QBluetoothLocalDevice::HostMode initialHostMode;
};

tst_QBluetoothServer::tst_QBluetoothServer()
{
}

tst_QBluetoothServer::~tst_QBluetoothServer()
{
}

void tst_QBluetoothServer::setHostMode(const QBluetoothAddress &localAdapter,
                                    QBluetoothLocalDevice::HostMode newHostMode)
{
    QBluetoothLocalDevice device(localAdapter);
    QSignalSpy hostModeSpy(&device, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));

    if (!device.isValid())
        return;

    if (device.hostMode() == newHostMode)
        return;

    //We distinguish powerOn() and setHostMode(HostConnectable) because
    //Android uses different permission levels, we want to avoid interaction with user
    //which implies usage of powerOn -> unfortunately powerOn() is not equivalent to HostConnectable
    //Unfortunately the discoverable mode always requires a user confirmation
    switch (newHostMode) {
    case QBluetoothLocalDevice::HostPoweredOff:
    case QBluetoothLocalDevice::HostDiscoverable:
    case QBluetoothLocalDevice::HostDiscoverableLimitedInquiry:
        device.setHostMode(newHostMode);
        break;
    case QBluetoothLocalDevice::HostConnectable:
        device.powerOn();
        break;
    }

    int connectTime = 5000;  // ms
    while (hostModeSpy.isEmpty() && connectTime > 0) {
        QTest::qWait(500);
        connectTime -= 500;
    }
}

void tst_QBluetoothServer::initTestCase()
{
    if (androidBluetoothEmulator())
        QSKIP("Skipping test on Android 12+ emulator, CI can timeout waiting for user input");
    qRegisterMetaType<QBluetooth::SecurityFlags>();
    qRegisterMetaType<QBluetoothServer::Error>();

    localDevice = new QBluetoothLocalDevice(this);

    QBluetoothLocalDevice device;
    if (!device.isValid())
        return;

    initialHostMode = device.hostMode();
#ifdef Q_OS_MACOS
    if (initialHostMode == QBluetoothLocalDevice::HostPoweredOff)
        return;
#endif

    setHostMode(device.address(), QBluetoothLocalDevice::HostConnectable);

    QBluetoothLocalDevice::HostMode hostMode= localDevice->hostMode();

    QVERIFY(hostMode != QBluetoothLocalDevice::HostPoweredOff);
}

void tst_QBluetoothServer::cleanupTestCase()
{
    if (localDevice)
        setHostMode(localDevice->address(), initialHostMode);
}

void tst_QBluetoothServer::tst_construction()
{
    {
        QBluetoothServer server(QBluetoothServiceInfo::RfcommProtocol);

        QVERIFY(!server.isListening());
        QCOMPARE(server.maxPendingConnections(), 1);
        QVERIFY(!server.hasPendingConnections());
        QVERIFY(server.nextPendingConnection() == 0);
        QCOMPARE(server.error(), QBluetoothServer::NoError);
        QCOMPARE(server.serverType(), QBluetoothServiceInfo::RfcommProtocol);
    }

    {
        QBluetoothServer server(QBluetoothServiceInfo::L2capProtocol);

        QVERIFY(!server.isListening());
        QCOMPARE(server.maxPendingConnections(), 1);
        QVERIFY(!server.hasPendingConnections());
        QVERIFY(server.nextPendingConnection() == 0);
        QCOMPARE(server.error(), QBluetoothServer::NoError);
        QCOMPARE(server.serverType(), QBluetoothServiceInfo::L2capProtocol);
    }
}

void tst_QBluetoothServer::tst_receive_data()
{
    QTest::addColumn<QBluetoothLocalDevice::HostMode>("hostmode");
    QTest::newRow("offline mode") << QBluetoothLocalDevice::HostPoweredOff;
    QTest::newRow("online mode") << QBluetoothLocalDevice::HostConnectable;
}

void tst_QBluetoothServer::tst_receive()
{
    QFETCH(QBluetoothLocalDevice::HostMode, hostmode);

    QBluetoothLocalDevice localDev;
#ifdef Q_OS_MACOS
    if (localDev.hostMode() == QBluetoothLocalDevice::HostPoweredOff)
        QSKIP("On OS X this test requires Bluetooth adapter ON");
#endif
    const QBluetoothAddress address = localDev.address();

    bool localDeviceAvailable = localDev.isValid();

    if (localDeviceAvailable) {
        // setHostMode is noop on OS X and winrt.
        setHostMode(address, hostmode);

        if (hostmode == QBluetoothLocalDevice::HostPoweredOff) {
#if !defined(Q_OS_MACOS) && !QT_CONFIG(winrt_bt)
            QCOMPARE(localDevice->hostMode(), hostmode);
#endif
        } else {
            QVERIFY(localDevice->hostMode() != QBluetoothLocalDevice::HostPoweredOff);
        }
    }
    QBluetoothServer server(QBluetoothServiceInfo::RfcommProtocol);
    QSignalSpy errorSpy(&server, SIGNAL(errorOccurred(QBluetoothServer::Error)));

    bool result = server.listen(address, 20);  // port == 20
    QTest::qWait(1000);

    if (!result) {
#ifndef Q_OS_ANDROID
        // Disable address check on Android as an actual device always returns
        // a valid address, while the emulator doesn't
        QCOMPARE(server.serverAddress(), QBluetoothAddress());
#endif
        QCOMPARE(server.serverPort(), quint16(0));
        QVERIFY(!errorSpy.isEmpty());
        QVERIFY(!server.isListening());
        if (!localDeviceAvailable) {
            QVERIFY(server.error() != QBluetoothServer::NoError);
        } else {
            //local device but poweredOff
            QCOMPARE(server.error(), QBluetoothServer::PoweredOffError);
        }
        return;
    }

    QVERIFY(result);

#if !QT_CONFIG(winrt_bt)
    QVERIFY(!QBluetoothLocalDevice::allDevices().isEmpty());
#endif
    QCOMPARE(server.error(), QBluetoothServer::NoError);
    QCOMPARE(server.serverAddress(), address);
    QCOMPARE(server.serverPort(), quint16(20));
    QVERIFY(server.isListening());
    QVERIFY(!server.hasPendingConnections());

    server.close();
    QCOMPARE(server.error(), QBluetoothServer::NoError);
    QVERIFY(server.serverAddress() == address || server.serverAddress() == QBluetoothAddress());
    QVERIFY(server.serverPort() == 0);
    QVERIFY(!server.isListening());
    QVERIFY(!server.hasPendingConnections());
}


QTEST_MAIN(tst_QBluetoothServer)

#include "tst_qbluetoothserver.moc"
