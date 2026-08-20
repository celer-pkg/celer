// Copyright (C) 2017-2015 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QCONNECTIONTCPIPBACKEND_P_H
#define QCONNECTIONTCPIPBACKEND_P_H

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

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

class TcpClientIo final : public QtROClientIoDevice
{
    Q_OBJECT

public:
    explicit TcpClientIo(QObject *parent = nullptr);
    ~TcpClientIo() override;

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
    QTcpSocket *m_socket;
};

class TcpServerIo final : public QtROServerIoDevice
{
    Q_OBJECT
public:
    explicit TcpServerIo(QTcpSocket *conn, QObject *parent = nullptr);

    QIODevice *connection() const override;
protected:
    void doClose() override;

private:
    QTcpSocket *m_connection;
};

class TcpServerImpl final : public QConnectionAbstractServer
{
    Q_OBJECT
    Q_DISABLE_COPY(TcpServerImpl)

public:
    explicit TcpServerImpl(QObject *parent);
    ~TcpServerImpl() override;

    bool hasPendingConnections() const override;
    QtROServerIoDevice *configureNewConnection() override;
    QUrl address() const override;
    bool listen(const QUrl &address) override;
    QAbstractSocket::SocketError serverError() const override;
    void close() override;

private:
    QTcpServer m_server;
    QUrl m_originalUrl; // necessary because of a QHostAddress bug
};

QT_END_NAMESPACE
#endif // QCONNECTIONTCPIPBACKEND_P_H
