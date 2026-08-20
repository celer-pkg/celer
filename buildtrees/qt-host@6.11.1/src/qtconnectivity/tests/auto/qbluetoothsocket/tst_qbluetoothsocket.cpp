// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QDebug>

#include <qbluetoothsocket.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>
#if QT_CONFIG(bluez)
#include <QtBluetooth/private/bluez5_helper_p.h>
#endif

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothServiceInfo::Protocol)

//same uuid as tests/bttestui
#define TEST_SERVICE_UUID "e8e10f95-1a70-4b27-9ccf-02010264e9c8"

// Max time to wait for connection

static const int MaxConnectTime = 60 * 1000;   // 1 minute in ms
static const int MaxReadWriteTime = 60 * 1000; // 1 minute in ms

class tst_QBluetoothSocket : public QObject
{
    Q_OBJECT

public:
    enum ClientConnectionShutdown {
        Error,
        Disconnect,
        Close,
        Abort,
    };

    tst_QBluetoothSocket();
    ~tst_QBluetoothSocket();

private slots:
    void initTestCase();

    void tst_construction_data();
    void tst_construction();

    void tst_serviceConnection();

    void tst_clientCommunication_data();
    void tst_clientCommunication();

    void tst_error();

    void tst_preferredSecurityFlags();

    void tst_unsupportedProtocolError();

public slots:
    void serviceDiscovered(const QBluetoothServiceInfo &info);
    void finished();
    void error(QBluetoothServiceDiscoveryAgent::Error error);
private:
    bool done_discovery;
    bool localDeviceFound;
    QBluetoothDeviceInfo remoteDevice;
    QBluetoothHostInfo localDevice;
    QBluetoothServiceInfo remoteServiceInfo;

};

Q_DECLARE_METATYPE(tst_QBluetoothSocket::ClientConnectionShutdown)

tst_QBluetoothSocket::tst_QBluetoothSocket()
{
    qRegisterMetaType<QBluetoothSocket::SocketState>();
    qRegisterMetaType<QBluetoothSocket::SocketError>();

    localDeviceFound = false; // true if we have a local adapter
    done_discovery = false; //true if we found remote device

    //Enable logging to facilitate debugging in case of error
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
}

tst_QBluetoothSocket::~tst_QBluetoothSocket()
{
}

void tst_QBluetoothSocket::initTestCase()
{
    // setup Bluetooth env
    const QList<QBluetoothHostInfo> foundDevices = QBluetoothLocalDevice::allDevices();
    if (foundDevices.isEmpty()) {
        qWarning() << "Missing local device";
        return;
    } else {
        localDevice = foundDevices.at(0);
        qDebug() << "Local device" << localDevice.name() << localDevice.address().toString();
    }

    localDeviceFound = true;

    //start the device
    QBluetoothLocalDevice device(localDevice.address());
    device.powerOn();


    //find the remote test server
    //the remote test server is tests/bttestui

    // Go find the server
    QBluetoothServiceDiscoveryAgent *sda = new QBluetoothServiceDiscoveryAgent(this);
    connect(sda, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)), this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(sda, SIGNAL(errorOccurred(QBluetoothServiceDiscoveryAgent::Error)), this,
            SLOT(error(QBluetoothServiceDiscoveryAgent::Error)));
    connect(sda, SIGNAL(finished()), this, SLOT(finished()));

    qDebug() << "Starting discovery";

    sda->setUuidFilter(QBluetoothUuid(QString(TEST_SERVICE_UUID)));
    sda->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

    for (int connectTime = MaxConnectTime; !done_discovery && connectTime > 0; connectTime -= 1000)
        QTest::qWait(1000);

    sda->stop();

    if (!remoteDevice.isValid()) {
        qWarning() << "#########################";
        qWarning() << "Unable to find test service";
        qWarning() << "Remote device may have to be changed into Discoverable mode";
        qWarning() << "#########################";
    }

    delete sda;
}

void tst_QBluetoothSocket::error(QBluetoothServiceDiscoveryAgent::Error error)
{
    qDebug() << "Received error" << error;
    done_discovery = true;
}

void tst_QBluetoothSocket::finished()
{
    qDebug() << "Finished";
    done_discovery = true;
}

