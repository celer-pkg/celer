// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qwebsocket.h"
#include "qwebsocket_p.h"
#include "qwebsocketprotocol_p.h"
#include "qwebsockethandshakerequest_p.h"
#include "qwebsockethandshakeresponse_p.h"
#include "qdefaultmaskgenerator_p.h"

#include <QtCore/QUrl>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QByteArray>
#include <QtCore/QtEndian>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>
#include <QtNetwork/QHostAddress>
#include <QtCore/QStringBuilder>   //for more efficient string concatenation
#ifndef QT_NONETWORKPROXY
#include <QtNetwork/QNetworkProxy>
#endif
#ifndef QT_NO_SSL
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#endif

#include <QtNetwork/private/qhttpheaderparser_p.h>
#include <QtNetwork/private/qauthenticator_p.h>

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <limits>
#include <memory>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {

constexpr quint64 MAX_OUTGOING_FRAME_SIZE_IN_BYTES = std::numeric_limits<int>::max() - 1;
constexpr quint64 DEFAULT_OUTGOING_FRAME_SIZE_IN_BYTES = 512 * 512 * 2; // default size of a frame when sending a message

// Based on isSeperator() from qtbase/src/network/access/qhsts.cpp
// https://datatracker.ietf.org/doc/html/rfc2616#section-2.2:
//
//     separators = "(" | ")" | "<" | ">" | "@"
//                | "," | ";" | ":" | "\" | <">
//                | "/" | "[" | "]" | "?" | "="
//                | "{" | "}" | SP | HT
// TODO: Should probably make things like this re-usable as private API of QtNetwork
bool isSeparator(char c)
{
    // separators     = "(" | ")" | "<" | ">" | "@"
    //                      | "," | ";" | ":" | "\" | <">
    //                      | "/" | "[" | "]" | "?" | "="
    //                      | "{" | "}" | SP | HT
    static const char separators[] = "()<>@,;:\\\"/[]?={} \t";
    static const char *end = separators + sizeof separators - 1;
    return std::find(separators, end, c) != end;
}

// https://datatracker.ietf.org/doc/html/rfc6455#section-4.1:
// 10.  The request MAY include a header field with the name
//      |Sec-WebSocket-Protocol|.  If present, this value indicates one
//      or more comma-separated subprotocol the client wishes to speak,
//      ordered by preference.  The elements that comprise this value
//      MUST be non-empty strings with characters in the range U+0021 to
//      U+007E not including separator characters as defined in
//      [RFC2616] and MUST all be unique strings.
bool isValidSubProtocolName(const QString &protocol)
{
    return std::all_of(protocol.begin(), protocol.end(), [](const QChar &c) {
        return c.unicode() >= 0x21 && c.unicode() <= 0x7E && !isSeparator(c.toLatin1());
    });
}

}

QWebSocketConfiguration::QWebSocketConfiguration() :
#ifndef QT_NO_SSL
    m_sslConfiguration(QSslConfiguration::defaultConfiguration()),
    m_ignoredSslErrors(),
    m_ignoreSslErrors(false),
#endif
#ifndef QT_NO_NETWORKPROXY
    m_proxy(QNetworkProxy::DefaultProxy),
#endif
    m_pSocket(nullptr)
{
}

/*!
    \internal
*/
QWebSocketPrivate::QWebSocketPrivate(const QString &origin, QWebSocketProtocol::Version version) :
    QObjectPrivate(),
    m_pSocket(nullptr),
    m_errorString(),
    m_version(version),
    m_resourceName(),
    m_request(),
    m_origin(origin),
    m_protocol(),
    m_extension(),
    m_socketState(QAbstractSocket::UnconnectedState),
    m_pauseMode(QAbstractSocket::PauseNever),
    m_readBufferSize(0),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_closeCode(QWebSocketProtocol::CloseCodeNormal),
    m_closeReason(),
    m_pingTimer(),
    m_configuration(),
    m_pMaskGenerator(&m_defaultMaskGenerator),
    m_defaultMaskGenerator(),
    m_outgoingFrameSize(DEFAULT_OUTGOING_FRAME_SIZE_IN_BYTES)
{
    m_pingTimer.start();
}

/*!
    \internal
*/
QWebSocketPrivate::QWebSocketPrivate(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version) :
    QObjectPrivate(),
    m_pSocket(pTcpSocket),
    m_errorString(pTcpSocket->errorString()),
    m_version(version),
    m_resourceName(),
    m_request(),
    m_origin(),
    m_protocol(),
    m_extension(),
    m_socketState(pTcpSocket->state()),
    m_pauseMode(pTcpSocket->pauseMode()),
    m_readBufferSize(pTcpSocket->readBufferSize()),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_closeCode(QWebSocketProtocol::CloseCodeNormal),
    m_closeReason(),
    m_pingTimer(),
    m_configuration(),
    m_pMaskGenerator(&m_defaultMaskGenerator),
    m_defaultMaskGenerator(),
    m_outgoingFrameSize(DEFAULT_OUTGOING_FRAME_SIZE_IN_BYTES)
{
    m_pingTimer.start();
}

/*!
    \internal
*/
void QWebSocketPrivate::init()
{
    Q_ASSERT(q_ptr);
    Q_ASSERT(m_pMaskGenerator);

    m_dataProcessor->setParent(q_ptr);
    m_pMaskGenerator->seed();

    if (m_pSocket) {
        makeConnections(m_pSocket);
    }
}

/*!
    \internal
*/
QWebSocketPrivate::~QWebSocketPrivate()
{
#ifdef Q_OS_WASM
    if (m_socketContext) {
        uint16_t m_readyState;
        emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);
        if (m_readyState == 1 || m_readyState == 0) {
            emscripten_websocket_close(m_socketContext, 1000,"");
        }
        emscripten_websocket_delete(m_socketContext);
    }
#endif
}

/*!
    \internal
*/
void QWebSocketPrivate::closeGoingAway()
{
    if (!m_pSocket)
        return;
    if (state() == QAbstractSocket::ConnectedState)
        close(QWebSocketProtocol::CloseCodeGoingAway, QWebSocket::tr("Connection closed"));
    releaseConnections(m_pSocket);
}

/*!
    \internal
 */
void QWebSocketPrivate::abort()
{
    if (m_pSocket)
        m_pSocket->abort();
}

/*!
    \internal
 */
QAbstractSocket::SocketError QWebSocketPrivate::error() const
{
    QAbstractSocket::SocketError err = QAbstractSocket::UnknownSocketError;
    if (Q_LIKELY(m_pSocket))
        err = m_pSocket->error();
    return err;
}

/*!
    \internal
 */
QString QWebSocketPrivate::errorString() const
{
    QString errMsg;
    if (!m_errorString.isEmpty())
        errMsg = m_errorString;
    else if (m_pSocket)
        errMsg = m_pSocket->errorString();
    return errMsg;
}

/*!
    \internal
 */
bool QWebSocketPrivate::flush()
{
    bool result = true;
    if (Q_LIKELY(m_pSocket))
        result = m_pSocket->flush();
    return result;
}

#ifndef Q_OS_WASM

