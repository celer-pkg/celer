// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:execute-external-code

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"

#include "bluez/bluez5_helper_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/adapter1_bluez5_p.h"

#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QProcess>
#include <QtCore/QScopeGuard>

#include <QtDBus/QDBusPendingCallWatcher>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace QtBluetoothPrivate; // for D-Bus wrappers

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
    QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
:   error(QBluetoothServiceDiscoveryAgent::NoError), m_deviceAdapterAddress(deviceAdapter), state(Inactive),
    mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery), singleDevice(false),
    q_ptr(qp)
{
    initializeBluez5();
    manager = new OrgFreedesktopDBusObjectManagerInterface(
            QStringLiteral("org.bluez"), QStringLiteral("/"), QDBusConnection::systemBus());
    qRegisterMetaType<QBluetoothServiceDiscoveryAgent::Error>();
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    delete manager;
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    qCDebug(QT_BT_BLUEZ) << "Discovery on: " << address.toString() << "Mode:" << DiscoveryMode();

    if (foundHostAdapterPath.isEmpty()) {
        // check that we match adapter addresses or use first if it wasn't specified

        bool ok = false;
        foundHostAdapterPath  = findAdapterForAddress(m_deviceAdapterAddress, &ok);
        if (!ok) {
            discoveredDevices.clear();
            error = QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Cannot access adapter during service discovery");
            emit q->errorOccurred(error);
            _q_serviceDiscoveryFinished();
            return;
        }

        if (foundHostAdapterPath.isEmpty()) {
            // Cannot find a local adapter
            // Abort any outstanding discoveries
            discoveredDevices.clear();

            error = QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Cannot find local Bluetooth adapter");
            emit q->errorOccurred(error);
            _q_serviceDiscoveryFinished();

            return;
        }
    }

    // ensure we didn't go offline yet
    OrgBluezAdapter1Interface adapter(QStringLiteral("org.bluez"),
                                      foundHostAdapterPath, QDBusConnection::systemBus());
    if (!adapter.powered()) {
        discoveredDevices.clear();

        error = QBluetoothServiceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothServiceDiscoveryAgent::tr("Local device is powered off");
        emit q->errorOccurred(error);

        _q_serviceDiscoveryFinished();
        return;
    }

    if (DiscoveryMode() == QBluetoothServiceDiscoveryAgent::MinimalDiscovery) {
        performMinimalServiceDiscovery(address);
    } else {
        runExternalSdpScan(address, QBluetoothAddress(adapter.address()));
    }
}

/* Bluez 5
 * src/tools/sdpscanner performs an SDP scan. This is
 * done out-of-process to avoid license issues. At this stage Bluez uses GPLv2.
 */
