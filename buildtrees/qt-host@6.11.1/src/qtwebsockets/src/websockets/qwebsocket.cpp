// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

/*!
    \class QWebSocket

    \inmodule QtWebSockets
    \since 5.3
    \brief Implements a TCP socket that talks the WebSocket protocol.

    WebSockets is a web technology providing full-duplex communications channels over
    a single TCP connection.
    The WebSocket protocol was standardized by the IETF as
    \l {RFC 6455} in 2011.
    QWebSocket can both be used in a client application and server application.

    This class was modeled after QAbstractSocket.

    QWebSocket currently does not support \l {WebSocket Extensions}.

    QWebSocket only supports version 13 of the WebSocket protocol, as outlined in
    \l {RFC 6455}.

    \note Some proxies do not understand certain HTTP headers used during a WebSocket handshake.
    In that case, non-secure WebSocket connections fail. The best way to mitigate against
    this problem is to use WebSocket over a secure connection.

    \warning To generate masks, this implementation of WebSockets uses the reasonably
    secure QRandomGenerator::global()->generate() function.
    For more information about the importance of good masking,
    see \l {"Talking to Yourself for Fun and Profit" by Lin-Shung Huang et al}.
    The best measure against attacks mentioned in the document above,
    is to use QWebSocket over a secure connection (\e wss://).
    In general, always be careful to not have 3rd party script access to
    a QWebSocket in your application.

    \sa QAbstractSocket, QTcpSocket

    \sa {QWebSocket client example}
*/

/*!
    \page echoclient.html
    \title QWebSocket client example
    \brief A sample WebSocket client that sends a message and displays the message that
    it receives back.

    \section1 Description
    The EchoClient example implements a WebSocket client that sends a message to a WebSocket server
    and dumps the answer that it gets back.
    This example should ideally be used with the EchoServer example.
    \section1 Code
    We start by connecting to the `connected()` signal.
    \snippet echoclient/echoclient.cpp constructor
    After the connection, we open the socket to the given \a url.

    \snippet echoclient/echoclient.cpp onConnected
    When the client is connected successfully, we connect to the `onTextMessageReceived()` signal,
    and send out "Hello, world!".
    If connected with the EchoServer, we will receive the same message back.

    \snippet echoclient/echoclient.cpp onTextMessageReceived
    Whenever a message is received, we write it out.
*/

/*!
  \fn void QWebSocket::connected()
  \brief Emitted when a connection is successfully established.
  A connection is successfully established when the socket is connected
  and the handshake was successful.
  \sa open(), disconnected()
*/
/*!
  \fn void QWebSocket::disconnected()
  \brief Emitted when the socket is disconnected.
  \sa close(), connected()
*/
/*!
    \fn void QWebSocket::aboutToClose()

    This signal is emitted when the socket is about to close.
    Connect this signal if you have operations that need to be performed before the socket closes
    (e.g., if you have data in a separate buffer that needs to be written to the device).

    \sa close()
 */
/*!
\fn void QWebSocket::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)

This signal can be emitted when a \a proxy that requires
authentication is used. The \a authenticator object can then be
filled in with the required details to allow authentication and
continue the connection.

\note It is not possible to use a QueuedConnection to connect to
this signal, as the connection will fail if the authenticator has
not been filled in with new information when the signal returns.

\sa QAuthenticator, QNetworkProxy
*/

/*!
    \fn void QWebSocket::authenticationRequired(QAuthenticator *authenticator)
    \since 6.6

    This signal is emitted when the server requires authentication.
    The \a authenticator object must then be filled in with the required details
    to allow authentication and continue the connection.

    If you know that the server may require authentication, you can set the
    username and password on the initial QUrl, using QUrl::setUserName and
    QUrl::setPassword. QWebSocket will still try to connect \e{once} without
    using the provided credentials.

    \note It is not possible to use a QueuedConnection to connect to
    this signal, as the connection will fail if the authenticator has
    not been filled in with new information when the signal returns.

    \sa QAuthenticator
*/

