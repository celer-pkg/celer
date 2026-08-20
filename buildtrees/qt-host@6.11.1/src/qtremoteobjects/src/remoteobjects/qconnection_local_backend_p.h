// Copyright (C) 2017-2015 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QCONNECTIONCLIENTFACTORY_P_H
#define QCONNECTIONCLIENTFACTORY_P_H

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

#include <QtNetwork/qlocalserver.h>
#include <QtNetwork/qlocalsocket.h>

QT_BEGIN_NAMESPACE

class LocalClientIo : public QtROClientIoDevice
{
    Q_OBJECT

public:
    explicit LocalClientIo(QObject *parent = nullptr);
    ~LocalClientIo() override;

    QIODevice *connection() const override;
    void connectToServer() override;
    bool isOpen() const override;

public Q_SLOTS:
    void onError(QLocalSocket::LocalSocketError error);
    void onStateChanged(QLocalSocket::LocalSocketState state);

protected:
    void doClose() override;
    void doDisconnectFromServer() override;
    QLocalSocket* m_socket;
};

#ifdef Q_OS_LINUX

class AbstractLocalClientIo final : public LocalClientIo
{
    Q_OBJECT

public:
    explicit AbstractLocalClientIo(QObject *parent = nullptr);
};

#endif // Q_OS_LINUX

class LocalServerIo final : public QtROServerIoDevice
{
    Q_OBJECT
public:
    explicit LocalServerIo(QLocalSocket *conn, QObject *parent = nullptr);

    QIODevice *connection() const override;
protected:
    void doClose() override;

private:
    QLocalSocket *m_connection;
};

class LocalServerImpl : public QConnectionAbstractServer
{
    Q_OBJECT
    Q_DISABLE_COPY(LocalServerImpl)

public:
    explicit LocalServerImpl(QObject *parent);
    ~LocalServerImpl() override;

    bool hasPendingConnections() const override;
    QtROServerIoDevice *configureNewConnection() override;
    QUrl address() const override;
    bool listen(const QUrl &address) override;
    QAbstractSocket::SocketError serverError() const override;
    void close() override;
    void setSocketOptions(QLocalServer::SocketOptions options);

protected:
    QLocalServer m_server;
};

#ifdef Q_OS_LINUX

class AbstractLocalServerImpl final : public LocalServerImpl
{
    Q_OBJECT

public:
    explicit AbstractLocalServerImpl(QObject *parent);
    QUrl address() const override;
};

#endif // Q_OS_LINUX

QT_END_NAMESPACE

#endif