void QBluetoothServiceDiscoveryAgentPrivate::runExternalSdpScan(
        const QBluetoothAddress &remoteAddress, const QBluetoothAddress &localAddress)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (!sdpScannerProcess) {
        const QString binPath = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath);
        QFileInfo fileInfo(binPath, QStringLiteral("sdpscanner"));
        if (!fileInfo.exists() || !fileInfo.isExecutable()) {
            _q_finishSdpScan(QBluetoothServiceDiscoveryAgent::InputOutputError,
                             QBluetoothServiceDiscoveryAgent::tr("Unable to find sdpscanner"),
                             QStringList());
            qCWarning(QT_BT_BLUEZ) << "Cannot find sdpscanner:"
                                   << fileInfo.canonicalFilePath();
            return;
        }

        sdpScannerProcess = new QProcess(q);
        sdpScannerProcess->setReadChannel(QProcess::StandardOutput);
        if (QT_BT_BLUEZ().isDebugEnabled())
            sdpScannerProcess->setProcessChannelMode(QProcess::ForwardedErrorChannel);
        // AXIVION Next Line Qt-Security-QProcessStart: expected behavior
        sdpScannerProcess->setProgram(fileInfo.canonicalFilePath());
        q->connect(sdpScannerProcess,
                   QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                   q, [this](int exitCode, QProcess::ExitStatus status){
            this->_q_sdpScannerDone(exitCode, status);
        });
    }

    QStringList arguments;
    arguments << remoteAddress.toString() << localAddress.toString();

    // No filter implies PUBLIC_BROWSE_GROUP based SDP scan
    if (!uuidFilter.isEmpty()) {
        arguments << QLatin1String("-u"); // cmd line option for list of uuids
        for (const QBluetoothUuid& uuid : std::as_const(uuidFilter))
            arguments << uuid.toString();
    }

    sdpScannerProcess->setArguments(arguments);
    sdpScannerProcess->start();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_sdpScannerDone(int exitCode, QProcess::ExitStatus status)
{
    if (status != QProcess::NormalExit || exitCode != 0) {
        qCWarning(QT_BT_BLUEZ) << "SDP scan failure" << status << exitCode;
        if (singleDevice) {
            _q_finishSdpScan(QBluetoothServiceDiscoveryAgent::InputOutputError,
                             QBluetoothServiceDiscoveryAgent::tr("Unable to perform SDP scan"),
                             QStringList());
        } else {
            // go to next device
            _q_finishSdpScan(QBluetoothServiceDiscoveryAgent::NoError, QString(), QStringList());
        }
        return;
    }

    QStringList xmlRecords;
    const QByteArray utf8Data = QByteArray::fromBase64(sdpScannerProcess->readAllStandardOutput());
    const QByteArrayView utf8View = utf8Data;

    // split the various xml docs up
    constexpr auto matcher = qMakeStaticByteArrayMatcher("<?xml");
    qsizetype next;
    qsizetype start = matcher.indexIn(utf8View, 0);
    if (start != -1) {
        do {
            next = matcher.indexIn(utf8View, start + 1);
            if (next != -1)
                xmlRecords.append(QString::fromUtf8(utf8View.sliced(start, next - start)));
            else
                xmlRecords.append(QString::fromUtf8(utf8View.sliced(start)));
            start = next;
        } while ( start != -1);
    }

    _q_finishSdpScan(QBluetoothServiceDiscoveryAgent::NoError, QString(), xmlRecords);
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_finishSdpScan(QBluetoothServiceDiscoveryAgent::Error errorCode,
                                                              const QString &errorDescription,
                                                              const QStringList &xmlRecords)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (errorCode != QBluetoothServiceDiscoveryAgent::NoError) {
        qCWarning(QT_BT_BLUEZ) << "SDP search failed for"
                              << (!discoveredDevices.isEmpty()
                                     ? discoveredDevices.at(0).address().toString()
                                     : QStringLiteral("<Unknown>"));
        // We have an error which we need to indicate and stop further processing
        discoveredDevices.clear();
        error = errorCode;
        errorString = errorDescription;
        emit q->errorOccurred(error);
    } else if (!xmlRecords.isEmpty() && discoveryState() != Inactive) {
        for (const QString &record : xmlRecords) {
            QBluetoothServiceInfo serviceInfo = parseServiceXml(record);

            //apply uuidFilter
            if (!uuidFilter.isEmpty()) {
                bool serviceNameMatched = uuidFilter.contains(serviceInfo.serviceUuid());
                bool serviceClassMatched = false;
                const QList<QBluetoothUuid> serviceClassUuids
                        = serviceInfo.serviceClassUuids();
                for (const QBluetoothUuid &id : serviceClassUuids) {
                    if (uuidFilter.contains(id)) {
                        serviceClassMatched = true;
                        break;
                    }
                }

                if (!serviceNameMatched && !serviceClassMatched)
                    continue;
            }

            if (!serviceInfo.isValid())
                continue;

            // Bluez sdpscanner declares custom uuids into the service class uuid list.
            // Let's move a potential custom uuid from QBluetoothServiceInfo::serviceClassUuids()
            // to QBluetoothServiceInfo::serviceUuid(). If there is more than one, just move the first uuid
            const QList<QBluetoothUuid> serviceClassUuids = serviceInfo.serviceClassUuids();
            for (const QBluetoothUuid &id : serviceClassUuids) {
                if (id.minimumSize() == 16) {
                    serviceInfo.setServiceUuid(id);
                    if (serviceInfo.serviceName().isEmpty()) {
                        serviceInfo.setServiceName(
                                    QBluetoothServiceDiscoveryAgent::tr("Custom Service"));
                    }
                    QBluetoothServiceInfo::Sequence modSeq =
                            serviceInfo.attribute(QBluetoothServiceInfo::ServiceClassIds).value<QBluetoothServiceInfo::Sequence>();
                    modSeq.removeOne(QVariant::fromValue(id));
                    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, modSeq);
                    break;
                }
            }

            if (!isDuplicatedService(serviceInfo)) {
                discoveredServices.append(serviceInfo);
                qCDebug(QT_BT_BLUEZ) << "Discovered services" << discoveredDevices.at(0).address().toString()
                                     << serviceInfo.serviceName() << serviceInfo.serviceUuid()
                                     << ">>>" << serviceInfo.serviceClassUuids();
                // Use queued connection to allow us finish the service looping; the application
                // might call stop() when it has detected the service-of-interest.
                QMetaObject::invokeMethod(q, "serviceDiscovered", Qt::QueuedConnection,
                                          Q_ARG(QBluetoothServiceInfo, serviceInfo));
            }
        }
    }

    _q_serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO << "Stop called";

    discoveredDevices.clear();
    setDiscoveryState(Inactive);

    // must happen after discoveredDevices.clear() above to avoid retrigger of next scan
    // while waitForFinished() is waiting
    if (sdpScannerProcess) { // Bluez 5
        if (sdpScannerProcess->state() != QProcess::NotRunning) {
            sdpScannerProcess->kill();
            sdpScannerProcess->waitForFinished();
        }
    }

    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();
}

