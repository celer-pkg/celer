// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSOCKET_OSX_P_H
#define QBLUETOOTHSOCKET_OSX_P_H

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

#ifdef QT_OSX_BLUETOOTH

#include "qbluetoothsocketbase_p.h"
#include "qbluetoothserviceinfo.h"
#include "darwin/btdelegates_p.h"
#include "qbluetoothsocket.h"
#include "darwin/btraii_p.h"

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE Q_INT64_C(16384)
#endif
// The order is important: bytearray before buffer:
#include <QtCore/qbytearray.h>
#include "qprivatelinearbuffer_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/QIODevice>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

class QBluetoothSocketPrivateDarwin : public QBluetoothSocketBasePrivate, public DarwinBluetooth::ChannelDelegate
{
    friend class QBluetoothSocket;
    friend class QBluetoothServer;

public:
    QBluetoothSocketPrivateDarwin();
    ~QBluetoothSocketPrivateDarwin();

    //
    bool ensureNativeSocket(QBluetoothServiceInfo::Protocol type) override;

    QString localName() const override;
    QBluetoothAddress localAddress() const override;
    quint16 localPort() const override;

    QString peerName() const override;
    QBluetoothAddress peerAddress() const override;
    quint16 peerPort() const override;

    void abort() override;
    void close() override;

    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 readData(char *data, qint64 maxSize) override;

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState,
                             QBluetoothSocket::OpenMode openMode) override;

    void connectToServiceHelper(const QBluetoothAddress &address, quint16 port,
                                QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothServiceInfo &service,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;

    void connectToService(const QBluetoothAddress &address, quint16 port,
                          QIODevice::OpenMode openMode) override;

    void _q_writeNotify();

private:
    // Create a socket from an external source (without connectToService).
    bool setRFCOMChannel(void *channel);
    bool setL2CAPChannel(void *channel);

    // L2CAP and RFCOMM delegate
    void setChannelError(IOReturn errorCode) override;
    void channelOpenComplete() override;
    void channelClosed() override;
    void readChannelData(void *data, std::size_t size) override;
    void writeComplete() override;

    QList<char> writeChunk;

    using L2CAPChannel = DarwinBluetooth::ScopedPointer;
    L2CAPChannel l2capChannel;

    using  RFCOMMChannel = L2CAPChannel;
    RFCOMMChannel rfcommChannel;
    // A trick to deal with channel open too fast (synchronously).
    bool isConnecting = false;
};

QT_END_NAMESPACE

#endif

#endif
