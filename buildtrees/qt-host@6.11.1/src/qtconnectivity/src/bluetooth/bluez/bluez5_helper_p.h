// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BLUEZ5_HELPER_H
#define BLUEZ5_HELPER_H

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

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/private/qtbluetoothglobal_p.h>

typedef QMap<QString, QVariantMap> InterfaceList;
typedef QMap<QDBusObjectPath, InterfaceList> ManagedObjectList;
typedef QMap<quint16, QDBusVariant> ManufacturerDataList;
typedef QMap<QString, QDBusVariant> ServiceDataList;

QT_DECL_METATYPE_EXTERN(InterfaceList, /* not exported */)
QT_DECL_METATYPE_EXTERN(ManufacturerDataList, /* not exported */)
QT_DECL_METATYPE_EXTERN(ServiceDataList, /* not exported */)
QT_DECL_METATYPE_EXTERN(ManagedObjectList, /* not exported */)

QT_BEGIN_NAMESPACE

void initializeBluez5();
bool isBluez5();

// exported for unit test purposes
Q_BLUETOOTH_EXPORT QVersionNumber bluetoothdVersion();

QString sanitizeNameForDBus(const QString& text);

QString findAdapterForAddress(const QBluetoothAddress &wantedAddress, bool *ok);

QString adapterWithDBusPeripheralInterface(const QBluetoothAddress &localAddress);

class QtBluezDiscoveryManagerPrivate;
class QtBluezDiscoveryManager : public QObject
{
    Q_OBJECT
public:
    QtBluezDiscoveryManager(QObject* parent = nullptr);
    ~QtBluezDiscoveryManager();
    static QtBluezDiscoveryManager *instance();

    bool registerDiscoveryInterest(const QString &adapterPath);
    void unregisterDiscoveryInterest(const QString &adapterPath);

    //void dumpState() const;

signals:
    void discoveryInterrupted(const QString &adapterPath);

private slots:
    void InterfacesRemoved(const QDBusObjectPath &object_path,
                           const QStringList &interfaces);
    void PropertiesChanged(const QString &interface,
                           const QVariantMap &changed_properties,
                           const QStringList &invalidated_properties,
                           const QDBusMessage &msg);

private:
    void removeAdapterFromMonitoring(const QString &dbusPath);

    QtBluezDiscoveryManagerPrivate *d;
};

QT_END_NAMESPACE

#endif