QBluetoothServiceInfo QBluetoothServiceDiscoveryAgentPrivate::parseServiceXml(
                            const QString& xmlRecord)
{
    QXmlStreamReader xml(xmlRecord);

    QBluetoothServiceInfo serviceInfo;
    serviceInfo.setDevice(discoveredDevices.at(0));

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.tokenType() == QXmlStreamReader::StartElement &&
            xml.name() == QLatin1String("attribute")) {
            quint16 attributeId =
                xml.attributes().value(QLatin1String("id")).toUShort(nullptr, 0);

            if (xml.readNextStartElement()) {
                const QVariant value = readAttributeValue(xml);
                serviceInfo.setAttribute(attributeId, value);
            }
        }
    }

    return serviceInfo;
}

// Bluez 5
void QBluetoothServiceDiscoveryAgentPrivate::performMinimalServiceDiscovery(const QBluetoothAddress &deviceAddress)
{
    if (foundHostAdapterPath.isEmpty()) {
        _q_serviceDiscoveryFinished();
        return;
    }

    Q_Q(QBluetoothServiceDiscoveryAgent);

    QDBusPendingReply<ManagedObjectList> reply = manager->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = reply.error().message();
            emit q->errorOccurred(error);
        }
        _q_serviceDiscoveryFinished();
        return;
    }

    QStringList uuidStrings;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();
            const QVariantMap &ifaceValues = jt.value();

            if (iface == QStringLiteral("org.bluez.Device1")) {
                if (deviceAddress.toString() == ifaceValues.value(QStringLiteral("Address")).toString()) {
                    uuidStrings = ifaceValues.value(QStringLiteral("UUIDs")).toStringList();
                    break;
                }
            }
        }
        if (!uuidStrings.isEmpty())
            break;
    }

    if (uuidStrings.isEmpty() || discoveredDevices.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "No uuids found for" << deviceAddress.toString();
         // nothing found -> go to next uuid
        _q_serviceDiscoveryFinished();
        return;
    }

    qCDebug(QT_BT_BLUEZ) << "Minimal uuid list for" << deviceAddress.toString() << uuidStrings;

    QBluetoothUuid uuid;
    for (qsizetype i = 0; i < uuidStrings.size(); ++i) {
        uuid = QBluetoothUuid(uuidStrings.at(i));
        if (uuid.isNull())
            continue;

        //apply uuidFilter
        if (!uuidFilter.isEmpty() && !uuidFilter.contains(uuid))
            continue;

        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setDevice(discoveredDevices.at(0));

        if (uuid.minimumSize() == 16) { // not derived from Bluetooth Base UUID
            serviceInfo.setServiceUuid(uuid);
            serviceInfo.setServiceName(QBluetoothServiceDiscoveryAgent::tr("Custom Service"));
        } else {
            // set uuid as service class id
            QBluetoothServiceInfo::Sequence classId;
            classId << QVariant::fromValue(uuid);
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
            QBluetoothUuid::ServiceClassUuid clsId
                    = static_cast<QBluetoothUuid::ServiceClassUuid>(uuid.data1 & 0xffff);
            serviceInfo.setServiceName(QBluetoothUuid::serviceClassToString(clsId));
        }

        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        {
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
            protocolDescriptorList.append(QVariant::fromValue(protocol));
        }
        {
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Att));
            protocolDescriptorList.append(QVariant::fromValue(protocol));
        }
        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

        //don't include the service if we already discovered it before
        if (!isDuplicatedService(serviceInfo)) {
            discoveredServices << serviceInfo;
            qCDebug(QT_BT_BLUEZ) << "Discovered services" << discoveredDevices.at(0).address().toString()
                                 << serviceInfo.serviceName();
            emit q->serviceDiscovered(serviceInfo);
        }
    }

    _q_serviceDiscoveryFinished();
}

