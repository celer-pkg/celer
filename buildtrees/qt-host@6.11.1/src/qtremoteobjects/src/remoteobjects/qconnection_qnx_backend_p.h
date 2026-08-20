// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QCONNECTIONQNXBACKEND_P_H
#define QCONNECTIONQNXBACKEND_P_H

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

#include "qconnectionfactories_p.h"
#include "qconnection_qnx_qiodevices.h"
#include "qconnection_qnx_server.h"

QT_BEGIN_NAMESPACE

/*!
    QtRO provides QtROClientIoDevice, QtROServerIoDevice and QConnectionAbstractServer
    as abstract interfaces to allow different backends to be used by QtRO. The
    concept behind these classes is that there needs to be a Host node, which
    has an address that can be connected to. Then there is a client object,
    which can be publicly constructed, and can connect to the server. When the
    server gets a connection request, it creates the server side of the
    connection, which communicates directly with the client. There are thus
    three abstractions, one for the server, one for the client-side of the
    connection, and the third for the server-side of the connection. The later
    two need to inherit from QIODevice.

    Creating a backend for something that is already implemented in Qt is a
    matter of creating the three needed abstractions. In the case of creating a
    QNX backend using QNX's Native Messaging, the backend needs to create the
    Server (which has an address for accepting connections), the client
    QIODevice, and the server side QIODevice. Since Native Messaging is one
    way, and recommends using pulses to support two-way communication, the
    logic for the client-side and server-side QIODevice are very different.
    Thus, three additional backend classes are needed as well.

    QnxClientIo implements the QtRO QtROClientIoDevice wrapper around the QNX
    specific QQnxNativeIo QIODevice (the client-side QIODevice).

    QnxServerIo implements the QtRO QtROServerIoDevice wrapper around the QNX
    specific QIOQnxSource QIODevice (the server-side QIODevice).

    QnxServerImpl implements the QtRO QConnectionAbstractServer wrapper around
    the QNX specific QQnxNativeServer, which is the server object listening for
    connections.

    Not sure if it is of interest to the Qt community, but it seems like
    QQnxNativeIo, QIOQnxSource and QQnxNativeServer could used as an optimized
    QLocalServer/QLocalSocket QPA for QNX.
*/

class QnxClientIo final : public QtROClientIoDevice
{
    Q_OBJECT

public:
    explicit QnxClientIo(QObject *parent = nullptr);
    ~QnxClientIo() override;

    QIODevice *connection() const override;
    void connectToServer() override;
    bool isOpen() const override;

public Q_SLOTS:
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);

protected:
    void doClose() override;
    void doDisconnectFromServer() override;
private:
    QQnxNativeIo *m_socket;
};

class QnxServerIo final : public QtROServerIoDevice
{
public:
    explicit QnxServerIo(QSharedPointer<QIOQnxSource> conn, QObject *parent = nullptr);

    QIODevice *connection() const override;
protected:
    void doClose() override;

private:
    //TODO Source or Replica
    QSharedPointer<QIOQnxSource> m_connection;
};

class QnxServerImpl final : public QConnectionAbstractServer
{
    Q_OBJECT

public:
    explicit QnxServerImpl(QObject *parent);
    ~QnxServerImpl() override;

    bool hasPendingConnections() const override;
    QtROServerIoDevice *configureNewConnection() override;
    QUrl address() const override;
    bool listen(const QUrl &address) override;
    QAbstractSocket::SocketError serverError() const override;
    void close() override;

private:
    QQnxNativeServer m_server;
};

QT_END_NAMESPACE

#endif // QCONNECTIONQNXBACKEND_P_H