/*!
    \fn void QWebSocket::stateChanged(QAbstractSocket::SocketState state);

    This signal is emitted whenever QWebSocket's state changes.
    The \a state parameter is the new state.

    \note QAbstractSocket::ConnectedState is emitted after the handshake
    with the server has succeeded.

    QAbstractSocket::SocketState is not a registered metatype, so for queued
    connections, you will have to register it with Q_REGISTER_METATYPE() and
    qRegisterMetaType().

    \sa state()
*/
/*!
    \fn void QWebSocket::readChannelFinished()

    This signal is emitted when the input (reading) stream is closed in this device.
    It is emitted as soon as the closing is detected.

    \sa close()
*/
/*!
    \fn void QWebSocket::bytesWritten(qint64 bytes)

    This signal is emitted every time a payload of data has been written to the socket.
    The \a bytes argument is set to the number of bytes that were written in this payload.

    \note This signal has the same meaning both for secure and non-secure WebSockets.
    As opposed to QSslSocket, bytesWritten() is only emitted when encrypted data is effectively
    written (see QSslSocket::encryptedBytesWritten()).
    \sa close()
*/

/*!
    \fn void QWebSocket::textFrameReceived(const QString &frame, bool isLastFrame);

    This signal is emitted whenever a text frame is received. The \a frame contains the data and
    \a isLastFrame indicates whether this is the last frame of the complete message.

    This signal can be used to process large messages frame by frame, instead of waiting for the
    complete message to arrive.

    \sa binaryFrameReceived()
*/
/*!
    \fn void QWebSocket::binaryFrameReceived(const QByteArray &frame, bool isLastFrame);

    This signal is emitted whenever a binary frame is received. The \a frame contains the data and
    \a isLastFrame indicates whether this is the last frame of the complete message.

    This signal can be used to process large messages frame by frame, instead of waiting for the
    complete message to arrive.

    \sa textFrameReceived()
*/
/*!
    \fn void QWebSocket::textMessageReceived(const QString &message);

    This signal is emitted whenever a text message is received. The \a message contains the
    received text.

    \sa binaryMessageReceived()
*/
/*!
    \fn void QWebSocket::binaryMessageReceived(const QByteArray &message);

    This signal is emitted whenever a binary message is received. The \a message contains the
    received bytes.

    \sa textMessageReceived()
*/
/*!
    \fn void QWebSocket::sslErrors(const QList<QSslError> &errors)
    QWebSocket emits this signal after the SSL handshake to indicate that one or more errors have
    occurred while establishing the identity of the peer.
    The errors are usually an indication that QWebSocket is unable to securely identify the peer.
    Unless any action is taken, the connection will be dropped after this signal has been emitted.
    If you want to continue connecting despite the errors that have occurred, you must call
    QWebSocket::ignoreSslErrors() from inside a slot connected to this signal.
    If you need to access the error list at a later point, you can call sslErrors()
    (without arguments).

    \a errors contains one or more errors that prevent QWebSocket from verifying the identity of
    the peer.

    \note You cannot use Qt::QueuedConnection when connecting to this signal, or calling
    QWebSocket::ignoreSslErrors() will have no effect.
*/
/*!
    \fn void QWebSocket::preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator)
    \since 5.8

    This signal is emitted if the SSL/TLS handshake negotiates a PSK
    ciphersuite, and therefore a PSK authentication is then required.

    When using PSK, the client must send to the server a valid identity and a
    valid pre shared key, in order for the SSL handshake to continue.
    Applications can provide this information in a slot connected to this
    signal, by filling in the passed \a authenticator object according to their
    needs.

    \note Ignoring this signal, or failing to provide the required credentials,
    will cause the handshake to fail, and therefore the connection to be aborted.

    \note The \a authenticator object is owned by the websocket and must not be
    deleted by the application.

    \sa QSslPreSharedKeyAuthenticator
    \sa QSslSocket::preSharedKeyAuthenticationRequired()
*/
/*!
    \fn void QWebSocket::peerVerifyError(const QSslError &error)
    \since 6.2

    QWebSocket can emit this signal several times during the SSL handshake,
    before encryption has been established, to indicate that an error has
    occurred while establishing the identity of the peer. The \a error is
    usually an indication that QWebSocket is unable to securely identify the
    peer.

    This signal provides you with an early indication when something's wrong.
    By connecting to this signal, you can manually choose to tear down the
    connection from inside the connected slot before the handshake has
    completed. If no action is taken, QWebSocket will proceed to emitting
    QWebSocket::sslErrors().

    \sa sslErrors()
*/
/*!
    \fn void QWebSocket::alertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
    \since 6.2

    QWebSocket emits this signal if an alert message was sent to a peer. \a level
    describes if it was a warning or a fatal error. \a type gives the code
    of the alert message. When a textual description of the alert message is
    available, it is supplied in \a description.

    \note This signal is mostly informational and can be used for debugging
    purposes, normally it does not require any actions from the application.
    \note Not all backends support this functionality.

    \sa alertReceived(), QSsl::AlertLevel, QSsl::AlertType
*/
/*!
    \fn void QWebSocket::alertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
    \since 6.2

    QWebSocket emits this signal if an alert message was received from a peer.
    \a level tells if the alert was fatal or it was a warning. \a type is the
    code explaining why the alert was sent. When a textual description of
    the alert message is available, it is supplied in \a description.

    \note The signal is mostly for informational and debugging purposes and does not
    require any handling in the application. If the alert was fatal, underlying
    backend will handle it and close the connection.
    \note Not all backends support this functionality.

    \sa alertSent(), QSsl::AlertLevel, QSsl::AlertType
*/
/*!
    \fn void QWebSocket::handshakeInterruptedOnError(const QSslError &error)
    \since 6.2

    QWebSocket emits this signal if a certificate verification error was
    found and if early error reporting was enabled in QSslConfiguration.
    An application is expected to inspect the \a error and decide if
    it wants to continue the handshake, or abort it and send an alert message
    to the peer. The signal-slot connection must be direct.

    \sa continueInterruptedHandshake(), sslErrors(), QSslConfiguration::setHandshakeMustInterruptOnError()
*/
/*!
    \fn void QWebSocket::pong(quint64 elapsedTime, const QByteArray &payload)

    Emitted when a pong message is received in reply to a previous ping.
    \a elapsedTime contains the roundtrip time in milliseconds and \a payload contains an optional
    payload that was sent with the ping.

    \sa ping()
  */