void tst_QBluetoothSocket::serviceDiscovered(const QBluetoothServiceInfo &info)
{
    qDebug() << "Found test service on:" << info.device().name() << info.device().address().toString();
    remoteDevice = info.device();
    remoteServiceInfo = info;
    done_discovery = true;
}

void tst_QBluetoothSocket::tst_construction_data()
{
    QTest::addColumn<QBluetoothServiceInfo::Protocol>("socketType");

    QTest::newRow("unknown protocol") << QBluetoothServiceInfo::UnknownProtocol;
    QTest::newRow("rfcomm socket") << QBluetoothServiceInfo::RfcommProtocol;
    QTest::newRow("l2cap socket") << QBluetoothServiceInfo::L2capProtocol;
}

void tst_QBluetoothSocket::tst_construction()
{
    QFETCH(QBluetoothServiceInfo::Protocol, socketType);

    {
        QBluetoothSocket socket;

        QCOMPARE(socket.socketType(), QBluetoothServiceInfo::UnknownProtocol);
        QCOMPARE(socket.error(), QBluetoothSocket::SocketError::NoSocketError);
        QCOMPARE(socket.errorString(), QString());
        QCOMPARE(socket.peerAddress(), QBluetoothAddress());
        QCOMPARE(socket.peerName(), QString());
        QCOMPARE(socket.peerPort(), quint16(0));
        QCOMPARE(socket.state(), QBluetoothSocket::SocketState::UnconnectedState);
        QCOMPARE(socket.socketDescriptor(), -1);
        QCOMPARE(socket.bytesAvailable(), 0);
        QCOMPARE(socket.bytesToWrite(), 0);
        QCOMPARE(socket.canReadLine(), false);
        QCOMPARE(socket.isSequential(), true);
        QCOMPARE(socket.atEnd(), true);
        QCOMPARE(socket.pos(), 0);
        QCOMPARE(socket.size(), 0);
        QCOMPARE(socket.isOpen(), false);
        QCOMPARE(socket.isReadable(), false);
        QCOMPARE(socket.isWritable(), false);
        QCOMPARE(socket.openMode(), QIODevice::NotOpen);

        QByteArray array = socket.readAll();
        QVERIFY(array.isEmpty());

        char buffer[10];
        int returnValue = socket.read((char*)&buffer, 10);
        QCOMPARE(returnValue, -1);
    }

    {
        QBluetoothSocket socket(socketType);
        QCOMPARE(socket.socketType(), socketType);
    }
}

void tst_QBluetoothSocket::tst_serviceConnection()
{
    if (!remoteServiceInfo.isValid())
        QSKIP("Remote service not found");

    /* Construction */
    QBluetoothSocket socket;

    QSignalSpy stateSpy(&socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)));

    QCOMPARE(socket.socketType(), QBluetoothServiceInfo::UnknownProtocol);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::UnconnectedState);

    /* Connection */
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy errorSpy(&socket, SIGNAL(errorOccurred(QBluetoothSocket::SocketError)));

    QCOMPARE(socket.openMode(), QIODevice::NotOpen);
    QCOMPARE(socket.isWritable(), false);
    QCOMPARE(socket.isReadable(), false);
    QCOMPARE(socket.isOpen(), false);

    socket.connectToService(remoteServiceInfo);

    QCOMPARE(stateSpy.size(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::SocketState::ConnectingState);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::ConnectingState);

    stateSpy.clear();

    int connectTime = MaxConnectTime;
    while (connectedSpy.isEmpty()  && errorSpy.isEmpty() && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    if (!errorSpy.isEmpty()) {
        qDebug() << errorSpy.takeFirst().at(0).toInt();
        QSKIP("Connection error");
    }
    QCOMPARE(connectedSpy.size(), 1);
    QCOMPARE(stateSpy.size(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::SocketState::ConnectedState);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::ConnectedState);

    QCOMPARE(socket.isWritable(), true);
    QCOMPARE(socket.isReadable(), true);
    QCOMPARE(socket.isOpen(), true);

    stateSpy.clear();

    //check the peer & local info
    QCOMPARE(socket.localAddress(), localDevice.address());
    QCOMPARE(socket.localName(), localDevice.name());
#ifdef Q_OS_WIN
    // On Windows the socket peer name (aka remotehost display name) seems to be
    // formed from the BT address and not necessarily the remoteDevice name
    QVERIFY(!socket.peerName().isEmpty());
#else
    QCOMPARE(socket.peerName(), remoteDevice.name());
#endif
    QCOMPARE(socket.peerAddress(), remoteDevice.address());

    /* Disconnection */
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));

    socket.abort(); // close() tested by other functions
    QCOMPARE(socket.isWritable(), false);
    QCOMPARE(socket.isReadable(), false);
    QCOMPARE(socket.isOpen(), false);
    QCOMPARE(socket.openMode(), QIODevice::NotOpen);

    QVERIFY(!stateSpy.isEmpty());
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::SocketState::ClosingState);

    int disconnectTime = MaxConnectTime;
    while (disconnectedSpy.isEmpty() && disconnectTime > 0) {
        QTest::qWait(1000);
        disconnectTime -= 1000;
    }

    QCOMPARE(disconnectedSpy.size(), 1);
    QCOMPARE(stateSpy.size(), 1);
    QCOMPARE(stateSpy.takeFirst().at(0).value<QBluetoothSocket::SocketState>(), QBluetoothSocket::SocketState::UnconnectedState);

    // The remote service needs time to close the connection and resume listening
    QTest::qSleep(100);
}

