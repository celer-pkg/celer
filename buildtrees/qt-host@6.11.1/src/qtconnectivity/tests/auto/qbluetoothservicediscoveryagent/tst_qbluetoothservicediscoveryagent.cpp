// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QDebug>
#include <QLoggingCategory>
#include <QVariant>
#include <QList>
#include "../../shared/bttestutil_p.h"

#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothserver.h>
#include <qbluetoothserviceinfo.h>

#if QT_CONFIG(bluez) && defined(QT_BUILD_INTERNAL)
#include <private/qbluetoothservicediscoveryagent_p.h>
#endif

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

// Maximum time to for bluetooth device scan
const int MaxScanTime = 5 * 60 * 1000;  // 5 minutes in ms

class tst_QBluetoothServiceDiscoveryAgent : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothServiceDiscoveryAgent();
    ~tst_QBluetoothServiceDiscoveryAgent();

public slots:
    void deviceDiscoveryDebug(const QBluetoothDeviceInfo &info);
    void serviceDiscoveryDebug(const QBluetoothServiceInfo &info);
    void serviceError(const QBluetoothServiceDiscoveryAgent::Error err);

private slots:
    void initTestCase();

    void tst_invalidBtAddress();
    void tst_serviceDiscovery_data();
    void tst_serviceDiscovery();
    void tst_serviceDiscoveryStop();
    void tst_serviceDiscoveryAdapters();
    void tst_parseServiceXmlRecursionLimit();

private:
    QList<QBluetoothDeviceInfo> devices;
    bool localDeviceAvailable;
};

tst_QBluetoothServiceDiscoveryAgent::tst_QBluetoothServiceDiscoveryAgent()
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    // start Bluetooth if not started
#ifndef Q_OS_MACOS
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    localDeviceAvailable = device->isValid();
    if (localDeviceAvailable) {
        device->powerOn();
        // wait for the device to switch bluetooth mode.
        QTest::qWait(1000);
    }
    delete device;
#else
    QBluetoothLocalDevice device;
    localDeviceAvailable = QBluetoothLocalDevice().hostMode() != QBluetoothLocalDevice::HostPoweredOff;
#endif

    qRegisterMetaType<QBluetoothDeviceInfo>();
    qRegisterMetaType<QList<QBluetoothUuid> >();
    qRegisterMetaType<QBluetoothServiceDiscoveryAgent::Error>();
    qRegisterMetaType<QBluetoothDeviceDiscoveryAgent::Error>();

}

tst_QBluetoothServiceDiscoveryAgent::~tst_QBluetoothServiceDiscoveryAgent()
{
}

void tst_QBluetoothServiceDiscoveryAgent::deviceDiscoveryDebug(const QBluetoothDeviceInfo &info)
{
    qDebug() << "Discovered device:" << info.address().toString() << info.name();
}


void tst_QBluetoothServiceDiscoveryAgent::serviceError(const QBluetoothServiceDiscoveryAgent::Error err)
{
    qDebug() << "Service discovery error" << err;
}

void tst_QBluetoothServiceDiscoveryAgent::initTestCase()
{
    if (localDeviceAvailable) {
        QBluetoothDeviceDiscoveryAgent discoveryAgent;

        QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
        QSignalSpy errorSpy(&discoveryAgent,
                            SIGNAL(errorOccurred(QBluetoothDeviceDiscoveryAgent::Error)));
        QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)));

        discoveryAgent.start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);

        // Wait for up to MaxScanTime for the scan to finish
        int scanTime = MaxScanTime;
        while (finishedSpy.isEmpty() && scanTime > 0) {
            QTest::qWait(1000);
            scanTime -= 1000;
        }

        // Expect finished signal with no error
        QVERIFY(finishedSpy.size() == 1);
        QVERIFY(errorSpy.isEmpty());

        devices = discoveryAgent.discoveredDevices();
    }
}

