// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:insecure-without-tls

#include "qconnection_tcpip_backend_p.h"

#include <QtNetwork/qhostinfo.h>

QT_BEGIN_NAMESPACE

TcpClientIo::TcpClientIo(QObject *parent)
    : QtROClientIoDevice(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::readyRead, this, &QtROClientIoDevice::readyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &TcpClientIo::onError);
    connect(m_socket, &QTcpSocket::stateChanged, this, &TcpClientIo::onStateChanged);
}

TcpClientIo::~TcpClientIo()
{
    close();
}

QIODevice *TcpClientIo::connection() const
{
    return m_socket;
}

void TcpClientIo::doClose()
{
    if (m_socket->isOpen()) {
        connect(m_socket, &QTcpSocket::disconnected, this, &QObject::deleteLater);
        m_socket->disconnectFromHost();
    } else {
        this->deleteLater();
    }
}

void TcpClientIo::doDisconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void TcpClientIo::connectToServer()
{
    if (isOpen())
        return;
    const QString &host = url().host();
    QHostAddress address(host);
    if (address.isNull())
        address = QHostInfo::fromName(host).addresses().value(0);

    if (address.isNull())
        qWarning("connectToServer(): Failed to resolve host %s", qUtf8Printable(host));
    else
        m_socket->connectToHost(address, url().port());
}

bool TcpClientIo::isOpen() const
{
    return (!isClosing() && (m_socket->state() == QAbstractSocket::ConnectedState
                             || m_socket->state() == QAbstractSocket::ConnectingState));
}

void TcpClientIo::onError(QAbstractSocket::SocketError error)
{
    qCDebug(QT_REMOTEOBJECT) << "onError" << error;

    switch (error) {
    case QAbstractSocket::HostNotFoundError:     //Host not there, wait and try again
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::NetworkError:
        emit shouldReconnect(this);
        break;
    case QAbstractSocket::AddressInUseError:
        //... TODO error reporting
        break;
    default:
        break;
    }
}

void TcpClientIo::onStateChanged(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::ClosingState && !isClosing()) {
        m_socket->abort();
        emit shouldReconnect(this);
    }
    if (state == QAbstractSocket::ConnectedState)
        initializeDataStream();
}


TcpServerIo::TcpServerIo(QTcpSocket *conn, QObject *parent)
    : QtROServerIoDevice(parent), m_connection(conn)
{
    m_connection->setParent(this);
    connect(conn, &QIODevice::readyRead, this, &QtROServerIoDevice::readyRead);
    connect(conn, &QAbstractSocket::disconnected, this, &QtROServerIoDevice::disconnected);
}

QIODevice *TcpServerIo::connection() const
{
    return m_connection;
}

void TcpServerIo::doClose()
{
    m_connection->disconnectFromHost();
}



TcpServerImpl::TcpServerImpl(QObject *parent)
    : QConnectionAbstractServer(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &QConnectionAbstractServer::newConnection);
}

TcpServerImpl::~TcpServerImpl()
{
    close();
}

QtROServerIoDevice *TcpServerImpl::configureNewConnection()
{
    if (!m_server.isListening())
        return nullptr;

    return new TcpServerIo(m_server.nextPendingConnection(), this);
}

bool TcpServerImpl::hasPendingConnections() const
{
    return m_server.hasPendingConnections();
}

QUrl TcpServerImpl::address() const
{
    return m_originalUrl;
}

bool TcpServerImpl::listen(const QUrl &address)
{
    QHostAddress host(address.host());
    if (host.isNull()) {
        if (address.host().isEmpty()) {
            host = QHostAddress::Any;
        } else {
            qCWarning(QT_REMOTEOBJECT) << address.host() << " is not an IP address, trying to resolve it";
            QHostInfo info = QHostInfo::fromName(address.host());
            if (info.addresses().isEmpty())
                host = QHostAddress::Any;
            else
                host = info.addresses().constFirst();
        }
    }

    bool ret = m_server.listen(host, quint16(address.port()));
    if (ret) {
        m_originalUrl.setScheme(QLatin1String("tcp"));
        m_originalUrl.setHost(m_server.serverAddress().toString());
        m_originalUrl.setPort(m_server.serverPort());
    }
    return ret;
}

QAbstractSocket::SocketError TcpServerImpl::serverError() const
{
    return m_server.serverError();
}

void TcpServerImpl::close()
{
    m_server.close();
}

QT_END_NAMESPACE
