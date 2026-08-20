// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSOCKET_BLUEZDBUS_H
#define QBLUETOOTHSOCKET_BLUEZDBUS_H

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

#include <QtDBus/qdbusunixfiledescriptor.h>

#include <QtNetwork/qlocalsocket.h>
#include <QDBusPendingCallWatcher>

namespace QtBluetoothPrivate {

class OrgBluezProfileManager1Interface;

} // namespace QtBluetoothPrivate

QT_BEGIN_NAMESPACE

class QLocalSocket;
class OrgBluezProfile1ContextInterface;

class QBluetoothSocketPrivateBluezDBus final: public QBluetoothSocketBasePrivate
{
    Q_OBJECT

public:
    QBluetoothSocketPrivateBluezDBus();
    ~QBluetoothSocketPrivateBluezDBus() override;

    void connectToServiceHelper(const QBluetoothAddress &address,
                          quint16 port,
                          QIODevice::OpenMode openMode) override;
    void connectToServiceHelper(const QBluetoothAddress &address,
                                const QBluetoothUuid &uuid,
                                QIODevice::OpenMode openMode);

    void connectToService(const QBluetoothServiceInfo &service,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, quint16 port,
                          QIODevice::OpenMode openMode) override;

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

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;

public slots:
    void connectToServiceReplyHandler(QDBusPendingCallWatcher *);

private:
    void remoteConnected(const QDBusUnixFileDescriptor &fd);
    void socketStateChanged(QLocalSocket::LocalSocketState newState);

    void clearSocket();

private:
    QtBluetoothPrivate::OrgBluezProfileManager1Interface *profileManager = nullptr;
    OrgBluezProfile1ContextInterface *profileContext = nullptr;
    QString remoteDevicePath;
    QString profileUuid;
    QString profilePath;
    QLocalSocket *localSocket = nullptr;
};

QT_END_NAMESPACE

#endif // QBLUETOOTHSOCKET_BLUEZDBUS_H
