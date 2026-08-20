// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include "dummy/dummy_helper_p.h"

#define QT_DEVICEDISCOVERY_DEBUG

QT_BEGIN_NAMESPACE

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
                const QBluetoothAddress &deviceAdapter,
                QBluetoothDeviceDiscoveryAgent *parent)
    :   lowEnergySearchTimeout(-1),
        q_ptr(parent)
{
    Q_UNUSED(deviceAdapter);
    printDummyWarning();
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    return false;
}

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return QBluetoothDeviceDiscoveryAgent::NoMethod;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    lastError = QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError;
    errorString = QBluetoothDeviceDiscoveryAgent::tr("Device discovery not supported on this platform");

    emit q->errorOccurred(lastError);
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
}



QT_END_NAMESPACE

#include "moc_qbluetoothdevicediscoveryagent_p.cpp"
