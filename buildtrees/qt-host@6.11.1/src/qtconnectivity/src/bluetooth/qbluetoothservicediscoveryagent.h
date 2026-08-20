// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSERVICEDISCOVERYAGENT_H
#define QBLUETOOTHSERVICEDISCOVERYAGENT_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <QtBluetooth/QBluetoothServiceInfo>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>

#if QT_CONFIG(bluez)
#include <QtCore/qprocess.h>
#endif

QT_BEGIN_NAMESPACE

class QBluetoothAddress;
class QBluetoothServiceDiscoveryAgentPrivate;

class Q_BLUETOOTH_EXPORT QBluetoothServiceDiscoveryAgent : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QBluetoothServiceDiscoveryAgent)

public:
    enum Error {
        NoError =  QBluetoothDeviceDiscoveryAgent::NoError,
        InputOutputError = QBluetoothDeviceDiscoveryAgent::InputOutputError,
        PoweredOffError = QBluetoothDeviceDiscoveryAgent::PoweredOffError,
        InvalidBluetoothAdapterError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError,
        MissingPermissionsError = QBluetoothDeviceDiscoveryAgent::MissingPermissionsError,
        UnknownError = QBluetoothDeviceDiscoveryAgent::UnknownError //=100
        //New Errors must be added after Unknown Error the space before UnknownError is reserved
        //for future device discovery errors
    };
    Q_ENUM(Error)

    enum DiscoveryMode {
        MinimalDiscovery,
        FullDiscovery
    };
    Q_ENUM(DiscoveryMode)

    explicit QBluetoothServiceDiscoveryAgent(QObject *parent = nullptr);
    explicit QBluetoothServiceDiscoveryAgent(const QBluetoothAddress &deviceAdapter, QObject *parent = nullptr);
    ~QBluetoothServiceDiscoveryAgent();

    bool isActive() const;

    Error error() const;
    QString errorString() const;

    QList<QBluetoothServiceInfo> discoveredServices() const;

    void setUuidFilter(const QList<QBluetoothUuid> &uuids);
    void setUuidFilter(const QBluetoothUuid &uuid);
    QList<QBluetoothUuid> uuidFilter() const;
    bool setRemoteAddress(const QBluetoothAddress &address);
    QBluetoothAddress remoteAddress() const;

public Q_SLOTS:
    void start(DiscoveryMode mode = MinimalDiscovery);
    void stop();
    void clear();

Q_SIGNALS:
    void serviceDiscovered(const QBluetoothServiceInfo &info);
    void finished();
    void canceled();
    void errorOccurred(QBluetoothServiceDiscoveryAgent::Error error);

private:
    QBluetoothServiceDiscoveryAgentPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif
