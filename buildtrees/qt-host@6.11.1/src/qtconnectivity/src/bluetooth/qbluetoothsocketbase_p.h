// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSOCKETBASEPRIVATE_P_H
#define QBLUETOOTHSOCKETBASEPRIVATE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qglobal.h>
#include <QObject>
#include <QtBluetooth/qbluetoothsocket.h>

#if defined(QT_ANDROID_BLUETOOTH)
#include <QtCore/QJniObject>
#endif

#if defined(QT_WINRT_BLUETOOTH)
#include <QtCore/QMutex>

#include <wrl.h>

namespace ABI {
    namespace Windows {
        namespace Networking {
            namespace Sockets {
                struct IStreamSocket;
            }
        }
        namespace Foundation {
            struct IAsyncAction;
            enum class AsyncStatus;
        }
    }
}
#endif // QT_WINRT_BLUETOOTH

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE Q_INT64_C(16384)
#endif
#include "qprivatelinearbuffer_p.h"

QT_FORWARD_DECLARE_CLASS(QSocketNotifier)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceDiscoveryAgent)

QT_BEGIN_NAMESPACE

class QBluetoothSocketBasePrivate : public QObject
{
    Q_OBJECT

public:
    explicit QBluetoothSocketBasePrivate(QObject *parent = nullptr);
    virtual ~QBluetoothSocketBasePrivate();

    virtual bool ensureNativeSocket(QBluetoothServiceInfo::Protocol type) = 0;

    virtual QString localName() const = 0;
    virtual QBluetoothAddress localAddress() const = 0;
    virtual quint16 localPort() const = 0;

    virtual QString peerName() const = 0;
    virtual QBluetoothAddress peerAddress() const = 0;
    virtual quint16 peerPort() const = 0;

    virtual void abort() = 0;
    virtual void close() = 0;

    virtual qint64 writeData(const char *data, qint64 maxSize) = 0;
    virtual qint64 readData(char *data, qint64 maxSize) = 0;

    virtual qint64 bytesAvailable() const = 0;
    virtual bool canReadLine() const = 0;
    virtual qint64 bytesToWrite() const = 0;

    virtual bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) = 0;


#if defined(QT_ANDROID_BLUETOOTH)
    virtual void connectToServiceHelper(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) = 0;
#else
    virtual void connectToServiceHelper(const QBluetoothAddress &address, quint16 port,
                                  QIODevice::OpenMode openMode) = 0;
#endif
    virtual void connectToService(const QBluetoothServiceInfo &service,
                                  QIODevice::OpenMode openMode) = 0;
    virtual void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                                  QIODevice::OpenMode openMode) = 0;
    virtual void connectToService(const QBluetoothAddress &address, quint16 port,
                                  QIODevice::OpenMode openMode) = 0;

#ifdef QT_ANDROID_BLUETOOTH
    virtual bool setSocketDescriptor(const QJniObject &socket, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) = 0;
#elif defined(QT_WINRT_BLUETOOTH)
    virtual bool setSocketDescriptor(Microsoft::WRL::ComPtr<ABI::Windows::Networking::Sockets::IStreamSocket> socket,
                             QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) = 0;
#endif

public:
    QPrivateLinearBuffer rxBuffer;
    QPrivateLinearBuffer txBuffer;
    int socket = -1;
    QBluetoothServiceInfo::Protocol socketType = QBluetoothServiceInfo::UnknownProtocol;
    QBluetoothSocket::SocketState state = QBluetoothSocket::SocketState::UnconnectedState;
    QBluetoothSocket::SocketError socketError = QBluetoothSocket::SocketError::NoSocketError;
    QSocketNotifier *readNotifier = nullptr;
    QSocketNotifier *connectWriteNotifier = nullptr;
    bool connecting = false;

    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothSocket::OpenMode openMode;
    QBluetooth::SecurityFlags secFlags;

    QString errorString;

protected:
    Q_DECLARE_PUBLIC(QBluetoothSocket)
    QBluetoothSocket *q_ptr;

#if QT_CONFIG(bluez)
public:
    quint8 lowEnergySocketType = 0;
#endif
};

static inline void convertAddress(const quint64 from, quint8 (&to)[6])
{
    to[0] = (from >> 0) & 0xff;
    to[1] = (from >> 8) & 0xff;
    to[2] = (from >> 16) & 0xff;
    to[3] = (from >> 24) & 0xff;
    to[4] = (from >> 32) & 0xff;
    to[5] = (from >> 40) & 0xff;
}

static inline quint64 convertAddress(const quint8 (&from)[6], quint64 *to = nullptr)
{
    const quint64 result = (quint64(from[0]) << 0) |
         (quint64(from[1]) << 8) |
         (quint64(from[2]) << 16) |
         (quint64(from[3]) << 24) |
         (quint64(from[4]) << 32) |
         (quint64(from[5]) << 40);
    if (to)
        *to = result;
    return result;
}

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKETBASE_P_H
