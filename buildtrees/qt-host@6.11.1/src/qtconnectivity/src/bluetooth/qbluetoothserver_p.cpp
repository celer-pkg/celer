// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserver.h"
#include "qbluetoothserver_p.h"
#include "qbluetoothsocket.h"
#ifndef QT_IOS_BLUETOOTH
#include "dummy/dummy_helper_p.h"
#endif

QT_BEGIN_NAMESPACE

QBluetoothServerPrivate::QBluetoothServerPrivate(QBluetoothServiceInfo::Protocol sType,
                                                 QBluetoothServer *parent)
    : serverType(sType),
      q_ptr(parent)
{
#ifndef QT_IOS_BLUETOOTH
    printDummyWarning();
#endif
    if (sType == QBluetoothServiceInfo::RfcommProtocol)
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    else
        socket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol);
}

QBluetoothServerPrivate::~QBluetoothServerPrivate()
{
    delete socket;
}

void QBluetoothServer::close()
{
}

bool QBluetoothServer::listen(const QBluetoothAddress &address, quint16 port)
{
    Q_UNUSED(address);
    Q_UNUSED(port);
    Q_D(QBluetoothServer);
    d->m_lastError = UnsupportedProtocolError;
    emit errorOccurred(d->m_lastError);
    return false;
}

void QBluetoothServer::setMaxPendingConnections(int numConnections)
{
    Q_UNUSED(numConnections);
}

bool QBluetoothServer::hasPendingConnections() const
{
    return false;
}

QBluetoothSocket *QBluetoothServer::nextPendingConnection()
{
    return nullptr;
}

QBluetoothAddress QBluetoothServer::serverAddress() const
{
    return QBluetoothAddress();
}

quint16 QBluetoothServer::serverPort() const
{
    return 0;
}

void QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    Q_UNUSED(security);
}

QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
{
    return QBluetooth::Security::NoSecurity;
}

QT_END_NAMESPACE
