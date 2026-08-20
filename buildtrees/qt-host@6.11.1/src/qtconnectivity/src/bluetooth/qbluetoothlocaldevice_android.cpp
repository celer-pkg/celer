// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothlocaldevice_p.h"
#include "android/localdevicebroadcastreceiver_p.h"
#include "android/androidutils_p.h"
#include "android/jni_android_p.h"
#include <QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothAddress>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate(
    QBluetoothLocalDevice *q, const QBluetoothAddress &address) :
    q_ptr(q)
{
    registerQBluetoothLocalDeviceMetaType();

    initialize(address);

    receiver = new LocalDeviceBroadcastReceiver(q_ptr);
    connect(receiver, &LocalDeviceBroadcastReceiver::hostModeStateChanged,
            this, &QBluetoothLocalDevicePrivate::processHostModeChange);
    connect(receiver, &LocalDeviceBroadcastReceiver::pairingStateChanged,
            this, &QBluetoothLocalDevicePrivate::processPairingStateChanged);
    connect(receiver, &LocalDeviceBroadcastReceiver::connectDeviceChanges,
            this, &QBluetoothLocalDevicePrivate::processConnectDeviceChanges);
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    receiver->unregisterReceiver();
    delete receiver;
    delete obj;
}

QJniObject *QBluetoothLocalDevicePrivate::adapter()
{
    return obj;
}

void QBluetoothLocalDevicePrivate::initialize(const QBluetoothAddress &address)
{
    QJniObject adapter = getDefaultBluetoothAdapter();

    if (!adapter.isValid()) {
        qCWarning(QT_BT_ANDROID) <<  "Device does not support Bluetooth";
        return;
    }

    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) <<  "Local device initialize() failed due to missing permissions";
        return;
    }

    obj = new QJniObject(adapter);
    if (!address.isNull()) {
        const QString localAddress = obj->callMethod<jstring>("getAddress").toString();
        if (localAddress != address.toString()) {
            // passed address not local one -> invalid
            delete obj;
            obj = nullptr;
        }
    }
}

bool QBluetoothLocalDevicePrivate::isValid() const
{
    return obj ? true : false;
}

void QBluetoothLocalDevicePrivate::processHostModeChange(QBluetoothLocalDevice::HostMode newMode)
{
    qCDebug(QT_BT_ANDROID) << "Processing host mode change:" << newMode
                           << ", pending transition:" << pendingConnectableHostModeTransition;
    if (!pendingConnectableHostModeTransition) {
        // If host mode is not in transition -> pass data on
        emit q_ptr->hostModeStateChanged(newMode);
        return;
    }

    // Host mode is in transition: check if the new mode is 'off' in which state
    // we can enter the targeted 'Connectable' state
    if (isValid() && newMode == QBluetoothLocalDevice::HostPoweredOff) {
        const bool success = (bool)QJniObject::callStaticMethod<jboolean>(
                    QtJniTypes::Traits<QtJniTypes::QtBtBroadcastReceiver>::className(),
                    "setEnabled");
        if (!success) {
            qCWarning(QT_BT_ANDROID) << "Transitioning Bluetooth from OFF to ON failed";
            emit q_ptr->errorOccurred(QBluetoothLocalDevice::UnknownError);
        }
    }
    pendingConnectableHostModeTransition = false;
}

// Return -1 if address is not part of a pending pairing request
// Otherwise it returns the index of address in pendingPairings
int QBluetoothLocalDevicePrivate::pendingPairing(const QBluetoothAddress &address)
{
    for (qsizetype i = 0; i < pendingPairings.size(); ++i) {
        if (pendingPairings.at(i).first == address)
            return i;
    }

    return -1;
}

void QBluetoothLocalDevicePrivate::processPairingStateChanged(
    const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    int index = pendingPairing(address);

    if (index < 0)
        return; // ignore unrelated pairing signals

    QPair<QBluetoothAddress, bool> entry = pendingPairings.takeAt(index);
    if ((entry.second && pairing == QBluetoothLocalDevice::Paired)
        || (!entry.second && pairing == QBluetoothLocalDevice::Unpaired)) {
        emit q_ptr->pairingFinished(address, pairing);
    } else {
        emit q_ptr->errorOccurred(QBluetoothLocalDevice::PairingError);
    }
}

