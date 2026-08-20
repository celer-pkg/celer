// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BLUEZ_PERIPHERAL_CONNECTION_MANAGER_P_H
#define BLUEZ_PERIPHERAL_CONNECTION_MANAGER_P_H

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

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothUuid>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QBluetoothLocalDevice;

/*
    QtBluezPeripheralConnectionManager determines
    - when remote Gatt client(s) connect and disconnect
    - the remote device details (name, address, mtu)

    'Connected' state is assumed when first client reads a characteristic.
    'Disconnected' state is assumed when all such clients have disconnected.
*/

class QtBluezPeripheralConnectionManager : public QObject
{
    Q_OBJECT
public:
    QtBluezPeripheralConnectionManager(const QBluetoothAddress& localAddress,
                                       QObject* parent = nullptr);
    void reset();
    void disconnectDevices();

public slots:
    void remoteDeviceAccessEvent(const QString& remoteDeviceObjectPath, quint16 mtu);

signals:
    void connectivityStateChanged(bool connected);
    void remoteDeviceChanged(const QBluetoothAddress& address, const QString& name, quint16 mtu);

private slots:
    void remoteDeviceDisconnected(const QBluetoothAddress& address);

private:
    struct RemoteDeviceDetails {
        QBluetoothAddress address;
        QString name;
        quint16 mtu;
    };
    bool m_connected{false};
    QString m_hostAdapterPath;
    QMap<QString, RemoteDeviceDetails> m_clients;
    QBluetoothLocalDevice* m_localDevice;
};

QT_END_NAMESPACE

#endif
