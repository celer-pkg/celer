// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QLoggingCategory>

#include "remotedevicemanager_p.h"
#include "bluez5_helper_p.h"
#include "device1_bluez5_p.h"
#include "objectmanager_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace QtBluetoothPrivate; // for D-Bus wrappers

/*!
    \class RemoteDeviceManager
    \inmodule QtBluetooth
    \internal

    Convenience wrapper around org.bluez.Device1 management.
    Very simple and not thread safe.
*/
RemoteDeviceManager::RemoteDeviceManager(
        const QBluetoothAddress &address, QObject *parent)
    : QObject(parent), localAddress(address)
{
    initializeBluez5();

    bool ok = false;
    adapterPath = findAdapterForAddress(address, &ok);
    if (!ok || adapterPath.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot initialize RemoteDeviceManager";
    }
}

bool RemoteDeviceManager::scheduleJob(JobType job, const QList<QBluetoothAddress> &remoteDevices)
{
    if (adapterPath.isEmpty())
        return false;

    for (const auto& remote : remoteDevices)
        jobQueue.push_back(std::make_pair(job, remote));

    QTimer::singleShot(0, this, [this](){ runQueue(); });
    return true;
}

void RemoteDeviceManager::runQueue()
{
    if (jobInProgress || adapterPath.isEmpty())
        return;

    if (jobQueue.empty())
        return;

    jobInProgress = true;
    switch (jobQueue.front().first) {
    case JobType::JobDisconnectDevice:
        disconnectDevice(jobQueue.front().second);
        break;
    default:
        break;
    }
}

void RemoteDeviceManager::prepareNextJob()
{
    Q_ASSERT(!jobQueue.empty());

    jobQueue.pop_front();
    jobInProgress = false;

    qCDebug(QT_BT_BLUEZ) << "RemoteDeviceManager job queue status:" << jobQueue.empty();
    if (jobQueue.empty())
        emit finished();
    else
        runQueue();
}

void RemoteDeviceManager::disconnectDevice(const QBluetoothAddress &remote)
{
    // collect initial set of information
    OrgFreedesktopDBusObjectManagerInterface managerBluez5(
                                               QStringLiteral("org.bluez"),
                                               QStringLiteral("/"),
                                               QDBusConnection::systemBus(), this);
    QDBusPendingReply<ManagedObjectList> reply = managerBluez5.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        QTimer::singleShot(0, this, [this](){ prepareNextJob(); });
        return;
    }

    bool jobStarted = false;
    ManagedObjectList managedObjectList = reply.value();
    for (auto it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (auto jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (path.path().indexOf(adapterPath) != 0)
                continue; //devices whose path doesn't start with same path we skip

            if (iface != QStringLiteral("org.bluez.Device1"))
                continue;

            const QBluetoothAddress foundAddress(ifaceList.value(iface).value(QStringLiteral("Address")).toString());
            if (foundAddress != remote)
                continue;

            // found the correct Device1 path
            OrgBluezDevice1Interface* device1 = new OrgBluezDevice1Interface(QStringLiteral("org.bluez"),
                                                                             path.path(),
                                                                             QDBusConnection::systemBus(),
                                                                             this);
            QDBusPendingReply<> asyncReply = device1->Disconnect();
            QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(asyncReply, this);
            const auto watcherFinished = [this, device1](QDBusPendingCallWatcher* call) {
                call->deleteLater();
                device1->deleteLater();
                prepareNextJob();
            };
            connect(watcher, &QDBusPendingCallWatcher::finished, this, watcherFinished);
            jobStarted = true;
            break;
        }
    }

    if (!jobStarted) {
        qCDebug(QT_BT_BLUEZ) << "RemoteDeviceManager JobDisconnectDevice failed";
        QTimer::singleShot(0, this, [this](){ prepareNextJob(); });
    }
}

QT_END_NAMESPACE

#include "moc_remotedevicemanager_p.cpp"