/*!
    \internal
 */
qint64 QWebSocketPrivate::sendTextMessage(const QString &message)
{
    return doWriteFrames(message.toUtf8(), false);
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::sendBinaryMessage(const QByteArray &data)
{
    return doWriteFrames(data, true);
}

#endif

#ifndef QT_NO_SSL
/*!
    \internal
 */
void QWebSocketPrivate::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    m_configuration.m_sslConfiguration = sslConfiguration;
}

/*!
    \internal
 */
QSslConfiguration QWebSocketPrivate::sslConfiguration() const
{
    return m_configuration.m_sslConfiguration;
}

/*!
    \internal
 */
void QWebSocketPrivate::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_configuration.m_ignoredSslErrors = errors;
    if (Q_LIKELY(m_pSocket)) {
        QSslSocket *pSslSocket = qobject_cast<QSslSocket *>(m_pSocket);
        if (Q_LIKELY(pSslSocket))
            pSslSocket->ignoreSslErrors(errors);
    }
}

/*!
 * \internal
 */
void QWebSocketPrivate::ignoreSslErrors()
{
    m_configuration.m_ignoreSslErrors = true;
    if (Q_LIKELY(m_pSocket)) {
        QSslSocket *pSslSocket = qobject_cast<QSslSocket *>(m_pSocket);
        if (Q_LIKELY(pSslSocket))
            pSslSocket->ignoreSslErrors();
    }
}

/*!
 * \internal
 */
void QWebSocketPrivate::continueInterruptedHandshake()
{
    if (Q_LIKELY(m_pSocket)) {
        QSslSocket *pSslSocket = qobject_cast<QSslSocket *>(m_pSocket);
        if (Q_LIKELY(pSslSocket))
            pSslSocket->continueInterruptedHandshake();
    }
}

/*!
* \internal
*/
void QWebSocketPrivate::_q_updateSslConfiguration()
{
    if (QSslSocket *sslSock = qobject_cast<QSslSocket *>(m_pSocket))
        m_configuration.m_sslConfiguration = sslSock->sslConfiguration();
}

#endif

QStringList QWebSocketPrivate::requestedSubProtocols() const
{
    auto subprotocolsRequestedInRawHeader = [this]() {
        QStringList protocols;
        QByteArrayView rawProtocols = m_request.headers().value("Sec-WebSocket-Protocol");
        QLatin1StringView rawProtocolsView(rawProtocols);
        const QStringList &optionsProtocols = m_options.subprotocols();
        for (auto &&entry : rawProtocolsView.tokenize(u',', Qt::SkipEmptyParts)) {
            if (QLatin1StringView trimmed = entry.trimmed(); !trimmed.isEmpty()) {
                if (!optionsProtocols.contains(trimmed))
                    protocols << trimmed;
            }
        }
        return protocols;
    };
    return m_options.subprotocols() + subprotocolsRequestedInRawHeader();
}

/*!
  Called from QWebSocketServer
  \internal
 */
QWebSocket *QWebSocketPrivate::upgradeFrom(QTcpSocket *pTcpSocket,
                                           const QWebSocketHandshakeRequest &request,
                                           const QWebSocketHandshakeResponse &response,
                                           QObject *parent)
{
    QWebSocket *pWebSocket = new QWebSocket(pTcpSocket, response.acceptedVersion(), parent);
    if (Q_LIKELY(pWebSocket)) {
        QNetworkRequest netRequest(request.requestUrl());
        netRequest.setHeaders(request.headers());
#ifndef QT_NO_SSL
        if (QSslSocket *sslSock = qobject_cast<QSslSocket *>(pTcpSocket))
            pWebSocket->setSslConfiguration(sslSock->sslConfiguration());
#endif
        QWebSocketHandshakeOptions options;
        options.setSubprotocols(request.protocols());

        pWebSocket->d_func()->setExtension(response.acceptedExtension());
        pWebSocket->d_func()->setOrigin(request.origin());
        pWebSocket->d_func()->setRequest(netRequest, options);
        pWebSocket->d_func()->setProtocol(response.acceptedProtocol());
        pWebSocket->d_func()->setResourceName(request.requestUrl().toString(QUrl::RemoveUserInfo));
        //a server should not send masked frames
        pWebSocket->d_func()->enableMasking(false);
    }

    return pWebSocket;
}

#ifndef Q_OS_WASM

/*!
    \internal
 */