#include "qwebsocket.h"
#include "qwebsocket_p.h"
#include "qwebsockethandshakeoptions.h"

#include <QtCore/QUrl>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QByteArray>
#include <QtNetwork/QHostAddress>

#include <QtCore/QDebug>

#include <limits>

QT_BEGIN_NAMESPACE

/*!
 * \brief Creates a new QWebSocket with the given \a origin,
 * the \a version of the protocol to use and \a parent.
 *
 * The \a origin of the client is as specified in \l {RFC 6454}.
 * (The \a origin is not required for non-web browser clients
 * (see \l {RFC 6455})).
 * The \a origin may not contain new line characters, otherwise the connection will be
 * aborted immediately during the handshake phase.
 * \note Currently only V13 (\l {RFC 6455}) is supported
 */
QWebSocket::QWebSocket(const QString &origin,
                       QWebSocketProtocol::Version version,
                       QObject *parent) :
    QObject(*(new QWebSocketPrivate(origin, version)), parent)
{
    Q_D(QWebSocket);
    d->init();
}

/*!
 * \brief Destroys the QWebSocket. Closes the socket if it is still open,
 * and releases any used resources.
 */
QWebSocket::~QWebSocket()
{
    Q_D(QWebSocket);
    d->closeGoingAway();
}

/*!
 * \brief Aborts the current socket and resets the socket.
 * Unlike close(), this function immediately closes the socket,
 * discarding any pending data in the write buffer.
 */
void QWebSocket::abort()
{
    Q_D(QWebSocket);
    d->abort();
}

/*!
 * Returns the type of error that last occurred
 * \sa errorString()
 */
QAbstractSocket::SocketError QWebSocket::error() const
{
    Q_D(const QWebSocket);
    return d->error();
}

//only called by QWebSocketPrivate::upgradeFrom
/*!
  \internal
 */
QWebSocket::QWebSocket(QTcpSocket *pTcpSocket,
                       QWebSocketProtocol::Version version, QObject *parent) :
    QObject(*(new QWebSocketPrivate(pTcpSocket, version)), parent)
{
    Q_D(QWebSocket);
    d->init();
}

/*!
 * Returns a human-readable description of the last error that occurred
 *
 * \sa error()
 */
QString QWebSocket::errorString() const
{
    Q_D(const QWebSocket);
    return d->errorString();
}

/*!
    This function writes as much as possible from the internal write buffer
    to the underlying network socket, without blocking.
    If any data was written, this function returns true; otherwise false is returned.
    Call this function if you need QWebSocket to start sending buffered data immediately.
    The number of bytes successfully written depends on the operating system.
    In most cases, you do not need to call this function,
    because QWebSocket will start sending data automatically
    once control goes back to the event loop.
*/
bool QWebSocket::flush()
{
    Q_D(QWebSocket);
    return d->flush();
}

