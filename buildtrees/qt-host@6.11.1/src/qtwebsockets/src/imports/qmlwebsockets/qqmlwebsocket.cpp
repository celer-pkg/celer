// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

/*!
    \qmltype WebSocket
    \nativetype QQmlWebSocket
    \since 5.3

    \inqmlmodule QtWebSockets
    \ingroup websockets-qml
    \brief QML interface to QWebSocket.

    WebSockets is a web technology providing full-duplex communications channels over a
    single TCP connection.
    The WebSocket protocol was standardized by the IETF as \l {RFC 6455} in 2011.
*/

/*!
  \qmlproperty url WebSocket::url
  Server url to connect to. The url must have one of 2 schemes: \e ws:// or \e wss://.
  When not supplied, then \e ws:// is used.
*/

/*!
  \property QQmlWebSocket::url
  \brief the server URL to connect to.

  The URL must have one of 2 schemes: \c{ws://} or \c{wss://}.
  When not supplied, \c{ws://} is used.
*/

/*!
  \qmlproperty list<string> WebSocket::requestedSubprotocols
  \since 6.4
  The list of WebSocket subprotocols to send in the WebSocket handshake.
*/

/*!
  \property QQmlWebSocket::requestedSubprotocols
  \since 6.4
  \brief the list of WebSocket subprotocols to send in the WebSocket handshake.
*/

/*!
  \qmlproperty string WebSocket::negotiatedSubprotocol
  \since 6.4
  The WebSocket subprotocol that has been negotiated with the server.
*/

/*!
  \property QQmlWebSocket::negotiatedSubprotocol
  \since 6.4
  \brief the WebSocket subprotocol that has been negotiated with the server.
*/

/*!
  \qmlproperty Status WebSocket::status
  Status of the WebSocket.

  The status can have the following values:
  \list
  \li WebSocket.Connecting
  \li WebSocket.Open
  \li WebSocket.Closing
  \li WebSocket.Closed
  \li WebSocket.Error
  \endlist
*/

/*!
  \property QQmlWebSocket::status
  \brief the status of the WebSocket.
*/

/*!
  \qmlproperty string WebSocket::errorString
  Contains a description of the last error that occurred. When no error occurrred,
  this string is empty.
*/

/*!
  \property QQmlWebSocket::errorString
  \brief a description of the last error that occurred.

  When no error occurred, this string is empty.
*/

/*!
  \qmlproperty bool WebSocket::active
  When set to true, a connection is made to the server with the given url.
  When set to false, the connection is closed.
  The default value is false.
*/

/*!
  \property QQmlWebSocket::active
  \brief whether the WebSocket connection is active.

  When set to \c true, a connection is made to the server with the given URL.
  When set to \c false, the connection is closed.
  The default value is \c false.
*/

/*!
  \qmlsignal WebSocket::textMessageReceived(QString message)
  This signal is emitted when a text message is received.
  \a message contains the bytes received.
*/

/*!
  \qmlsignal WebSocket::binaryMessageReceived(QString message)
  \since 5.8
  This signal is emitted when a binary message is received.
  \a message contains the bytes received.
*/

/*!
  \qmlsignal WebSocket::statusChanged(Status status)
  This signal is emitted when the status of the WebSocket changes.
  The \a status argument provides the current status.

  \sa {QtWebSockets::}{WebSocket::status}
*/

/*!
  \qmlmethod void WebSocket::sendTextMessage(string message)
  Sends \a message to the server.
*/

/*!
  \qmlmethod void WebSocket::sendBinaryMessage(ArrayBuffer message)
  \since 5.8
  Sends the parameter \a message to the server.
*/

/*!
  \qmlmethod void WebSocket::ping()
  \qmlmethod void WebSocket::ping(ArrayBuffer payload)
  \since 6.10
  Pings the server to indicate that the connection is still alive.
  An additional \a payload can be sent along with the ping message.

  The size of the \a payload cannot be bigger than 125 bytes.
  If it is larger, the \a payload is clipped to 125 bytes.

  \note QWebSocket and QWebSocketServer handles ping requests internally,
  which means they automatically send back a pong response to the peer.

  \sa pong()
*/

/*!
  \qmlsignal WebSocket::pong(quint64 elapsedTime, ArrayBuffer payload)
  \since 6.10
  Emitted when a pong message is received in reply to a previous ping.
  \a elapsedTime contains the roundtrip time in milliseconds and \a payload contains an optional
  payload that was sent with the ping.

  \sa ping()
*/

#include "qqmlwebsocket.h"
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketHandshakeOptions>

QT_BEGIN_NAMESPACE

/*!
    \class QQmlWebSocket
    \inmodule QtWebSockets
    \internal
*/
QQmlWebSocket::QQmlWebSocket(QObject *parent) :
    QObject(parent),
    m_webSocket(),
    m_status(Closed),
    m_url(),
    m_isActive(false),
    m_componentCompleted(true),
    m_errorString()
{
}

QQmlWebSocket::QQmlWebSocket(QWebSocket *socket, QObject *parent) :
    QObject(parent),
    m_status(Closed),
    m_url(socket->requestUrl()),
    m_requestedProtocols(socket->handshakeOptions().subprotocols()),
    m_isActive(true),
    m_componentCompleted(true),
    m_errorString(socket->errorString())
{
    setSocket(socket);
    onStateChanged(socket->state());
}

QQmlWebSocket::~QQmlWebSocket()
{
    if (m_webSocket)
        m_webSocket->disconnect();
}

