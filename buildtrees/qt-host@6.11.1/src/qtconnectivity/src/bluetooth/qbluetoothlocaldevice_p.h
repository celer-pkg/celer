// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2014 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHLOCALDEVICE_P_H
#define QBLUETOOTHLOCALDEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtBluetooth/qtbluetoothglobal.h>

#include "qbluetoothlocaldevice.h"

#if QT_CONFIG(bluez)
#include <QObject>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QSet>
#include "bluez/bluez5_helper_p.h"

namespace QtBluetoothPrivate {

class OrgBluezAdapter1Interface;
class OrgFreedesktopDBusPropertiesInterface;
class OrgFreedesktopDBusObjectManagerInterface;
class OrgBluezDevice1Interface;

} // namespace QtBluetoothPrivate

QT_BEGIN_NAMESPACE
class QDBusPendingCallWatcher;
QT_END_NAMESPACE
#endif

#ifdef QT_WINRT_BLUETOOTH
#include "qbluetoothutils_winrt_p.h"
#include <winrt/Windows.Devices.Bluetooth.h>
QT_BEGIN_NAMESPACE
struct PairingWorker;
QT_END_NAMESPACE
#endif

#ifdef QT_ANDROID_BLUETOOTH
#include <jni.h>
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/QPair>
#endif

QT_BEGIN_NAMESPACE

extern void registerQBluetoothLocalDeviceMetaType();

class QBluetoothAddress;

#ifdef QT_ANDROID_BLUETOOTH
class LocalDeviceBroadcastReceiver;
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
public:
    QBluetoothLocalDevicePrivate(
        QBluetoothLocalDevice *q, const QBluetoothAddress &address = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    QJniObject *adapter();
    void initialize(const QBluetoothAddress &address);
    static bool startDiscovery();
    static bool cancelDiscovery();
    static bool isDiscovering();
    bool isValid() const;

private slots:
    void processHostModeChange(QBluetoothLocalDevice::HostMode newMode);
    void processPairingStateChanged(const QBluetoothAddress &address,
                                    QBluetoothLocalDevice::Pairing pairing);
    void processConnectDeviceChanges(const QBluetoothAddress &address, bool isConnectEvent);

private:
    QBluetoothLocalDevice *q_ptr;
    QJniObject *obj = nullptr;

    int pendingPairing(const QBluetoothAddress &address);

public:
    LocalDeviceBroadcastReceiver *receiver;
    bool pendingConnectableHostModeTransition = false;
    QList<QPair<QBluetoothAddress, bool> > pendingPairings;

    QList<QBluetoothAddress> connectedDevices;
};

#elif QT_CONFIG(bluez)
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                 QBluetoothAddress localAddress = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    QSet<QBluetoothAddress> connectedDevicesSet;
    QtBluetoothPrivate::OrgBluezAdapter1Interface *adapter = nullptr;
    QtBluetoothPrivate::OrgFreedesktopDBusPropertiesInterface *adapterProperties = nullptr;
    QtBluetoothPrivate::OrgFreedesktopDBusObjectManagerInterface *manager = nullptr;
    QMap<QString, QtBluetoothPrivate::OrgFreedesktopDBusPropertiesInterface *> deviceChangeMonitors;

    QList<QBluetoothAddress> connectedDevices() const;
    bool isValid() const;

    QBluetoothAddress localAddress;
    QBluetoothAddress address;
    QBluetoothLocalDevice::Pairing pairing;
    QtBluetoothPrivate::OrgBluezDevice1Interface *pairingTarget = nullptr;
    QTimer *pairingDiscoveryTimer = nullptr;
    QBluetoothLocalDevice::HostMode currentMode;
    int pendingHostModeChange;
    bool pairingRequestCanceled = false;

public slots:
    void pairingCompleted(QDBusPendingCallWatcher *);
    void requestPairing(const QBluetoothAddress &address,
                        QBluetoothLocalDevice::Pairing targetPairing);

private Q_SLOTS:
    void PropertiesChanged(const QString &interface,
                           const QVariantMap &changed_properties,
                           const QStringList &invalidated_properties,
                           const QDBusMessage &signal);
    void InterfacesAdded(const QDBusObjectPath &object_path,
                         InterfaceList interfaces_and_properties);
    void InterfacesRemoved(const QDBusObjectPath &object_path,
                           const QStringList &interfaces);
    void processPairing(const QString &objectPath, QBluetoothLocalDevice::Pairing target);
    void pairingDiscoveryTimedOut();

private:
    void connectDeviceChanges();

    QString deviceAdapterPath;

    QBluetoothLocalDevice *q_ptr;

    void initializeAdapter();
};

#elif defined(QT_WINRT_BLUETOOTH)
class QBluetoothLocalDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QBluetoothLocalDevice)
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice *q,
                                 QBluetoothAddress = QBluetoothAddress());
    ~QBluetoothLocalDevicePrivate();

    bool isValid() const;

    void updateAdapterState(QBluetoothLocalDevice::HostMode mode);
    Q_SLOT void onAdapterRemoved(winrt::hstring id);
    Q_SLOT void onAdapterAdded(winrt::hstring id);
    Q_SLOT void radioModeChanged(winrt::hstring id, QBluetoothLocalDevice::HostMode mode);
    Q_SLOT void onDeviceAdded(const QBluetoothAddress &address);
    Q_SLOT void onDeviceRemoved(const QBluetoothAddress &address);

    QBluetoothLocalDevice *q_ptr;
    winrt::com_ptr<PairingWorker> mPairingWorker;
    winrt::Windows::Devices::Bluetooth::BluetoothAdapter mAdapter;
    winrt::hstring mDeviceId;
    QString mAdapterName;
    QBluetoothLocalDevice::HostMode mMode;
    winrt::event_token mModeChangeToken;

signals:
    void updateMode(winrt::hstring id, QBluetoothLocalDevice::HostMode mode);
};
#elif !defined(QT_OSX_BLUETOOTH) // dummy backend
class QBluetoothLocalDevicePrivate : public QObject
{
public:
    QBluetoothLocalDevicePrivate(QBluetoothLocalDevice * = nullptr,
                                 QBluetoothAddress = QBluetoothAddress())
    {
    }

    bool isValid() const
    {
        return false;
    }
};
#endif

QT_END_NAMESPACE

#endif // QBLUETOOTHLOCALDEVICE_P_H
