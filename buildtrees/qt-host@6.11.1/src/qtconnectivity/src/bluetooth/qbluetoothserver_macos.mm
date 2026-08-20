// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "darwin/btsocketlistener_p.h"
#include "qbluetoothserver_p.h"

// The order is important: a workround for
// a private header included by private header
// (incorrectly handled dependencies).
#include "qbluetoothsocketbase_p.h"
#include "qbluetoothsocket_macos_p.h"

#include "qbluetoothlocaldevice.h"
#include "darwin/btutility_p.h"
#include "qbluetoothserver.h"
#include "qbluetoothsocket.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qvariant.h>
#include <QtCore/qglobal.h>
#include <QtCore/qmutex.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

#include <limits>

QT_BEGIN_NAMESPACE

namespace {

using DarwinBluetooth::RetainPolicy;
using ServiceInfo = QBluetoothServiceInfo;

QMap<quint16, QBluetoothServerPrivate *> &busyPSMs()
{
    static QMap<quint16, QBluetoothServerPrivate *> psms;
    return psms;
}

QMap<quint16, QBluetoothServerPrivate *> &busyChannels()
{
    static QMap<quint16, QBluetoothServerPrivate *> channels;
    return channels;
}

typedef QMap<quint16, QBluetoothServerPrivate *>::iterator ServerMapIterator;

} // unnamed namespace

QBluetoothServerPrivate::QBluetoothServerPrivate(ServiceInfo::Protocol type,
                                                 QBluetoothServer *parent)
    : serverType(type),
      q_ptr(parent),
      port(0)
{
    if (serverType == ServiceInfo::UnknownProtocol)
        qCWarning(QT_BT_DARWIN) << "unknown protocol";
}

QBluetoothServerPrivate::~QBluetoothServerPrivate()
{
    const QMutexLocker lock(&channelMapMutex());
    unregisterServer(this);
}

bool QBluetoothServerPrivate::startListener(quint16 realPort)
{
    Q_ASSERT_X(realPort, Q_FUNC_INFO, "invalid port");

    if (serverType == ServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_DARWIN) << "invalid protocol";
        return false;
    }

    if (!listener) {
        listener.reset([[DarwinBTSocketListener alloc] initWithListener:this],
                       RetainPolicy::noInitialRetain);
    }

    bool result = false;
    if (serverType == ServiceInfo::RfcommProtocol)
        result = [listener.getAs<DarwinBTSocketListener>() listenRFCOMMConnectionsWithChannelID:realPort];
    else
        result = [listener.getAs<DarwinBTSocketListener>() listenL2CAPConnectionsWithPSM:realPort];

    if (!result)
        listener.reset();

    return result;
}

bool QBluetoothServerPrivate::isListening() const
{
    if (serverType == ServiceInfo::UnknownProtocol)
        return false;

    const QMutexLocker lock(&QBluetoothServerPrivate::channelMapMutex());
    return QBluetoothServerPrivate::registeredServer(q_ptr->serverPort(), serverType);
}

void QBluetoothServerPrivate::stopListener()
{
    listener.reset();
}

void QBluetoothServerPrivate::openNotifyRFCOMM(void *generic)
{
    auto channel = static_cast<IOBluetoothRFCOMMChannel *>(generic);

    Q_ASSERT_X(listener, Q_FUNC_INFO, "invalid listener (nil)");
    Q_ASSERT_X(channel, Q_FUNC_INFO, "invalid channel (nil)");
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    PendingConnection newConnection(channel, RetainPolicy::doInitialRetain);
    pendingConnections.append(newConnection);

    emit q_ptr->newConnection();
}

void QBluetoothServerPrivate::openNotifyL2CAP(void *generic)
{
    auto channel = static_cast<IOBluetoothL2CAPChannel *>(generic);

    Q_ASSERT_X(listener, Q_FUNC_INFO, "invalid listener (nil)");
    Q_ASSERT_X(channel, Q_FUNC_INFO, "invalid channel (nil)");
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    PendingConnection newConnection(channel, RetainPolicy::doInitialRetain);
    pendingConnections.append(newConnection);

    emit q_ptr->newConnection();
}

