// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>

#include <QtCore/qcoreapplication.h>

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include "bluez/bluez5_helper_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/adapter1_bluez5_p.h"
#include "bluez/device1_bluez5_p.h"
#include "bluez/properties_p.h"
#include "bluez/bluetoothmanagement_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace QtBluetoothPrivate; // for D-Bus wrappers

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
    const QBluetoothAddress &deviceAdapter, QBluetoothDeviceDiscoveryAgent *parent) :
    adapterAddress(deviceAdapter),
    q_ptr(parent)
{
    initializeBluez5();
    manager = new OrgFreedesktopDBusObjectManagerInterface(
                                       QStringLiteral("org.bluez"),
                                       QStringLiteral("/"),
                                       QDBusConnection::systemBus(), parent);
    QObject::connect(manager,
                     &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                     q_ptr,
                     [this](const QDBusObjectPath &objectPath, InterfaceList interfacesAndProperties) {
        this->_q_InterfacesAdded(objectPath, interfacesAndProperties);
    });

    // start private address monitoring
    BluetoothManagement::instance();
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    delete adapter;
}

//TODO: Qt6 remove the pendingCancel/pendingStart logic as it is cumbersome.
//      It is a behavior change across all platforms and was initially done
//      for Bluez. The behavior should be similar to QBluetoothServiceDiscoveryAgent
//      PendingCancel creates issues whereby the agent is still shutting down
//      but isActive() below already returns false. This means the isActive() is
//      out of sync with the finished() and cancel() signal.

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false; //TODO Qt6: remove pending[Cancel|Start] logic (see comment above)

    return adapter;
}

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return (ClassicMethod | LowEnergyMethod);
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
{
    if (pendingCancel == true) {
        pendingStart = true;
        return;
    }

    lastError = QBluetoothDeviceDiscoveryAgent::NoError;
    errorString.clear();
    discoveredDevices.clear();
    devicesProperties.clear();

    Q_Q(QBluetoothDeviceDiscoveryAgent);

    bool ok = false;
    const QString adapterPath = findAdapterForAddress(adapterAddress, &ok);
    if (!ok || adapterPath.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot find Bluez 5 adapter for device search" << ok;
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Cannot find valid Bluetooth adapter.");
        q->errorOccurred(lastError);
        return;
    }

    adapter = new OrgBluezAdapter1Interface(QStringLiteral("org.bluez"), adapterPath,
                                            QDBusConnection::systemBus());

    if (!adapter->powered()) {
        qCDebug(QT_BT_BLUEZ) << "Aborting device discovery due to offline Bluetooth Adapter";
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device is powered off");
        delete adapter;
        adapter = nullptr;
        emit q->errorOccurred(lastError);
        return;
    }

    QVariantMap map;
    if (methods == (QBluetoothDeviceDiscoveryAgent::LowEnergyMethod|QBluetoothDeviceDiscoveryAgent::ClassicMethod))
        map.insert(QStringLiteral("Transport"), QStringLiteral("auto"));
    else if (methods & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod)
        map.insert(QStringLiteral("Transport"), QStringLiteral("le"));
    else
        map.insert(QStringLiteral("Transport"), QStringLiteral("bredr"));

    // older BlueZ 5.x versions don't have this function
    // filterReply returns UnknownMethod which we ignore
    QDBusPendingReply<> filterReply = adapter->SetDiscoveryFilter(map);
    filterReply.waitForFinished();
    if (filterReply.isError()) {
        if (filterReply.error().type() == QDBusError::Other
                    && filterReply.error().name() == QStringLiteral("org.bluez.Error.Failed")) {
            qCDebug(QT_BT_BLUEZ) << "Discovery method" << methods << "not supported";
            lastError = QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("One or more device discovery methods "
                                                             "are not supported on this platform");
            delete adapter;
            adapter = nullptr;
            emit q->errorOccurred(lastError);
            return;
        } else if (filterReply.error().type() != QDBusError::UnknownMethod) {
            qCDebug(QT_BT_BLUEZ) << "SetDiscoveryFilter failed:" << filterReply.error();
        }
    }

    QtBluezDiscoveryManager::instance()->registerDiscoveryInterest(adapter->path());
    QObject::connect(QtBluezDiscoveryManager::instance(), &QtBluezDiscoveryManager::discoveryInterrupted,
                     q, [this](const QString &path){
        this->_q_discoveryInterrupted(path);
    });
    OrgFreedesktopDBusPropertiesInterface *prop = new OrgFreedesktopDBusPropertiesInterface(
                QStringLiteral("org.bluez"), QStringLiteral(""), QDBusConnection::systemBus());
    QObject::connect(prop, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                     q, [this](const QString &interface, const QVariantMap &changedProperties,
                     const QStringList &invalidatedProperties,
                     const QDBusMessage &signal) {
        this->_q_PropertiesChanged(interface, signal.path(), changedProperties, invalidatedProperties);
    });

    // remember what we have to cleanup
    propertyMonitors.append(prop);

    // collect initial set of information
    QDBusPendingReply<ManagedObjectList> reply = manager->GetManagedObjects();
    reply.waitForFinished();
    if (!reply.isError()) {
        ManagedObjectList managedObjectList = reply.value();
        for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
            const QDBusObjectPath &path = it.key();
            const InterfaceList &ifaceList = it.value();

            for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
                const QString &iface = jt.key();

                if (iface == QStringLiteral("org.bluez.Device1")) {

                    if (path.path().indexOf(adapter->path()) != 0)
                        continue; //devices whose path doesn't start with same path we skip

                    deviceFound(path.path(), jt.value());
                    if (!isActive()) // Can happen if stop() was called from a slot in user code.
                      return;
                }
            }
        }
    }

    // wait interval and sum up what was found
    if (!discoveryTimer) {
        discoveryTimer = new QTimer(q);
        discoveryTimer->setSingleShot(true);
        QObject::connect(discoveryTimer, &QTimer::timeout,
                         q, [this]() {
            this->_q_discoveryFinished();
        });
    }

    if (lowEnergySearchTimeout > 0) { // otherwise no timeout and stop() required
        discoveryTimer->setInterval(lowEnergySearchTimeout);
        discoveryTimer->start();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    if (!adapter)
        return;

    qCDebug(QT_BT_BLUEZ) << Q_FUNC_INFO;
    pendingCancel = true;
    pendingStart = false;
    _q_discoveryFinished();
}