void QBluetoothLocalDevicePrivate::processConnectDeviceChanges(const QBluetoothAddress &address,
                                                               bool isConnectEvent)
{
    if (isConnectEvent) { // connect event
        if (connectedDevices.contains(address))
            return;
        connectedDevices.append(address);
        emit q_ptr->deviceConnected(address);
    } else { // disconnect event
        connectedDevices.removeAll(address);
        emit q_ptr->deviceDisconnected(address);
    }
}

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, QBluetoothAddress()))
{
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent) :
    QObject(parent),
    d_ptr(new QBluetoothLocalDevicePrivate(this, address))
{
}

QString QBluetoothLocalDevice::name() const
{
    if (d_ptr->adapter())
        return d_ptr->adapter()->callMethod<jstring>("getName").toString();

    return QString();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    QString result;
    if (d_ptr->adapter())
        result = d_ptr->adapter()->callMethod<jstring>("getAddress").toString();

    QBluetoothAddress address(result);
    return address;
}

void QBluetoothLocalDevice::powerOn()
{
    if (hostMode() != HostPoweredOff)
        return;

    if (d_ptr->adapter()) {
        bool success(false);
        if (QNativeInterface::QAndroidApplication::sdkVersion() >= 31) {
            success = (bool)QJniObject::callStaticMethod<jboolean>(
                        QtJniTypes::Traits<QtJniTypes::QtBtBroadcastReceiver>::className(),
                        "setEnabled");
        } else {
            success = (bool)d_ptr->adapter()->callMethod<jboolean>("enable");
        }
        if (!success) {
            qCWarning(QT_BT_ANDROID) << "Enabling bluetooth failed";
            emit errorOccurred(QBluetoothLocalDevice::UnknownError);
        }
    }
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode requestedMode)
{
    QBluetoothLocalDevice::HostMode nextMode = requestedMode;
    if (requestedMode == HostDiscoverableLimitedInquiry)
        nextMode = HostDiscoverable;

    if (nextMode == hostMode())
        return;

    switch (nextMode) {

    case QBluetoothLocalDevice::HostPoweredOff: {
        bool success = false;
        if (d_ptr->adapter()) {
            if (QNativeInterface::QAndroidApplication::sdkVersion() >= 31) {
                success = (bool)QJniObject::callStaticMethod<jboolean>(
                            QtJniTypes::Traits<QtJniTypes::QtBtBroadcastReceiver>::className(),
                            "setDisabled");
            } else {
                success = (bool)d_ptr->adapter()->callMethod<jboolean>("disable");
            }
        }
        if (!success) {
            qCWarning(QT_BT_ANDROID) << "Unable to power off the adapter";
            emit errorOccurred(QBluetoothLocalDevice::UnknownError);
        }
        break;
    }

    case QBluetoothLocalDevice::HostConnectable: {
        if (hostMode() == QBluetoothLocalDevice::HostDiscoverable) {
            // On Android 'Discoverable' is actually 'CONNECTABLE_DISCOVERABLE', and
            // it seems we cannot go directly from "Discoverable" to "Connectable". Instead
            // we need to go to disabled mode first and then to the 'Connectable' mode
            setHostMode(QBluetoothLocalDevice::HostPoweredOff);
            d_ptr->pendingConnectableHostModeTransition = true;
        } else {
            const bool success = (bool)QJniObject::callStaticMethod<jboolean>(
                        QtJniTypes::Traits<QtJniTypes::QtBtBroadcastReceiver>::className(),
                        "setEnabled");
            if (!success) {
                qCWarning(QT_BT_ANDROID) << "Unable to enable the Bluetooth";
                emit errorOccurred(QBluetoothLocalDevice::UnknownError);
            }
        }
        break;
    }

    case QBluetoothLocalDevice::HostDiscoverable: {
        if (!ensureAndroidPermission(QBluetoothPermission::Advertise)) {
            qCWarning(QT_BT_ANDROID) << "Local device setHostMode() failed due to "
                                        "missing permissions";
            emit errorOccurred(QBluetoothLocalDevice::MissingPermissionsError);
            return;
        }
        const bool success = (bool)QJniObject::callStaticMethod<jboolean>(
                    QtJniTypes::Traits<QtJniTypes::QtBtBroadcastReceiver>::className(),
                    "setDiscoverable");
        if (!success) {
            qCWarning(QT_BT_ANDROID) << "Unable to set Bluetooth as discoverable";
            emit errorOccurred(QBluetoothLocalDevice::UnknownError);
        }
        break;
    }
    default:
        qCWarning(QT_BT_ANDROID) << "setHostMode() unsupported host mode:" << nextMode;
        break;
    }
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (d_ptr->adapter()) {
        jint scanMode = d_ptr->adapter()->callMethod<jint>("getScanMode");

        switch (scanMode) {
        case 20:     // BluetoothAdapter.SCAN_MODE_NONE
            return HostPoweredOff;
        case 21:     // BluetoothAdapter.SCAN_MODE_CONNECTABLE
            return HostConnectable;
        case 23:     // BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE
            return HostDiscoverable;
        default:
            break;
        }
    }

    return HostPoweredOff;
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    // As a static class function we need to ensure permissions here (in addition to initialize())
    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) <<  "Local device allDevices() failed due to "
                                     "missing permissions";
        return {};
    }
    // Android only supports max of one device (so far)
    QList<QBluetoothHostInfo> localDevices;

    QJniObject o = getDefaultBluetoothAdapter();
    if (o.isValid()) {
        QBluetoothHostInfo info;
        info.setName(o.callMethod<jstring>("getName").toString());
        info.setAddress(QBluetoothAddress(o.callMethod<jstring>("getAddress").toString()));
        localDevices.append(info);
    }
    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    if (address.isNull()) {
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::PairingError));
        return;
    }

    const Pairing previousPairing = pairingStatus(address);
    Pairing newPairing = pairing;
    if (pairing == AuthorizedPaired) // AuthorizedPaired same as Paired on Android
        newPairing = Paired;

    if (previousPairing == newPairing) {
        QMetaObject::invokeMethod(this, "pairingFinished", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothAddress, address),
                                  Q_ARG(QBluetoothLocalDevice::Pairing, newPairing));
        return;
    }

    if (!d_ptr->adapter()) {
        qCWarning(QT_BT_ANDROID) <<  "Unable to pair, invalid adapter";
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::PairingError));
        return;
    }

    QJniObject inputString = QJniObject::fromString(address.toString());
    jboolean success = QJniObject::callStaticMethod<jboolean>(
        QtJniTypes::Traits<QtJniTypes::QtBtBroadcastReceiver>::className(),
        "setPairingMode",
        inputString.object<jstring>(),
        jboolean(newPairing == Paired ? JNI_TRUE : JNI_FALSE));

    if (!success) {
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothLocalDevice::Error,
                                        QBluetoothLocalDevice::PairingError));
    } else {
        d_ptr->pendingPairings.append(qMakePair(address, newPairing == Paired ? true : false));
    }
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(
    const QBluetoothAddress &address) const
{
    if (address.isNull() || !d_ptr->adapter())
        return Unpaired;

    QJniObject inputString = QJniObject::fromString(address.toString());
    QJniObject remoteDevice
        = d_ptr->adapter()->callMethod<QtJniTypes::BluetoothDevice>("getRemoteDevice",
                                                                    inputString.object<jstring>());

    if (!remoteDevice.isValid())
        return Unpaired;

    jint bondState = remoteDevice.callMethod<jint>("getBondState");
    switch (bondState) {
    case 12: // BluetoothDevice.BOND_BONDED
        return Paired;
    default:
        break;
    }

    return Unpaired;
}

QList<QBluetoothAddress> QBluetoothLocalDevice::connectedDevices() const
{
    /*
     * Android does not have an API to list all connected devices. We have to collect
     * the information based on a few indicators.
     *
     * Primarily we detect connected devices by monitoring connect/disconnect signals.
     * Unfortunately the list may only be complete after very long monitoring time.
     * However there are some Android APIs which provide the list of connected devices
     * for specific Bluetooth profiles. QtBluetoothBroadcastReceiver.getConnectedDevices()
     * returns a few connections of common profiles. The returned list is not complete either
     * but at least it can complement our already detected connections.
     */
    using namespace QtJniTypes;

    const auto connectedDevices = QtBtBroadcastReceiver::callStaticMethod<String[]>("getConnectedDevices");

    if (!connectedDevices.isValid())
        return d_ptr->connectedDevices;

    QList<QBluetoothAddress> knownAddresses = d_ptr->connectedDevices;
    for (const auto &device : connectedDevices) {
        QBluetoothAddress address(device.toString());
        if (!address.isNull() && !knownAddresses.contains(address))
            knownAddresses.append(address);
    }

    return knownAddresses;
}

QT_END_NAMESPACE