qint64 QQmlWebSocket::sendTextMessage(const QString &message)
{
    if (m_status != Open) {
        setErrorString(tr("Messages can only be sent when the socket is open."));
        setStatus(Error);
        return 0;
    }
    return m_webSocket->sendTextMessage(message);
}

qint64 QQmlWebSocket::sendBinaryMessage(const QByteArray &message)
{
    if (m_status != Open) {
        setErrorString(tr("Messages can only be sent when the socket is open."));
        setStatus(Error);
        return 0;
    }
    return m_webSocket->sendBinaryMessage(message);
}

void QQmlWebSocket::ping()
{
    ping({});
}

void QQmlWebSocket::ping(const QByteArray &payload)
{
    if (m_status != Open) {
        setErrorString(tr("Messages can only be sent when the socket is open."));
        setStatus(Error);
    }
    m_webSocket->ping(payload);
}

QStringList QQmlWebSocket::requestedSubprotocols() const
{
    return m_requestedProtocols;
}

void QQmlWebSocket::setRequestedSubprotocols(const QStringList &requestedSubprotocols)
{
    if (m_requestedProtocols == requestedSubprotocols)
        return;

    m_requestedProtocols = requestedSubprotocols;
    emit requestedSubprotocolsChanged();
}

QUrl QQmlWebSocket::url() const
{
    return m_url;
}

void QQmlWebSocket::setUrl(const QUrl &url)
{
    if (m_url == url) {
        return;
    }
    if (m_webSocket && (m_status == Open)) {
        m_webSocket->close();
    }
    m_url = url;
    Q_EMIT urlChanged();
    open();
}

QString QQmlWebSocket::negotiatedSubprotocol() const
{
    return m_negotiatedProtocol;
}

QQmlWebSocket::Status QQmlWebSocket::status() const
{
    return m_status;
}

QString QQmlWebSocket::errorString() const
{
    return m_errorString;
}

void QQmlWebSocket::classBegin()
{
    m_componentCompleted = false;
    m_errorString = tr("QQmlWebSocket is not ready.");
    m_status = Closed;
}

void QQmlWebSocket::componentComplete()
{
    setSocket(new QWebSocket);

    m_componentCompleted = true;

    open();
}

void QQmlWebSocket::setSocket(QWebSocket *socket)
{
    m_webSocket.reset(socket);
    if (m_webSocket) {
        // explicit ownership via QScopedPointer
        m_webSocket->setParent(nullptr);
        connect(m_webSocket.data(), &QWebSocket::textMessageReceived,
                this, &QQmlWebSocket::textMessageReceived);
        connect(m_webSocket.data(), &QWebSocket::binaryMessageReceived,
                this, &QQmlWebSocket::binaryMessageReceived);
        connect(m_webSocket.data(), &QWebSocket::pong,
                this, &QQmlWebSocket::pong);
        connect(m_webSocket.data(), &QWebSocket::errorOccurred,
                this, &QQmlWebSocket::onError);
        connect(m_webSocket.data(), &QWebSocket::stateChanged,
                this, &QQmlWebSocket::onStateChanged);
    }
}

void QQmlWebSocket::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    setErrorString(m_webSocket->errorString());
    setStatus(Error);
}

void QQmlWebSocket::onStateChanged(QAbstractSocket::SocketState state)
{
    switch (state)
    {
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::BoundState:
        case QAbstractSocket::HostLookupState:
        {
            setStatus(Connecting);
            break;
        }
        case QAbstractSocket::UnconnectedState:
        {
            setStatus(Closed);
            break;
        }
        case QAbstractSocket::ConnectedState:
        {
            setStatus(Open);
            break;
        }
        case QAbstractSocket::ClosingState:
        {
            setStatus(Closing);
            break;
        }
        default:
        {
            setStatus(Connecting);
            break;
        }
    }
}

void QQmlWebSocket::setStatus(QQmlWebSocket::Status status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;
    if (status != Error) {
        setErrorString();
    }
    Q_EMIT statusChanged(m_status);

    const auto protocol = m_status == Open && m_webSocket ? m_webSocket->subprotocol() : QString();
    if (m_negotiatedProtocol != protocol) {
        m_negotiatedProtocol = protocol;
        Q_EMIT negotiatedSubprotocolChanged();
    }
}

void QQmlWebSocket::setActive(bool active)
{
    if (m_isActive == active) {
        return;
    }
    m_isActive = active;
    Q_EMIT activeChanged(m_isActive);
    if (!m_componentCompleted) {
        return;
    }
    if (m_isActive) {
        open();
    } else {
        close();
    }
}

bool QQmlWebSocket::isActive() const
{
    return m_isActive;
}

void QQmlWebSocket::open()
{
    if (m_componentCompleted && m_isActive && m_url.isValid() && Q_LIKELY(m_webSocket)) {
        QWebSocketHandshakeOptions opt;
        opt.setSubprotocols(m_requestedProtocols);
        m_webSocket->open(m_url, opt);
    }
}

void QQmlWebSocket::close()
{
    if (m_componentCompleted && Q_LIKELY(m_webSocket)) {
        m_webSocket->close();
    }
}

void QQmlWebSocket::setErrorString(QString errorString)
{
    if (m_errorString == errorString) {
        return;
    }
    m_errorString = errorString;
    Q_EMIT errorStringChanged(m_errorString);
}

QT_END_NAMESPACE