QVariant QBluetoothServiceDiscoveryAgentPrivate::readAttributeValue(QXmlStreamReader &xml,
                                                                    int depth)
{
    auto skippingCurrentElementByDefault = qScopeGuard([&] { xml.skipCurrentElement(); });

    if (depth > kMaxSdpRecursionDepth) {
        qCWarning(QT_BT_BLUEZ) << "SDP XML attribute recursion depth exceeded";
        return QVariant();
    }

    if (xml.name() == QLatin1String("boolean")) {
        return xml.attributes().value(QLatin1String("value")) == QLatin1String("true");
    } else if (xml.name() == QLatin1String("uint8")) {
        quint8 value = xml.attributes().value(QLatin1String("value")).toUShort(nullptr, 0);
        return value;
    } else if (xml.name() == QLatin1String("uint16")) {
        quint16 value = xml.attributes().value(QLatin1String("value")).toUShort(nullptr, 0);
        return value;
    } else if (xml.name() == QLatin1String("uint32")) {
        quint32 value = xml.attributes().value(QLatin1String("value")).toUInt(nullptr, 0);
        return value;
    } else if (xml.name() == QLatin1String("uint64")) {
        quint64 value = xml.attributes().value(QLatin1String("value")).toULongLong(nullptr, 0);
        return value;
    } else if (xml.name() == QLatin1String("uuid")) {
        QBluetoothUuid uuid;
        const QStringView value = xml.attributes().value(QLatin1String("value"));
        if (value.startsWith(QLatin1String("0x"))) {
            if (value.size() == 6) {
                quint16 v = value.toUShort(nullptr, 0);
                uuid = QBluetoothUuid(v);
            } else if (value.size() == 10) {
                quint32 v = value.toUInt(nullptr, 0);
                uuid = QBluetoothUuid(v);
            }
        } else {
            uuid = QBluetoothUuid(value.toString());
        }
        return QVariant::fromValue(uuid);
    } else if (xml.name() == QLatin1String("text") || xml.name() == QLatin1String("url")) {
        const QStringView value = xml.attributes().value(QLatin1String("value"));
        if (xml.attributes().value(QLatin1String("encoding")) == QLatin1String("hex"))
            return QString::fromUtf8(QByteArray::fromHex(value.toLatin1()));
        return value.toString();
    } else if (xml.name() == QLatin1String("sequence")) {
        QBluetoothServiceInfo::Sequence sequence;

        skippingCurrentElementByDefault.dismiss(); // we skip several elements here

        while (xml.readNextStartElement()) {
            QVariant value = readAttributeValue(xml, depth + 1);
            sequence.append(value);
        }

        return QVariant::fromValue<QBluetoothServiceInfo::Sequence>(sequence);
    } else {
        qCWarning(QT_BT_BLUEZ) << "unknown attribute type"
                               << xml.name()
                               << xml.attributes().value(QLatin1String("value"));
        return QVariant();
    }
}

QT_END_NAMESPACE
