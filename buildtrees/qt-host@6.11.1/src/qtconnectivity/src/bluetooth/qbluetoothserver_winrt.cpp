// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserver.h"
#include "qbluetoothserver_p.h"
#include "qbluetoothsocket.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothsocket_winrt_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/private/qfunctions_winrt_p.h>

#include <windows.networking.h>
#include <windows.networking.connectivity.h>
#include <windows.networking.sockets.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Sockets;
using namespace ABI::Windows::Networking::Connectivity;

typedef ITypedEventHandler<StreamSocketListener *, StreamSocketListenerConnectionReceivedEventArgs *> ClientConnectedHandler;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

QHash<QBluetoothServerPrivate *, int> __fakeServerPorts;

QBluetoothServerPrivate::QBluetoothServerPrivate(QBluetoothServiceInfo::Protocol sType,
                                                 QBluetoothServer *parent)
    : serverType(sType), q_ptr(parent)
{
    mainThreadCoInit(this);
}

QBluetoothServerPrivate::~QBluetoothServerPrivate()
{
    deactivateActiveListening();
    __fakeServerPorts.remove(this);
    // If we do not reset that pointer, socketListener will go out of scope after CoUninitialize was
    // called, which will lead to a crash.
    socketListener = nullptr;
    mainThreadCoUninit(this);
}

bool QBluetoothServerPrivate::isListening() const
{
    return __fakeServerPorts.contains(const_cast<QBluetoothServerPrivate *>(this));
}

bool QBluetoothServerPrivate::initiateActiveListening(const QString &serviceName)
{
    HStringReference serviceNameRef(reinterpret_cast<LPCWSTR>(serviceName.utf16()));

    ComPtr<IAsyncAction> bindAction;
    HRESULT hr = socketListener->BindServiceNameAsync(serviceNameRef.Get(), &bindAction);
    Q_ASSERT_SUCCEEDED(hr);
    hr = QWinRTFunctions::await(bindAction);
    Q_ASSERT_SUCCEEDED(hr);
    return true;
}

bool QBluetoothServerPrivate::deactivateActiveListening()
{
    if (!isListening())
        return true;

    HRESULT hr;
    hr = socketListener->remove_ConnectionReceived(connectionToken);
    Q_ASSERT_SUCCEEDED(hr);
    return true;
}

HRESULT QBluetoothServerPrivate::handleClientConnection(IStreamSocketListener *listener,
                                                        IStreamSocketListenerConnectionReceivedEventArgs *args)
{
    Q_Q(QBluetoothServer);
    if (!socketListener || socketListener.Get() != listener) {
        qCDebug(QT_BT_WINDOWS) << "Accepting connection from wrong listener. We should not be here.";
        Q_UNREACHABLE();
        return S_OK;
    }

    HRESULT hr;
    ComPtr<IStreamSocket> socket;
    hr = args->get_Socket(&socket);
    Q_ASSERT_SUCCEEDED(hr);
    QMutexLocker locker(&pendingConnectionsMutex);
    if (pendingConnections.size() < maxPendingConnections) {
        qCDebug(QT_BT_WINDOWS) << "Accepting connection";
        pendingConnections.append(socket);
        locker.unlock();
        q->newConnection();
    } else {
        qCDebug(QT_BT_WINDOWS) << "Refusing connection";
    }

    return S_OK;
}

void QBluetoothServer::close()
{
    Q_D(QBluetoothServer);

    d->deactivateActiveListening();
    __fakeServerPorts.remove(d);
}

bool QBluetoothServer::listen(const QBluetoothAddress &address, quint16 port)
{
    Q_UNUSED(address);
    Q_D(QBluetoothServer);
    if (serverType() != QBluetoothServiceInfo::RfcommProtocol) {
        d->m_lastError = UnsupportedProtocolError;
        emit errorOccurred(d->m_lastError);
        return false;
    }

    if (isListening())
        return false;

    HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Networking_Sockets_StreamSocketListener).Get(),
                            &d->socketListener);
    Q_ASSERT_SUCCEEDED(hr);
    hr = d->socketListener->add_ConnectionReceived(Callback<ClientConnectedHandler>(d, &QBluetoothServerPrivate::handleClientConnection).Get(),
                                                   &d->connectionToken);
    Q_ASSERT_SUCCEEDED(hr);

    //We can not register an actual Rfcomm port, because the platform does not allow it
    //but we need a way to associate a server with a service
    if (port == 0) { //Try to assign a non taken port id
        for (int i = 1; ; i++){
            if (__fakeServerPorts.key(i) == 0) {
                port = i;
                break;
            }
        }
    }

    if (__fakeServerPorts.key(port) == 0) {
        __fakeServerPorts[d] = port;

        qCDebug(QT_BT_WINDOWS) << "Port" << port << "registered";
    } else {
        qCWarning(QT_BT_WINDOWS) << "server with port" << port << "already registered or port invalid";
        d->m_lastError = ServiceAlreadyRegisteredError;
        emit errorOccurred(d->m_lastError);
        return false;
    }

    return true;
}

void QBluetoothServer::setMaxPendingConnections(int numConnections)
{
    Q_D(QBluetoothServer);
    QMutexLocker locker(&d->pendingConnectionsMutex);
    if (d->pendingConnections.size() > numConnections) {
        qCWarning(QT_BT_WINDOWS) << "There are currently more than" << numConnections << "connections"
                               << "pending. Number of maximum pending connections was not changed.";
        return;
    }

    d->maxPendingConnections = numConnections;
}

bool QBluetoothServer::hasPendingConnections() const
{
    Q_D(const QBluetoothServer);
    QMutexLocker locker(&d->pendingConnectionsMutex);
    return !d->pendingConnections.isEmpty();
}

QBluetoothSocket *QBluetoothServer::nextPendingConnection()
{
    Q_D(QBluetoothServer);
    if (d->pendingConnections.isEmpty())
        return nullptr;

    ComPtr<IStreamSocket> socket = d->pendingConnections.takeFirst();

    QBluetoothSocket *newSocket = new QBluetoothSocket();
    bool success = newSocket->d_ptr->setSocketDescriptor(socket, d->serverType);
    if (!success) {
        delete newSocket;
        newSocket = nullptr;
    }

    return newSocket;
}

QBluetoothAddress QBluetoothServer::serverAddress() const
{
    QList<QBluetoothHostInfo> hosts = QBluetoothLocalDevice::allDevices();

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

void QBluetoothServer::setSecurityFlags(QBluetooth::SecurityFlags security)
{
    Q_UNUSED(security);
}

QBluetooth::SecurityFlags QBluetoothServer::securityFlags() const
{
    return QBluetooth::Security::NoSecurity;
}

QT_END_NAMESPACE
