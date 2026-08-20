// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothsocket.h"
#include "qbluetoothsocket_bluez_p.h"
#include "qbluetoothdeviceinfo.h"

#include "bluez/objectmanager_p.h"
#include <QtBluetooth/QBluetoothLocalDevice>
#include "bluez/bluez_data_p.h"

#include <qplatformdefs.h>
#include <QtCore/private/qcore_unix_p.h>

#include <QtCore/QLoggingCategory>

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <QtCore/QSocketNotifier>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

using namespace QtBluetoothPrivate; // for D-Bus wrappers

QBluetoothSocketPrivateBluez::QBluetoothSocketPrivateBluez()
    : QBluetoothSocketBasePrivate()
{
    secFlags = QBluetooth::Security::Authorization;
}

QBluetoothSocketPrivateBluez::~QBluetoothSocketPrivateBluez()
{
    delete readNotifier;
    readNotifier = nullptr;
    delete connectWriteNotifier;
    connectWriteNotifier = nullptr;

    // If the socket wasn't closed/aborted make sure we free the socket file descriptor
    if (socket != -1)
        QT_CLOSE(socket);
}

bool QBluetoothSocketPrivateBluez::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    if (socket != -1) {
        if (socketType == type)
            return true;

        delete readNotifier;
        readNotifier = nullptr;
        delete connectWriteNotifier;
        connectWriteNotifier = nullptr;
        QT_CLOSE(socket);
    }

    socketType = type;

    switch (type) {
    case QBluetoothServiceInfo::L2capProtocol:
        socket = ::socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
        break;
    case QBluetoothServiceInfo::RfcommProtocol:
        socket = ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
        break;
    default:
        socket = -1;
    }

    if (socket == -1)
        return false;

    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    Q_Q(QBluetoothSocket);
    readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
    QObject::connect(readNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(_q_readNotify()));
    connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);
    QObject::connect(connectWriteNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(_q_writeNotify()));

    connectWriteNotifier->setEnabled(false);
    readNotifier->setEnabled(false);


    return true;
}

void QBluetoothSocketPrivateBluez::connectToServiceHelper(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    int result = -1;

    if (socket == -1 && !ensureNativeSocket(socketType)) {
        errorString = QBluetoothSocket::tr("Unknown socket error");
        q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        return;
    }

    // apply preferred security level
    // ignore QBluetooth::Security::Authentication -> not used anymore by kernel
    struct bt_security security;
    memset(&security, 0, sizeof(security));

    if (secFlags & QBluetooth::Security::Authorization)
        security.level = BT_SECURITY_LOW;
    if (secFlags & QBluetooth::Security::Encryption)
        security.level = BT_SECURITY_MEDIUM;
    if (secFlags & QBluetooth::Security::Secure)
        security.level = BT_SECURITY_HIGH;

    if (setsockopt(socket, SOL_BLUETOOTH, BT_SECURITY,
                   &security, sizeof(security)) != 0) {
        qCWarning(QT_BT_BLUEZ) << "Failed to set socket option, closing socket for safety" << errno;
        qCWarning(QT_BT_BLUEZ) << "Error: " << qt_error_string(errno);
        errorString = QBluetoothSocket::tr("Cannot set connection security level");
        q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        return;
    }

    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;

        memset(&addr, 0, sizeof(addr));
        addr.rc_family = AF_BLUETOOTH;
        addr.rc_channel = port;

        convertAddress(address.toUInt64(), addr.rc_bdaddr.b);

        connectWriteNotifier->setEnabled(true);
        readNotifier->setEnabled(true);

        result = ::connect(socket, (sockaddr *)&addr, sizeof(addr));
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;

        memset(&addr, 0, sizeof(addr));
        addr.l2_family = AF_BLUETOOTH;
        // This is an ugly hack but the socket class does what's needed already.
        // For L2CP GATT we need a channel rather than a socket and the LE address type
        // We don't want to make this public API offering for now especially since
        // only Linux (of all platforms supported by this library) supports this type
        // of socket.

#if QT_CONFIG(bluez) && !defined(QT_BLUEZ_NO_BTLE)
        if (lowEnergySocketType) {
            addr.l2_cid = htobs(port);
            addr.l2_bdaddr_type = lowEnergySocketType;
        } else {
            addr.l2_psm = htobs(port);
        }
#else
        addr.l2_psm = htobs(port);
#endif

        convertAddress(address.toUInt64(), addr.l2_bdaddr.b);

        connectWriteNotifier->setEnabled(true);
        readNotifier->setEnabled(true);

        result = ::connect(socket, (sockaddr *)&addr, sizeof(addr));
    }

    if (result >= 0 || (result == -1 && errno == EINPROGRESS)) {
        connecting = true;
        q->setSocketState(QBluetoothSocket::SocketState::ConnectingState);
        q->setOpenMode(openMode);
    } else {
        errorString = qt_error_string(errno);
        q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
    }
}

