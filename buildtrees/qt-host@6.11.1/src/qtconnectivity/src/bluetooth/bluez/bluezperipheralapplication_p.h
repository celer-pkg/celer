// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BLUEZ_PERIPHERAL_APPLICATION_P_H
#define BLUEZ_PERIPHERAL_APPLICATION_P_H

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

#include <QtBluetooth/private/qlowenergycontroller_bluezdbus_p.h>
#include "bluez5_helper_p.h"

#include <QtBluetooth/QBluetoothAddress>
#include <QtCore/QCoreApplication>

namespace QtBluetoothPrivate {

class OrgFreedesktopDBusObjectManagerAdaptor;
class OrgBluezGattManager1Interface;

} // namespace QtBluetoothPrivate

QT_BEGIN_NAMESPACE

class QLowEnergyControllerPrivateBluezDBus;
class QtBluezPeripheralService;
class QtBluezPeripheralCharacteristic;
class QtBluezPeripheralDescriptor;
class QtBluezPeripheralConnectionManager;

class QtBluezPeripheralApplication : public QObject
{
    Q_OBJECT

public:
    QtBluezPeripheralApplication(const QString& localAdapterPath, QObject* parent = nullptr);
    ~QtBluezPeripheralApplication();

    // Register the application and its services to DBus & Bluez
    void registerApplication();
    // Unregister the application and its services from DBus & Bluez.
    // Calling this doesn't invalidate the services
    void unregisterApplication();
    // Unregister and release all resources, invalidates services
    void reset();

    void addService(const QLowEnergyServiceData &serviceData,
                    QSharedPointer<QLowEnergyServicePrivate> servicePrivate,
                    QLowEnergyHandle serviceHandle);

    // Call these when the user application has updated the attribute value
    // Returns whether the new value was accepted
    bool localCharacteristicWrite(QLowEnergyHandle handle, const QByteArray& value);
    bool localDescriptorWrite(QLowEnergyHandle handle, const QByteArray& value);

    // Returns true if application has services and is not registered
    bool registrationNeeded();

    // org.freedesktop.DBus.ObjectManager
    Q_INVOKABLE ManagedObjectList GetManagedObjects();

signals:
    void errorOccurred();
    void registered();

    // Emitted when remote device reads a characteristic
    void remoteDeviceAccessEvent(const QString& remoteDeviceObjectPath, quint16 mtu);

    // These are emitted when remote has written a new value
    void characteristicValueUpdatedByRemote(QLowEnergyHandle handle, const QByteArray& value);
    void descriptorValueUpdatedByRemote(QLowEnergyHandle characteristicHandle,
                                QLowEnergyHandle descriptorHandle,
                                const QByteArray& value);
private:
    void registerServices();
    void unregisterServices();

    QLowEnergyHandle handleForCharacteristic(QBluetoothUuid uuid,
                                             QSharedPointer<QLowEnergyServicePrivate> service);
    QLowEnergyHandle handleForDescriptor(QBluetoothUuid uuid,
                                         QSharedPointer<QLowEnergyServicePrivate> service,
                                         QLowEnergyHandle characteristicHandle);

    QMap<QLowEnergyHandle, QtBluezPeripheralService*> m_services;
    QMap<QLowEnergyHandle, QtBluezPeripheralCharacteristic*> m_characteristics;
    QMap<QLowEnergyHandle, QtBluezPeripheralDescriptor*> m_descriptors;

    QString m_objectPath;
    QtBluetoothPrivate::OrgFreedesktopDBusObjectManagerAdaptor* m_objectManager{};
    QtBluetoothPrivate::OrgBluezGattManager1Interface* m_gattManager{};
    bool m_applicationRegistered{false};
};

QT_END_NAMESPACE

#endif
