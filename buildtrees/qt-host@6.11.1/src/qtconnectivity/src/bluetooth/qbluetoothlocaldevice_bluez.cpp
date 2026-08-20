// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>
#include <QtCore/QRandomGenerator>
#include <QtDBus/QDBusContext>

#include "qbluetoothlocaldevice.h"
#include "qbluetoothaddress.h"
#include "qbluetoothlocaldevice_p.h"

#include "bluez/bluez5_helper_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/properties_p.h"
#include "bluez/adapter1_bluez5_p.h"
#include "bluez/device1_bluez5_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace QtBluetoothPrivate; // for D-Bus wrappers

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this))
{
    d_ptr->currentMode = hostMode();
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
    d_ptr->currentMode = hostMode();
}

QString QBluetoothLocalDevice::name() const
{
    if (d_ptr->adapter)
        return d_ptr->adapter->alias();

    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    if (d_ptr->adapter)
        return QBluetoothAddress(d_ptr->adapter->address());

    return QBluetoothAddress();
}

void QBluetoothLocalDevice::powerOn()
{
    if (d_ptr->adapter)
        d_ptr->adapter->setPowered(true);
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!isValid())
        return;

    Q_D(QBluetoothLocalDevice);

    if (d->pendingHostModeChange != -1) {
        qCWarning(QT_BT_BLUEZ) << "setHostMode() ignored due to already pending mode change";
        return;
    }

    switch (mode) {
    case HostDiscoverableLimitedInquiry:
    case HostDiscoverable:
        if (hostMode() == HostPoweredOff) {
            // We first have to wait for BT to be powered on,
            // then we can set the host mode correctly
            d->pendingHostModeChange = static_cast<int>(HostDiscoverable);
            d->adapter->setPowered(true);
        } else {
            d->adapter->setDiscoverable(true);
        }
        break;
    case HostConnectable:
        if (hostMode() == HostPoweredOff) {
            d->pendingHostModeChange = static_cast<int>(HostConnectable);
            d->adapter->setPowered(true);
        } else {
            d->adapter->setDiscoverable(false);
        }
        break;
    case HostPoweredOff:
        d->adapter->setPowered(false);
        break;
    }
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (d_ptr->adapter) {
        if (!d_ptr->adapter->powered())
            return HostPoweredOff;
        else if (d_ptr->adapter->discoverable())
            return HostDiscoverable;
        else if (d_ptr->adapter->powered())
            return HostConnectable;
    }

    return HostPoweredOff;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    return d_ptr->connectedDevices();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> localDevices;

    initializeBluez5();
    OrgFreedesktopDBusObjectManagerInterface manager(
            QStringLiteral("org.bluez"), QStringLiteral("/"), QDBusConnection::systemBus());
    QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError())
        return localDevices;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin();
         it != managedObjectList.constEnd(); ++it) {
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd();
             ++jt) {
            const QString &iface = jt.key();
            const QVariantMap &ifaceValues = jt.value();

            if (iface == QStringLiteral("org.bluez.Adapter1")) {
                QBluetoothHostInfo hostInfo;
                const QString temp = ifaceValues.value(QStringLiteral("Address")).toString();

                hostInfo.setAddress(QBluetoothAddress(temp));
                if (hostInfo.address().isNull())
                    continue;
                hostInfo.setName(ifaceValues.value(QStringLiteral("Name")).toString());
                localDevices.append(hostInfo);
            }
        }
    }
    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    if (!isValid() || address.isNull()) {
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::PairingError));
        return;
    }

    const Pairing current_pairing = pairingStatus(address);
    if (current_pairing == pairing) {
        if (d_ptr->adapter) {
            // A possibly running discovery or pending pairing request should be canceled
            if (d_ptr->pairingDiscoveryTimer && d_ptr->pairingDiscoveryTimer->isActive()) {
                d_ptr->pairingDiscoveryTimer->stop();
            }

            if (d_ptr->pairingTarget) {
                qCDebug(QT_BT_BLUEZ) << "Cancelling pending pairing request to" << d_ptr->pairingTarget->address();
                QDBusPendingReply<> cancelReply = d_ptr->pairingTarget->CancelPairing();
                d_ptr->pairingRequestCanceled = true;
                cancelReply.waitForFinished();
                delete d_ptr->pairingTarget;
                d_ptr->pairingTarget = nullptr;
            }
        }
        QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothAddress, address),
                                  Q_ARG(QBluetoothLocalDevice::Pairing, pairing));
        return;
    }

    d_ptr->requestPairing(address, pairing);
}