void tst_QBluetoothServiceDiscoveryAgent::tst_serviceDiscoveryStop()
{
    if (!localDeviceAvailable)
        QSKIP("This test requires Bluetooth adapter in powered ON state");

    QBluetoothServiceDiscoveryAgent discoveryAgent;
    QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
    QSignalSpy canceledSpy(&discoveryAgent, SIGNAL(canceled()));

    // Verify we get the correct signals on start-stop
    discoveryAgent.start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
    QVERIFY(discoveryAgent.isActive());
    discoveryAgent.stop();
    QTRY_COMPARE(canceledSpy.size(), 1);
    QVERIFY(!discoveryAgent.isActive());
    // Wait a bit to see that there are no latent signals
    QTest::qWait(200);
    QCOMPARE(canceledSpy.size(), 1);
    QCOMPARE(finishedSpy.size(), 0);
}


void tst_QBluetoothServiceDiscoveryAgent::tst_invalidBtAddress()
{
#ifdef Q_OS_MACOS
    if (!localDeviceAvailable)
        QSKIP("On OS X this test requires Bluetooth adapter in powered ON state");
#endif
    QBluetoothServiceDiscoveryAgent *discoveryAgent = new QBluetoothServiceDiscoveryAgent(QBluetoothAddress("11:11:11:11:11:11"));

    QCOMPARE(discoveryAgent->error(), QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError);
    discoveryAgent->start();
    QCOMPARE(discoveryAgent->isActive(), false);
    delete discoveryAgent;

    discoveryAgent = new QBluetoothServiceDiscoveryAgent(QBluetoothAddress());
    QCOMPARE(discoveryAgent->error(), QBluetoothServiceDiscoveryAgent::NoError);
    if (!QBluetoothLocalDevice::allDevices().isEmpty()) {
        discoveryAgent->start();
        QCOMPARE(discoveryAgent->isActive(), true);
    }
    delete discoveryAgent;
}

void tst_QBluetoothServiceDiscoveryAgent::serviceDiscoveryDebug(const QBluetoothServiceInfo &info)
{
    qDebug() << "Discovered service on"
             << info.device().name() << info.device().address().toString();
    qDebug() << "\tService name:" << info.serviceName() << "cached" << info.device().isCached();
    qDebug() << "\tDescription:"
             << info.attribute(QBluetoothServiceInfo::ServiceDescription).toString();
    qDebug() << "\tProvider:" << info.attribute(QBluetoothServiceInfo::ServiceProvider).toString();
    qDebug() << "\tL2CAP protocol service multiplexer:" << info.protocolServiceMultiplexer();
    qDebug() << "\tRFCOMM server channel:" << info.serverChannel();
}

#if 0
static void dumpAttributeVariant(const QVariant &var, const QString indent)
{
    if (!var.isValid()) {
        qDebug("%sEmpty", indent.toLocal8Bit().constData());
        return;
    }

    if (var.typeId() == qMetaTypeId<QBluetoothServiceInfo::Sequence>()) {
        qDebug("%sSequence", indent.toLocal8Bit().constData());
        const QBluetoothServiceInfo::Sequence *sequence = static_cast<const QBluetoothServiceInfo::Sequence *>(var.data());
        for (const QVariant &v : *sequence)
            dumpAttributeVariant(v, indent + '\t');
    } else if (var.typeId() == qMetaTypeId<QBluetoothServiceInfo::Alternative>()) {
        qDebug("%sAlternative", indent.toLocal8Bit().constData());
        const QBluetoothServiceInfo::Alternative *alternative = static_cast<const QBluetoothServiceInfo::Alternative *>(var.data());
        for (const QVariant &v : *alternative)
            dumpAttributeVariant(v, indent + '\t');
    } else if (var.typeId() == qMetaTypeId<QBluetoothUuid>()) {
        QBluetoothUuid uuid = var.value<QBluetoothUuid>();
        switch (uuid.minimumSize()) {
        case 0:
            qDebug("%suuid NULL", indent.toLocal8Bit().constData());
            break;
        case 2:
            qDebug("%suuid %04x", indent.toLocal8Bit().constData(), uuid.toUInt16());
            break;
        case 4:
            qDebug("%suuid %08x", indent.toLocal8Bit().constData(), uuid.toUInt32());
            break;
        case 16: {
            qDebug("%suuid %s", indent.toLocal8Bit().constData(), uuid.toByteArray(QUuid::Id128).constData());
            break;
        }
        default:
            qDebug("%suuid ???", indent.toLocal8Bit().constData());
        }
    } else {
        switch (var.typeId()) {
        case QMetaType::UInt:
            qDebug("%suint %u", indent.toLocal8Bit().constData(), var.toUInt());
            break;
        case QMetaType::Int:
            qDebug("%sint %d", indent.toLocal8Bit().constData(), var.toInt());
            break;
        case QMetaType::QString:
            qDebug("%sstring %s", indent.toLocal8Bit().constData(), var.toString().toLocal8Bit().constData());
            break;
        case QMetaType::Bool:
            qDebug("%sbool %d", indent.toLocal8Bit().constData(), var.toBool());
            break;
        case QMetaType::QUrl:
            qDebug("%surl %s", indent.toLocal8Bit().constData(), var.toUrl().toString().toLocal8Bit().constData());
            break;
        default:
            qDebug("%sunknown", indent.toLocal8Bit().constData());
        }
    }
}