// Returns invalid QBluetoothDeviceInfo in case of error
static QBluetoothDeviceInfo createDeviceInfoFromBluez5Device(const QVariantMap& properties)
{
    const QBluetoothAddress btAddress(properties[QStringLiteral("Address")].toString());
    if (btAddress.isNull())
        return QBluetoothDeviceInfo();

    const QString btName = properties[QStringLiteral("Alias")].toString();
    quint32 btClass = properties[QStringLiteral("Class")].toUInt();

    QBluetoothDeviceInfo deviceInfo(btAddress, btName, btClass);
    deviceInfo.setRssi(qvariant_cast<short>(properties[QStringLiteral("RSSI")]));

    QList<QBluetoothUuid> uuids;
    bool foundLikelyLowEnergyUuid = false;
    const QStringList foundUuids = qvariant_cast<QStringList>(properties[QStringLiteral("UUIDs")]);
    for (const auto &u: foundUuids) {
        const QBluetoothUuid id(u);
        if (id.isNull())
            continue;

        if (!foundLikelyLowEnergyUuid) {
            //once we found one BTLE service we are done
            bool ok = false;
            quint16 shortId = id.toUInt16(&ok);
            quint16 genericAccessInt = static_cast<quint16>(QBluetoothUuid::ServiceClassUuid::GenericAccess);
            if (ok && ((shortId & genericAccessInt) == genericAccessInt))
                foundLikelyLowEnergyUuid = true;
        }
        uuids.append(id);
    }
    deviceInfo.setServiceUuids(uuids);

    if (!btClass) {
        deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    } else {
        deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateCoreConfiguration);
        if (foundLikelyLowEnergyUuid)
            deviceInfo.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
    }

    const ManufacturerDataList deviceManufacturerData = qdbus_cast<ManufacturerDataList>(properties[QStringLiteral("ManufacturerData")]);
    for (const auto &[key, value] : deviceManufacturerData.asKeyValueRange())
        deviceInfo.setManufacturerData(key, value.variant().toByteArray());

    const ServiceDataList deviceServiceData =
            qdbus_cast<ServiceDataList>(properties[QStringLiteral("ServiceData")]);
    for (const auto &[key, value] : deviceServiceData.asKeyValueRange())
        deviceInfo.setServiceData(QBluetoothUuid(key), value.variant().toByteArray());

    return deviceInfo;
}