void QBluetoothLocalDevicePrivate::requestPairing(const QBluetoothAddress &targetAddress,
                                                  QBluetoothLocalDevice::Pairing targetPairing)
{
    if (!isValid())
        return;

    //are we already discovering something? -> abort those attempts
    if (pairingDiscoveryTimer && pairingDiscoveryTimer->isActive()) {
        pairingDiscoveryTimer->stop();
        QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(adapter->path());
    }

    if (pairingTarget) {
        delete pairingTarget;
        pairingTarget = nullptr;
    }

    // pairing implies that the device was found
    // if we cannot find it we may have to turn on Discovery mode for a limited amount of time

    // check device doesn't already exist
    QDBusPendingReply<ManagedObjectList> reply = manager->GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        emit q_ptr->errorOccurred(QBluetoothLocalDevice::PairingError);
        return;
    }

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Device1")) {

                OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                path.path(),
                                                QDBusConnection::systemBus());
                if (targetAddress == QBluetoothAddress(device.address())) {
                    qCDebug(QT_BT_BLUEZ) << "Initiating direct pair to" << targetAddress.toString();
                    //device exist -> directly work with it
                    processPairing(path.path(), targetPairing);
                    return;
                }
            }
        }
    }

    //no device matching -> turn on discovery
    QtBluezDiscoveryManager::instance()->registerDiscoveryInterest(adapter->path());

    address = targetAddress;
    pairing = targetPairing;
    if (!pairingDiscoveryTimer) {
        pairingDiscoveryTimer = new QTimer(this);
        pairingDiscoveryTimer->setSingleShot(true);
        pairingDiscoveryTimer->setInterval(20000); //20s
        connect(pairingDiscoveryTimer, &QTimer::timeout,
                this, &QBluetoothLocalDevicePrivate::pairingDiscoveryTimedOut);
    }

    qCDebug(QT_BT_BLUEZ) << "Initiating discovery for pairing on" << targetAddress.toString();
    pairingDiscoveryTimer->start();
}

/*!
 * \internal
 *
 * Found a matching device. Now we must ensure its pairing/trusted state is as desired.
 * If it has to be paired then we need another roundtrip through the event loop
 * while we wait for the user to accept the pairing dialogs.
 */
void QBluetoothLocalDevicePrivate::processPairing(const QString &objectPath,
                                                  QBluetoothLocalDevice::Pairing target)
{
    if (pairingTarget)
        delete pairingTarget;

    //stop possibly running discovery
    if (pairingDiscoveryTimer && pairingDiscoveryTimer->isActive()) {
        pairingDiscoveryTimer->stop();

        QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(adapter->path());
    }

    pairingTarget = new OrgBluezDevice1Interface(QStringLiteral("org.bluez"), objectPath,
                                                 QDBusConnection::systemBus(), this);
    const QBluetoothAddress targetAddress(pairingTarget->address());

    Q_Q(QBluetoothLocalDevice);

    switch (target) {
    case QBluetoothLocalDevice::Unpaired: {
        delete pairingTarget;
        pairingTarget = nullptr;

        QDBusPendingReply<> removeReply = adapter->RemoveDevice(QDBusObjectPath(objectPath));
        auto watcher = new QDBusPendingCallWatcher(removeReply, this);
        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, [q, targetAddress](QDBusPendingCallWatcher* watcher){
            QDBusPendingReply<> reply = *watcher;
            if (reply.isError())
                emit q->errorOccurred(QBluetoothLocalDevice::PairingError);
            else
                emit q->pairingFinished(targetAddress, QBluetoothLocalDevice::Unpaired);

            watcher->deleteLater();
        });
        break;
    }
    case QBluetoothLocalDevice::Paired:
    case QBluetoothLocalDevice::AuthorizedPaired:
        pairing = target;

        if (!pairingTarget->paired()) {
            qCDebug(QT_BT_BLUEZ) << "Sending pairing request to" << pairingTarget->address();
            //initiate the pairing
            QDBusPendingReply<> pairReply = pairingTarget->Pair();
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pairReply, this);
            connect(watcher, &QDBusPendingCallWatcher::finished,
                    this, &QBluetoothLocalDevicePrivate::pairingCompleted);
            return;
        }

        //already paired but Trust level must be adjusted
        if (target == QBluetoothLocalDevice::AuthorizedPaired && !pairingTarget->trusted())
            pairingTarget->setTrusted(true);
        else if (target == QBluetoothLocalDevice::Paired && pairingTarget->trusted())
            pairingTarget->setTrusted(false);

        delete pairingTarget;
        pairingTarget = nullptr;

        emit q->pairingFinished(targetAddress, target);

        break;
    default:
        break;
    }
}

