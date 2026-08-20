// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qwebsocketserver.h"
#include "qwebsocketserver_p.h"
#include "qwebsocketprotocol.h"
#include "qwebsockethandshakerequest_p.h"
#include "qwebsockethandshakeresponse_p.h"
#include "qwebsocket.h"
#include "qwebsocket_p.h"
#include "qwebsocketcorsauthenticator.h"
#include <limits>

#ifndef QT_NO_SSL
#include "QtNetwork/QSslServer"
#endif
#include <QtCore/QTimer>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkProxy>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketServerPrivate::QWebSocketServerPrivate(const QString &serverName,
                                                 QWebSocketServerPrivate::SslMode secureMode) :
    QObjectPrivate(),
    m_pTcpServer(nullptr),
    m_serverName(serverName),
    m_secureMode(secureMode),
    m_pendingConnections(),
    m_error(QWebSocketProtocol::CloseCodeNormal),
    m_errorString(),
    m_maxPendingConnections(30),
    m_handshakeTimeout(10000)
{}

/*!
    \internal
 */
void QWebSocketServerPrivate::init()
{
    Q_Q(QWebSocketServer);

#ifdef Q_OS_WASM
    qWarning("QWebSocketServer: WebSocket servers are not supported by Qt for WebAssembly");
#endif

    if (m_secureMode == NonSecureMode) {
        m_pTcpServer = new QTcpServer(q);
        if (Q_LIKELY(m_pTcpServer))
            QObjectPrivate::connect(m_pTcpServer, &QTcpServer::pendingConnectionAvailable, this,
                                    &QWebSocketServerPrivate::onNewConnection);
        else
            qFatal("Could not allocate memory for tcp server.");
    } else {
#ifndef QT_NO_SSL
        QSslServer *pSslServer = new QSslServer(q);
        m_pTcpServer = pSslServer;
        // Update the QSslServer with the timeout we have:
        setHandshakeTimeout(m_handshakeTimeout);
        if (Q_LIKELY(m_pTcpServer)) {
            QObjectPrivate::connect(pSslServer, &QTcpServer::pendingConnectionAvailable, this,
                                    &QWebSocketServerPrivate::onNewConnection,
                                    Qt::QueuedConnection);
            QObject::connect(pSslServer, &QSslServer::peerVerifyError, q,
                             [q](QSslSocket *socket, const QSslError &error) {
                                    Q_UNUSED(socket);
                                    Q_EMIT q->peerVerifyError(error);
                             });
            QObject::connect(pSslServer, &QSslServer::sslErrors, q,
                             [q](QSslSocket *socket, const QList<QSslError> &errors) {
                                    Q_EMIT q->sslErrors(errors);
                                    Q_EMIT q->sslErrorsOccurred(socket, errors);
                             });
            QObject::connect(pSslServer, &QSslServer::preSharedKeyAuthenticationRequired, q,
                             [q](QSslSocket *socket,
                                 QSslPreSharedKeyAuthenticator *authenticator) {
                                    Q_UNUSED(socket);
                                    Q_EMIT q->preSharedKeyAuthenticationRequired(authenticator);
                             });
            QObject::connect(pSslServer, &QSslServer::alertSent, q,
                             [q](QSslSocket *socket, QSsl::AlertLevel level,
                                 QSsl::AlertType type, const QString &description) {
                                    Q_UNUSED(socket);
                                    Q_EMIT q->alertSent(level, type, description);
                                 });
            QObject::connect(pSslServer, &QSslServer::alertReceived, q,
                             [q](QSslSocket *socket, QSsl::AlertLevel level,
                                 QSsl::AlertType type, const QString &description) {
                                    Q_UNUSED(socket);
                                    Q_EMIT q->alertReceived(level, type, description);
                                 });
            QObject::connect(pSslServer, &QSslServer::handshakeInterruptedOnError, q,
                             [q](QSslSocket *socket, const QSslError &error) {
                                    Q_UNUSED(socket);
                                    Q_EMIT q->handshakeInterruptedOnError(error);
                                });
        }
#else
        qFatal("SSL not supported on this platform.");
#endif
    }
    QObject::connect(m_pTcpServer, &QTcpServer::acceptError, q, &QWebSocketServer::acceptError);
}

/*!
    \internal
 */
QWebSocketServerPrivate::~QWebSocketServerPrivate()
{
}

/*!
    \internal
 */