/*!
    \brief Sends the given \a message over the socket as a text message and
    returns the number of bytes actually sent.

    \sa sendBinaryMessage()
 */
qint64 QWebSocket::sendTextMessage(const QString &message)
{
    Q_D(QWebSocket);
    return d->sendTextMessage(message);
}

/*!
    \brief Sends the given \a data over the socket as a binary message and
    returns the number of bytes actually sent.

    \sa sendTextMessage()
 */
qint64 QWebSocket::sendBinaryMessage(const QByteArray &data)
{
    Q_D(QWebSocket);
    return d->sendBinaryMessage(data);
}

/*!
    \brief Gracefully closes the socket with the given \a closeCode and \a reason.

    Any data in the write buffer is flushed before the socket is closed.
    The \a closeCode is a QWebSocketProtocol::CloseCode indicating the reason to close, and
    \a reason describes the reason of the closure more in detail. All control
    frames, including the Close frame, are limited to 125 bytes. Since two of
    these are used for \a closeCode the maximum length of \a reason is 123! If
    \a reason exceeds this limit it will be truncated.
 */
void QWebSocket::close(QWebSocketProtocol::CloseCode closeCode, const QString &reason)
{
    Q_D(QWebSocket);
    d->close(closeCode, reason);
}

/*!
    \brief Opens a WebSocket connection using the given \a url.

    If the url contains newline characters (\\r\\n), then the error signal will be emitted
    with QAbstractSocket::ConnectionRefusedError as error type.
 */
void QWebSocket::open(const QUrl &url)
{
    Q_D(QWebSocket);
    QNetworkRequest request(url);
    d->open(request, QWebSocketHandshakeOptions{}, true);
}

/*!
    \brief Opens a WebSocket connection using the given \a request.
    \since 5.6

    The \a request url will be used to open the WebSocket connection.
    Headers present in the request will be sent to the server in the upgrade request,
    together with the ones needed for the websocket handshake.
 */
void QWebSocket::open(const QNetworkRequest &request)
{
    Q_D(QWebSocket);
    d->open(request, QWebSocketHandshakeOptions{}, true);
}

/*!
    \brief Opens a WebSocket connection using the given \a url and \a options.
    \since 6.4

    If the url contains newline characters (\\r\\n), then the error signal will be emitted
    with QAbstractSocket::ConnectionRefusedError as error type.

    Additional options for the WebSocket handshake such as subprotocols can be specified in
    \a options.
 */
void QWebSocket::open(const QUrl &url, const QWebSocketHandshakeOptions &options)
{
    Q_D(QWebSocket);
    QNetworkRequest request(url);
    d->open(request, options, true);
}

/*!
    \brief Opens a WebSocket connection using the given \a request and \a options.
    \since 6.4

    The \a request url will be used to open the WebSocket connection.
    Headers present in the request will be sent to the server in the upgrade request,
    together with the ones needed for the websocket handshake.

    Additional options for the WebSocket handshake such as subprotocols can be specified in
    \a options.
 */
void QWebSocket::open(const QNetworkRequest &request, const QWebSocketHandshakeOptions &options)
{
    Q_D(QWebSocket);
    d->open(request, options, true);
}

/*!
    \brief Pings the server to indicate that the connection is still alive.
    An additional \a payload can be sent along with the ping message.

    The size of the \a payload cannot be bigger than 125 bytes.
    If it is larger, the \a payload is clipped to 125 bytes.

    \note QWebSocket and QWebSocketServer handles ping requests internally,
    which means they automatically send back a pong response to the peer.

    \sa pong()
 */
void QWebSocket::ping(const QByteArray &payload)
{
    Q_D(QWebSocket);
    d->ping(payload);
}

#ifndef QT_NO_SSL
/*!
    This slot tells QWebSocket to ignore errors during QWebSocket's
    handshake phase and continue connecting. If you want to continue
    with the connection even if errors occur during the handshake
    phase, then you must call this slot, either from a slot connected
    to sslErrors(), or before the handshake phase. If you don't call
    this slot, either in response to errors or before the handshake,
    the connection will be dropped after the sslErrors() signal has
    been emitted.

    \warning Be sure to always let the user inspect the errors
    reported by the sslErrors() signal, and only call this method
    upon confirmation from the user that proceeding is ok.
    If there are unexpected errors, the connection should be aborted.
    Calling this method without inspecting the actual errors will
    most likely pose a security risk for your application. Use it
    with great care!

    \sa sslErrors(), QSslSocket::ignoreSslErrors(), QNetworkReply::ignoreSslErrors()
*/
void QWebSocket::ignoreSslErrors()
{
    Q_D(QWebSocket);
    d->ignoreSslErrors();
}