static inline void dumpServiceInfoAttributes(const QBluetoothServiceInfo &info)
{
    const QList<quint16> attributes = info.attributes();
    for (quint16 id : attributes) {
        dumpAttributeVariant(info.attribute(id), QString("\t"));
    }
}
#endif


void tst_QBluetoothServiceDiscoveryAgent::tst_serviceDiscovery_data()
{
    if (devices.isEmpty())
        QSKIP("This test requires an in-range bluetooth device");

    QTest::addColumn<QBluetoothDeviceInfo>("deviceInfo");
    QTest::addColumn<QList<QBluetoothUuid> >("uuidFilter");
    QTest::addColumn<QBluetoothServiceDiscoveryAgent::Error>("serviceDiscoveryError");

    // Only need to test the first 5 live devices
    int max = 5;
    for (const QBluetoothDeviceInfo &info : std::as_const(devices)) {
        if (info.isCached())
            continue;
        QTest::newRow("default filter") << info << QList<QBluetoothUuid>()
            << QBluetoothServiceDiscoveryAgent::NoError;
        if (!--max)
            break;
        //QTest::newRow("public browse group") << info << (QList<QBluetoothUuid>() << QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);
        //QTest::newRow("l2cap") << info << (QList<QBluetoothUuid>() << QBluetoothUuid::ProtocolUuid::L2cap);
    }
    QTest::newRow("all devices") << QBluetoothDeviceInfo() << QList<QBluetoothUuid>()
        << QBluetoothServiceDiscoveryAgent::NoError;
}

