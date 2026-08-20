// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:insecure-local-connections

#include "qconnection_local_backend_p.h"

QT_BEGIN_NAMESPACE

LocalClientIo::LocalClientIo(QObject *parent)
    : QtROClientIoDevice(parent)
    , m_socket(new QLocalSocket(this))
{
    connect(m_socket, &QLocalSocket::readyRead, this, &QtROClientIoDevice::readyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &LocalClientIo::onError);
    connect(m_socket, &QLocalSocket::stateChanged, this, &LocalClientIo::onStateChanged);
}

LocalClientIo::~LocalClientIo()
{
    close();
}

QIODevice *LocalClientIo::connection() const
{
    return m_socket;
}

void LocalClientIo::doClose()
{
    if (m_socket->isOpen()) {
        connect(m_socket, &QLocalSocket::disconnected, this, &QObject::deleteLater);
        m_socket->disconnectFromServer();
    } else {
        this->deleteLater();
    }
}

void LocalClientIo::doDisconnectFromServer()
{
    m_socket->disconnectFromServer();
}

void LocalClientIo::connectToServer()
{
#ifdef Q_OS_ANDROID
    if (!m_socket->socketOptions().testFlag(QLocalSocket::AbstractNamespaceOption))
        qWarning() << "It is recommended to use 'localabstract' over 'local' on Android.";
#endif
    if (!isOpen())
        m_socket->connectToServer(url().path());
}

bool LocalClientIo::isOpen() const
{
    return !isClosing() && (m_socket->state() == QLocalSocket::ConnectedState
                            || m_socket->state() == QLocalSocket::ConnectingState);
}

void LocalClientIo::onError(QLocalSocket::LocalSocketError error)
{
    qCDebug(QT_REMOTEOBJECT) << "onError" << error << m_socket->serverName();

    switch (error) {
    case QLocalSocket::ServerNotFoundError:
    case QLocalSocket::UnknownSocketError:
    case QLocalSocket::PeerClosedError:
        //Host not there, wait and try again
        emit shouldReconnect(this);
        break;
    case QLocalSocket::ConnectionError:
    case QLocalSocket::ConnectionRefusedError:
        //... TODO error reporting
#ifdef Q_OS_UNIX
        emit shouldReconnect(this);
#endif
        break;
    case QLocalSocket::SocketAccessError:
        emit setError(QRemoteObjectNode::SocketAccessError);
        break;
    default:
        break;
    }
}

void LocalClientIo::onStateChanged(QLocalSocket::LocalSocketState state)
{
    if (state == QLocalSocket::ClosingState && !isClosing()) {
        m_socket->abort();
        emit shouldReconnect(this);
    }
    if (state == QLocalSocket::ConnectedState)
        initializeDataStream();
}

LocalServerIo::LocalServerIo(QLocalSocket *conn, QObject *parent)
    : QtROServerIoDevice(parent), m_connection(conn)
{
    m_connection->setParent(this);
    connect(conn, &QIODevice::readyRead, this, &QtROServerIoDevice::readyRead);
    connect(conn, &QLocalSocket::disconnected, this, &QtROServerIoDevice::disconnected);
}

QIODevice *LocalServerIo::connection() const
{
    return m_connection;
}

void LocalServerIo::doClose()
{
    m_connection->disconnectFromServer();
}

LocalServerImpl::LocalServerImpl(QObject *parent)
    : QConnectionAbstractServer(parent)
{
    connect(&m_server, &QLocalServer::newConnection, this, &QConnectionAbstractServer::newConnection);
}

LocalServerImpl::~LocalServerImpl()
{
    m_server.close();
}

QtROServerIoDevice *LocalServerImpl::configureNewConnection()
{
    if (!m_server.isListening())
        return nullptr;

    return new LocalServerIo(m_server.nextPendingConnection(), this);
}

bool LocalServerImpl::hasPendingConnections() const
{
    return m_server.hasPendingConnections();
}

QUrl LocalServerImpl::address() const
{
    QUrl result;
    result.setPath(m_server.serverName());
    result.setScheme(QRemoteObjectStringLiterals::local());

    return result;
}

bool LocalServerImpl::listen(const QUrl &address)
{
#ifdef Q_OS_ANDROID
    if (!m_server.socketOptions().testFlag(QLocalServer::AbstractNamespaceOption))
        qWarning() << "It is recommended to use 'localabstract' over 'local' on Android.";
#endif
#ifdef Q_OS_UNIX
    bool res = m_server.listen(address.path());
    if (!res) {
        QLocalServer::removeServer(address.path());
        res = m_server.listen(address.path());
    }
    return res;
#else
    return m_server.listen(address.path());
#endif
}

QAbstractSocket::SocketError LocalServerImpl::serverError() const
{
    return m_server.serverError();
}

void LocalServerImpl::close()
{
    m_server.close();
}

void LocalServerImpl::setSocketOptions(QLocalServer::SocketOptions options)
{
    m_server.setSocketOptions(options);
}

#ifdef Q_OS_LINUX

AbstractLocalClientIo::AbstractLocalClientIo(QObject *parent)
    : LocalClientIo(parent)
{
    m_socket->setSocketOptions(QLocalSocket::AbstractNamespaceOption);
}

AbstractLocalServerImpl::AbstractLocalServerImpl(QObject *parent)
    : LocalServerImpl(parent)
{
    m_server.setSocketOptions(QLocalServer::AbstractNamespaceOption);
}

QUrl AbstractLocalServerImpl::address() const
{
    QUrl result;
    result.setPath(m_server.serverName());
    result.setScheme(QRemoteObjectStringLiterals::localabstract());

    return result;
}

#endif // Q_OS_LINUX

QT_END_NAMESPACE
