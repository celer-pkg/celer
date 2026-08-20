// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include "bluetoothbaseclass.h"

#include <QBluetoothDeviceDiscoveryAgent>

#include <QTimer>
#include <QVariant>

#include <QQmlEngine>

QT_BEGIN_NAMESPACE
class QBluetoothDeviceInfo;
QT_END_NAMESPACE

class DeviceInfo;
class DeviceHandler;

class DeviceFinder: public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QVariant devices READ devices NOTIFY devicesChanged)

    QML_ELEMENT
    QML_UNCREATABLE("This class is not intended to be created directly")

public:
    DeviceFinder(DeviceHandler *handler, QObject *parent = nullptr);
    ~DeviceFinder();

    bool scanning() const;
    QVariant devices();

public slots:
    void startSearch();
    void connectToService(const QString &address);

private slots:
    void addDevice(const QBluetoothDeviceInfo &device);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();

signals:
    void scanningChanged();
    void devicesChanged();

private:
    void resetMessages();

    DeviceHandler *m_deviceHandler;
    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;
    QList<DeviceInfo *> m_devices;

    QTimer m_demoTimer;
};

#endif // DEVICEFINDER_H