void QBluetoothSocketPrivateBluez::connectToService(
        const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState
            && q->state() != QBluetoothSocket::SocketState::ServiceLookupState) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluez::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    // we are checking the service protocol and not socketType()
    // socketType will change in ensureNativeSocket()
    if (service.socketProtocol() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocket::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    if (service.protocolServiceMultiplexer() > 0) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::L2capProtocol);

        if (!ensureNativeSocket(QBluetoothServiceInfo::L2capProtocol)) {
            errorString = QBluetoothSocket::tr("Unknown socket error");
            q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
            return;
        }
        connectToServiceHelper(service.device().address(), service.protocolServiceMultiplexer(),
                               openMode);
    } else if (service.serverChannel() > 0) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::RfcommProtocol);

        if (!ensureNativeSocket(QBluetoothServiceInfo::RfcommProtocol)) {
            errorString = QBluetoothSocket::tr("Unknown socket error");
            q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
            return;
        }
        connectToServiceHelper(service.device().address(), service.serverChannel(), openMode);
    } else {
        // try doing service discovery to see if we can find the socket
        if (service.serviceUuid().isNull()
                && !service.serviceClassUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort)) {
            qCWarning(QT_BT_BLUEZ) << "No port, no PSM, and no UUID provided. Unable to connect";
            return;
        }
        qCDebug(QT_BT_BLUEZ) << "Need a port/psm, doing discovery";
        q->doDeviceDiscovery(service, openMode);
    }
}

void QBluetoothSocketPrivateBluez::connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid,
        QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluez::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    if (q->socketType() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluez::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    QBluetoothServiceInfo service;
    QBluetoothDeviceInfo device(address, QString(), QBluetoothDeviceInfo::MiscellaneousDevice);
    service.setDevice(device);
    service.setServiceUuid(uuid);
    q->doDeviceDiscovery(service, openMode);
}

void QBluetoothSocketPrivateBluez::connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->socketType() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluez::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluez::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }
    connectToServiceHelper(address, port, openMode);
}

void QBluetoothSocketPrivateBluez::_q_writeNotify()
{
    Q_Q(QBluetoothSocket);
    if (connecting && state == QBluetoothSocket::SocketState::ConnectingState){
        int errorno, len;
        len = sizeof(errorno);
        ::getsockopt(socket, SOL_SOCKET, SO_ERROR, &errorno, (socklen_t*)&len);
        if(errorno) {
            errorString = qt_error_string(errorno);
            q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
            return;
        }

        q->setSocketState(QBluetoothSocket::SocketState::ConnectedState);

        connectWriteNotifier->setEnabled(false);
        connecting = false;
    }
    else {
        if (txBuffer.size() == 0) {
            connectWriteNotifier->setEnabled(false);
            return;
        }

        char buf[1024];

        const auto size = txBuffer.read(buf, 1024);
        const auto writtenBytes = qt_safe_write(socket, buf, size);
        if (writtenBytes < 0) {
            switch (errno) {
            case EAGAIN:
                txBuffer.ungetBlock(buf, size);
                break;
            default:
                // every other case returns error
                errorString = QBluetoothSocket::tr("Network Error: %1").arg(qt_error_string(errno)) ;
                q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
                break;
            }
        } else {
            if (writtenBytes < size) {
                // add remainder back to buffer
                char* remainder = buf + writtenBytes;
                txBuffer.ungetBlock(remainder, size - writtenBytes);
            }
            if (writtenBytes > 0)
                emit q->bytesWritten(writtenBytes);
        }

        if (txBuffer.size()) {
            connectWriteNotifier->setEnabled(true);
        }
        else if (state == QBluetoothSocket::SocketState::ClosingState) {
            connectWriteNotifier->setEnabled(false);
            this->close();
        }
    }
}

void QBluetoothSocketPrivateBluez::_q_readNotify()
{
    Q_Q(QBluetoothSocket);
    char *writePointer = rxBuffer.reserve(QPRIVATELINEARBUFFER_BUFFERSIZE);
//    qint64 readFromDevice = q->readData(writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
    const auto readFromDevice = ::read(socket, writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
    rxBuffer.chop(QPRIVATELINEARBUFFER_BUFFERSIZE - (readFromDevice < 0 ? 0 : readFromDevice));
    if(readFromDevice <= 0){
        int errsv = errno;
        readNotifier->setEnabled(false);
        connectWriteNotifier->setEnabled(false);
        errorString = qt_error_string(errsv);
        qCWarning(QT_BT_BLUEZ) << Q_FUNC_INFO << socket << "error:" << readFromDevice << errorString;
        if (errsv == EHOSTDOWN)
            q->setSocketError(QBluetoothSocket::SocketError::HostNotFoundError);
        else if (errsv == ECONNRESET)
            q->setSocketError(QBluetoothSocket::SocketError::RemoteHostClosedError);
        else
            q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);

        q->disconnectFromService();
    }
    else {
        emit q->readyRead();
    }
}