void tst_QBluetoothSocket::tst_clientCommunication_data()
{
    QTest::addColumn<QStringList>("data");

    {
        QStringList data;
        data << QStringLiteral("Echo: Test line one.\n");
        data << QStringLiteral("Echo: Test line two, with longer data.\n");

        QTest::newRow("two line test") << data;
    }
}

void tst_QBluetoothSocket::tst_clientCommunication()
{
    if (!remoteServiceInfo.isValid())
        QSKIP("Remote service not found");
    QFETCH(QStringList, data);

    /* Construction */
    QBluetoothSocket socket(QBluetoothServiceInfo::RfcommProtocol);

    QSignalSpy stateSpy(&socket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)));

    QCOMPARE(socket.socketType(), QBluetoothServiceInfo::RfcommProtocol);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::UnconnectedState);

    /* Connection */
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));

    QCOMPARE(socket.isWritable(), false);
    QCOMPARE(socket.isReadable(), false);
    QCOMPARE(socket.isOpen(), false);
    QCOMPARE(socket.openMode(), QIODevice::NotOpen);
    socket.connectToService(remoteServiceInfo);

    QCOMPARE(stateSpy.size(), 1);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::SocketState::ConnectingState);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::ConnectingState);

    stateSpy.clear();

    int connectTime = MaxConnectTime;
    while (connectedSpy.isEmpty() && connectTime > 0) {
        QTest::qWait(1000);
        connectTime -= 1000;
    }

    QCOMPARE(socket.isWritable(), true);
    QCOMPARE(socket.isReadable(), true);
    QCOMPARE(socket.isOpen(), true);

    QCOMPARE(connectedSpy.size(), 1);
    QCOMPARE(stateSpy.size(), 1);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::SocketState::ConnectedState);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::ConnectedState);

    stateSpy.clear();

    /* Read / Write */
    QCOMPARE(socket.bytesToWrite(), qint64(0));
    QCOMPARE(socket.bytesAvailable(), qint64(0));

    {
        /* Send line by line with event loop */
        for (const QString &line : std::as_const(data)) {
            QSignalSpy readyReadSpy(&socket, SIGNAL(readyRead()));
            QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

            qint64 dataWritten = socket.write(line.toUtf8());

            if (socket.openMode() & QIODevice::Unbuffered)
                QCOMPARE(socket.bytesToWrite(), qint64(0));
            else
                QCOMPARE(socket.bytesToWrite(), qint64(line.size()));

            QCOMPARE(dataWritten, qint64(line.size()));

            int readWriteTime = MaxReadWriteTime;
            while ((bytesWrittenSpy.isEmpty() || readyReadSpy.isEmpty()) && readWriteTime > 0) {
                QTest::qWait(1000);
                readWriteTime -= 1000;
            }

            QCOMPARE(bytesWrittenSpy.size(), 1);
            QCOMPARE(bytesWrittenSpy.at(0).at(0).toLongLong(), qint64(line.size()));

            readWriteTime = MaxReadWriteTime;
            while (readyReadSpy.isEmpty() && readWriteTime > 0) {
                QTest::qWait(1000);
                readWriteTime -= 1000;
            }

            QCOMPARE(readyReadSpy.size(), 1);

            if (socket.openMode() & QIODevice::Unbuffered)
                QVERIFY(socket.bytesAvailable() <= qint64(line.size()));
            else
                QCOMPARE(socket.bytesAvailable(), qint64(line.size()));

            QVERIFY(socket.canReadLine());

            QByteArray echoed = socket.readAll();

            QCOMPARE(line.toUtf8(), echoed);
        }
    }

    QCOMPARE(socket.isWritable(), true);
    QCOMPARE(socket.isReadable(), true);
    QCOMPARE(socket.isOpen(), true);

    {
        /* Send all at once */
        QSignalSpy readyReadSpy(&socket, SIGNAL(readyRead()));
        QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

        QString joined = data.join(QString());
        qint64 dataWritten = socket.write(joined.toUtf8());

        if (socket.openMode() & QIODevice::Unbuffered)
            QCOMPARE(socket.bytesToWrite(), qint64(0));
        else
            QCOMPARE(socket.bytesToWrite(), qint64(joined.size()));

        QCOMPARE(dataWritten, qint64(joined.size()));

        int readWriteTime = MaxReadWriteTime;
        while ((bytesWrittenSpy.isEmpty() || readyReadSpy.isEmpty()) && readWriteTime > 0) {
            QTest::qWait(1000);
            readWriteTime -= 1000;
        }

        QCOMPARE(bytesWrittenSpy.size(), 1);
        QCOMPARE(bytesWrittenSpy.at(0).at(0).toLongLong(), qint64(joined.size()));
        QVERIFY(!readyReadSpy.isEmpty());

        if (socket.openMode() & QIODevice::Unbuffered)
            QVERIFY(socket.bytesAvailable() <= qint64(joined.size()));
        else
            QCOMPARE(socket.bytesAvailable(), qint64(joined.size()));

        QVERIFY(socket.canReadLine());

        QByteArray echoed = socket.readAll();

        QCOMPARE(joined.toUtf8(), echoed);
    }

    /* Disconnection */
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));

    socket.disconnectFromService();

    QCOMPARE(socket.isWritable(), false);
    QCOMPARE(socket.isReadable(), false);
    QCOMPARE(socket.isOpen(), false);
    QCOMPARE(socket.openMode(), QIODevice::NotOpen);

    int disconnectTime = MaxConnectTime;
    while (disconnectedSpy.isEmpty() && disconnectTime > 0) {
        QTest::qWait(1000);
        disconnectTime -= 1000;
    }

    QCOMPARE(disconnectedSpy.size(), 1);
    QCOMPARE(stateSpy.size(), 2);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::SocketState::ClosingState);
    QCOMPARE(qvariant_cast<QBluetoothSocket::SocketState>(stateSpy.takeFirst().at(0)), QBluetoothSocket::SocketState::UnconnectedState);

    // The remote service needs time to close the connection and resume listening
    QTest::qSleep(100);
}