QMutex &QBluetoothServerPrivate::channelMapMutex()
{
    static QMutex mutex;
    return mutex;
}

bool QBluetoothServerPrivate::channelIsBusy(quint16 channelID)
{
    // External lock is required.
    return busyChannels().contains(channelID);
}

quint16 QBluetoothServerPrivate::findFreeChannel()
{
    // External lock is required.
    for (quint16 i = 1; i <= 30; ++i) {
        if (!busyChannels().contains(i))
            return i;
    }

    return 0; //Invalid port.
}

bool QBluetoothServerPrivate::psmIsBusy(quint16 psm)
{
    // External lock is required.
    return busyPSMs().contains(psm);
}

quint16 QBluetoothServerPrivate::findFreePSM()
{
    // External lock is required.
    for (quint16 i = 1, e = std::numeric_limits<qint16>::max(); i < e; i += 2) {
        if (!psmIsBusy(i))
            return i;
    }

    return 0; // Invalid PSM.
}

void QBluetoothServerPrivate::registerServer(QBluetoothServerPrivate *server, quint16 port)
{
    // External lock is required + port must be free.
    Q_ASSERT_X(server, Q_FUNC_INFO, "invalid server (null)");

    const ServiceInfo::Protocol type = server->serverType;
    if (type == ServiceInfo::RfcommProtocol) {
        Q_ASSERT_X(!channelIsBusy(port), Q_FUNC_INFO, "port is busy");
        busyChannels()[port] = server;
    } else if (type == ServiceInfo::L2capProtocol) {
        Q_ASSERT_X(!psmIsBusy(port), Q_FUNC_INFO, "port is busy");
        busyPSMs()[port] = server;
    } else {
        qCWarning(QT_BT_DARWIN) << "can not register a server "
                                   "with unknown protocol type";
    }
}

QBluetoothServerPrivate *QBluetoothServerPrivate::registeredServer(quint16 port, QBluetoothServiceInfo::Protocol protocol)
{
    // Eternal lock is required.
    if (protocol == ServiceInfo::RfcommProtocol) {
        ServerMapIterator it = busyChannels().find(port);
        if (it != busyChannels().end())
            return it.value();
    } else if (protocol == ServiceInfo::L2capProtocol) {
        ServerMapIterator it = busyPSMs().find(port);
        if (it != busyPSMs().end())
            return it.value();
    } else {
        qCWarning(QT_BT_DARWIN) << "invalid protocol";
    }

    return nullptr;
}

void QBluetoothServerPrivate::unregisterServer(QBluetoothServerPrivate *server)
{
    // External lock is required.
    const ServiceInfo::Protocol type = server->serverType;
    const quint16 port = server->port;

    if (type == ServiceInfo::RfcommProtocol) {
        ServerMapIterator it = busyChannels().find(port);
        if (it != busyChannels().end()) {
            busyChannels().erase(it);
        } else {
            qCWarning(QT_BT_DARWIN) << "server is not registered";
        }
    } else if (type == ServiceInfo::L2capProtocol) {
        ServerMapIterator it = busyPSMs().find(port);
        if (it != busyPSMs().end()) {
            busyPSMs().erase(it);
        } else {
            qCWarning(QT_BT_DARWIN) << "server is not registered";
        }
    } else {
        qCWarning(QT_BT_DARWIN) << "invalid protocol";
    }
}

void QBluetoothServer::close()
{
    d_ptr->listener.reset();

    // Needs a lock :(
    const QMutexLocker lock(&d_ptr->channelMapMutex());
    d_ptr->unregisterServer(d_ptr);
    d_ptr->port = 0;
}

