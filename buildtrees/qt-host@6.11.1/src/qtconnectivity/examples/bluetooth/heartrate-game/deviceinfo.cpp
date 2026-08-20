// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "deviceinfo.h"
#include "heartrate-global.h"

#include <QBluetoothAddress>
#include <QBluetoothUuid>

using namespace Qt::StringLiterals;

DeviceInfo::DeviceInfo(const QBluetoothDeviceInfo &info):
    m_device(info)
{
}

QBluetoothDeviceInfo DeviceInfo::getDevice() const
{
    return m_device;
}

QString DeviceInfo::getName() const
{
    if (simulator)
        return u"Demo BT Device"_s;
    return m_device.name();
}

QString DeviceInfo::getAddress() const
{
    if (simulator)
        return u"00:11:22:33:44:55"_s;
#ifdef Q_OS_DARWIN
    // workaround for Core Bluetooth:
    return m_device.deviceUuid().toString();
#else
    return m_device.address().toString();
#endif
}

void DeviceInfo::setDevice(const QBluetoothDeviceInfo &device)
{
    m_device = device;
    emit deviceChanged();
}
