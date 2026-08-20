// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "connectionhandler.h"
#include "heartrate-global.h"

#include <qtbluetooth-config.h>

#include <QtSystemDetection>

#if QT_CONFIG(permissions)
#include <QCoreApplication>
#include <QPermissions>
#endif

ConnectionHandler::ConnectionHandler(QObject *parent) : QObject(parent)
{
    initLocalDevice();
}

bool ConnectionHandler::alive() const
{

#ifdef QT_PLATFORM_UIKIT
    return true;

#else
    if (simulator)
        return true;
    return m_localDevice && m_localDevice->isValid()
            && m_localDevice->hostMode() != QBluetoothLocalDevice::HostPoweredOff;
#endif
}

bool ConnectionHandler::hasPermission() const
{
    return m_hasPermission;
}

bool ConnectionHandler::requiresAddressType() const
{
#if QT_CONFIG(bluez)
    return true;
#else
    return false;
#endif
}

QString ConnectionHandler::name() const
{
    return m_localDevice ? m_localDevice->name() : QString();
}

QString ConnectionHandler::address() const
{
    return m_localDevice ? m_localDevice->address().toString() : QString();
}

void ConnectionHandler::hostModeChanged(QBluetoothLocalDevice::HostMode /*mode*/)
{
    emit deviceChanged();
}

void ConnectionHandler::initLocalDevice()
{
#if QT_CONFIG(permissions)
    QBluetoothPermission permission{};
    permission.setCommunicationModes(QBluetoothPermission::Access);
    switch (qApp->checkPermission(permission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(permission, this, &ConnectionHandler::initLocalDevice);
        return;
    case Qt::PermissionStatus::Denied:
        return;
    case Qt::PermissionStatus::Granted:
        break; // proceed to initialization
    }
#endif
    m_localDevice = new QBluetoothLocalDevice(this);
    connect(m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged,
            this, &ConnectionHandler::hostModeChanged);
    m_hasPermission = true;
    emit deviceChanged();
}
