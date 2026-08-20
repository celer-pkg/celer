// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#ifndef QBLUETOOTHSOCKET_ANDROID_P_H
#define QBLUETOOTHSOCKET_ANDROID_P_H

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

#include "qbluetoothsocketbase_p.h"

#include <QtCore/QJniObject>
#include <QtCore/QPointer>
#include "android/inputstreamthread_p.h"
#include <jni.h>

QT_BEGIN_NAMESPACE

class QBluetoothSocketPrivateAndroid final : public QBluetoothSocketBasePrivate
{
    Q_OBJECT
    friend class QBluetoothServerPrivate;

public:
    QBluetoothSocketPrivateAndroid();
    ~QBluetoothSocketPrivateAndroid() override;

    //On Android we connect using the uuid not the port
    void connectToServiceHelper(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;

    void connectToService(const QBluetoothServiceInfo &service,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, quint16 port,
                          QIODevice::OpenMode openMode) override;

    bool fallBackReversedConnect(const QBluetoothUuid &uuid);

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

    bool setSocketDescriptor(const QJniObject &socket, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;

    static QBluetoothUuid reverseUuid(const QBluetoothUuid &serviceUuid);

    QJniObject adapter;
    QJniObject socketObject;
    QJniObject remoteDevice;
    QJniObject inputStream;
    QJniObject outputStream;
    InputStreamThread *inputThread;

public slots:
    void socketConnectSuccess(const QJniObject &socket);
    void defaultSocketConnectFailed(const QJniObject & socket,
                                    const QJniObject &targetUuid,
                                    const QBluetoothUuid &qtTargetUuid);
    void fallbackSocketConnectFailed(const QJniObject &socket,
                                     const QJniObject &targetUuid);
    void inputThreadError(int errorCode);

signals:
    void connectJavaSocket();
    void closeJavaSocket();

};

// QTBUG-61392 related
// Private API to disable the silent behavior to reverse a remote service's
// UUID. In rare cases the workaround behavior might not be desirable as
// it may lead to connects to incorrect services.
extern bool useReverseUuidWorkAroundConnect;

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_ANDROID_P_H
