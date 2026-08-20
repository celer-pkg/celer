// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "android/serveracceptancethread_p.h"
#include "android/androidutils_p.h"
#include "android/jni_android_p.h"
#include "qbluetoothserver.h"
#include "qbluetoothserver_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothsocket_android_p.h"
#include "qbluetoothlocaldevice.h"

#include <QCoreApplication>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

QHash<QBluetoothServerPrivate*, int> __fakeServerPorts;

QBluetoothServerPrivate::QBluetoothServerPrivate(QBluetoothServiceInfo::Protocol sType,
                                                 QBluetoothServer *parent)
    : serverType(sType), q_ptr(parent)
{
    thread = new ServerAcceptanceThread();
    thread->setMaxPendingConnections(maxPendingConnections);
}

QBluetoothServerPrivate::~QBluetoothServerPrivate()
{
    Q_Q(QBluetoothServer);
    if (isListening())
        q->close();

    __fakeServerPorts.remove(this);

    thread->deleteLater();
    thread = nullptr;
}

bool QBluetoothServerPrivate::initiateActiveListening(
        const QBluetoothUuid& uuid, const QString &serviceName)
{
    qCDebug(QT_BT_ANDROID) << "Initiate active listening" << uuid.toString() << serviceName;

    if (uuid.isNull() || serviceName.isEmpty())
        return false;

    //no change of SP profile details -> do nothing
    if (uuid == m_uuid && serviceName == m_serviceName && thread->isRunning())
        return true;

    m_uuid = uuid;
    m_serviceName = serviceName;
    thread->setServiceDetails(m_uuid, m_serviceName, securityFlags);

    thread->run();
    if (!thread->isRunning())
        return false;

    return true;
}

bool QBluetoothServerPrivate::deactivateActiveListening()
{
    if (isListening()) {
        //suppress last error signal due to intended closure
        thread->disconnect();
        thread->stop();
    }
    return true;
}

bool QBluetoothServerPrivate::isListening() const
{
    return __fakeServerPorts.contains(const_cast<QBluetoothServerPrivate *>(this));
}

void QBluetoothServer::close()
{
    Q_D(QBluetoothServer);

    __fakeServerPorts.remove(d);
    if (d->thread->isRunning()) {
        //suppress last error signal due to intended closure
        d->thread->disconnect();
        d->thread->stop();
    }
}

bool QBluetoothServer::listen(const QBluetoothAddress &localAdapter, quint16 port)
{
    Q_D(QBluetoothServer);
    if (serverType() != QBluetoothServiceInfo::RfcommProtocol) {
        d->m_lastError = UnsupportedProtocolError;
        emit errorOccurred(d->m_lastError);
        return false;
    }

    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) << "Bluetooth server listen() failed due to missing permissions";
        d->m_lastError = QBluetoothServer::MissingPermissionsError;
        emit errorOccurred(d->m_lastError);
        return false;
    }

    const QList<QBluetoothHostInfo> localDevices = QBluetoothLocalDevice::allDevices();
    if (localDevices.isEmpty()) {
        qCWarning(QT_BT_ANDROID) << "Device does not support Bluetooth";
        d->m_lastError = QBluetoothServer::UnknownError;
        emit errorOccurred(d->m_lastError);
        return false; //no Bluetooth device
    }

    if (!localAdapter.isNull()) {
        bool found = false;
        for (const QBluetoothHostInfo &hostInfo : localDevices) {
            if (hostInfo.address() == localAdapter) {
                found = true;
                break;
            }
        }

        if (!found) {
            qCWarning(QT_BT_ANDROID) << localAdapter.toString() << "is not a valid local Bt adapter";
            return false;
        }
    }

    if (d->isListening())
        return false;

    //check Bluetooth is available and online
    QJniObject btAdapter = getDefaultBluetoothAdapter();

    if (!btAdapter.isValid()) {
        qCWarning(QT_BT_ANDROID) << "Device does not support Bluetooth";
        d->m_lastError = QBluetoothServer::UnknownError;
        emit errorOccurred(d->m_lastError);
        return false;
    }

    const int state = btAdapter.callMethod<jint>("getState");
    if (state != 12 ) { //BluetoothAdapter.STATE_ON
        d->m_lastError = QBluetoothServer::PoweredOffError;
        emit errorOccurred(d->m_lastError);
        qCWarning(QT_BT_ANDROID) << "Bluetooth device is powered off";
        return false;
    }

    //We can not register an actual Rfcomm port, because the platform does not allow it
    //but we need a way to associate a server with a service
    if (port == 0) { //Try to assign a non taken port id
        for (int i=1; ; i++){
            if (__fakeServerPorts.key(i) == 0) {
                port = i;
                break;
            }
        }
    }

    if (__fakeServerPorts.key(port) == 0) {
        __fakeServerPorts[d] = port;

        qCDebug(QT_BT_ANDROID) << "Port" << port << "registered";
    } else {
        qCWarning(QT_BT_ANDROID) << "server with port" << port << "already registered or port invalid";
        d->m_lastError = ServiceAlreadyRegisteredError;
        emit errorOccurred(d->m_lastError);
        return false;
    }

    connect(d->thread, SIGNAL(newConnection()),
            this, SIGNAL(newConnection()));
    connect(d->thread, SIGNAL(errorOccurred(QBluetoothServer::Error)), this,
            SIGNAL(errorOccurred(QBluetoothServer::Error)), Qt::QueuedConnection);

    return true;
}

void QBluetoothServer::setMaxPendingConnections(int numConnections)
{
    Q_D(QBluetoothServer);
    d->maxPendingConnections = numConnections;
    d->thread->setMaxPendingConnections(numConnections);
}

QBluetoothAddress QBluetoothServer::serverAddress() const
{
    //Android only supports one local adapter
    QList<QBluetoothHostInfo> hosts = QBluetoothLocalDevice::allDevices();
    Q_ASSERT(hosts.size() <= 1);

    if (hosts.isEmpty())
        return QBluetoothAddress();
    else
        return hosts.at(0).address();
}

quint16 QBluetoothServer::serverPort() const
{
    //We return the fake port
    Q_D(const QBluetoothServer);
    return __fakeServerPorts.value((QBluetoothServerPrivate*)d, 0);
}

bool QBluetoothServer::hasPendingConnections() const
{
    Q_D(const QBluetoothServer);

    return d->thread->hasPendingConnections();
}

QBluetoothSocket *QBluetoothServer::nextPendingConnection()
{
    Q_D(const QBluetoothServer);

    QJniObject socket = d->thread->nextPendingConnection();
    if (!socket.isValid())
        return 0;


    QBluetoothSocket *newSocket = new QBluetoothSocket();
    bool success = newSocket->d_ptr->setSocketDescriptor(socket, d->serverType);
    if (!success) {
        delete newSocket;
        newSocket = nullptr;
    }

    return newSocket;
}

void QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    Q_D(QBluetoothServer);
    d->securityFlags = security;
}

QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
{
    Q_D(const QBluetoothServer);
    return d->securityFlags;
}

QT_END_NAMESPACE

