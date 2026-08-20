// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:insecure-local-connections

#include "qconnection_qnx_backend_p.h"

QT_BEGIN_NAMESPACE

QnxClientIo::QnxClientIo(QObject *parent)
    : QtROClientIoDevice(parent)
    , m_socket(new QQnxNativeIo(this))
{
    connect(m_socket, &QQnxNativeIo::readyRead, this, &QtROClientIoDevice::readyRead);
    connect(m_socket,
            static_cast<void(QQnxNativeIo::*)(QAbstractSocket::SocketError)>(&QQnxNativeIo::error),
            this,
            &QnxClientIo::onError);
    connect(m_socket, &QQnxNativeIo::stateChanged, this, &QnxClientIo::onStateChanged);
}

QnxClientIo::~QnxClientIo()
{
    close();
}

QIODevice *QnxClientIo::connection() const
{
    return m_socket;
}

void QnxClientIo::doClose()
{
    if (m_socket->isOpen()) {
        connect(m_socket, &QQnxNativeIo::disconnected, this, &QObject::deleteLater);
        m_socket->disconnectFromServer();
    } else {
        deleteLater();
    }
}

void QnxClientIo::doDisconnectFromServer()
{
    m_socket->disconnectFromServer();
}

void QnxClientIo::connectToServer()
{
    if (!isOpen())
        m_socket->connectToServer(url().path());
}

bool QnxClientIo::isOpen() const
{
    return !isClosing() && (m_socket->state() == QAbstractSocket::ConnectedState
                            || m_socket->state() == QAbstractSocket::ConnectingState);
}

void QnxClientIo::onError(QAbstractSocket::SocketError error)
{
    qCDebug(QT_REMOTEOBJECT) << "onError" << error << m_socket->serverName();

    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        m_socket->close();
        qCWarning(QT_REMOTEOBJECT) << "RemoteHostClosedError";
        Q_FALLTHROUGH();
    case QAbstractSocket::HostNotFoundError:     //Host not there, wait and try again
    case QAbstractSocket::AddressInUseError:
    case QAbstractSocket::ConnectionRefusedError:
        //... TODO error reporting
        emit shouldReconnect(this);
        break;
    default:
        break;
    }
}

void QnxClientIo::onStateChanged(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::ClosingState && !isClosing()) {
        m_socket->abort();
        emit shouldReconnect(this);
    } else if (state == QAbstractSocket::ConnectedState)
        initializeDataStream();
}

QnxServerIo::QnxServerIo(QSharedPointer<QIOQnxSource> conn, QObject *parent)
    : QtROServerIoDevice(parent), m_connection(conn)
{
    connect(conn.data(), &QIODevice::readyRead, this, &QtROServerIoDevice::readyRead);
    connect(conn.data(), &QIOQnxSource::disconnected, this, &QtROServerIoDevice::disconnected);
}

QIODevice *QnxServerIo::connection() const
{
    return m_connection.data();
}

void QnxServerIo::doClose()
{
    m_connection->close();
}

QnxServerImpl::QnxServerImpl(QObject *parent)
    : QConnectionAbstractServer(parent)
{
    connect(&m_server,
            &QQnxNativeServer::newConnection,
            this,
            &QConnectionAbstractServer::newConnection);
}

QnxServerImpl::~QnxServerImpl()
{
    m_server.close();
}

QtROServerIoDevice *QnxServerImpl::configureNewConnection()
{
    if (!m_server.isListening())
        return nullptr;

    return new QnxServerIo(m_server.nextPendingConnection(), this);
}

bool QnxServerImpl::hasPendingConnections() const
{
    return m_server.hasPendingConnections();
}

QUrl QnxServerImpl::address() const
{
    QUrl result;
    result.setPath(m_server.serverName());
    result.setScheme(QStringLiteral("qnx"));

    return result;
}

bool QnxServerImpl::listen(const QUrl &address)
{
    return m_server.listen(address.path());
}

QAbstractSocket::SocketError QnxServerImpl::serverError() const
{
    //TODO implement on QQnxNativeServer and here
    //return m_server.serverError();
    return QAbstractSocket::AddressInUseError;
}

void QnxServerImpl::close()
{
    close();
}

QT_END_NAMESPACE