void tst_QBluetoothSocket::tst_error()
{
    QBluetoothSocket socket;
    QSignalSpy errorSpy(&socket, SIGNAL(errorOccurred(QBluetoothSocket::SocketError)));
    QCOMPARE(errorSpy.size(), 0);
    const QBluetoothSocket::SocketError e = socket.error();

    QVERIFY(e == QBluetoothSocket::SocketError::NoSocketError);

    QVERIFY(socket.errorString() == QString());
}

void tst_QBluetoothSocket::tst_preferredSecurityFlags()
{
    QBluetoothSocket socket;

    //test default values
#if defined(QT_ANDROID_BLUETOOTH) || defined(QT_OSX_BLUETOOTH)
    QCOMPARE(socket.preferredSecurityFlags(), QBluetooth::Security::Secure);
#elif QT_CONFIG(bluez)
    // The bluezdbus socket uses "NoSecurity" by default, whereas the non-dbus bluez
    // socket uses "Authorization" by default
    if (bluetoothdVersion() >= QVersionNumber(5, 46))
        QCOMPARE(socket.preferredSecurityFlags(), QBluetooth::Security::NoSecurity);
    else
        QCOMPARE(socket.preferredSecurityFlags(), QBluetooth::Security::Authorization);
#else
    QCOMPARE(socket.preferredSecurityFlags(), QBluetooth::Security::NoSecurity);
#endif

    socket.setPreferredSecurityFlags(QBluetooth::Security::Authentication
                                     | QBluetooth::Security::Encryption);

#if defined(QT_OSX_BLUETOOTH)
    QCOMPARE(socket.preferredSecurityFlags(), QBluetooth::Security::Secure);
#else
    QCOMPARE(socket.preferredSecurityFlags(),
             QBluetooth::Security::Encryption | QBluetooth::Security::Authentication);
#endif
}