void QBluetoothLocalDevicePrivate::pairingDiscoveryTimedOut()
{
    qCWarning(QT_BT_BLUEZ) << "Discovery for pairing purposes failed. Cannot find parable device.";

    QtBluezDiscoveryManager::instance()->unregisterDiscoveryInterest(adapter->path());

    emit q_ptr->errorOccurred(QBluetoothLocalDevice::PairingError);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    if (address.isNull())
        return Unpaired;

    if (isValid())
    {
        QDBusPendingReply<ManagedObjectList> reply = d_ptr->manager->GetManagedObjects();
        reply.waitForFinished();
        if (reply.isError())
            return Unpaired;

        ManagedObjectList managedObjectList = reply.value();
        for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
            const QDBusObjectPath &path = it.key();
            const InterfaceList &ifaceList = it.value();

            for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
                const QString &iface = jt.key();

                if (iface == QStringLiteral("org.bluez.Device1")) {

                    OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                    path.path(),
                                                    QDBusConnection::systemBus());

                    if (address == QBluetoothAddress(device.address())) {
                        if (device.trusted() && device.paired())
                            return AuthorizedPaired;
                        else if (device.paired())
                            return Paired;
                        else
                            return Unpaired;
                    }
                }
            }
        }
    }

    return Unpaired;
}

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                                           QBluetoothAddress address) :
        localAddress(address),
        pendingHostModeChange(-1),
        q_ptr(q)
{
    registerQBluetoothLocalDeviceMetaType();
    initializeBluez5();
    initializeAdapter();

    connectDeviceChanges();
}

bool objectPathIsForThisDevice(const QString &adapterPath, const QString &objectPath)
{
    return (!adapterPath.isEmpty() && objectPath.startsWith(adapterPath));
}

void QBluetoothLocalDevicePrivate::connectDeviceChanges()
{
    if (isValid()) {
        //setup property change notifications for all existing devices
        QDBusPendingReply<ManagedObjectList> reply = manager->GetManagedObjects();
        reply.waitForFinished();
        if (reply.isError())
            return;

        OrgFreedesktopDBusPropertiesInterface *monitor = nullptr;

        ManagedObjectList managedObjectList = reply.value();
        for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
            const QDBusObjectPath &path = it.key();
            const InterfaceList &ifaceList = it.value();

            // don't track connected devices from other adapters but the current
            if (!objectPathIsForThisDevice(deviceAdapterPath, path.path()))
                continue;

            for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
                const QString &iface = jt.key();
                const QVariantMap &ifaceValues = jt.value();

                if (iface == QStringLiteral("org.bluez.Device1")) {
                    monitor = new OrgFreedesktopDBusPropertiesInterface(QStringLiteral("org.bluez"),
                                                                        path.path(),
                                                                        QDBusConnection::systemBus(), this);
                    connect(monitor, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                            this, &QBluetoothLocalDevicePrivate::PropertiesChanged);
                    deviceChangeMonitors.insert(path.path(), monitor);

                    if (ifaceValues.value(QStringLiteral("Connected"), false).toBool()) {
                        QBluetoothAddress address(ifaceValues.value(QStringLiteral("Address")).toString());
                        connectedDevicesSet.insert(address);
                    }
                }
            }
        }
    }
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    delete adapter;
    delete adapterProperties;
    delete manager;
    delete pairingTarget;

    qDeleteAll(deviceChangeMonitors);
}