void QWebSocketServerPrivate::close(bool aboutToDestroy)
{
    Q_Q(QWebSocketServer);
    m_pTcpServer->close();
    while (!m_pendingConnections.isEmpty()) {
        QWebSocket *pWebSocket = m_pendingConnections.dequeue();
        pWebSocket->close(QWebSocketProtocol::CloseCodeGoingAway,
                          QWebSocketServer::tr("Server closed."));
        pWebSocket->deleteLater();
    }
    if (!aboutToDestroy) {
        //emit signal via the event queue, so the server gets time
        //to process any hanging events, like flushing buffers aso
        QMetaObject::invokeMethod(q, "closed", Qt::QueuedConnection);
    }
}

/*!
    \internal
 */
QString QWebSocketServerPrivate::errorString() const
{
    if (m_errorString.isEmpty())
        return m_pTcpServer->errorString();
    else
        return m_errorString;
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::hasPendingConnections() const
{
    return !m_pendingConnections.isEmpty();
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::isListening() const
{
    return m_pTcpServer->isListening();
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::listen(const QHostAddress &address, quint16 port)
{
    bool success = m_pTcpServer->listen(address, port);
    if (!success)
        setErrorFromSocketError(m_pTcpServer->serverError(), m_pTcpServer->errorString());
    return success;
}

/*!
    \internal
 */
int QWebSocketServerPrivate::maxPendingConnections() const
{
    return m_maxPendingConnections;
}

/*!
    \internal
 */
void QWebSocketServerPrivate::addPendingConnection(QWebSocket *pWebSocket)
{
    if (m_pendingConnections.size() < maxPendingConnections())
        m_pendingConnections.enqueue(pWebSocket);
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setErrorFromSocketError(QAbstractSocket::SocketError error,
                                                      const QString &errorDescription)
{
    Q_UNUSED(error);
    setError(QWebSocketProtocol::CloseCodeAbnormalDisconnection, errorDescription);
}

/*!
    \internal
 */
QWebSocket *QWebSocketServerPrivate::nextPendingConnection()
{
    QWebSocket *pWebSocket = nullptr;
    if (Q_LIKELY(!m_pendingConnections.isEmpty()))
        pWebSocket = m_pendingConnections.dequeue();
    return pWebSocket;
}

/*!
    \internal
 */
void QWebSocketServerPrivate::pauseAccepting()
{
    m_pTcpServer->pauseAccepting();
}

#ifndef QT_NO_NETWORKPROXY
/*!
    \internal
 */
QNetworkProxy QWebSocketServerPrivate::proxy() const
{
    return m_pTcpServer->proxy();
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setProxy(const QNetworkProxy &networkProxy)
{
    m_pTcpServer->setProxy(networkProxy);
}
#endif
/*!
    \internal
 */
void QWebSocketServerPrivate::resumeAccepting()
{
    m_pTcpServer->resumeAccepting();
}

/*!
    \internal
 */
QHostAddress QWebSocketServerPrivate::serverAddress() const
{
    return m_pTcpServer->serverAddress();
}

/*!
    \internal
 */
QWebSocketProtocol::CloseCode QWebSocketServerPrivate::serverError() const
{
    return m_error;
}

/*!
    \internal
 */
quint16 QWebSocketServerPrivate::serverPort() const
{
    return m_pTcpServer->serverPort();
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setMaxPendingConnections(int numConnections)
{
    if (m_pTcpServer->maxPendingConnections() <= numConnections)
        m_pTcpServer->setMaxPendingConnections(numConnections + 1);
    m_maxPendingConnections = numConnections;
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setHandshakeTimeout(int msec)
{
#if QT_CONFIG(ssl)
    if (auto *server = qobject_cast<QSslServer *>(m_pTcpServer)) {
        int timeout = msec;
        // Since QSslServer doesn't deal with negative numbers we set a very
        // large one instead to keep some level of compatibility:
        if (timeout < 0)
            timeout = std::numeric_limits<int>::max();
        server->setHandshakeTimeout(timeout);
    }
#endif
    m_handshakeTimeout = msec;
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_pTcpServer->setSocketDescriptor(socketDescriptor);
}

/*!
    \internal
 */
qintptr QWebSocketServerPrivate::socketDescriptor() const
{
    return m_pTcpServer->socketDescriptor();
}

/*!
    \internal
 */
QList<QWebSocketProtocol::Version> QWebSocketServerPrivate::supportedVersions() const
{
    QList<QWebSocketProtocol::Version> supportedVersions;
    supportedVersions << QWebSocketProtocol::currentVersion();	//we only support V13
    return supportedVersions;
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setSupportedSubprotocols(const QStringList &protocols)
{
    m_supportedSubprotocols = protocols;
}

/*!
    \internal
 */
QStringList QWebSocketServerPrivate::supportedSubprotocols() const
{
    return m_supportedSubprotocols;
}

/*!
    \internal
 */
QStringList QWebSocketServerPrivate::supportedExtensions() const
{
    QStringList supportedExtensions;
    return supportedExtensions;	//no extensions are currently supported
}

/*!
  \internal
 */
void QWebSocketServerPrivate::setServerName(const QString &serverName)
{
    if (m_serverName != serverName)
        m_serverName = serverName;
}

/*!
  \internal
 */
QString QWebSocketServerPrivate::serverName() const
{
    return m_serverName;
}

/*!
  \internal
 */
QWebSocketServerPrivate::SslMode QWebSocketServerPrivate::secureMode() const
{
    return m_secureMode;
}

#ifndef QT_NO_SSL
void QWebSocketServerPrivate::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    if (m_secureMode == SecureMode)
        qobject_cast<QSslServer *>(m_pTcpServer)->setSslConfiguration(sslConfiguration);
}

QSslConfiguration QWebSocketServerPrivate::sslConfiguration() const
{
    if (m_secureMode == SecureMode)
        return qobject_cast<QSslServer *>(m_pTcpServer)->sslConfiguration();
    else
        return QSslConfiguration::defaultConfiguration();
}
#endif

void QWebSocketServerPrivate::setError(QWebSocketProtocol::CloseCode code, const QString &errorString)
{
    if ((m_error != code) || (m_errorString != errorString)) {
        Q_Q(QWebSocketServer);
        m_error = code;
        m_errorString = errorString;
        Q_EMIT q->serverError(code);
    }
}

/*!
    \internal
 */
void QWebSocketServerPrivate::onNewConnection()
{
    while (m_pTcpServer->hasPendingConnections()) {
        QTcpSocket *pTcpSocket = m_pTcpServer->nextPendingConnection();
        Q_ASSERT(pTcpSocket);
        handleConnection(pTcpSocket);
    }
}

/*!
    \internal
 */
void QWebSocketServerPrivate::onSocketDisconnected()
{
    Q_Q(QWebSocketServer);
    QObject *sender = q->sender();
    if (Q_LIKELY(sender)) {
        QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender);
        if (Q_LIKELY(pTcpSocket))
            pTcpSocket->deleteLater();
    }
}

/*!
    \internal
 */
void QWebSocketServerPrivate::handshakeReceived()
{
    Q_Q(QWebSocketServer);
    QObject *sender = q->sender();
    if (Q_UNLIKELY(!sender)) {
        return;
    }
    QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender);
    if (Q_UNLIKELY(!pTcpSocket)) {
        return;
    }
    //When using Google Chrome the handshake in received in two parts.
    //Therefore, the readyRead signal is emitted twice.
    //This is a guard against the BEAST attack.
    //See: https://www.imperialviolet.org/2012/01/15/beastfollowup.html
    //For Safari, the handshake is delivered at once
    //FIXME: For FireFox, the readyRead signal is never emitted
    //This is a bug in FireFox (see https://bugzilla.mozilla.org/show_bug.cgi?id=594502)

    // According to RFC822 the body is separated from the headers by a null line (CRLF)
    const QByteArray& endOfHeaderMarker = QByteArrayLiteral("\r\n\r\n");

    const qint64 byteAvailable = pTcpSocket->bytesAvailable();
    QByteArray header = pTcpSocket->peek(byteAvailable);
    const int endOfHeaderIndex = header.indexOf(endOfHeaderMarker);
    if (endOfHeaderIndex < 0) {
        //then we don't have our header complete yet
        //check that no one is trying to exhaust our virtual memory
        const qint64 maxHeaderLength = QWebSocketPrivate::MAX_HEADERLINE_LENGTH
            * QWebSocketPrivate::MAX_HEADERLINES + endOfHeaderMarker.size();
        if (Q_UNLIKELY(byteAvailable > maxHeaderLength)) {
            pTcpSocket->close();
            setError(QWebSocketProtocol::CloseCodeTooMuchData,
                 QWebSocketServer::tr("Header is too large."));
        }
        return;
    }
    const int headerSize = endOfHeaderIndex + endOfHeaderMarker.size();

    disconnect(pTcpSocket, &QTcpSocket::readyRead,
               this, &QWebSocketServerPrivate::handshakeReceived);
    bool success = false;
    bool isSecure = (m_secureMode == SecureMode);

    if (Q_UNLIKELY(m_pendingConnections.size() >= maxPendingConnections())) {
        pTcpSocket->close();
        setError(QWebSocketProtocol::CloseCodeAbnormalDisconnection,
                 QWebSocketServer::tr("Too many pending connections."));
        return;
    }

    //don't read past the header
    header.resize(headerSize);
    //remove our header from the tcpSocket
    qint64 skippedSize = pTcpSocket->skip(headerSize);

    if (Q_UNLIKELY(skippedSize != headerSize)) {
        pTcpSocket->close();
        setError(QWebSocketProtocol::CloseCodeProtocolError,
                 QWebSocketServer::tr("Read handshake request header failed."));
        return;
    }

    QWebSocketHandshakeRequest request(pTcpSocket->peerPort(), isSecure);
    request.readHandshake(header, QWebSocketPrivate::MAX_HEADERLINE_LENGTH);

    if (request.isValid()) {
        QWebSocketCorsAuthenticator corsAuthenticator(request.origin());
        Q_EMIT q->originAuthenticationRequired(&corsAuthenticator);

        QWebSocketHandshakeResponse response(request,
                                             m_serverName,
                                             corsAuthenticator.allowed(),
                                             supportedVersions(),
                                             supportedSubprotocols(),
                                             supportedExtensions());

        if (Q_LIKELY(response.isValid())) {
            QTextStream httpStream(pTcpSocket);
            httpStream << response;
            httpStream.flush();

            if (Q_LIKELY(response.canUpgrade())) {
                QWebSocket *pWebSocket = QWebSocketPrivate::upgradeFrom(pTcpSocket,
                                                                        request,
                                                                        response);
                if (Q_LIKELY(pWebSocket)) {
                    finishHandshakeTimeout(pTcpSocket);
                    addPendingConnection(pWebSocket);
                    Q_EMIT q->newConnection();
                    success = true;
                } else {
                    setError(QWebSocketProtocol::CloseCodeAbnormalDisconnection,
                             QWebSocketServer::tr("Upgrade to WebSocket failed."));
                }
            }
            else {
                setError(response.error(), response.errorString());
            }
        } else {
            setError(QWebSocketProtocol::CloseCodeProtocolError,
                     QWebSocketServer::tr("Invalid response received."));
        }
    }
    if (!success) {
        pTcpSocket->close();
    }
}

void QWebSocketServerPrivate::handleConnection(QTcpSocket *pTcpSocket) const
{
    if (Q_LIKELY(pTcpSocket)) {
        startHandshakeTimeout(pTcpSocket);

        // Use a queued connection because a QSslSocket needs the event loop to process incoming
        // data. If not queued, data is incomplete when handshakeReceived is called.
        QObjectPrivate::connect(pTcpSocket, &QTcpSocket::readyRead,
                                this, &QWebSocketServerPrivate::handshakeReceived,
                                Qt::QueuedConnection);

        // We received some data! We must emit now to be sure that handshakeReceived is called
        // since the data could have been received before the signal and slot was connected.
        if (pTcpSocket->bytesAvailable()) {
            Q_EMIT pTcpSocket->readyRead();
        }

        QObjectPrivate::connect(pTcpSocket, &QTcpSocket::disconnected,
                                this, &QWebSocketServerPrivate::onSocketDisconnected);
    }
}

void QWebSocketServerPrivate::startHandshakeTimeout(QTcpSocket *pTcpSocket) const
{
    if (m_handshakeTimeout < 0)
        return;

    QTimer *handshakeTimer = new QTimer(pTcpSocket);
    handshakeTimer->setSingleShot(true);
    handshakeTimer->setObjectName(QStringLiteral("handshakeTimer"));
    QObject::connect(handshakeTimer, &QTimer::timeout, pTcpSocket, &QAbstractSocket::close);
    handshakeTimer->start(m_handshakeTimeout);
}

void QWebSocketServerPrivate::finishHandshakeTimeout(QTcpSocket *pTcpSocket)
{
    if (QTimer *handshakeTimer = pTcpSocket->findChild<QTimer *>(QStringLiteral("handshakeTimer"))) {
        handshakeTimer->stop();
        delete handshakeTimer;
    }
}

QT_END_NAMESPACE