void QBluetoothSocketPrivateBluez::abort()
{
    delete readNotifier;
    readNotifier = nullptr;
    delete connectWriteNotifier;
    connectWriteNotifier = nullptr;

    // We don't transition through Closing for abort, so
    // we don't call disconnectFromService or
    // QBluetoothSocket::close
    QT_CLOSE(socket);
    socket = -1;

    Q_Q(QBluetoothSocket);

    q->setOpenMode(QIODevice::NotOpen);
    q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    emit q->readChannelFinished();
}

QString QBluetoothSocketPrivateBluez::localName() const
{
    const QBluetoothAddress address = localAddress();
    if (address.isNull())
        return QString();

    QBluetoothLocalDevice device(address);
    return device.name();
}

QBluetoothAddress QBluetoothSocketPrivateBluez::localAddress() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return QBluetoothAddress(convertAddress(addr.rc_bdaddr.b));
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return QBluetoothAddress(convertAddress(addr.l2_bdaddr.b));
    }

    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateBluez::localPort() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.rc_channel;
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.l2_psm;
    }

    return 0;
}

QString QBluetoothSocketPrivateBluez::peerName() const
{
    quint64 bdaddr;

    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) < 0)
            return QString();

        convertAddress(addr.rc_bdaddr.b, &bdaddr);
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) < 0)
            return QString();

        convertAddress(addr.l2_bdaddr.b, &bdaddr);
    } else {
        qCWarning(QT_BT_BLUEZ) << "peerName() called on socket of unknown type";
        return QString();
    }

    const QString peerAddress = QBluetoothAddress(bdaddr).toString();

    initializeBluez5();
    OrgFreedesktopDBusObjectManagerInterface manager(
            QStringLiteral("org.bluez"), QStringLiteral("/"), QDBusConnection::systemBus());
    QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError())
        return QString();

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin();
         it != managedObjectList.constEnd(); ++it) {
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd();
             ++jt) {
            const QString &iface = jt.key();
            const QVariantMap &ifaceValues = jt.value();

            if (iface == QStringLiteral("org.bluez.Device1")) {
                if (ifaceValues.value(QStringLiteral("Address")).toString() == peerAddress)
                    return ifaceValues.value(QStringLiteral("Alias")).toString();
            }
        }
    }
    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateBluez::peerAddress() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return QBluetoothAddress(convertAddress(addr.rc_bdaddr.b));
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return QBluetoothAddress(convertAddress(addr.l2_bdaddr.b));
    }

    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateBluez::peerPort() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.rc_channel;
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(socket, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.l2_psm;
    }

    return 0;
}

qint64 QBluetoothSocketPrivateBluez::writeData(const char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    if (q->openMode() & QIODevice::Unbuffered) {
        auto sz = ::qt_safe_write(socket, data, maxSize);
        if (sz < 0) {
            switch (errno) {
            case EAGAIN:
                sz = 0;
                break;
            default:
                errorString = QBluetoothSocket::tr("Network Error: %1").arg(qt_error_string(errno));
                q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
            }
        }

        if (sz > 0)
            emit q->bytesWritten(sz);

        return sz;
    }
    else {

        if(!connectWriteNotifier)
            return -1;

        if(txBuffer.size() == 0) {
            connectWriteNotifier->setEnabled(true);
            QMetaObject::invokeMethod(this, "_q_writeNotify", Qt::QueuedConnection);
        }

        char *txbuf = txBuffer.reserve(maxSize);
        memcpy(txbuf, data, maxSize);

        return maxSize;
    }
}

qint64 QBluetoothSocketPrivateBluez::readData(char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    if (!rxBuffer.isEmpty())
        return rxBuffer.read(data, maxSize);

    return 0;
}

void QBluetoothSocketPrivateBluez::close()
{
    // If we have pending data on the write buffer, wait until it has been written,
    // after which this close() will be called again
    if (txBuffer.size() > 0)
        connectWriteNotifier->setEnabled(true);
    else
        abort();
}

bool QBluetoothSocketPrivateBluez::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType_,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    delete readNotifier;
    readNotifier = nullptr;
    delete connectWriteNotifier;
    connectWriteNotifier = nullptr;

    socketType = socketType_;
    if (socket != -1)
        QT_CLOSE(socket);

    socket = socketDescriptor;

    // ensure that O_NONBLOCK is set on new connections.
    int flags = fcntl(socket, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
    QObject::connect(readNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(_q_readNotify()));
    connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);
    QObject::connect(connectWriteNotifier, SIGNAL(activated(QSocketDescriptor)), this, SLOT(_q_writeNotify()));

    q->setOpenMode(openMode);
    q->setSocketState(socketState);

    return true;
}

qint64 QBluetoothSocketPrivateBluez::bytesAvailable() const
{
    return rxBuffer.size();
}

qint64 QBluetoothSocketPrivateBluez::bytesToWrite() const
{
    return txBuffer.size();
}

bool QBluetoothSocketPrivateBluez::canReadLine() const
{
    return rxBuffer.canReadLine();
}

QT_END_NAMESPACE

#include "moc_qbluetoothsocket_bluez_p.cpp"
