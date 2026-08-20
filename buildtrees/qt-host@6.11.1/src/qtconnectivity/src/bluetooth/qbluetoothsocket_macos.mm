// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothservicediscoveryagent.h"
// The order is important (the first header contains
// the base class for a private socket) - workaround for
// dependencies problem.
#include "qbluetoothsocketbase_p.h"
#include "qbluetoothsocket_macos_p.h"

#include "darwin/btrfcommchannel_p.h"
#include "darwin/btl2capchannel_p.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothdeviceinfo.h"
#include "darwin/btutility_p.h"
#include "darwin/uistrings_p.h"
#include "qbluetoothsocket.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/q26numeric.h>

#include <algorithm>
#include <limits>

QT_BEGIN_NAMESPACE

namespace {

using DarwinBluetooth::RetainPolicy;

} // unnamed namespace

QBluetoothSocketPrivateDarwin::QBluetoothSocketPrivateDarwin()
  : writeChunk(std::numeric_limits<UInt16>::max())
{
    q_ptr = nullptr;
}

QBluetoothSocketPrivateDarwin::~QBluetoothSocketPrivateDarwin()
{
}

bool QBluetoothSocketPrivateDarwin::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    // For now - very simplistic, we don't call it in this file, public class
    // only calls it in a ctor, setting the protocol RFCOMM (in case of Android)
    // or, indeed, doing, socket-related initialization in BlueZ backend.
    Q_ASSERT(socketType == QBluetoothServiceInfo::UnknownProtocol);
    socketType = type;
    return type != QBluetoothServiceInfo::UnknownProtocol;
}

QString QBluetoothSocketPrivateDarwin::localName() const
{
    const QBluetoothLocalDevice device;
    return device.name();
}

QBluetoothAddress QBluetoothSocketPrivateDarwin::localAddress() const
{
    const QBluetoothLocalDevice device;
    return device.address();
}

quint16 QBluetoothSocketPrivateDarwin::localPort() const
{
    return 0;
}

QString QBluetoothSocketPrivateDarwin::peerName() const
{
    QT_BT_MAC_AUTORELEASEPOOL;

    NSString *nsName = nil;
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        if (rfcommChannel)
            nsName = [rfcommChannel.getAs<DarwinBTRFCOMMChannel>() peerName];
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        if (l2capChannel)
            nsName = [l2capChannel.getAs<DarwinBTL2CAPChannel>() peerName];
    }

    if (nsName)
        return QString::fromNSString(nsName);

    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateDarwin::peerAddress() const
{
    BluetoothDeviceAddress addr = {};
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        if (rfcommChannel)
            addr = [rfcommChannel.getAs<DarwinBTRFCOMMChannel>() peerAddress];
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        if (l2capChannel)
            addr = [l2capChannel.getAs<DarwinBTL2CAPChannel>() peerAddress];
    }

    return DarwinBluetooth::qt_address(&addr);
}

quint16 QBluetoothSocketPrivateDarwin::peerPort() const
{
    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        if (rfcommChannel)
            return [rfcommChannel.getAs<DarwinBTRFCOMMChannel>() getChannelID];
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        if (l2capChannel)
            return [l2capChannel.getAs<DarwinBTL2CAPChannel>() getPSM];
    }

    return 0;
}

void QBluetoothSocketPrivateDarwin::abort()
{
    // Can never be called while we're in connectToService:
    Q_ASSERT_X(!isConnecting, Q_FUNC_INFO, "internal inconsistency - "
               "still in connectToService()");

    if (socketType == QBluetoothServiceInfo::RfcommProtocol)
        rfcommChannel.reset();
    else if (socketType == QBluetoothServiceInfo::L2capProtocol)
        l2capChannel.reset();

    Q_ASSERT(q_ptr);

    q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    emit q_ptr->readChannelFinished();

}

void QBluetoothSocketPrivateDarwin::close()
{
    // Can never be called while we're in connectToService:
    Q_ASSERT_X(!isConnecting, Q_FUNC_INFO, "internal inconsistency - "
               "still in connectToService()");

    if (!txBuffer.size())
        abort();
}


qint64 QBluetoothSocketPrivateDarwin::writeData(const char *data, qint64 maxSize)
{
    Q_ASSERT_X(data, Q_FUNC_INFO, "invalid data (null)");
    Q_ASSERT_X(maxSize > 0, Q_FUNC_INFO, "invalid data size");

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QCoreApplication::translate(SOCKET, SOC_NOWRITE);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    // We do not have a real socket API under the hood,
    // IOBluetoothL2CAPChannel is buffered (writeAsync).

    if (!txBuffer.size())
        QMetaObject::invokeMethod(this, [this](){_q_writeNotify();}, Qt::QueuedConnection);

    const int adjustedMaxSize = q26::saturate_cast<int>(maxSize);
    char *dst = txBuffer.reserve(adjustedMaxSize);
    std::copy(data, data + adjustedMaxSize, dst);

    return adjustedMaxSize;
}

