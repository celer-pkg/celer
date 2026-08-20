// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qmodbustcpserver.h"
#include "qmodbustcpserver_p.h"

#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

/*!
    \class QModbusTcpServer
    \inmodule QtSerialBus
    \since 5.8

    \brief The QModbusTcpServer class represents a Modbus server that uses a
    TCP server for its communication with the Modbus client.

    Communication via Modbus requires the interaction between a single Modbus
    client instance and single Modbus server. This class provides the Modbus
    server implementation via a TCP server.

    Modbus TCP networks can have multiple servers. Servers are read/written by
    a client device represented by \l QModbusTcpClient.
*/

/*!
    Constructs a QModbusTcpServer with the specified \a parent. The
    \l serverAddress preset is \c 255.
*/
QModbusTcpServer::QModbusTcpServer(QObject *parent)
    : QModbusServer(*new QModbusTcpServerPrivate, parent)
{
    Q_D(QModbusTcpServer);
    d->setupTcpServer();
    setServerAddress(0xff);
}

/*!
    Destroys the QModbusTcpServer instance.
*/
QModbusTcpServer::~QModbusTcpServer()
{
    close();
}

/*!
    \internal
*/
QModbusTcpServer::QModbusTcpServer(QModbusTcpServerPrivate &dd, QObject *parent)
    : QModbusServer(dd, parent)
{
    Q_D(QModbusTcpServer);
    d->setupTcpServer();
}

/*!
    \reimp
*/
bool QModbusTcpServer::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusTcpServer);
    if (d->m_tcpServer->isListening())
        return false;

    const QUrl url = QUrl::fromUserInput(d->m_networkAddress + QStringLiteral(":")
        + QString::number(d->m_networkPort));

    if (!url.isValid()) {
        setError(tr("Invalid connection settings for TCP communication specified."),
            QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "(TCP server) Invalid host:" << url.host() << "or port:"
            << url.port();
        return false;
    }

    if (d->m_tcpServer->listen(QHostAddress(url.host()), quint16(url.port())))
        setState(QModbusDevice::ConnectedState);
    else
        setError(d->m_tcpServer->errorString(), QModbusDevice::ConnectionError);

    return state() == QModbusDevice::ConnectedState;
}

/*!
    \reimp
*/
void QModbusTcpServer::close()
{
    if (state() == QModbusDevice::UnconnectedState)
        return;

    Q_D(QModbusTcpServer);

    if (d->m_tcpServer->isListening())
        d->m_tcpServer->close();

    const auto childSockets =
            d->m_tcpServer->findChildren<QTcpSocket *>(Qt::FindDirectChildrenOnly);
    for (auto socket : childSockets)
        socket->disconnectFromHost();

    setState(QModbusDevice::UnconnectedState);
}

/*!
    \reimp

    Processes the Modbus client request specified by \a request and returns a
    Modbus response.

    The following Modbus function codes are filtered out as they are serial
    line only according to the Modbus Application Protocol Specification 1.1b:
    \list
        \li \l QModbusRequest::ReadExceptionStatus
        \li \l QModbusRequest::Diagnostics
        \li \l QModbusRequest::GetCommEventCounter
        \li \l QModbusRequest::GetCommEventLog
        \li \l QModbusRequest::ReportServerId
    \endlist
    A request to the TCP server will be answered with a Modbus exception
    response with the exception code QModbusExceptionResponse::IllegalFunction.
*/
QModbusResponse QModbusTcpServer::processRequest(const QModbusPdu &request)
{
    switch (request.functionCode()) {
    case QModbusRequest::ReadExceptionStatus:
    case QModbusRequest::Diagnostics:
    case QModbusRequest::GetCommEventCounter:
    case QModbusRequest::GetCommEventLog:
    case QModbusRequest::ReportServerId:
        return QModbusExceptionResponse(request.functionCode(),
            QModbusExceptionResponse::IllegalFunction);
    default:
        break;
    }
    return QModbusServer::processRequest(request);
}

/*!
  Installs  an \a observer that can be used to obtain notifications when a
  new TCP client connects to this server instance. In addition, the \a observer
  can be used to reject the incoming TCP connection.

  QModbusTcpServer takes ownership of the given \a observer. Any previously set
  observer will be deleted. The observer can be uninstalled by calling this
  function with \c nullptr as parameter.

  \sa QModbusTcpConnectionObserver
  \since 5.13
*/
void QModbusTcpServer::installConnectionObserver(QModbusTcpConnectionObserver *observer)
{
    Q_D(QModbusTcpServer);

    d->m_observer.reset(observer);
}

/*!
    \class QModbusTcpConnectionObserver
    \inmodule QtSerialBus
    \since 5.13

    \brief The QModbusTcpConnectionObserver class represents the interface for
    objects that can be passed to \l QModbusTcpServer::installConnectionObserver.

    The interface must be implemented by the developer to be able to monitor
    every incoming TCP connection from another Modbus client.

    \sa QModbusTcpServer::installConnectionObserver
*/

QModbusTcpConnectionObserver::~QModbusTcpConnectionObserver()
{
}

/*!
  \fn bool QModbusTcpConnectionObserver::acceptNewConnection(QTcpSocket *newClient)

  This function is a callback for every incoming TCP connection. The user should
  provide \a newClient to receive a notification when a new client connection
  is established and to determine whether the connection is to be accepted.

  The function should return \c true if the connection is to be accepted. Otherwise,
  the socket is closed/rejected.
*/

/*!
  \fn void QModbusTcpServer::modbusClientDisconnected(QTcpSocket *modbusClient)

  This signal is emitted when a current TCP based \a modbusClient disconnects
  from this Modbus TCP server. Note that there might be several TCP clients
  connected at the same time.

  Notifications on incoming new connections can be received by installing a
  QModbusTcpConnectionObserver via \l installConnectionObserver().

  \sa installConnectionObserver
  \since 5.13
*/

QT_END_NAMESPACE