void QBluetoothLocalDevicePrivate::initializeAdapter()
{
    if (adapter)
        return;

    //get all local adapters
    if (!manager)
        manager = new OrgFreedesktopDBusObjectManagerInterface(
                                                     QStringLiteral("org.bluez"),
                                                     QStringLiteral("/"),
                                                     QDBusConnection::systemBus(), this);

    connect(manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
            this, &QBluetoothLocalDevicePrivate::InterfacesAdded);
    connect(manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this, &QBluetoothLocalDevicePrivate::InterfacesRemoved);

    bool ok = true;
    const QString adapterPath = findAdapterForAddress(localAddress, &ok);
    if (!ok || adapterPath.isEmpty())
        return;

    deviceAdapterPath = adapterPath;
    adapter = new OrgBluezAdapter1Interface(QStringLiteral("org.bluez"), adapterPath,
                                            QDBusConnection::systemBus(), this);

    if (adapter) {
        //hook up propertiesChanged for current adapter
        adapterProperties = new OrgFreedesktopDBusPropertiesInterface(
                QStringLiteral("org.bluez"), adapter->path(),
                QDBusConnection::systemBus(), this);
        connect(adapterProperties, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                this, &QBluetoothLocalDevicePrivate::PropertiesChanged);
    }
}

void QBluetoothLocalDevicePrivate::PropertiesChanged(const QString &interface,
                                                     const QVariantMap &changed_properties,
                                                     const QStringList &/*invalidated_properties*/,
                                                     const QDBusMessage &)
{
    //qDebug() << "Change" << interface << changed_properties;
    if (interface == QStringLiteral("org.bluez.Adapter1")) {
        //update host mode
        if (changed_properties.contains(QStringLiteral("Discoverable"))
                || changed_properties.contains(QStringLiteral("Powered"))) {

            QBluetoothLocalDevice::HostMode mode;

            if (!adapter->powered()) {
                mode = QBluetoothLocalDevice::HostPoweredOff;
            } else {
                if (adapter->discoverable())
                    mode = QBluetoothLocalDevice::HostDiscoverable;
                else
                    mode = QBluetoothLocalDevice::HostConnectable;

                if (pendingHostModeChange != -1) {

                    if (static_cast<int>(mode) != pendingHostModeChange) {
                        adapter->setDiscoverable(
                                pendingHostModeChange
                                == static_cast<int>(QBluetoothLocalDevice::HostDiscoverable));
                        pendingHostModeChange = -1;
                        return;
                    }
                    pendingHostModeChange = -1;
                }
            }

            if (mode != currentMode)
                emit q_ptr->hostModeStateChanged(mode);

            currentMode = mode;
        }
    } else if (interface == QStringLiteral("org.bluez.Device1")
               && changed_properties.contains(QStringLiteral("Connected"))) {
        // update list of connected devices
        OrgFreedesktopDBusPropertiesInterface *senderIface =
                qobject_cast<OrgFreedesktopDBusPropertiesInterface*>(sender());
        if (!senderIface)
            return;

        const QString currentPath = senderIface->path();
        bool isConnected = changed_properties.value(QStringLiteral("Connected"), false).toBool();
        OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), currentPath,
                                        QDBusConnection::systemBus());
        const QBluetoothAddress changedAddress(device.address());
        bool isInSet = connectedDevicesSet.contains(changedAddress);
        if (isConnected && !isInSet) {
            connectedDevicesSet.insert(changedAddress);
            emit q_ptr->deviceConnected(changedAddress);
        } else if (!isConnected && isInSet) {
            connectedDevicesSet.remove(changedAddress);
            emit q_ptr->deviceDisconnected(changedAddress);
        }
    }
}

void QBluetoothLocalDevicePrivate::InterfacesAdded(const QDBusObjectPath &object_path, InterfaceList interfaces_and_properties)
{
    if (interfaces_and_properties.contains(QStringLiteral("org.bluez.Device1"))
        && !deviceChangeMonitors.contains(object_path.path())) {
        // a new device was added which we need to add to list of known devices

        if (objectPathIsForThisDevice(deviceAdapterPath, object_path.path())) {
            OrgFreedesktopDBusPropertiesInterface *monitor = new OrgFreedesktopDBusPropertiesInterface(
                                               QStringLiteral("org.bluez"),
                                               object_path.path(),
                                               QDBusConnection::systemBus());
            connect(monitor, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                    this, &QBluetoothLocalDevicePrivate::PropertiesChanged);
            deviceChangeMonitors.insert(object_path.path(), monitor);

            const QVariantMap ifaceValues = interfaces_and_properties.value(QStringLiteral("org.bluez.Device1"));
            if (ifaceValues.value(QStringLiteral("Connected"), false).toBool()) {
                QBluetoothAddress address(ifaceValues.value(QStringLiteral("Address")).toString());
                connectedDevicesSet.insert(address);
                emit q_ptr->deviceConnected(address);
            }
        }
    }

    if (pairingDiscoveryTimer && pairingDiscoveryTimer->isActive()
        && interfaces_and_properties.contains(QStringLiteral("org.bluez.Device1"))) {
        //device discovery for pairing found new remote device
        OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                        object_path.path(), QDBusConnection::systemBus());
        if (!address.isNull() && address == QBluetoothAddress(device.address()))
            processPairing(object_path.path(), pairing);
    }
}

