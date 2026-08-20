// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BLUEZ_PERIPHERAL_OBJECTS_P_H
#define BLUEZ_PERIPHERAL_OBJECTS_P_H

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

#include "bluez5_helper_p.h"

#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyServiceData>

namespace QtBluetoothPrivate {

class OrgFreedesktopDBusPropertiesAdaptor;
class OrgBluezGattCharacteristic1Adaptor;
class OrgBluezGattDescriptor1Adaptor;
class OrgBluezGattService1Adaptor;

} // namespace QtBluetoothPrivate

QT_BEGIN_NAMESPACE

// The QtBluezPeripheralGattObject is the base class for services, characteristics, and descriptors
class QtBluezPeripheralGattObject : public QObject
{
    Q_OBJECT

public:
    QtBluezPeripheralGattObject(const QString& objectPath, const QString& uuid,
               QLowEnergyHandle handle, QObject* parent = nullptr);
    virtual ~QtBluezPeripheralGattObject();

    // List of properties exposed by this object, used when Bluez inquiries details
    virtual InterfaceList properties() const = 0;

    bool registerObject();
    void unregisterObject();

public:
    // DBus object path
    QString objectPath;
    // UUID of the gatt object
    QString uuid;
    // QtBluetooth internal handle and reference to the application
    // to read and write values towards the Qt API
    QLowEnergyHandle handle;
    // Bluez DBus Gatt objects need to provide this
    QtBluetoothPrivate::OrgFreedesktopDBusPropertiesAdaptor* propertiesAdaptor{};

signals:
    void remoteDeviceAccessEvent(const QString& remoteDeviceObjectPath, quint16 mtu);

protected:
    void accessEvent(const QVariantMap& options);

private:
    bool m_registered = false;
};

class QtBluezPeripheralDescriptor : public QtBluezPeripheralGattObject
{
    Q_OBJECT

public:
    QtBluezPeripheralDescriptor(const QLowEnergyDescriptorData& descriptorData,
                                const QString& characteristicPath, quint16 ordinal,
                                QLowEnergyHandle handle, QLowEnergyHandle characteristicHandle,
                                QObject* parent);

    InterfaceList properties() const final;

    // org.bluez.GattDescriptor1
    // This function is invoked when remote device reads the value. Sets error if any
    Q_INVOKABLE QByteArray ReadValue(const QVariantMap &options, QString &error);

    // org.bluez.GattDescriptor1
    // This function is invoked when remote device writes a value. Returns Bluez DBus error if any
    Q_INVOKABLE QString WriteValue(const QByteArray &value, const QVariantMap &options);

    // Call this function when value has been updated locally (server/user application side)
    bool localValueUpdate(const QByteArray& value);

signals:
    void valueUpdatedByRemote(QLowEnergyHandle characteristicHandle,
                              QLowEnergyHandle descriptorHandle, const QByteArray& value);

private:
    void initializeFlags(const QLowEnergyDescriptorData& data);

    QtBluetoothPrivate::OrgBluezGattDescriptor1Adaptor* m_adaptor{};
    QString m_characteristicPath;
    QByteArray m_value;
    QStringList m_flags;
    QLowEnergyHandle m_characteristicHandle;
};


class QtBluezPeripheralCharacteristic : public QtBluezPeripheralGattObject
{
    Q_OBJECT

public:
    QtBluezPeripheralCharacteristic(const QLowEnergyCharacteristicData& characteristicData,
                                    const QString& servicePath, quint16 ordinal,
                                    QLowEnergyHandle handle, QObject* parent);

    InterfaceList properties() const final;

    // org.bluez.GattCharacteristic1
    // This function is invoked when remote device reads the value. Sets error if any
    Q_INVOKABLE QByteArray ReadValue(const QVariantMap &options, QString& error);

    // org.bluez.GattCharacteristic1
    // This function is invoked when remote device writes a value. Returns Bluez DBus error if any
    Q_INVOKABLE QString WriteValue(const QByteArray &value, const QVariantMap &options);

    // org.bluez.GattCharacteristic1
    // These are called when remote client enables or disables NTF/IND
    Q_INVOKABLE void StartNotify();
    Q_INVOKABLE void StopNotify();

    // Call this function when value has been updated locally (server/user application side)
    bool localValueUpdate(const QByteArray& value);

signals:
    void valueUpdatedByRemote(QLowEnergyHandle handle, const QByteArray& value);

private:
    void initializeValue(const QByteArray& value);
    void initializeFlags(const QLowEnergyCharacteristicData& data);

    QtBluetoothPrivate::OrgBluezGattCharacteristic1Adaptor* m_adaptor{};
    QString m_servicePath;
    bool m_notifying{false};
    QByteArray m_value;
    QStringList m_flags;
    int m_minimumValueLength;
    int m_maximumValueLength;
};

class QtBluezPeripheralService : public QtBluezPeripheralGattObject
{
    Q_OBJECT
public:
    QtBluezPeripheralService(const QLowEnergyServiceData &serviceData,
                             const QString& applicationPath, quint16 ordinal,
                             QLowEnergyHandle handle, QObject* parent);

    InterfaceList properties() const final;
    void addIncludedService(const QString& objectPath);

private:
    const bool m_isPrimary;
    QtBluetoothPrivate::OrgBluezGattService1Adaptor* m_adaptor{};
    QList<QDBusObjectPath> m_includedServices;
};


QT_END_NAMESPACE

#endif