bool QBluetoothServer::listen(const QBluetoothAddress &address, quint16 port)
{
    DarwinBluetooth::qt_test_iobluetooth_runloop();

    if (d_ptr->listener) {
        qCWarning(QT_BT_DARWIN) << "already in listen mode, close server first";
        return false;
    }

    const QBluetoothLocalDevice device(address);
    if (!device.isValid()) {
        qCWarning(QT_BT_DARWIN) << "device does not support Bluetooth or"
                                << address.toString()
                                << "is not a valid local adapter";
        d_ptr->m_lastError = UnknownError;
        emit errorOccurred(UnknownError);
        return false;
    }

    const QBluetoothLocalDevice::HostMode hostMode = device.hostMode();
    if (hostMode == QBluetoothLocalDevice::HostPoweredOff) {
        qCWarning(QT_BT_DARWIN) << "Bluetooth device is powered off";
        d_ptr->m_lastError = PoweredOffError;
        emit errorOccurred(PoweredOffError);
        return false;
    }

    const ServiceInfo::Protocol type = d_ptr->serverType;

    if (type == ServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_DARWIN) << "invalid protocol";
        d_ptr->m_lastError = UnsupportedProtocolError;
        emit errorOccurred(d_ptr->m_lastError);
        return false;
    }

    d_ptr->m_lastError = QBluetoothServer::NoError;

    // Now we have to register a (fake) port, doing a proper (?) lock.
    const QMutexLocker lock(&d_ptr->channelMapMutex());

    if (port) {
        if (type == ServiceInfo::RfcommProtocol) {
            if (d_ptr->channelIsBusy(port)) {
                qCWarning(QT_BT_DARWIN) << "server port:" << port
                                        << "already registered";
                d_ptr->m_lastError = ServiceAlreadyRegisteredError;
            }
        } else {
            if (d_ptr->psmIsBusy(port)) {
                qCWarning(QT_BT_DARWIN) << "server port:" << port
                                        << "already registered";
                d_ptr->m_lastError = ServiceAlreadyRegisteredError;
            }
        }
    } else {
        type == ServiceInfo::RfcommProtocol ? port = d_ptr->findFreeChannel()
                                       : port = d_ptr->findFreePSM();
    }

    if (d_ptr->m_lastError != QBluetoothServer::NoError) {
        emit errorOccurred(d_ptr->m_lastError);
        return false;
    }

    if (!port) {
        qCWarning(QT_BT_DARWIN) << "all ports are busy";
        d_ptr->m_lastError = ServiceAlreadyRegisteredError;
        emit errorOccurred(d_ptr->m_lastError);
        return false;
    }

    // It's a fake port, the real one will be different
    // (provided after a service was registered).
    d_ptr->port = port;
    d_ptr->registerServer(d_ptr, port);
    d_ptr->listener.reset([[DarwinBTSocketListener alloc] initWithListener:d_ptr],
                          RetainPolicy::noInitialRetain);

    return true;
}

void QBluetoothServer::setMaxPendingConnections(int numConnections)
{
    d_ptr->maxPendingConnections = numConnections;
}

bool QBluetoothServer::hasPendingConnections() const
{
    return d_ptr->pendingConnections.size();
}

QBluetoothSocket *QBluetoothServer::nextPendingConnection()
{
    if (!d_ptr->pendingConnections.size())
        return nullptr;

    std::unique_ptr<QBluetoothSocket> newSocket = std::make_unique<QBluetoothSocket>();
    QBluetoothServerPrivate::PendingConnection channel(d_ptr->pendingConnections.front());

    // Remove it even if we have some errors below.
    d_ptr->pendingConnections.pop_front();

    if (d_ptr->serverType == ServiceInfo::RfcommProtocol) {
        if (!static_cast<QBluetoothSocketPrivateDarwin *>(newSocket->d_ptr)->setRFCOMChannel(channel.getAs<IOBluetoothRFCOMMChannel>()))
            return nullptr;
    } else {
        if (!static_cast<QBluetoothSocketPrivateDarwin *>(newSocket->d_ptr)->setL2CAPChannel(channel.getAs<IOBluetoothL2CAPChannel>()))
            return nullptr;
    }

    return newSocket.release();
}

QBluetoothAddress QBluetoothServer::serverAddress() const
{
    return QBluetoothLocalDevice().address();
}

quint16 QBluetoothServer::serverPort() const
{
    return d_ptr->port;
}

void QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    Q_UNUSED(security);
    Q_UNIMPLEMENTED();
}

QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
{
    Q_UNIMPLEMENTED();
    return QBluetooth::Security::NoSecurity;
}

QT_END_NAMESPACE