/*!
    \since 6.2

    If an application wants to conclude a handshake even after receiving
    handshakeInterruptedOnError() signal, it must call this function.
    This call must be done from a slot function attached to the signal.
    The signal-slot connection must be direct.

    \sa handshakeInterruptedOnError(), QSslConfiguration::setHandshakeMustInterruptOnError()
*/
void QWebSocket::continueInterruptedHandshake()
{
    Q_D(QWebSocket);
    d->continueInterruptedHandshake();
}

/*!
    \overload

    This method tells QWebSocket to ignore the errors given in \a errors.

    Note that you can set the expected certificate in the SSL error:
    If, for instance, you want to connect to a server that uses
    a self-signed certificate, consider the following snippet:

    \snippet src_websockets_ssl_qwebsocket.cpp 6

    Multiple calls to this function will replace the list of errors that
    were passed in previous calls.
    You can clear the list of errors you want to ignore by calling this
    function with an empty list.

    \sa sslErrors()
*/
void QWebSocket::ignoreSslErrors(const QList<QSslError> &errors)
{
    Q_D(QWebSocket);
    d->ignoreSslErrors(errors);
}

/*!
    Sets the socket's SSL configuration to be the contents of \a sslConfiguration.

    This function sets the local certificate, the ciphers, the private key and
    the CA certificates to those stored in \a sslConfiguration.
    It is not possible to set the SSL-state related fields.
    \sa sslConfiguration()
 */
void QWebSocket::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    Q_D(QWebSocket);
    d->setSslConfiguration(sslConfiguration);
}

/*!
    Returns the socket's SSL configuration state.
    The default SSL configuration of a socket is to use the default ciphers,
    default CA certificates, no local private key or certificate.
    The SSL configuration also contains fields that can change with time without notice.

    \sa setSslConfiguration()
 */
QSslConfiguration QWebSocket::sslConfiguration() const
{
    Q_D(const QWebSocket);
    return d->sslConfiguration();
}

#endif  //not QT_NO_SSL

/*!
    \brief Returns the version the socket is currently using.
 */
QWebSocketProtocol::Version QWebSocket::version() const
{
    Q_D(const QWebSocket);
    return d->version();
}

/*!
    \brief Returns the name of the resource currently accessed.
 */
QString QWebSocket::resourceName() const
{
    Q_D(const QWebSocket);
    return d->resourceName();
}

/*!
    \brief Returns the url the socket is connected to or will connect to.
 */
QUrl QWebSocket::requestUrl() const
{
    Q_D(const QWebSocket);
    return d->request().url();
}

/*!
    \brief Returns the request that was or will be used to open this socket.
    \since 5.6
 */
QNetworkRequest QWebSocket::request() const
{
    Q_D(const QWebSocket);
    return d->request();
}

/*!
    \brief Returns the handshake options that were used to open this socket.
    \since 6.4
 */
QWebSocketHandshakeOptions QWebSocket::handshakeOptions() const
{
    Q_D(const QWebSocket);
    return d->handshakeOptions();
}

/*!
    \brief Returns the current origin.
 */
QString QWebSocket::origin() const
{
    Q_D(const QWebSocket);
    return d->origin();
}

/*!
    \brief Returns the used WebSocket protocol.
    \since 6.4
 */
QString QWebSocket::subprotocol() const
{
    Q_D(const QWebSocket);
    return d->protocol();
}

/*!
    \brief Returns the code indicating why the socket was closed.
    \sa QWebSocketProtocol::CloseCode, closeReason()
 */
QWebSocketProtocol::CloseCode QWebSocket::closeCode() const
{
    Q_D(const QWebSocket);
    return d->closeCode();
}

/*!
    \brief Returns the reason why the socket was closed.
    \sa closeCode()
 */
QString QWebSocket::closeReason() const
{
    Q_D(const QWebSocket);
    return d->closeReason();
}

/*!
    \brief Returns the current state of the socket.
 */
QAbstractSocket::SocketState QWebSocket::state() const
{
    Q_D(const QWebSocket);
    return d->state();
}