void QBluetoothLocalDevicePrivate::InterfacesRemoved(const QDBusObjectPath &object_path,
                                                     const QStringList &interfaces)
{
    if (deviceChangeMonitors.contains(object_path.path())
            && interfaces.contains(QLatin1String("org.bluez.Device1"))) {

        if (objectPathIsForThisDevice(deviceAdapterPath, object_path.path())) {
            //a device was removed
            delete deviceChangeMonitors.take(object_path.path());

            //the path contains the address (e.g.: /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX)
            //-> use it to update current list of connected devices
            QString addressString = object_path.path().right(17);
            addressString.replace(QStringLiteral("_"), QStringLiteral(":"));
            const QBluetoothAddress address(addressString);
            bool found = connectedDevicesSet.remove(address);
            if (found)
                emit q_ptr->deviceDisconnected(address);
        }
    }

    if (adapter && object_path.path() == adapter->path()
        && interfaces.contains(QLatin1String("org.bluez.Adapter1"))) {
        qCDebug(QT_BT_BLUEZ) << "Adapter" << adapter->path() << "was removed";
        // current adapter was removed -> invalidate the instance
        delete adapter;
        adapter = nullptr;
        manager->deleteLater();
        manager = nullptr;
        delete adapterProperties;
        adapterProperties = nullptr;

        delete pairingTarget;
        pairingTarget = nullptr;

        // turn  off connectivity monitoring
        qDeleteAll(deviceChangeMonitors);
        deviceChangeMonitors.clear();
        connectedDevicesSet.clear();
    }
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return adapter && manager;
}

QList<QBluetoothAddress> QBluetoothLocalDevicePrivate::connectedDevices() const
{
    return connectedDevicesSet.values();
}

void QBluetoothLocalDevicePrivate::pairingCompleted(QDBusPendingCallWatcher *watcher)
{
    Q_Q(QBluetoothLocalDevice);
    QDBusPendingReply<> reply = *watcher;

    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Failed to create pairing" << reply.error().name();
        const bool canceledByUs =
                (reply.error().name() == QStringLiteral("org.bluez.Error.AuthenticationCanceled"))
                    && pairingRequestCanceled;
        if (!canceledByUs)
            emit q->errorOccurred(QBluetoothLocalDevice::PairingError);

        pairingRequestCanceled = false;
        watcher->deleteLater();
        return;
    }

    pairingRequestCanceled = false;

    if (adapter) {
        if (!pairingTarget) {
            qCWarning(QT_BT_BLUEZ) << "Pairing target expected but found null pointer.";
            emit q->errorOccurred(QBluetoothLocalDevice::PairingError);
            watcher->deleteLater();
            return;
        }

        if (!pairingTarget->paired()) {
            qCWarning(QT_BT_BLUEZ) << "Device was not paired as requested";
            emit q->errorOccurred(QBluetoothLocalDevice::PairingError);
            watcher->deleteLater();
            return;
        }

        const QBluetoothAddress targetAddress(pairingTarget->address());

        if (pairing == QBluetoothLocalDevice::AuthorizedPaired && !pairingTarget->trusted())
            pairingTarget->setTrusted(true);
        else if (pairing == QBluetoothLocalDevice::Paired && pairingTarget->trusted())
            pairingTarget->setTrusted(false);

        delete pairingTarget;
        pairingTarget = nullptr;

        emit q->pairingFinished(targetAddress, pairing);
    }

    watcher->deleteLater();
}

QT_END_NAMESPACE

#include "moc_qbluetoothlocaldevice_p.cpp"