qint64 QBluetoothSocketPrivateDarwin::readData(char *data, qint64 maxSize)
{
    if (!data)
        return 0;

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QCoreApplication::translate(SOCKET, SOC_NOREAD);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    if (!rxBuffer.isEmpty())
        return rxBuffer.read(data, int(maxSize));

    return 0;
}

qint64 QBluetoothSocketPrivateDarwin::bytesAvailable() const
{
    return rxBuffer.size();
}

bool QBluetoothSocketPrivateDarwin::canReadLine() const
{
    return rxBuffer.canReadLine();
}

qint64 QBluetoothSocketPrivateDarwin::bytesToWrite() const
{
    return txBuffer.size();
}

bool QBluetoothSocketPrivateDarwin::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                                                  QBluetoothSocket::SocketState socketState, QIODevice::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType);
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);

    qCWarning(QT_BT_DARWIN) << "setting a socket descriptor is not supported by IOBluetooth";
    // Noop on macOS.
    return false;
}

void QBluetoothSocketPrivateDarwin::connectToServiceHelper(const QBluetoothAddress &address, quint16 port,
                                                     QIODevice::OpenMode openMode)
{
    Q_UNUSED(address);
    Q_UNUSED(port);
    Q_UNUSED(openMode);
}

void QBluetoothSocketPrivateDarwin::connectToService(const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_ASSERT(q_ptr);

    DarwinBluetooth::qt_test_iobluetooth_runloop();

    if (state!= QBluetoothSocket::SocketState::UnconnectedState && state != QBluetoothSocket::SocketState::ServiceLookupState) {
        qCWarning(QT_BT_DARWIN)  << "called on a busy socket";
        errorString = QCoreApplication::translate(SOCKET, SOC_CONNECT_IN_PROGRESS);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    // Report this problem early, potentially avoid device discovery:
    if (service.socketProtocol() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "cannot connect with 'UnknownProtocol' type";
        errorString = QCoreApplication::translate(SOCKET, SOC_NETWORK_ERROR);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    socketType = service.socketProtocol();

    if (service.protocolServiceMultiplexer() > 0) {
        connectToService(service.device().address(),
                         quint16(service.protocolServiceMultiplexer()),
                         openMode);
    } else if (service.serverChannel() > 0) {
        connectToService(service.device().address(),
                         quint16(service.serverChannel()),
                         openMode);
    } else {
        // Try service discovery.
        if (service.serviceUuid().isNull()) {
            qCWarning(QT_BT_DARWIN) << "No port, no PSM, and no "
                                       "UUID provided, unable to connect";
            return;
        }

        q_ptr->doDeviceDiscovery(service, openMode);
    }
}

void QBluetoothSocketPrivateDarwin::connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                                               QIODevice::OpenMode openMode)
{
    Q_ASSERT(q_ptr);

    DarwinBluetooth::qt_test_iobluetooth_runloop();

    // Report this problem early, avoid device discovery:
    if (socketType == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "cannot connect with 'UnknownProtocol' type";
        errorString = QCoreApplication::translate(SOCKET, SOC_NETWORK_ERROR);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    if (state != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_DARWIN) << "called on a busy socket";
        errorString = QCoreApplication::translate(SOCKET, SOC_CONNECT_IN_PROGRESS);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    QBluetoothDeviceInfo device(address, QString(), QBluetoothDeviceInfo::MiscellaneousDevice);
    QBluetoothServiceInfo service;
    service.setDevice(device);
    service.setServiceUuid(uuid);
    q_ptr->doDeviceDiscovery(service, openMode);
}

void QBluetoothSocketPrivateDarwin::connectToService(const QBluetoothAddress &address, quint16 port,
                                               QIODevice::OpenMode mode)
{
    Q_ASSERT(q_ptr);

    DarwinBluetooth::qt_test_iobluetooth_runloop();

    if (socketType == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "cannot connect with 'UnknownProtocol' type";
        errorString = QCoreApplication::translate(SOCKET, SOC_NETWORK_ERROR);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    Q_ASSERT_X(state == QBluetoothSocket::SocketState::ServiceLookupState || state == QBluetoothSocket::SocketState::UnconnectedState,
               Q_FUNC_INFO, "invalid state");

    q_ptr->setOpenMode(mode);

    socketError = QBluetoothSocket::SocketError::NoSocketError;
    errorString.clear();
    rxBuffer.clear();
    txBuffer.clear();

    IOReturn status = kIOReturnError;
    // Setting socket state on q_ptr will emit a signal,
    // we'd like to avoid any signal until this function completes.
    const QBluetoothSocket::SocketState oldState = state;
    // To prevent other connectToService calls (from QBluetoothSocket):
    // and also avoid signals in delegate callbacks.
    state = QBluetoothSocket::SocketState::ConnectingState;
    // We're still inside this function:
    isConnecting = true;

    // We'll later (or now) have to set an open mode on QBluetoothSocket.
    openMode = mode;

    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        rfcommChannel.reset([[DarwinBTRFCOMMChannel alloc] initWithDelegate:this], RetainPolicy::noInitialRetain);
        if (rfcommChannel)
            status = [rfcommChannel.getAs<DarwinBTRFCOMMChannel>() connectAsyncToDevice:address withChannelID:port];
        else
            status = kIOReturnNoMemory;
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        l2capChannel.reset([[DarwinBTL2CAPChannel alloc] initWithDelegate:this], RetainPolicy::noInitialRetain);
        if (l2capChannel)
            status = [l2capChannel.getAs<DarwinBTL2CAPChannel>() connectAsyncToDevice:address withPSM:port];
        else
            status = kIOReturnNoMemory;
    }

    // We're probably still connecting, but at least are leaving this function:
    isConnecting = false;

    // QBluetoothSocket will change the state and also emit
    // a signal later if required.
    if (status == kIOReturnSuccess && socketError == QBluetoothSocket::SocketError::NoSocketError) {
        if (state == QBluetoothSocket::SocketState::ConnectedState) {
            // Callback 'channelOpenComplete' fired before
            // connectToService finished:
            state = oldState;
            // Connected, setOpenMode on a QBluetoothSocket.
            q_ptr->setOpenMode(openMode);
            q_ptr->setSocketState(QBluetoothSocket::SocketState::ConnectedState);
            if (rxBuffer.size()) // We also have some data already ...
                emit q_ptr->readyRead();
        } else if (state == QBluetoothSocket::SocketState::UnconnectedState) {
            // Even if we have some data, we can not read it if
            // state != ConnectedState.
            rxBuffer.clear();
            state = oldState;
            q_ptr->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        } else {
            // No error and we're connecting ...
            state = oldState;
            q_ptr->setSocketState(QBluetoothSocket::SocketState::ConnectingState);
        }
    } else {
        state = oldState;
        if (status != kIOReturnSuccess)
            errorString = DarwinBluetooth::qt_error_string(status);
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
    }
}

void QBluetoothSocketPrivateDarwin::_q_writeNotify()
{
    Q_ASSERT_X(socketType == QBluetoothServiceInfo::L2capProtocol
               || socketType == QBluetoothServiceInfo::RfcommProtocol,
               Q_FUNC_INFO, "invalid socket type");
    Q_ASSERT_X(l2capChannel || rfcommChannel, Q_FUNC_INFO,
               "invalid socket (no open channel)");
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    if (txBuffer.size()) {
        const bool isL2CAP = socketType == QBluetoothServiceInfo::L2capProtocol;
        writeChunk.resize(isL2CAP ? std::numeric_limits<UInt16>::max() :
                          [rfcommChannel.getAs<DarwinBTRFCOMMChannel>() getMTU]);

        const auto size = txBuffer.read(writeChunk.data(), writeChunk.size());
        IOReturn status = kIOReturnError;
        if (!isL2CAP)
            status = [rfcommChannel.getAs<DarwinBTRFCOMMChannel>() writeAsync:writeChunk.data() length:UInt16(size)];
        else
            status = [l2capChannel.getAs<DarwinBTL2CAPChannel>() writeAsync:writeChunk.data() length:UInt16(size)];

        if (status != kIOReturnSuccess) {
            errorString = QCoreApplication::translate(SOCKET, SOC_NETWORK_ERROR);
            q_ptr->setSocketError(QBluetoothSocket::SocketError::NetworkError);
            return;
        } else {
            emit q_ptr->bytesWritten(size);
        }
    }

    if (!txBuffer.size() && state == QBluetoothSocket::SocketState::ClosingState)
        close();
}

bool QBluetoothSocketPrivateDarwin::setRFCOMChannel(void *generic)
{
    // A special case "constructor": on OS X we do not have a real listening socket,
    // instead a bluetooth server "listens" for channel open notifications and
    // creates (if asked by a user later) a "socket" object
    // for this connection. This function initializes
    // a "socket" from such an external channel (reported by a notification).
    auto channel = static_cast<IOBluetoothRFCOMMChannel *>(generic);
    // It must be a newborn socket!
    Q_ASSERT_X(socketError == QBluetoothSocket::SocketError::NoSocketError
               && state == QBluetoothSocket::SocketState::UnconnectedState && !rfcommChannel && !l2capChannel,
               Q_FUNC_INFO, "unexpected socket state");

    openMode = QIODevice::ReadWrite;
    rfcommChannel.reset([[DarwinBTRFCOMMChannel alloc] initWithDelegate:this channel:channel],
                        RetainPolicy::noInitialRetain);
    if (rfcommChannel) {// We do not handle errors, up to an external user.
        q_ptr->setOpenMode(QIODevice::ReadWrite);
        state = QBluetoothSocket::SocketState::ConnectedState;
        socketType = QBluetoothServiceInfo::RfcommProtocol;
    }

    return rfcommChannel;
}

bool QBluetoothSocketPrivateDarwin::setL2CAPChannel(void *generic)
{
    // A special case "constructor": on OS X we do not have a real listening socket,
    // instead a bluetooth server "listens" for channel open notifications and
    // creates (if asked by a user later) a "socket" object
    // for this connection. This function initializes
    // a "socket" from such an external channel (reported by a notification).
    auto channel = static_cast<IOBluetoothL2CAPChannel *>(generic);

    // It must be a newborn socket!
    Q_ASSERT_X(socketError == QBluetoothSocket::SocketError::NoSocketError
               && state == QBluetoothSocket::SocketState::UnconnectedState && !l2capChannel && !rfcommChannel,
               Q_FUNC_INFO, "unexpected socket state");

    openMode = QIODevice::ReadWrite;
    l2capChannel.reset([[DarwinBTL2CAPChannel alloc] initWithDelegate:this channel:channel], RetainPolicy::noInitialRetain);
    if (l2capChannel) {// We do not handle errors, up to an external user.
        q_ptr->setOpenMode(QIODevice::ReadWrite);
        state = QBluetoothSocket::SocketState::ConnectedState;
        socketType = QBluetoothServiceInfo::L2capProtocol;
    }

    return l2capChannel;
}

void QBluetoothSocketPrivateDarwin::setChannelError(IOReturn errorCode)
{
    Q_UNUSED(errorCode);

    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    if (isConnecting) {
        // The delegate's method was called while we are still in
        // connectToService ... will emit a moment later.
        socketError = QBluetoothSocket::SocketError::UnknownSocketError;
    } else {
        q_ptr->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
    }
}

void QBluetoothSocketPrivateDarwin::channelOpenComplete()
{
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    if (!isConnecting) {
        q_ptr->setOpenMode(openMode);
        q_ptr->setSocketState(QBluetoothSocket::SocketState::ConnectedState);
    } else {
        state = QBluetoothSocket::SocketState::ConnectedState;
        // We are still in connectToService, it'll care
        // about signals!
    }
}

void QBluetoothSocketPrivateDarwin::channelClosed()
{
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    // Channel was closed by IOBluetooth and we can not write any data
    // (thus close/abort probably will not work).

    if (!isConnecting) {
        if (state == QBluetoothSocket::SocketState::ConnectedState)
            q_ptr->setSocketError(QBluetoothSocket::SocketError::RemoteHostClosedError);
        q_ptr->setOpenMode(QIODevice::NotOpen);
        q_ptr->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        emit q_ptr->readChannelFinished();
    } else {
        state = QBluetoothSocket::SocketState::UnconnectedState;
        // We are still in connectToService and do not want
        // to emit any signals yet.
    }
}

void QBluetoothSocketPrivateDarwin::readChannelData(void *data, std::size_t size)
{
    Q_ASSERT_X(data, Q_FUNC_INFO, "invalid data (null)");
    Q_ASSERT_X(size, Q_FUNC_INFO, "invalid data size (0)");
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    const char *src = static_cast<char *>(data);

    if (size > std::numeric_limits<int>::max())
        qCWarning(QT_BT_DARWIN) << Q_FUNC_INFO << "Truncating the incoming data to INT_MAX";

    const int adjustedSize = q26::saturate_cast<int>(size);
    char *dst = rxBuffer.reserve(adjustedSize);
    std::copy(src, src + adjustedSize, dst);

    if (!isConnecting) {
        // If we're still in connectToService, do not emit.
        emit q_ptr->readyRead();
    }   // else connectToService must check and emit readyRead!
}

void QBluetoothSocketPrivateDarwin::writeComplete()
{
    _q_writeNotify();
}

QT_END_NAMESPACE
