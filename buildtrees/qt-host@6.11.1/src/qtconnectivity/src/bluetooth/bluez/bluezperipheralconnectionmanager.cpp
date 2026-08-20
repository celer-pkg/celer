// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "bluezperipheralconnectionmanager_p.h"
#include "device1_bluez5_p.h"

#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtDBus/QDBusConnection>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace Qt::StringLiterals;
using namespace QtBluetoothPrivate; // for D-Bus wrappers

QtBluezPeripheralConnectionManager::QtBluezPeripheralConnectionManager(
        const QBluetoothAddress& localAddress, QObject* parent)
    : QObject(parent),
      m_localDevice(new QBluetoothLocalDevice(localAddress, this))
{
    QObject::connect(m_localDevice, &QBluetoothLocalDevice::deviceDisconnected,
                     this, &QtBluezPeripheralConnectionManager::remoteDeviceDisconnected);
}

void QtBluezPeripheralConnectionManager::remoteDeviceAccessEvent(
        const QString& remoteDeviceObjectPath, quint16 mtu)
{
    if (m_clients.contains(remoteDeviceObjectPath))
        return; // Already aware of the client

    std::unique_ptr<OrgBluezDevice1Interface> device{new OrgBluezDevice1Interface(
                                                     "org.bluez"_L1, remoteDeviceObjectPath,
                                                     QDBusConnection::systemBus(), this)};

    qCDebug(QT_BT_BLUEZ) << "New LE Gatt client connected: " << remoteDeviceObjectPath
                         << device->address() << device->name() << "mtu:" << mtu;

    RemoteDeviceDetails details{QBluetoothAddress{device->address()}, device->name(), mtu};

    m_clients.insert(remoteDeviceObjectPath, details);
    if (!m_connected) {
        m_connected = true;
        emit connectivityStateChanged(true);
    }
    emit remoteDeviceChanged(details.address, details.name, details.mtu);
}

void QtBluezPeripheralConnectionManager::remoteDeviceDisconnected(const QBluetoothAddress& address)
{
    // Find if the disconnected device was gatt client
    bool remoteDetailsChanged{false};
    for (auto it = m_clients.begin(); it != m_clients.end(); it++) {
        if (it.value().address == address) {
            qCDebug(QT_BT_BLUEZ) << "LE Gatt client disconnected:" << address;
            remoteDetailsChanged = true;
            m_clients.remove(it.key());
            break;
        }
    }

    if (!remoteDetailsChanged)
        return;

    if (m_clients.isEmpty() && m_connected) {
        m_connected = false;
        emit connectivityStateChanged(false);
    }

    // If a client disconnected but there are others, pick any other.
    // Qt API doesn't distinguish between clients
    if (!m_clients.isEmpty()) {
        emit remoteDeviceChanged(m_clients.last().address,
                                 m_clients.last().name, m_clients.last().mtu);
    }
}

void QtBluezPeripheralConnectionManager::disconnectDevices()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); it++) {
        std::unique_ptr<OrgBluezDevice1Interface> device{new OrgBluezDevice1Interface(
                        "org.bluez"_L1, it.key(), QDBusConnection::systemBus())};
        device->Disconnect();
    }
    reset();
}

void QtBluezPeripheralConnectionManager::reset()
{
    m_connected = false;
    m_clients.clear();
}

QT_END_NAMESPACE

#include "moc_bluezperipheralconnectionmanager_p.cpp"