void tst_QBluetoothSocket::tst_unsupportedProtocolError()
{
#if defined(QT_ANDROID_BLUETOOTH)
    QSKIP("Android platform (re)sets RFCOMM socket type, nothing to test");
#endif
    // This socket has 'UnknownProtocol' socketType.
    // Any attempt to connectToService should end in
    // UnsupportedProtocolError.
    QBluetoothSocket socket;
    QCOMPARE(socket.socketType(), QBluetoothServiceInfo::UnknownProtocol);
    QVERIFY(socket.error() == QBluetoothSocket::SocketError::NoSocketError);
    QVERIFY(socket.errorString() == QString());

    QSignalSpy errorSpy(&socket, SIGNAL(errorOccurred(QBluetoothSocket::SocketError)));

    // 1. Stop early with 'UnsupportedProtocolError'.
    QBluetoothServiceInfo dummyServiceInfo;
    socket.connectToService(dummyServiceInfo, QIODevice::ReadWrite);
    QTRY_COMPARE_WITH_TIMEOUT(errorSpy.size(), 1, 1000);
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).toInt(), int(QBluetoothSocket::SocketError::UnsupportedProtocolError));
    QVERIFY(socket.errorString().size() != 0);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::UnconnectedState);

    errorSpy.clear();

    // 2. Stop early with UnsupportedProtocolError (before testing an invalid address/port).
    socket.connectToService(QBluetoothAddress(), 1, QIODevice::ReadWrite);
    QTRY_COMPARE_WITH_TIMEOUT(errorSpy.size(), 1, 1000);
    QCOMPARE(errorSpy.size(), 1);
#if QT_CONFIG(bluez)
    // Bluez dbus socket does not support connecting to port and gives different error code
    if (bluetoothdVersion() >= QVersionNumber(5, 46)) {
        QCOMPARE(errorSpy.takeFirst().at(0).toInt(),
                 int(QBluetoothSocket::SocketError::ServiceNotFoundError));
    } else {
        QCOMPARE(errorSpy.takeFirst().at(0).toInt(),
                 int(QBluetoothSocket::SocketError::UnsupportedProtocolError));
    }
#else
    QCOMPARE(errorSpy.takeFirst().at(0).toInt(), int(QBluetoothSocket::SocketError::UnsupportedProtocolError));
#endif
    QVERIFY(socket.errorString().size() != 0);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::UnconnectedState);

    errorSpy.clear();

    // 3. Stop early (ignoring an invalid address/uuid).
    socket.connectToService(QBluetoothAddress(), QBluetoothUuid(), QIODevice::ReadWrite);
    QTRY_COMPARE_WITH_TIMEOUT(errorSpy.size(), 1, 1000);
    QCOMPARE(errorSpy.size(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).toInt(), int(QBluetoothSocket::SocketError::UnsupportedProtocolError));
    QVERIFY(socket.errorString().size() != 0);
    QCOMPARE(socket.state(), QBluetoothSocket::SocketState::UnconnectedState);
}

QTEST_MAIN(tst_QBluetoothSocket)

#include "tst_qbluetoothsocket.moc"