void tst_QBluetoothServiceDiscoveryAgent::tst_serviceDiscoveryAdapters()
{
    QBluetoothLocalDevice localDevice;
    const QList<QBluetoothHostInfo> adapters = localDevice.allDevices();
    if (adapters.size() > 1) {
        if (devices.isEmpty())
            QSKIP("This test requires an in-range bluetooth device");

        QVarLengthArray<QBluetoothAddress> addresses;

        for (const auto &adapter : adapters) {
            addresses.append(adapter.address());
        }
        QBluetoothServer server(QBluetoothServiceInfo::RfcommProtocol);
        QBluetoothUuid uuid(QBluetoothUuid::ProtocolUuid::Ftp);
        server.listen(addresses[0]);
        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, "serviceName");
        QBluetoothServiceInfo::Sequence publicBrowse;
        publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

        QBluetoothServiceInfo::Sequence profileSequence;
        QBluetoothServiceInfo::Sequence classId;
        classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
        classId << QVariant::fromValue(quint16(0x100));
        profileSequence.append(QVariant::fromValue(classId));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                                 profileSequence);

        serviceInfo.setServiceUuid(uuid);

        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        QBluetoothServiceInfo::Sequence protocol;
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
        protocol.clear();

        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
                 << QVariant::fromValue(quint8(server.serverPort()));

        protocolDescriptorList.append(QVariant::fromValue(protocol));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                                 protocolDescriptorList);
        QVERIFY(serviceInfo.registerService());

        QVERIFY(server.isListening());
        qDebug() << "Scanning address " << addresses[0].toString();
        QBluetoothServiceDiscoveryAgent discoveryAgent(addresses[1]);
        bool setAddress = discoveryAgent.setRemoteAddress(addresses[0]);

        QVERIFY(setAddress);

        QVERIFY(!discoveryAgent.isActive());

        QVERIFY(discoveryAgent.discoveredServices().isEmpty());

        QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));

        discoveryAgent.start();
        int scanTime = MaxScanTime;
        while (finishedSpy.isEmpty() && scanTime > 0) {
            QTest::qWait(1000);
            scanTime -= 1000;
        }

        const QList<QBluetoothServiceInfo> discServices = discoveryAgent.discoveredServices();
        QVERIFY(!discServices.empty());

        qsizetype counter = 0;
        for (const QBluetoothServiceInfo &service : discServices) {
            if (uuid == service.serviceUuid())
                counter++;
        }
        QCOMPARE(counter, 1);
    }

}


void tst_QBluetoothServiceDiscoveryAgent::tst_serviceDiscovery()
{
    if (devices.isEmpty())
        QSKIP("This test requires an in-range bluetooth device");

    QFETCH(QBluetoothDeviceInfo, deviceInfo);
    QFETCH(QList<QBluetoothUuid>, uuidFilter);
    QFETCH(QBluetoothServiceDiscoveryAgent::Error, serviceDiscoveryError);

    QBluetoothLocalDevice localDevice;
    qDebug() << "Scanning address" << deviceInfo.address().toString() << deviceInfo.name();
    QBluetoothServiceDiscoveryAgent discoveryAgent(localDevice.address());
    bool setAddress = discoveryAgent.setRemoteAddress(deviceInfo.address());

    QVERIFY(setAddress);

    QVERIFY(!discoveryAgent.isActive());

    QVERIFY(discoveryAgent.discoveredServices().isEmpty());

    QVERIFY(discoveryAgent.uuidFilter().isEmpty());

    discoveryAgent.setUuidFilter(uuidFilter);

    QVERIFY(discoveryAgent.uuidFilter() == uuidFilter);

    QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
    QSignalSpy errorSpy(&discoveryAgent,
                        SIGNAL(errorOccurred(QBluetoothServiceDiscoveryAgent::Error)));
    QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)));
    connect(&discoveryAgent, SIGNAL(errorOccurred(QBluetoothServiceDiscoveryAgent::Error)), this,
            SLOT(serviceError(QBluetoothServiceDiscoveryAgent::Error)));

    discoveryAgent.start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

    /*
     * Either we wait for discovery agent to run its course (e.g. Bluez 4) or
     * we have an immediate result (e.g. Bluez 5)
     */
    QVERIFY(discoveryAgent.isActive() || !finishedSpy.isEmpty());

    // Wait for up to MaxScanTime for the scan to finish
    int scanTime = 20000;
    while (finishedSpy.isEmpty() && scanTime > 0) {
        QTest::qWait(1000);
        scanTime -= 1000;
    }

    if (discoveryAgent.error()) {
        qDebug() << "Device failed to respond to SDP, skipping device" << discoveryAgent.error() << discoveryAgent.errorString();
        return;
    }

    QVERIFY(discoveryAgent.error() == serviceDiscoveryError);
    QVERIFY(discoveryAgent.errorString() == QString());

    // Expect finished signal with no error
    if (scanTime)
        QVERIFY(finishedSpy.size() == 1);

    QVERIFY(errorSpy.isEmpty());

    //if (!discoveryAgent.discoveredServices().isEmpty() && expected_failures++ <2){
    if (discoveredSpy.isEmpty()) {
        qDebug() << "Device failed to return any results, skipping device" << discoveryAgent.discoveredServices().size();
        return;
    }

    // All returned QBluetoothServiceInfo should be valid.
    bool servicesFound = !discoveredSpy.isEmpty();
    while (!discoveredSpy.isEmpty()) {
        const QVariant v = discoveredSpy.takeFirst().at(0);
        // Work around limitation in QMetaType and moc.
        // QBluetoothServiceInfo is registered with metatype as QBluetoothServiceInfo
        // moc sees it as the unqualified QBluetoothServiceInfo.
        if (v.userType() == qMetaTypeId<QBluetoothServiceInfo>())
        {
            const QBluetoothServiceInfo info =
                *reinterpret_cast<const QBluetoothServiceInfo*>(v.constData());

            QVERIFY(info.isValid());
            QVERIFY(!info.isRegistered());

#if 0
            qDebug() << info.device().name() << info.device().address().toString();
            qDebug() << "\tService name:" << info.serviceName();
            if (info.protocolServiceMultiplexer() >= 0)
                qDebug() << "\tL2CAP protocol service multiplexer:" << info.protocolServiceMultiplexer();
            if (info.serverChannel() >= 0)
                qDebug() << "\tRFCOMM server channel:" << info.serverChannel();
            //dumpServiceInfoAttributes(info);
#endif
        } else {
            QFAIL("Unknown type returned by service discovery");
        }

    }

    if (servicesFound)
        QVERIFY(!discoveryAgent.discoveredServices().isEmpty());
    discoveryAgent.clear();
    QVERIFY(discoveryAgent.discoveredServices().isEmpty());

    discoveryAgent.stop();
    QVERIFY(!discoveryAgent.isActive());
}