/*!
    Returns the local address
 */
QHostAddress QWebSocket::localAddress() const
{
    Q_D(const QWebSocket);
    return d->localAddress();
}

/*!
    Returns the local port
 */
quint16 QWebSocket::localPort() const
{
    Q_D(const QWebSocket);
    return d->localPort();
}

/*!
    Returns the pause mode of this socket
 */
QAbstractSocket::PauseModes QWebSocket::pauseMode() const
{
    Q_D(const QWebSocket);
    return d->pauseMode();
}

/*!
    Returns the peer address
 */
QHostAddress QWebSocket::peerAddress() const
{
    Q_D(const QWebSocket);
    return d->peerAddress();
}

/*!
    Returns the peerName
 */
QString QWebSocket::peerName() const
{
    Q_D(const QWebSocket);
    return d->peerName();
}

/*!
    Returns the peerport
 */
quint16 QWebSocket::peerPort() const
{
    Q_D(const QWebSocket);
    return d->peerPort();
}

#ifndef QT_NO_NETWORKPROXY
/*!
    Returns the currently configured proxy
 */
QNetworkProxy QWebSocket::proxy() const
{
    Q_D(const QWebSocket);
    return d->proxy();
}

/*!
    Sets the proxy to \a networkProxy
 */
void QWebSocket::setProxy(const QNetworkProxy &networkProxy)
{
    Q_D(QWebSocket);
    d->setProxy(networkProxy);
}
#endif

/*!
    Sets the generator to use for creating masks to \a maskGenerator.
    The default QWebSocket generator can be reset by supplying a \e nullptr.
    The mask generator can be changed at any time, even while the connection is open.
 */
void QWebSocket::setMaskGenerator(const QMaskGenerator *maskGenerator)
{
    Q_D(QWebSocket);
    d->setMaskGenerator(maskGenerator);
}

/*!
    Returns the mask generator that is currently used by this QWebSocket.
 */
const QMaskGenerator *QWebSocket::maskGenerator() const
{
    Q_D(const QWebSocket);
    return d->maskGenerator();
}

/*!
    Returns the size in bytes of the readbuffer that is used by the socket.
 */
qint64 QWebSocket::readBufferSize() const
{
    Q_D(const QWebSocket);
    return d->readBufferSize();
}

/*!
    Continues data transfer on the socket. This method should only be used after the socket
    has been set to pause upon notifications and a notification has been received.
    The only notification currently supported is sslErrors().
    Calling this method if the socket is not paused results in undefined behavior.

    \sa pauseMode(), setPauseMode()
 */
void QWebSocket::resume()
{
    Q_D(QWebSocket);
    d->resume();
}

/*!
    Controls whether to pause upon receiving a notification. The \a pauseMode parameter specifies
    the conditions in which the socket should be paused.

    The only notification currently supported is sslErrors().
    If set to PauseOnSslErrors, data transfer on the socket will be paused
    and needs to be enabled explicitly again by calling resume().
    By default, this option is set to PauseNever. This option must be called
    before connecting to the server, otherwise it will result in undefined behavior.

    \sa pauseMode(), resume()
 */
void QWebSocket::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
    Q_D(QWebSocket);
    d->setPauseMode(pauseMode);
}

/*!
    Sets the size of QWebSocket's internal read buffer to be \a size bytes.

    If the buffer size is limited to a certain size, QWebSocket won't buffer more than
    this size of data.
    Exceptionally, a buffer size of 0 means that the read buffer is unlimited and
    all incoming data is buffered. This is the default.
    This option is useful if you only read the data at certain points in time
    (for example, in a real-time streaming application) or if you want to protect your socket against
    receiving too much data, which may eventually cause your application to run out of memory.

    \sa readBufferSize()
*/
void QWebSocket::setReadBufferSize(qint64 size)
{
    Q_D(QWebSocket);
    d->setReadBufferSize(size);
}

/*!
    Returns \c true if the socket is ready for reading and writing; otherwise
    returns \c false.
 */
bool QWebSocket::isValid() const
{
    Q_D(const QWebSocket);
    return d->isValid();
}

/*!
    \since 5.12
    Returns the number of bytes that are waiting to be written. The bytes are written when control
    goes back to the event loop or when flush() is called.

    \sa flush
 */
qint64 QWebSocket::bytesToWrite() const
{
    Q_D(const QWebSocket);
    return d->m_pSocket ? d->m_pSocket->bytesToWrite() : 0;
}