void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    if (Q_UNLIKELY(!m_pSocket))
        return;
    Q_Q(QWebSocket);
    if (!m_isClosingHandshakeSent) {
        m_closeCode = closeCode;
        // 125 is the maximum length of a control frame, and 2 bytes are used for the close code:
        const QByteArray reasonUtf8 = reason.toUtf8().left(123);
        m_closeReason = QString::fromUtf8(reasonUtf8);
        const quint16 code = qToBigEndian<quint16>(closeCode);
        QByteArray payload;
        payload.append(static_cast<const char *>(static_cast<const void *>(&code)), 2);
        if (!reasonUtf8.isEmpty())
            payload.append(reasonUtf8);
        quint32 maskingKey = 0;
        if (m_mustMask) {
            maskingKey = generateMaskingKey();
            QWebSocketProtocol::mask(payload.data(), quint64(payload.size()), maskingKey);
        }
        QByteArray frame = getFrameHeader(QWebSocketProtocol::OpCodeClose,
                                          quint64(payload.size()), maskingKey, true);

        Q_ASSERT(payload.size() <= 125);
        frame.append(payload);
        m_pSocket->write(frame);
        m_pSocket->flush();

        m_isClosingHandshakeSent = true;

        Q_EMIT q->aboutToClose();
    }
    static constexpr auto closeTimerName = "_closeTimer"_L1;
    if (!m_isClosingHandshakeReceived) {
        QTimer *closeTimer = new QTimer(q);
        closeTimer->setObjectName(closeTimerName);
        closeTimer->setInterval(3000);
        closeTimer->setSingleShot(true);
        QObject::connect(q, &QWebSocket::disconnected, closeTimer, &QTimer::stop);
        QObject::connect(q, &QWebSocket::disconnected, closeTimer, &QTimer::deleteLater);
        QObject::connect(closeTimer, &QTimer::timeout, m_pSocket, &QTcpSocket::close);
        QObject::connect(closeTimer, &QTimer::timeout, closeTimer, &QTimer::deleteLater);
        closeTimer->start();
    } else {
        m_pSocket->close();
        if (QTimer *closeTimer = q->findChild<QTimer *>(closeTimerName)) {
            closeTimer->stop();
            delete closeTimer;
        }
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::open(const QNetworkRequest &request,
                             const QWebSocketHandshakeOptions &options, bool mask)
{
    //just delete the old socket for the moment;
    //later, we can add more 'intelligent' handling by looking at the URL

    Q_Q(QWebSocket);
    QUrl url = request.url();
    if (!url.isValid() || url.toString().contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("Invalid URL."));
        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        return;
    }
    if (m_pSocket) {
        releaseConnections(m_pSocket);
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
    //if (m_url != url)
    if (Q_LIKELY(!m_pSocket)) {
        m_dataProcessor->clear();
        m_isClosingHandshakeReceived = false;
        m_isClosingHandshakeSent = false;

        setRequest(request, options);
        if (url.path().isEmpty())
            url.setPath(QStringLiteral("/"));
        QString resourceName = url.path(QUrl::FullyEncoded);
        // Check for encoded \r\n
        if (resourceName.contains(QStringLiteral("%0D%0A"))) {
            setRequest(QNetworkRequest());  //clear request
            setErrorString(QWebSocket::tr("Invalid resource name."));
            emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
            return;
        }
        if (!url.query().isEmpty()) {
            if (!resourceName.endsWith(QChar::fromLatin1('?'))) {
                resourceName.append(QChar::fromLatin1('?'));
            }
            resourceName.append(url.query(QUrl::FullyEncoded));
        }
        if (resourceName.isEmpty())
            resourceName = QStringLiteral("/");
        setResourceName(resourceName);
        enableMasking(mask);

    #ifndef QT_NO_SSL
        if (url.scheme() == QStringLiteral("wss")) {
            if (!QSslSocket::supportsSsl()) {
                const QString message =
                        QWebSocket::tr("SSL Sockets are not supported on this platform.");
                setErrorString(message);
                emitErrorOccurred(QAbstractSocket::UnsupportedSocketOperationError);
            } else {
                QSslSocket *sslSocket = new QSslSocket(q);
                m_pSocket = sslSocket;
                if (Q_LIKELY(m_pSocket)) {
                    QObject::connect(sslSocket, &QSslSocket::connected, sslSocket, [sslSocket]() {
                        sslSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
                        sslSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
                    });
                    m_pSocket->setReadBufferSize(m_readBufferSize);
                    m_pSocket->setPauseMode(m_pauseMode);

                    makeConnections(m_pSocket);
                    setSocketState(QAbstractSocket::ConnectingState);

                    sslSocket->setSslConfiguration(m_configuration.m_sslConfiguration);

                    if (!request.peerVerifyName().isEmpty())
                        sslSocket->setPeerVerifyName(request.peerVerifyName());

                    if (Q_UNLIKELY(m_configuration.m_ignoreSslErrors))
                        sslSocket->ignoreSslErrors();
                    else
                        sslSocket->ignoreSslErrors(m_configuration.m_ignoredSslErrors);
    #ifndef QT_NO_NETWORKPROXY
                    sslSocket->setProxy(m_configuration.m_proxy);
                    m_pSocket->setProtocolTag(QStringLiteral("https"));
    #endif
                    sslSocket->connectToHostEncrypted(url.host(), quint16(url.port(443)));
                } else {
                    const QString message = QWebSocket::tr("Out of memory.");
                    setErrorString(message);
                    emitErrorOccurred(QAbstractSocket::SocketResourceError);
                }
            }
        } else
    #endif
        if (url.scheme() == QStringLiteral("ws")) {
            m_pSocket = new QTcpSocket(q);
            if (Q_LIKELY(m_pSocket)) {
                QObject::connect(m_pSocket, &QTcpSocket::connected, m_pSocket, [this](){
                    m_pSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
                    m_pSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
                });
                m_pSocket->setReadBufferSize(m_readBufferSize);
                m_pSocket->setPauseMode(m_pauseMode);

                makeConnections(m_pSocket);
                setSocketState(QAbstractSocket::ConnectingState);
    #ifndef QT_NO_NETWORKPROXY
                m_pSocket->setProxy(m_configuration.m_proxy);
                m_pSocket->setProtocolTag(QStringLiteral("http"));
    #endif
                m_pSocket->connectToHost(url.host(), quint16(url.port(80)));
            } else {
                const QString message = QWebSocket::tr("Out of memory.");
                setErrorString(message);
                emitErrorOccurred(QAbstractSocket::SocketResourceError);
            }
        } else {
            const QString message =
                    QWebSocket::tr("Unsupported WebSocket scheme: %1").arg(url.scheme());
            setErrorString(message);
            emitErrorOccurred(QAbstractSocket::UnsupportedSocketOperationError);
        }
    }
}

#endif

/*!
    \internal
 */
void QWebSocketPrivate::ping(const QByteArray &payload)
{
    QByteArray payloadTruncated = payload.left(125);
    m_pingTimer.start();
    quint32 maskingKey = 0;
    if (m_mustMask)
        maskingKey = generateMaskingKey();
    QByteArray pingFrame = getFrameHeader(QWebSocketProtocol::OpCodePing,
                                          quint64(payloadTruncated.size()),
                                          maskingKey, true);
    if (m_mustMask)
        QWebSocketProtocol::mask(&payloadTruncated, maskingKey);
    pingFrame.append(payloadTruncated);
    qint64 ret = writeFrame(pingFrame);
    Q_UNUSED(ret);
}

/*!
  \internal
    Sets the version to use for the WebSocket protocol;
    this must be set before the socket is opened.
*/
void QWebSocketPrivate::setVersion(QWebSocketProtocol::Version version)
{
    if (m_version != version)
        m_version = version;
}

/*!
    \internal
    Sets the resource name of the connection; must be set before the socket is openend
*/
void QWebSocketPrivate::setResourceName(const QString &resourceName)
{
    if (m_resourceName != resourceName)
        m_resourceName = resourceName;
}

/*!
  \internal
 */
void QWebSocketPrivate::setRequest(const QNetworkRequest &request,
                                   const QWebSocketHandshakeOptions &options)
{
    if (m_request != request)
        m_request = request;
    if (m_options != options)
        m_options = options;
}

/*!
  \internal
 */
void QWebSocketPrivate::setOrigin(const QString &origin)
{
    if (m_origin != origin)
        m_origin = origin;
}

/*!
  \internal
 */
void QWebSocketPrivate::setProtocol(const QString &protocol)
{
    if (m_protocol != protocol)
        m_protocol = protocol;
}

/*!
  \internal
 */
void QWebSocketPrivate::setExtension(const QString &extension)
{
    if (m_extension != extension)
        m_extension = extension;
}

/*!
  \internal
 */
void QWebSocketPrivate::enableMasking(bool enable)
{
    if (m_mustMask != enable)
        m_mustMask = enable;
}

/*!
 * \internal
 */
void QWebSocketPrivate::makeConnections(QTcpSocket *pTcpSocket)
{
    Q_ASSERT(pTcpSocket);
    Q_Q(QWebSocket);

    if (Q_LIKELY(pTcpSocket)) {
        //pass through signals
        QObjectPrivate::connect(pTcpSocket, &QAbstractSocket::errorOccurred, this,
                                &QWebSocketPrivate::emitErrorOccurred);
#ifndef QT_NO_NETWORKPROXY
        QObject::connect(pTcpSocket, &QAbstractSocket::proxyAuthenticationRequired, q,
                         &QWebSocket::proxyAuthenticationRequired);
#endif // QT_NO_NETWORKPROXY
        QObject::connect(pTcpSocket, &QAbstractSocket::readChannelFinished, q,
                         &QWebSocket::readChannelFinished);
        QObject::connect(pTcpSocket, &QAbstractSocket::aboutToClose, q, &QWebSocket::aboutToClose);

        QObjectPrivate::connect(pTcpSocket, &QObject::destroyed,
                                this, &QWebSocketPrivate::socketDestroyed);

        //catch signals
        QObjectPrivate::connect(pTcpSocket, &QAbstractSocket::stateChanged, this,
                                &QWebSocketPrivate::processStateChanged);
        QObjectPrivate::connect(pTcpSocket, &QAbstractSocket::readyRead, this,
                                &QWebSocketPrivate::processData);
#ifndef QT_NO_SSL
        const QSslSocket * const sslSocket = qobject_cast<const QSslSocket *>(pTcpSocket);
        if (sslSocket) {
            QObject::connect(sslSocket, &QSslSocket::preSharedKeyAuthenticationRequired, q,
                             &QWebSocket::preSharedKeyAuthenticationRequired);
            QObject::connect(sslSocket, &QSslSocket::encryptedBytesWritten, q,
                             &QWebSocket::bytesWritten);
            QObjectPrivate::connect(sslSocket,
                                    QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
                                    this, &QWebSocketPrivate::_q_updateSslConfiguration);
            QObject::connect(sslSocket,
                             QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
                             q, &QWebSocket::sslErrors);
            QObjectPrivate::connect(sslSocket, &QSslSocket::encrypted,
                                    this, &QWebSocketPrivate::_q_updateSslConfiguration);
            QObject::connect(sslSocket, &QSslSocket::peerVerifyError,
                             q, &QWebSocket::peerVerifyError);
            QObject::connect(sslSocket, &QSslSocket::alertSent,
                             q, &QWebSocket::alertSent);
            QObject::connect(sslSocket, &QSslSocket::alertReceived,
                             q, &QWebSocket::alertReceived);
            QObject::connect(sslSocket, &QSslSocket::handshakeInterruptedOnError,
                             q, &QWebSocket::handshakeInterruptedOnError);
        } else
#endif // QT_NO_SSL
        {
            QObject::connect(pTcpSocket, &QAbstractSocket::bytesWritten, q,
                             &QWebSocket::bytesWritten);
        }
    }

    QObject::connect(m_dataProcessor, &QWebSocketDataProcessor::textFrameReceived, q,
                     &QWebSocket::textFrameReceived);
    QObject::connect(m_dataProcessor, &QWebSocketDataProcessor::binaryFrameReceived, q,
                     &QWebSocket::binaryFrameReceived);
    QObject::connect(m_dataProcessor, &QWebSocketDataProcessor::binaryMessageReceived, q,
                     &QWebSocket::binaryMessageReceived);
    QObject::connect(m_dataProcessor, &QWebSocketDataProcessor::textMessageReceived, q,
                     &QWebSocket::textMessageReceived);
    QObjectPrivate::connect(m_dataProcessor, &QWebSocketDataProcessor::errorEncountered, this,
                            &QWebSocketPrivate::close);
    QObjectPrivate::connect(m_dataProcessor, &QWebSocketDataProcessor::pingReceived, this,
                            &QWebSocketPrivate::processPing);
    QObjectPrivate::connect(m_dataProcessor, &QWebSocketDataProcessor::pongReceived, this,
                            &QWebSocketPrivate::processPong);
    QObjectPrivate::connect(m_dataProcessor, &QWebSocketDataProcessor::closeReceived, this,
                            &QWebSocketPrivate::processClose);

    //fire readyread, in case we already have data inside the tcpSocket
    if (pTcpSocket->bytesAvailable())
        Q_EMIT pTcpSocket->readyRead();
}

/*!
 * \internal
 */
void QWebSocketPrivate::releaseConnections(const QTcpSocket *pTcpSocket)
{
    if (Q_LIKELY(pTcpSocket)) {
        // Explicitly disconnect this signal to avoid warning being printed about a destroyed-signal
        // being disconnected with the wildcard disconnect below
        disconnect(pTcpSocket, &QObject::destroyed, this, &QWebSocketPrivate::socketDestroyed);
        pTcpSocket->disconnect();
        // Re-connect, we need 'socketDestroyed' slot executed
        QObjectPrivate::connect(pTcpSocket, &QObject::destroyed,
                                this, &QWebSocketPrivate::socketDestroyed);
    }
    m_dataProcessor->disconnect();
}

/*!
    \internal
 */
QWebSocketProtocol::Version QWebSocketPrivate::version() const
{
    return m_version;
}

/*!
    \internal
 */
QString QWebSocketPrivate::resourceName() const
{
    return m_resourceName;
}

/*!
    \internal
 */
QNetworkRequest QWebSocketPrivate::request() const
{
    return m_request;
}

/*!
    \internal
 */
QString QWebSocketPrivate::origin() const
{
    return m_origin;
}

/*!
    \internal
 */
QWebSocketHandshakeOptions QWebSocketPrivate::handshakeOptions() const
{
    return m_options;
}

/*!
    \internal
 */
QString QWebSocketPrivate::protocol() const
{
    return m_protocol;
}

/*!
    \internal
 */
QString QWebSocketPrivate::extension() const
{
    return m_extension;
}

/*!
 * \internal
 */
QWebSocketProtocol::CloseCode QWebSocketPrivate::closeCode() const
{
    return m_closeCode;
}

/*!
 * \internal
 */
QString QWebSocketPrivate::closeReason() const
{
    return m_closeReason;
}

/*!
 * \internal
 */
QByteArray QWebSocketPrivate::getFrameHeader(QWebSocketProtocol::OpCode opCode,
                                             quint64 payloadLength, quint32 maskingKey,
                                             bool lastFrame)
{
    QByteArray header;
    bool ok = payloadLength <= 0x7FFFFFFFFFFFFFFFULL;

    if (Q_LIKELY(ok)) {
        //FIN, RSV1-3, opcode (RSV-1, RSV-2 and RSV-3 are zero)
        quint8 byte = static_cast<quint8>((opCode & 0x0F) | (lastFrame ? 0x80 : 0x00));
        header.append(static_cast<char>(byte));

        byte = 0x00;
        if (maskingKey != 0)
            byte |= 0x80;
        if (payloadLength <= 125) {
            byte |= static_cast<quint8>(payloadLength);
            header.append(static_cast<char>(byte));
        } else if (payloadLength <= 0xFFFFU) {
            byte |= 126;
            header.append(static_cast<char>(byte));
            quint16 swapped = qToBigEndian<quint16>(static_cast<quint16>(payloadLength));
            header.append(static_cast<const char *>(static_cast<const void *>(&swapped)), 2);
        } else if (payloadLength <= 0x7FFFFFFFFFFFFFFFULL) {
            byte |= 127;
            header.append(static_cast<char>(byte));
            quint64 swapped = qToBigEndian<quint64>(payloadLength);
            header.append(static_cast<const char *>(static_cast<const void *>(&swapped)), 8);
        }

        if (maskingKey != 0) {
            const quint32 mask = qToBigEndian<quint32>(maskingKey);
            header.append(static_cast<const char *>(static_cast<const void *>(&mask)),
                          sizeof(quint32));
        }
    } else {
        setErrorString(QStringLiteral("WebSocket::getHeader: payload too big!"));
        emitErrorOccurred(QAbstractSocket::DatagramTooLargeError);
    }

    return header;
}

/*!
 * \internal
 */
qint64 QWebSocketPrivate::doWriteFrames(const QByteArray &data, bool isBinary)
{
    qint64 payloadWritten = 0;
    if (Q_UNLIKELY(!m_pSocket) || (state() != QAbstractSocket::ConnectedState))
        return payloadWritten;

    const QWebSocketProtocol::OpCode firstOpCode = isBinary ?
                QWebSocketProtocol::OpCodeBinary : QWebSocketProtocol::OpCodeText;

    int numFrames = data.size() / int(outgoingFrameSize());
    QByteArray tmpData(data);
    tmpData.detach();
    char *payload = tmpData.data();
    quint64 sizeLeft = quint64(data.size()) % outgoingFrameSize();
    if (Q_LIKELY(sizeLeft))
        ++numFrames;

    //catch the case where the payload is zero bytes;
    //in this case, we still need to send a frame
    if (Q_UNLIKELY(numFrames == 0))
        numFrames = 1;
    quint64 currentPosition = 0;
    quint64 bytesLeft = quint64(data.size());

    for (int i = 0; i < numFrames; ++i) {
        quint32 maskingKey = 0;
        if (m_mustMask)
            maskingKey = generateMaskingKey();

        const bool isLastFrame = (i == (numFrames - 1));
        const bool isFirstFrame = (i == 0);

        const quint64 size = qMin(bytesLeft, outgoingFrameSize());
        const QWebSocketProtocol::OpCode opcode = isFirstFrame ? firstOpCode
                                                               : QWebSocketProtocol::OpCodeContinue;

        //write header
        m_pSocket->write(getFrameHeader(opcode, size, maskingKey, isLastFrame));

        //write payload
        if (Q_LIKELY(size > 0)) {
            char *currentData = payload + currentPosition;
            if (m_mustMask)
                QWebSocketProtocol::mask(currentData, size, maskingKey);
            qint64 written = m_pSocket->write(currentData, static_cast<qint64>(size));
            if (Q_LIKELY(written > 0)) {
                payloadWritten += written;
            } else {
                m_pSocket->flush();
                setErrorString(QWebSocket::tr("Error writing bytes to socket: %1.")
                               .arg(m_pSocket->errorString()));
                emitErrorOccurred(QAbstractSocket::NetworkError);
                break;
            }
        }
        currentPosition += size;
        bytesLeft -= size;
    }
    if (Q_UNLIKELY(payloadWritten != data.size())) {
        setErrorString(QWebSocket::tr("Bytes written %1 != %2.")
                       .arg(payloadWritten).arg(data.size()));
        emitErrorOccurred(QAbstractSocket::NetworkError);
    }
    return payloadWritten;
}

/*!
    \internal
 */
quint32 QWebSocketPrivate::generateMaskingKey() const
{
    return m_pMaskGenerator->nextMask();
}

/*!
    \internal
 */
QByteArray QWebSocketPrivate::generateKey() const
{
    QByteArray key;

    for (int i = 0; i < 4; ++i) {
        const quint32 tmp = m_pMaskGenerator->nextMask();
        key.append(static_cast<const char *>(static_cast<const void *>(&tmp)), sizeof(quint32));
    }

    return key.toBase64();
}


/*!
    \internal
 */
QString QWebSocketPrivate::calculateAcceptKey(const QByteArray &key) const
{
    const QByteArray tmpKey = key + QByteArrayLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    const QByteArray hash = QCryptographicHash::hash(tmpKey, QCryptographicHash::Sha1).toBase64();
    return QString::fromLatin1(hash);
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::writeFrames(const QList<QByteArray> &frames)
{
    qint64 written = 0;
    if (Q_LIKELY(m_pSocket)) {
        QList<QByteArray>::const_iterator it;
        for (it = frames.cbegin(); it < frames.cend(); ++it)
            written += writeFrame(*it);
    }
    return written;
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::writeFrame(const QByteArray &frame)
{
    qint64 written = 0;
    if (Q_LIKELY(m_pSocket))
        written = m_pSocket->write(frame);
    return written;
}

static QString msgUnsupportedAuthenticateChallenges(qsizetype count)
{
    // Keep the error on a single line so it can easily be searched for:
    //: 'WWW-Authenticate' is the HTTP header.
    return count == 1
        ? QWebSocket::tr("QWebSocketPrivate::processHandshake: "
                         "Unsupported WWW-Authenticate challenge encountered.")
        : QWebSocket::tr("QWebSocketPrivate::processHandshake: "
                         "Unsupported WWW-Authenticate challenges encountered.");
}

//called on the client for a server handshake response
/*!
    \internal
 */
void QWebSocketPrivate::processHandshake(QTcpSocket *pSocket)
{
    Q_Q(QWebSocket);
    if (Q_UNLIKELY(!pSocket))
        return;

    static const QByteArray endOfHeaderMarker = QByteArrayLiteral("\r\n\r\n");
    const qint64 byteAvailable = pSocket->bytesAvailable();
    QByteArray available = pSocket->peek(byteAvailable);
    const int endOfHeaderIndex = available.indexOf(endOfHeaderMarker);
    if (endOfHeaderIndex < 0) {
        //then we don't have our header complete yet
        //check that no one is trying to exhaust our virtual memory
        const qint64 maxHeaderLength = MAX_HEADERLINE_LENGTH * MAX_HEADERLINES + endOfHeaderMarker.size();
        if (Q_UNLIKELY(byteAvailable > maxHeaderLength)) {
            setErrorString(QWebSocket::tr("Header is too large"));
            emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        }
        return;
    }
    const int headerSize = endOfHeaderIndex + endOfHeaderMarker.size();
    //don't read past the header
    QByteArrayView headers = QByteArrayView(available).first(headerSize);
    //remove our header from the tcpSocket
    qint64 skippedSize = pSocket->skip(headerSize);

    if (Q_UNLIKELY(skippedSize != headerSize)) {
        setErrorString(QWebSocket::tr("Read handshake request header failed"));
        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        return;
    }

    QHttpHeaderParser parser;
    static const QByteArray endOfStatusMarker = QByteArrayLiteral("\r\n");
    const int endOfStatusIndex = headers.indexOf(endOfStatusMarker);
    const QByteArrayView status = headers.first(endOfStatusIndex);

    if (!parser.parseStatus(status)) {
        setErrorString(QWebSocket::tr("Read handshake request status failed"));
        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        return;
    }

    if (!parser.parseHeaders(headers.sliced(endOfStatusIndex + endOfStatusMarker.size()))) {
        setErrorString(QWebSocket::tr("Parsing handshake request header failed"));
        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        return;
    }

    const QString acceptKey = QString::fromLatin1(parser.combinedHeaderValue(
                                QByteArrayLiteral("sec-websocket-accept")));
    const QString upgrade = QString::fromLatin1(parser.combinedHeaderValue(
                                QByteArrayLiteral("upgrade")));
    const QString connection = QString::fromLatin1(parser.combinedHeaderValue(
                                QByteArrayLiteral("connection")));
#if 0 // unused for the moment
    const QString extensions = QString::fromLatin1(parser.combinedHeaderValue(
                                QByteArrayLiteral("sec-websocket-extensions"));
#endif
    const QString protocol = QString::fromLatin1(parser.combinedHeaderValue(
                                QByteArrayLiteral("sec-websocket-protocol")));
    if (!protocol.isEmpty() && !requestedSubProtocols().contains(protocol)) {
        setErrorString(QWebSocket::tr("WebSocket server has chosen protocol %1 which has not been "
                                      "requested")
                               .arg(protocol));
        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        return;
    }

    const QString version = QString::fromLatin1(parser.combinedHeaderValue(
                                QByteArrayLiteral("sec-websocket-version")));
    bool ok = false;
    QString errorDescription;
    switch (parser.getStatusCode()) {
    case 101: {
        //HTTP/x.y 101 Switching Protocols
        //TODO: do not check the httpStatusText right now
        ok = (acceptKey.size() > 0
              && parser.getMajorVersion() > 0 && parser.getMinorVersion() > 0
              && upgrade.compare(u"websocket", Qt::CaseInsensitive) == 0
              && connection.compare(u"upgrade", Qt::CaseInsensitive) == 0);

        if (ok) {
            const QString accept = calculateAcceptKey(m_key);
            if (accept != acceptKey) {
                ok = false;
                errorDescription = QWebSocket::tr(
                        "Accept-Key received from server %1 does not match the client key %2.")
                            .arg(acceptKey, accept);
            }
        } else {
            const QString upgradeParms = QLatin1String(
                    "Accept-key size: %1, version: %2.%3, upgrade: %4, connection: %5").arg(
                    QString::number(acceptKey.size()), QString::number(parser.getMajorVersion()),
                    QString::number(parser.getMinorVersion()), upgrade, connection);
            errorDescription = QWebSocket::tr(
                "Invalid parameter encountered during protocol upgrade: %1").arg(upgradeParms);
        }
        break;
    }
    case 400: {
        //HTTP/1.1 400 Bad Request
        if (!version.isEmpty()) {
            const QStringList versions = version.split(QStringLiteral(", "), Qt::SkipEmptyParts);
            if (!versions.contains(QString::number(QWebSocketProtocol::currentVersion()))) {
                //if needed to switch protocol version, then we are finished here
                //because we cannot handle other protocols than the RFC one (v13)
                errorDescription =
                    QWebSocket::tr("Handshake: Server requests a version that we don't support: %1.")
                        .arg(versions.join(QStringLiteral(", ")));
            } else {
                //we tried v13, but something different went wrong
                errorDescription =
                    QWebSocket::tr("QWebSocketPrivate::processHandshake: Unknown error condition encountered. Aborting connection.");
            }
        } else {
            errorDescription =
                QWebSocket::tr("QWebSocketPrivate::processHandshake: Unknown error condition encountered. Aborting connection.");
        }
        break;
    }
    case 401: {
        // HTTP/1.1 401 UNAUTHORIZED
        if (m_authenticator.isNull())
            m_authenticator.detach();
        auto *priv = QAuthenticatorPrivate::getPrivate(m_authenticator);
        const QList<QByteArray> challenges = parser.headerFieldValues("WWW-Authenticate");
        const bool isSupported = std::any_of(challenges.begin(), challenges.end(),
                                             QAuthenticatorPrivate::isMethodSupported);
        if (isSupported)
            priv->parseHttpResponse(parser.headers(), /*isProxy=*/false, m_request.url().host());
        if (!isSupported || priv->method == QAuthenticatorPrivate::None) {
            errorDescription = msgUnsupportedAuthenticateChallenges(challenges.size());
            break;
        }

        const QUrl url = m_request.url();
        const bool hasCredentials = !url.userName().isEmpty() || !url.password().isEmpty();
        if (hasCredentials) {
            m_authenticator.setUser(url.userName());
            m_authenticator.setPassword(url.password());
            // Unset username and password so we don't try it again
            QUrl copy = url;
            copy.setUserName({});
            copy.setPassword({});
            m_request.setUrl(copy);
        }
        if (priv->phase == QAuthenticatorPrivate::Done) { // No user/pass from URL:
            emit q->authenticationRequired(&m_authenticator);
            if (priv->phase == QAuthenticatorPrivate::Done) {
                // user/pass was not updated:
                errorDescription = QWebSocket::tr(
                        "QWebSocket::processHandshake: Host requires authentication");
                break;
            }
        }
        m_needsResendWithCredentials = true;
        if (parser.firstHeaderField("Connection").compare("close", Qt::CaseInsensitive) == 0)
            m_needsReconnect = true;
        else
            m_bytesToSkipBeforeNewResponse = parser.firstHeaderField("Content-Length").toInt();
        break;
    }
    default: {
        errorDescription =
            QWebSocket::tr("QWebSocketPrivate::processHandshake: Unhandled http status code: %1 (%2).")
                    .arg(parser.getStatusCode()).arg(parser.getReasonPhrase());
    }
    }

    if (ok) {
        // handshake succeeded
        setProtocol(protocol);
        setSocketState(QAbstractSocket::ConnectedState);
        Q_EMIT q->connected();
    } else if (m_needsResendWithCredentials) {
        if (m_needsReconnect && m_pSocket->state() != QAbstractSocket::UnconnectedState) {
            // Disconnect here, then in processStateChanged() we reconnect when
            // we are unconnected.
            m_pSocket->disconnectFromHost();
        } else {
            // I'm cheating, this is how a handshake starts:
            processStateChanged(QAbstractSocket::ConnectedState);
        }
        return;
    } else {
        // handshake failed
        setErrorString(errorDescription);
        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
        if (m_pSocket->state() != QAbstractSocket::UnconnectedState)
            m_pSocket->disconnectFromHost();
    }
}

/*!
    \internal
*/
static QString removeZoneId(const QString& hostPort)
{
    const auto closeBracketIndex = hostPort.indexOf(u']');
    const auto percentIndex = hostPort.indexOf(u'%');
    // Only perform a minimal sanity check, as at this point the
    // URI parts host and port were already used successfully to
    // connect to the host.
    if (!hostPort.startsWith(u'[') || closeBracketIndex == -1
        || percentIndex == -1 || percentIndex > closeBracketIndex) {
        return hostPort;
    }

    return hostPort.left(percentIndex) + hostPort.mid(closeBracketIndex);
}

/*!
    \internal
 */
void QWebSocketPrivate::processStateChanged(QAbstractSocket::SocketState socketState)
{
    Q_ASSERT(m_pSocket);
    Q_Q(QWebSocket);
    QAbstractSocket::SocketState webSocketState = this->state();

    switch (socketState) {
    case QAbstractSocket::ConnectedState:
#ifndef QT_NO_SSL
        if (QSslSocket *sslSock = qobject_cast<QSslSocket *>(m_pSocket))
            m_configuration.m_sslConfiguration = sslSock->sslConfiguration();
#endif
        if (webSocketState == QAbstractSocket::ConnectingState) {
            m_key = generateKey();

            QList<QPair<QString, QString> > headers;
            QString hostHeaderOverride;
            const auto h = m_request.headers();
            for (qsizetype i = 0; i < h.size(); ++i) {
                const auto name = h.nameAt(i);
                const auto value = h.valueAt(i);

                if (name.compare("Host", Qt::CaseInsensitive) == 0) {
                    hostHeaderOverride = QString::fromLatin1(value);
                    continue;
                }
                // protocols handled separately below
                if (name.compare("Sec-WebSocket-Protocol", Qt::CaseInsensitive) == 0)
                    continue;
                headers << qMakePair(QString::fromLatin1(name),
                                     QString::fromLatin1(value));
            }
            const QStringList subProtocols = requestedSubProtocols();

            // Perform authorization if needed:
            if (m_needsResendWithCredentials) {
                m_needsResendWithCredentials = false;
                // Based on QHttpNetworkRequest::uri:
                auto uri = [](QUrl url) -> QByteArray {
                    QUrl::FormattingOptions format(QUrl::RemoveFragment | QUrl::RemoveUserInfo
                                                   | QUrl::FullyEncoded);
                    if (url.path().isEmpty())
                        url.setPath(QStringLiteral("/"));
                    else
                        format |= QUrl::NormalizePathSegments;
                    return url.toEncoded(format);
                };
                auto *priv = QAuthenticatorPrivate::getPrivate(m_authenticator);
                Q_ASSERT(priv);
                QByteArray response = priv->calculateResponse("GET", uri(m_request.url()),
                                                              m_request.url().host());
                if (!response.isEmpty())
                    headers << qMakePair(u"Authorization"_s, QString::fromLatin1(response));
            }

            const auto format = QUrl::RemoveScheme | QUrl::RemoveUserInfo
                                | QUrl::RemovePath | QUrl::RemoveQuery
                                | QUrl::RemoveFragment;
            // mid(2) removes the prefix "//" from the remaining url
            const QString host = hostHeaderOverride.isEmpty()
                    ? removeZoneId(m_request.url().toString(format).mid(2))
                    : hostHeaderOverride;
            const QString handshake = createHandShakeRequest(m_resourceName,
                                                             host,
                                                             origin(),
                                                             QString(),
                                                             subProtocols,
                                                             m_key,
                                                             headers);
            if (handshake.isEmpty()) {
                m_pSocket->abort();
                emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);
                return;
            }
            m_pSocket->write(handshake.toLatin1());
        }
        break;

    case QAbstractSocket::ClosingState:
        if (webSocketState == QAbstractSocket::ConnectedState)
            setSocketState(QAbstractSocket::ClosingState);
        break;

    case QAbstractSocket::UnconnectedState:
        if (m_needsReconnect) {
            // Need to reinvoke the lambda queued because the underlying socket
            // isn't done cleaning up yet...
            auto reconnect = [this]() {
                m_needsReconnect = false;
                const QUrl url = m_request.url();
#if QT_CONFIG(ssl)
                const bool isEncrypted = url.scheme().compare(u"wss", Qt::CaseInsensitive) == 0;
                if (isEncrypted) {
                    // This has to work because we did it earlier; this is just us
                    // reconnecting!
                    auto *sslSocket = qobject_cast<QSslSocket *>(m_pSocket);
                    Q_ASSERT(sslSocket);
                    sslSocket->connectToHostEncrypted(url.host(), quint16(url.port(443)));
                } else
#endif
                {
                    m_pSocket->connectToHost(url.host(), quint16(url.port(80)));
                }
            };
            QMetaObject::invokeMethod(q, reconnect, Qt::QueuedConnection);
        } else if (webSocketState != QAbstractSocket::UnconnectedState) {
            setSocketState(QAbstractSocket::UnconnectedState);
            Q_EMIT q->disconnected();
        }
        break;

    case QAbstractSocket::HostLookupState:
    case QAbstractSocket::ConnectingState:
    case QAbstractSocket::BoundState:
    case QAbstractSocket::ListeningState:
        //do nothing
        //to make C++ compiler happy;
        break;
    }
}

void QWebSocketPrivate::socketDestroyed(QObject *socket)
{
    Q_ASSERT(m_pSocket);
    if (m_pSocket == socket)
        m_pSocket = nullptr;
}

/*!
 \internal
 */
void QWebSocketPrivate::processData()
{
    if (!m_pSocket) // disconnected with data still in-bound
        return;
    if (state() == QAbstractSocket::ConnectingState) {
        if (m_bytesToSkipBeforeNewResponse > 0)
            m_bytesToSkipBeforeNewResponse -= m_pSocket->skip(m_bytesToSkipBeforeNewResponse);
        if (m_bytesToSkipBeforeNewResponse > 0 || !m_pSocket->canReadLine())
            return;
        processHandshake(m_pSocket);
       // That may have changed state(), recheck in the next 'if' below.
    }
    if (state() != QAbstractSocket::ConnectingState) {
        while (m_pSocket->bytesAvailable()) {
            if (!m_dataProcessor->process(m_pSocket))
                return;
        }
    }
}

/*!
 \internal
 */
void QWebSocketPrivate::processPing(const QByteArray &data)
{
    Q_ASSERT(m_pSocket);
    quint32 maskingKey = 0;
    if (m_mustMask)
        maskingKey = generateMaskingKey();
    m_pSocket->write(getFrameHeader(QWebSocketProtocol::OpCodePong,
                                    unsigned(data.size()),
                                    maskingKey,
                                    true));
    if (data.size() > 0) {
        QByteArray maskedData = data;
        if (m_mustMask)
            QWebSocketProtocol::mask(&maskedData, maskingKey);
        m_pSocket->write(maskedData);
    }
}

/*!
 \internal
 */
void QWebSocketPrivate::processPong(const QByteArray &data)
{
    Q_Q(QWebSocket);
    Q_EMIT q->pong(static_cast<quint64>(m_pingTimer.elapsed()), data);
}

/*!
 \internal
 */
void QWebSocketPrivate::processClose(QWebSocketProtocol::CloseCode closeCode, QString closeReason)
{
    m_isClosingHandshakeReceived = true;
    close(closeCode, closeReason);
}

/*!
    \internal
 */
QString QWebSocketPrivate::createHandShakeRequest(QString resourceName,
                                                  QString host,
                                                  QString origin,
                                                  QString extensions,
                                                  const QStringList &protocols,
                                                  QByteArray key,
                                                  const QList<QPair<QString, QString> > &headers)
{
    QStringList handshakeRequest;
    if (resourceName.contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("The resource name contains newlines. " \
                                      "Possible attack detected."));
        return QString();
    }
    if (host.contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("The hostname contains newlines. " \
                                      "Possible attack detected."));
        return QString();
    }
    if (origin.contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("The origin contains newlines. " \
                                      "Possible attack detected."));
        return QString();
    }
    if (extensions.contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("The extensions attribute contains newlines. " \
                                      "Possible attack detected."));
        return QString();
    }

    handshakeRequest << QStringLiteral("GET ") % resourceName % QStringLiteral(" HTTP/1.1") <<
                        QStringLiteral("Host: ") % host <<
                        QStringLiteral("Upgrade: websocket") <<
                        QStringLiteral("Connection: Upgrade") <<
                        QStringLiteral("Sec-WebSocket-Key: ") % QString::fromLatin1(key);
    if (!origin.isEmpty())
        handshakeRequest << QStringLiteral("Origin: ") % origin;
    handshakeRequest << QStringLiteral("Sec-WebSocket-Version: ")
                            % QString::number(QWebSocketProtocol::currentVersion());
    if (extensions.size() > 0)
        handshakeRequest << QStringLiteral("Sec-WebSocket-Extensions: ") % extensions;

    const QStringList validProtocols = [&] {
        QStringList validProtocols;
        validProtocols.reserve(protocols.size());
        for (const auto &p : protocols) {
            if (isValidSubProtocolName(p))
                validProtocols.append(p);
            else
                qWarning() << "Ignoring invalid WebSocket subprotocol name" << p;
        }

        return validProtocols;
    }();

    if (!validProtocols.isEmpty()) {
        handshakeRequest << QStringLiteral("Sec-WebSocket-Protocol: ")
                                % validProtocols.join(QLatin1String(", "));
    }

    for (const auto &header : headers)
        handshakeRequest << header.first % QStringLiteral(": ") % header.second;

    handshakeRequest << QStringLiteral("\r\n");

    return handshakeRequest.join(QStringLiteral("\r\n"));
}