void tst_QBluetoothServiceDiscoveryAgent::tst_parseServiceXmlRecursionLimit()
{
#if QT_CONFIG(bluez) && defined(QT_BUILD_INTERNAL)
    // Build an SDP XML record with deeply nested <sequence> elements.
    // Without a recursion limit this would overflow the stack.
    const int nestingDepth = QBluetoothServiceDiscoveryAgentPrivate::kMaxSdpRecursionDepth * 3;

    QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
                  "<record>"
                  "<attribute id=\"0x0001\">"_L1;

    for (int i = 0; i < nestingDepth; ++i)
        xml += "<sequence>"_L1;
    xml += "<uint16 value=\"0x1101\" />"_L1;
    for (int i = 0; i < nestingDepth; ++i)
        xml += "</sequence>"_L1;

    xml += "</attribute></record>"_L1;

    QBluetoothServiceDiscoveryAgentPrivate priv(nullptr, QBluetoothAddress());
    // parseServiceXml accesses discoveredDevices.at(0), so provide a dummy
    QBluetoothDeviceInfo dummyInfo{ QBluetoothAddress("00:11:22:33:44:55"_L1), "Dummy"_L1, 0 };
    priv.discoveredDevices.emplace_back(std::move(dummyInfo));

    // Must not crash or hang; expect the recursion-limit warning
    QTest::ignoreMessage(QtWarningMsg, "SDP XML attribute recursion depth exceeded");
    const QBluetoothServiceInfo info = priv.parseServiceXml(xml);

    // Walk the nested sequences and count the actual depth parsed.
    // The recursion limit must have kicked in: we expect many levels
    // but strictly fewer than nestingDepth.
    int actualDepth = 0;
    QVariant v = info.attribute(0x0001);
    while (v.canConvert<QBluetoothServiceInfo::Sequence>()) {
        const auto seq = v.value<QBluetoothServiceInfo::Sequence>();
        if (seq.isEmpty())
            break;
        ++actualDepth;
        v = seq.first();
    }
    QVERIFY(actualDepth > 1);
    QVERIFY(actualDepth < nestingDepth);
#else
    QSKIP("This test is only available on Linux developer build with BlueZ");
#endif
}

QTEST_MAIN(tst_QBluetoothServiceDiscoveryAgent)

#include "tst_qbluetoothservicediscoveryagent.moc"