/*!
    \since 5.15
    Sets the maximum allowed size of an incoming websocket frame to \a maxAllowedIncomingFrameSize.
    If an incoming frame exceeds this limit, the peer gets disconnected.
    The accepted range is between 0 and maxIncomingFrameSize(), default is maxIncomingFrameSize().
    The purpose of this function is to avoid exhausting virtual memory.

    \sa maxAllowedIncomingFrameSize()
 */
void QWebSocket::setMaxAllowedIncomingFrameSize(quint64 maxAllowedIncomingFrameSize)
{
    Q_D(QWebSocket);
    d->setMaxAllowedIncomingFrameSize(maxAllowedIncomingFrameSize);
}

/*!
    \since 5.15
    Returns the maximum allowed size of an incoming websocket frame.

    \sa setMaxAllowedIncomingFrameSize()
 */
quint64 QWebSocket::maxAllowedIncomingFrameSize() const
{
    Q_D(const QWebSocket);
    return d->maxAllowedIncomingFrameSize();
}

/*!
    \since 5.15
    Sets the maximum allowed size of an incoming websocket message to \a maxAllowedIncomingMessageSize.
    If an incoming message exceeds this limit, the peer gets disconnected.
    The accepted range is between 0 and maxIncomingMessageSize(), default is maxIncomingMessageSize().
    The purpose of this function is to avoid exhausting virtual memory.

    \sa maxAllowedIncomingMessageSize()
 */
void QWebSocket::setMaxAllowedIncomingMessageSize(quint64 maxAllowedIncomingMessageSize)
{
    Q_D(QWebSocket);
    d->setMaxAllowedIncomingMessageSize(maxAllowedIncomingMessageSize);
}

/*!
    \since 5.15
    Returns the maximum allowed size of an incoming websocket message.

    \sa setMaxAllowedIncomingMessageSize()
 */
quint64 QWebSocket::maxAllowedIncomingMessageSize() const
{
    Q_D(const QWebSocket);
    return d->maxAllowedIncomingMessageSize();
}

/*!
    \since 5.15
    Returns the maximum supported size of an incoming websocket message for this websocket
    implementation.
 */
quint64 QWebSocket::maxIncomingMessageSize()
{
    return QWebSocketPrivate::maxIncomingMessageSize();
}

/*!
    \since 5.15
    Returns the maximum supported size of an incoming websocket frame for this websocket
    implementation.
 */
quint64 QWebSocket::maxIncomingFrameSize()
{
    return QWebSocketPrivate::maxIncomingFrameSize();
}

/*!
    \since 5.15
    Sets the maximum size of an outgoing websocket frame to \a outgoingFrameSize.
    The accepted range is between 0 and maxOutgoingFrameSize(), default is 512kB.
    The purpose of this function is to adapt to the maximum allowed frame size
    of the receiver.

    \sa outgoingFrameSize()
 */
void QWebSocket::setOutgoingFrameSize(quint64 outgoingFrameSize)
{
    Q_D(QWebSocket);
    d->setOutgoingFrameSize(outgoingFrameSize);
}

/*!
    \since 5.15
    Returns the maximum size of an outgoing websocket frame.

    \sa setOutgoingFrameSize()
 */
quint64 QWebSocket::outgoingFrameSize() const
{
    Q_D(const QWebSocket);
    return d->outgoingFrameSize();
}

/*!
    \since 5.15
    Returns the maximum supported size of an outgoing websocket frame for this websocket
    implementation.
 */
quint64 QWebSocket::maxOutgoingFrameSize()
{
    return QWebSocketPrivate::maxOutgoingFrameSize();
}

/*!
    \fn void QWebSocket::errorOccurred(QAbstractSocket::SocketError error);

    \since 6.5
    \brief This signal is emitted after an error occurred.

    The \a error parameter describes the type of error that occurred.

    QAbstractSocket::SocketError is not a registered metatype, so for queued
    connections, you will have to register it with Q_DECLARE_METATYPE() and
    qRegisterMetaType().

    \sa error(), errorString()
*/
#if QT_DEPRECATED_SINCE(6, 5)
/*!
    \fn void QWebSocket::error(QAbstractSocket::SocketError error);
    \deprecated [6.5] Use errorOccurred(QAbstractSocket::SocketError error) instead.
*/
#endif

QT_END_NAMESPACE
