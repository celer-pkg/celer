// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothsocket.h"
#include "qbluetoothsocket_dummy_p.h"
#ifndef QT_IOS_BLUETOOTH
#include "dummy/dummy_helper_p.h"
#endif

QT_BEGIN_NAMESPACE

QBluetoothSocketPrivateDummy::QBluetoothSocketPrivateDummy()
{
    secFlags = QBluetooth::Security::NoSecurity;
#ifndef QT_IOS_BLUETOOTH
    printDummyWarning();
#endif
}

QBluetoothSocketPrivateDummy::~QBluetoothSocketPrivateDummy()
{
}

bool QBluetoothSocketPrivateDummy::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    socketType = type;
    return false;
}

void QBluetoothSocketPrivateDummy::connectToServiceHelper(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_UNUSED(openMode);
    Q_UNUSED(address);
    Q_UNUSED(port);
}

void QBluetoothSocketPrivateDummy::connectToService(
        const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_UNUSED(service);
    Q_UNUSED(openMode);

    Q_Q(QBluetoothSocket);

    qWarning() << "Using non-functional QBluetoothSocketPrivateDummy";
    errorString = QBluetoothSocket::tr("Socket type not supported");
    q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
}

void QBluetoothSocketPrivateDummy::connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid, QIODevice::OpenMode openMode)
{
    Q_UNUSED(address);
    Q_UNUSED(uuid);
    Q_UNUSED(openMode);

    Q_Q(QBluetoothSocket);

    qWarning() << "Using non-functional QBluetoothSocketPrivateDummy";
    errorString = QBluetoothSocket::tr("Socket type not supported");
    q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
}

void QBluetoothSocketPrivateDummy::connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_UNUSED(address);
    Q_UNUSED(port);
    Q_UNUSED(openMode);

    Q_Q(QBluetoothSocket);

    qWarning() << "Using non-functional QBluetoothSocketPrivateDummy";
    errorString = QBluetoothSocket::tr("Socket type not supported");
    q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
}

void QBluetoothSocketPrivateDummy::abort()
{
}

QString QBluetoothSocketPrivateDummy::localName() const
{
    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateDummy::localAddress() const
{
    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateDummy::localPort() const
{
    return 0;
}

QString QBluetoothSocketPrivateDummy::peerName() const
{
    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateDummy::peerAddress() const
{
    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateDummy::peerPort() const
{
    return 0;
}

qint64 QBluetoothSocketPrivateDummy::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }
    return -1;
}

qint64 QBluetoothSocketPrivateDummy::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    return -1;
}

void QBluetoothSocketPrivateDummy::close()
{
}

bool QBluetoothSocketPrivateDummy::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType);
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);
    return false;
}

qint64 QBluetoothSocketPrivateDummy::bytesAvailable() const
{
    return 0;
}

bool QBluetoothSocketPrivateDummy::canReadLine() const
{
    return false;
}

qint64 QBluetoothSocketPrivateDummy::bytesToWrite() const
{
    return 0;
}

QT_END_NAMESPACE