void QBluetoothDeviceDiscoveryAgentPrivate::deviceFound(const QString &devicePath,
                                                        const QVariantMap &properties)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!q->isActive())
        return;

    auto deviceAdapter = qvariant_cast<QDBusObjectPath>(properties[QStringLiteral("Adapter")]);
    if (deviceAdapter.path() != adapter->path())
        return;

    // read information
    QBluetoothDeviceInfo deviceInfo = createDeviceInfoFromBluez5Device(properties);
    if (!deviceInfo.isValid()) // no point reporting an empty address
        return;

    qCDebug(QT_BT_BLUEZ) << "Discovered: " << deviceInfo.name() << deviceInfo.address()
                         << "Num UUIDs" << deviceInfo.serviceUuids().size()
                         << "total device" << discoveredDevices.size() << "cached"
                         << "RSSI" << deviceInfo.rssi()
                         << "Num ManufacturerData" << deviceInfo.manufacturerData().size()
                         << "Num ServiceData" << deviceInfo.serviceData().size();

    // Cache the properties so we do not have to access dbus every time to get a value
    devicesProperties[devicePath] = properties;

    for (qsizetype i = 0; i < discoveredDevices.size(); ++i) {
        if (discoveredDevices[i].address() == deviceInfo.address()) {
            if (lowEnergySearchTimeout > 0 && discoveredDevices[i] == deviceInfo) {
                qCDebug(QT_BT_BLUEZ) << "Duplicate: " << deviceInfo.address();
                return;
            }
            discoveredDevices.replace(i, deviceInfo);

            emit q->deviceDiscovered(deviceInfo);
            return; // this works if the list doesn't contain duplicates. Don't let it.
        }
    }

    discoveredDevices.append(deviceInfo);
    emit q->deviceDiscovered(deviceInfo);
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_InterfacesAdded(const QDBusObjectPath &object_path,
                                                               InterfaceList interfaces_and_properties)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!q->isActive())
        return;

    if (interfaces_and_properties.contains(QStringLiteral("org.bluez.Device1"))) {
        // device interfaces belonging to different adapter
        // will be filtered out by deviceFound();
        deviceFound(object_path.path(),
                    interfaces_and_properties[QStringLiteral("org.bluez.Device1")]);
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_discoveryFinished()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (discoveryTimer)
        discoveryTimer->stop();

    QtBluezDiscoveryManager::instance()->disconnect(q);
    QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(adapter->path());

    qDeleteAll(propertyMonitors);
    propertyMonitors.clear();

    delete adapter;
    adapter = nullptr;

    if (pendingCancel && !pendingStart) {
        pendingCancel = false;
        emit q->canceled();
    } else if (pendingStart) {
        pendingStart = false;
        pendingCancel = false;
        start(QBluetoothDeviceDiscoveryAgent::ClassicMethod
              | QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    } else {
        emit q->finished();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_discoveryInterrupted(const QString &path)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!q->isActive())
        return;

    if (path == adapter->path()) {
        qCWarning(QT_BT_BLUEZ) << "Device discovery aborted due to unexpected adapter changes from another process.";

        if (discoveryTimer)
            discoveryTimer->stop();

        QtBluezDiscoveryManager::instance()->disconnect(q);
        // no need to call unregisterDiscoveryInterest since QtBluezDiscoveryManager
        // does this automatically when emitting discoveryInterrupted(QString) signal

        delete adapter;
        adapter = nullptr;

        errorString = QBluetoothDeviceDiscoveryAgent::tr("Bluetooth adapter error");
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        emit q->errorOccurred(lastError);
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::_q_PropertiesChanged(const QString &interface,
                                                                 const QString &path,
                                                                 const QVariantMap &changed_properties,
                                                                 const QStringList &invalidated_properties)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (interface != QStringLiteral("org.bluez.Device1"))
        return;

    if (!devicesProperties.contains(path))
        return;

    // Update the cached properties before checking changed_properties for RSSI and ManufacturerData
    // so the cached properties are always up to date.
    QVariantMap & properties = devicesProperties[path];
    for (QVariantMap::const_iterator it = changed_properties.constBegin();
         it != changed_properties.constEnd(); ++it) {
        properties[it.key()] = it.value();
    }

    for (const QString & property : invalidated_properties)
        properties.remove(property);

    const auto info = createDeviceInfoFromBluez5Device(properties);
    if (!info.isValid())
        return;

    if (changed_properties.contains(QStringLiteral("RSSI"))
        || changed_properties.contains(QStringLiteral("ManufacturerData"))) {

        for (qsizetype i = 0; i < discoveredDevices.size(); ++i) {
            if (discoveredDevices[i].address() == info.address()) {
                QBluetoothDeviceInfo::Fields updatedFields = QBluetoothDeviceInfo::Field::None;
                if (changed_properties.contains(QStringLiteral("RSSI"))) {
                    qCDebug(QT_BT_BLUEZ) << "Updating RSSI for" << info.address()
                                         << changed_properties.value(QStringLiteral("RSSI"));
                    discoveredDevices[i].setRssi(
                                changed_properties.value(QStringLiteral("RSSI")).toInt());
                    updatedFields.setFlag(QBluetoothDeviceInfo::Field::RSSI);
                }
                if (changed_properties.contains(QStringLiteral("ManufacturerData"))) {
                    qCDebug(QT_BT_BLUEZ) << "Updating ManufacturerData for" << info.address();
                    ManufacturerDataList changedManufacturerData =
                            qdbus_cast< ManufacturerDataList >(changed_properties.value(QStringLiteral("ManufacturerData")));

                    const QList<quint16> keys = changedManufacturerData.keys();
                    bool wasNewValue = false;
                    for (quint16 key : keys) {
                        bool added = discoveredDevices[i].setManufacturerData(key, changedManufacturerData.value(key).variant().toByteArray());
                        wasNewValue = (wasNewValue || added);
                    }

                    if (wasNewValue)
                        updatedFields.setFlag(QBluetoothDeviceInfo::Field::ManufacturerData);
                }

                if (lowEnergySearchTimeout > 0) {
                    if (discoveredDevices[i] != info) { // field other than manufacturer or rssi changed
                        if (discoveredDevices.at(i).name() == info.name()) {
                            qCDebug(QT_BT_BLUEZ) << "Almost Duplicate " << info.address()
                                                   << info.name() << "- replacing in place";
                            discoveredDevices.replace(i, info);
                            emit q->deviceDiscovered(info);
                        }
                    } else {
                        if (!updatedFields.testFlag(QBluetoothDeviceInfo::Field::None))
                            emit q->deviceUpdated(discoveredDevices[i], updatedFields);
                    }

                    return;
                }

                discoveredDevices.replace(i, info);
                emit q_ptr->deviceDiscovered(discoveredDevices[i]);

                if (!updatedFields.testFlag(QBluetoothDeviceInfo::Field::None))
                    emit q->deviceUpdated(discoveredDevices[i], updatedFields);
                return;
            }
        }
    }
}
QT_END_NAMESPACE