/*!
    \internal
 */
QAbstractSocket::SocketState QWebSocketPrivate::state() const
{
    return m_socketState;
}

/*!
    \internal
 */
void QWebSocketPrivate::setSocketState(QAbstractSocket::SocketState state)
{
    Q_Q(QWebSocket);
    if (m_socketState != state) {
        m_socketState = state;
        Q_EMIT q->stateChanged(m_socketState);
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setMaxAllowedIncomingFrameSize(quint64 maxAllowedIncomingFrameSize)
{
    m_dataProcessor->setMaxAllowedFrameSize(maxAllowedIncomingFrameSize);
}

/*!
    \internal
 */
quint64 QWebSocketPrivate::maxAllowedIncomingFrameSize() const
{
    return m_dataProcessor->maxAllowedFrameSize();
}

/*!
    \internal
 */
void QWebSocketPrivate::setMaxAllowedIncomingMessageSize(quint64 maxAllowedIncomingMessageSize)
{
    m_dataProcessor->setMaxAllowedMessageSize(maxAllowedIncomingMessageSize);
}

/*!
    \internal
 */
quint64 QWebSocketPrivate::maxAllowedIncomingMessageSize() const
{
    return m_dataProcessor->maxAllowedMessageSize();
}

/*!
    \internal
 */
quint64 QWebSocketPrivate::maxIncomingMessageSize()
{
    return QWebSocketDataProcessor::maxMessageSize();
}

/*!
    \internal
 */
quint64 QWebSocketPrivate::maxIncomingFrameSize()
{
    return QWebSocketDataProcessor::maxFrameSize();
}

/*!
    \internal
 */
void QWebSocketPrivate::setOutgoingFrameSize(quint64 outgoingFrameSize)
{
    if (outgoingFrameSize <= maxOutgoingFrameSize())
        m_outgoingFrameSize = outgoingFrameSize;
}

/*!
    \internal
 */
quint64 QWebSocketPrivate::outgoingFrameSize() const
{
    return m_outgoingFrameSize;
}

/*!
    \internal
 */
quint64 QWebSocketPrivate::maxOutgoingFrameSize()
{
    return MAX_OUTGOING_FRAME_SIZE_IN_BYTES;
}


/*!
    \internal
 */
void QWebSocketPrivate::setErrorString(const QString &errorString)
{
    if (m_errorString != errorString)
        m_errorString = errorString;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::localAddress() const
{
    QHostAddress address;
    if (Q_LIKELY(m_pSocket))
        address = m_pSocket->localAddress();
    return address;
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::localPort() const
{
    quint16 port = 0;
    if (Q_LIKELY(m_pSocket))
        port = m_pSocket->localPort();
    return port;
}

/*!
    \internal
 */
QAbstractSocket::PauseModes QWebSocketPrivate::pauseMode() const
{
    return m_pauseMode;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::peerAddress() const
{
    QHostAddress address;
    if (Q_LIKELY(m_pSocket))
        address = m_pSocket->peerAddress();
    return address;
}

/*!
    \internal
 */
QString QWebSocketPrivate::peerName() const
{
    QString name;
    if (Q_LIKELY(m_pSocket))
        name = m_pSocket->peerName();
    return name;
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::peerPort() const
{
    quint16 port = 0;
    if (Q_LIKELY(m_pSocket))
        port = m_pSocket->peerPort();
    return port;
}

#ifndef QT_NO_NETWORKPROXY
/*!
    \internal
 */
QNetworkProxy QWebSocketPrivate::proxy() const
{
    return m_configuration.m_proxy;
}

/*!
    \internal
 */
void QWebSocketPrivate::setProxy(const QNetworkProxy &networkProxy)
{
    if (m_configuration.m_proxy != networkProxy)
        m_configuration.m_proxy = networkProxy;
}
#endif  //QT_NO_NETWORKPROXY

/*!
    \internal
 */
void QWebSocketPrivate::setMaskGenerator(const QMaskGenerator *maskGenerator)
{
    if (!maskGenerator)
        m_pMaskGenerator = &m_defaultMaskGenerator;
    else if (maskGenerator != m_pMaskGenerator)
        m_pMaskGenerator = const_cast<QMaskGenerator *>(maskGenerator);
}

/*!
    \internal
 */
const QMaskGenerator *QWebSocketPrivate::maskGenerator() const
{
    Q_ASSERT(m_pMaskGenerator);
    return m_pMaskGenerator;
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::readBufferSize() const
{
    return m_readBufferSize;
}

/*!
    \internal
 */
void QWebSocketPrivate::resume()
{
    if (Q_LIKELY(m_pSocket))
        m_pSocket->resume();
}

/*!
  \internal
 */
void QWebSocketPrivate::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
    m_pauseMode = pauseMode;
    if (Q_LIKELY(m_pSocket))
        m_pSocket->setPauseMode(m_pauseMode);
}

/*!
    \internal
 */
void QWebSocketPrivate::setReadBufferSize(qint64 size)
{
    m_readBufferSize = size;
    if (Q_LIKELY(m_pSocket))
        m_pSocket->setReadBufferSize(m_readBufferSize);
}

void QWebSocketPrivate::emitErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_Q(QWebSocket);
    Q_EMIT q->errorOccurred(error);
#if QT_DEPRECATED_SINCE(6, 5)
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    Q_EMIT q->error(error);
    QT_WARNING_POP
#endif
}

#ifndef Q_OS_WASM
/*!
    \internal
 */
bool QWebSocketPrivate::isValid() const
{
    return (m_pSocket && m_pSocket->isValid() &&
            (m_socketState == QAbstractSocket::ConnectedState));
}
#endif

QT_END_NAMESPACE
