// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qwebsocket_p.h"

#if QT_CONFIG(thread)
#include <QtCore/qthread.h>
#include <emscripten/threading.h>
#endif

#include <emscripten.h>
#include <emscripten/websocket.h>
#include <emscripten/val.h>

#include <QHostAddress>

static EM_BOOL q_onWebSocketErrorCallback(int eventType,
                                          const EmscriptenWebSocketErrorEvent *e,
                                          void *userData)
{
    Q_UNUSED(eventType)
    Q_UNUSED(e)

    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate *>(userData);
    Q_ASSERT (wsp);

    emit wsp->q_func()->errorOccurred(wsp->error());
    return EM_FALSE;
}

static EM_BOOL q_onWebSocketCloseCallback(int eventType,
                                          const EmscriptenWebSocketCloseEvent *emCloseEvent,
                                          void *userData)
{
    Q_UNUSED(eventType)
    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate *>(userData);
    Q_ASSERT (wsp);

    wsp->setSocketClosed(emCloseEvent);
    return EM_FALSE;
}

static EM_BOOL q_onWebSocketOpenCallback(int eventType,
                                         const EmscriptenWebSocketOpenEvent *e, void *userData)
{
    Q_UNUSED(eventType)
    Q_UNUSED(e)

    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate *>(userData);
    Q_ASSERT (wsp);

    wsp->setSocketState(QAbstractSocket::ConnectedState);
    emit wsp->q_func()->connected();
    return EM_FALSE;
}

static EM_BOOL q_onWebSocketIncomingMessageCallback(int eventType,
                                                    const EmscriptenWebSocketMessageEvent *e,
                                                    void *userData)
{
    Q_UNUSED(eventType)
    QWebSocketPrivate *wsp = reinterpret_cast<QWebSocketPrivate *>(userData);
    Q_ASSERT(wsp);

    if (!e->isText) {
        QByteArray buffer(reinterpret_cast<const char *>(e->data), e->numBytes);
        if (!buffer.isEmpty())
            emit wsp->q_func()->binaryMessageReceived(buffer);
    } else {
        QString buffer = QString::fromUtf8(reinterpret_cast<const char *>(e->data), e->numBytes - 1);
        emit wsp->q_func()->textMessageReceived(buffer);
    }

    return 0;
}

qint64 QWebSocketPrivate::sendTextMessage(const QString &message)
{
    int result = 0;
    emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);

    if (m_readyState == 1) {
        QByteArray messageArray = message.toUtf8();
        result = emscripten_websocket_send_utf8_text(m_socketContext, messageArray);
        if (result < 0)
            emitErrorOccurred(QAbstractSocket::UnknownSocketError);
        else
            return messageArray.length();
    } else
        qWarning() << "Could not send message. Websocket is not open";

    return result;
}

qint64 QWebSocketPrivate::sendBinaryMessage(const QByteArray &data)
{
    int result = 0;
    emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);
    if (m_readyState == 1) {
        result = emscripten_websocket_send_binary(
                m_socketContext, const_cast<void *>(reinterpret_cast<const void *>(data.constData())),
                data.size());
        if (result < 0)
            emitErrorOccurred(QAbstractSocket::UnknownSocketError);
        else
            return data.size();
    } else
        qWarning() << "Could not send message. Websocket is not open";

    return result;
}

void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    Q_Q(QWebSocket);
    m_closeCode = closeCode;
    m_closeReason = reason;
    Q_EMIT q->aboutToClose();
    setSocketState(QAbstractSocket::ClosingState);

    emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);

    if (m_readyState == 1 || m_readyState == 0) {
        emscripten_websocket_close(m_socketContext, (int)closeCode, reason.toUtf8());
    }
    setSocketState(QAbstractSocket::UnconnectedState);
    emit q->disconnected();
    emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);
}

void QWebSocketPrivate::open(const QNetworkRequest &request,
                             const QWebSocketHandshakeOptions &options, bool mask)
{
    Q_UNUSED(mask);

    emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);

    if ((m_readyState == 1 || m_readyState == 3) && m_socketContext != 0) {
        emitErrorOccurred(QAbstractSocket::OperationError);
        return;
    }

    const QUrl url = request.url();

    emscripten::val navProtocol = emscripten::val::global("self")["location"]["protocol"];

    //  An insecure WebSocket connection may not be initiated from a page loaded over HTTPS.
    // and causes emscripten to assert
    bool isSecureContext = (navProtocol.as<std::string>().find("https") == 0);

    if (!url.isValid()
            || url.toString().contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("Connection refused"));

        emitErrorOccurred(QAbstractSocket::ConnectionRefusedError);

        return;
    }
    // exception for localhost/127.0.0.1/[::1]
    // localhost has special privledges

    QHostAddress hostAddress(url.host());

    bool hostAddressIsLocal = (hostAddress == QHostAddress::LocalHost
            || hostAddress == QHostAddress::LocalHostIPv6);

    if (url.host() != QStringLiteral("localhost") && !hostAddressIsLocal) {
        if (isSecureContext && url.scheme() == QStringLiteral("ws")) {
            const QString message =
                    QWebSocket::tr("Unsupported WebSocket scheme: %1").arg(url.scheme());
            setErrorString(message);
            emitErrorOccurred(QAbstractSocket::UnsupportedSocketOperationError);
            return;
        }
    }

    setRequest(request, options);

    EmscriptenWebSocketCreateAttributes attr;

    emscripten_websocket_init_create_attributes(&attr); // memset
    QByteArray thisUrl = url.toString(QUrl::FullyEncoded).toUtf8();
    attr.url = thisUrl.constData();

#if QT_CONFIG(thread)
    // see https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/websocket.h
    // choose a default: create websocket on calling thread
    attr.createOnMainThread = false;
#endif
    // HTML WebSockets do not support arbitrary request headers, but
    // do support the WebSocket protocol header. This header is
    // required for some use cases like MQTT.

    // add user subprotocol options
    QStringList protocols = requestedSubProtocols();
    QByteArray secProto;
    if (!protocols.isEmpty()) {
        // comma-separated list of protocol strings, no spaces
        secProto = protocols.join(QStringLiteral(",")).toLatin1();
        attr.protocols = secProto.constData();
    }

    // create and connect
    setSocketState(QAbstractSocket::ConnectingState);
    m_socketContext = emscripten_websocket_new(&attr);

    if (m_socketContext <= 0) { // m_readyState might not be changed yet
        // error
        emitErrorOccurred(QAbstractSocket::UnknownSocketError);
        return;
    }

#if QT_CONFIG(thread)
    emscripten_websocket_set_onopen_callback_on_thread(m_socketContext, (void *)this,
                                                       q_onWebSocketOpenCallback,
                                                       (quintptr)QThread::currentThreadId());
    emscripten_websocket_set_onmessage_callback_on_thread(m_socketContext, (void *)this,
                                                          q_onWebSocketIncomingMessageCallback,
                                                          (quintptr)QThread::currentThreadId());
    emscripten_websocket_set_onerror_callback_on_thread(m_socketContext, (void *)this,
                                                        q_onWebSocketErrorCallback,
                                                        (quintptr)QThread::currentThreadId());
    emscripten_websocket_set_onclose_callback_on_thread(m_socketContext, (void *)this,
                                                        q_onWebSocketCloseCallback,
                                                        (quintptr)QThread::currentThreadId());
#else
    emscripten_websocket_set_onopen_callback(m_socketContext, (void *)this,
                                             q_onWebSocketOpenCallback);
    emscripten_websocket_set_onmessage_callback(m_socketContext, (void *)this,
                                                q_onWebSocketIncomingMessageCallback);
    emscripten_websocket_set_onerror_callback(m_socketContext, (void *)this,
                                              q_onWebSocketErrorCallback);
    emscripten_websocket_set_onclose_callback(m_socketContext, (void *)this,
                                              q_onWebSocketCloseCallback);
#endif
}

bool QWebSocketPrivate::isValid() const
{
    return (m_socketContext > 0 && m_socketState == QAbstractSocket::ConnectedState);
}

void QWebSocketPrivate::setSocketClosed(const EmscriptenWebSocketCloseEvent *emCloseEvent)
{
    Q_Q(QWebSocket);
    m_closeCode = (QWebSocketProtocol::CloseCode)emCloseEvent->code;

    m_closeReason = QString::fromUtf8(emCloseEvent->reason);

    if (m_closeReason.isEmpty()) {
        m_closeReason = closeCodeToString(m_closeCode);
    }

    if (m_socketState == QAbstractSocket::ConnectedState) {
        Q_EMIT q->aboutToClose();
        setSocketState(QAbstractSocket::ClosingState);
    }

    if (!emCloseEvent->wasClean) {
        m_errorString = QStringLiteral("The remote host closed the connection");
        emit q->errorOccurred(error());
    }

    emscripten_websocket_get_ready_state(m_socketContext, &m_readyState);

    if (m_readyState == 3) { // closed
        setSocketState(QAbstractSocket::UnconnectedState);
        emit q->disconnected();
        emscripten_websocket_delete(emCloseEvent->socket);
        m_socketContext = 0;
    }
}

QString QWebSocketPrivate::closeCodeToString(QWebSocketProtocol::CloseCode code)
{
    switch (code) {
        case QWebSocketProtocol::CloseCodeNormal: return QStringLiteral("Normal closure");
        case QWebSocketProtocol::CloseCodeGoingAway: return QStringLiteral("Going away");
        case QWebSocketProtocol::CloseCodeProtocolError: return QStringLiteral("Protocol error");
        case QWebSocketProtocol::CloseCodeDatatypeNotSupported: return QStringLiteral("Unsupported data");
        case QWebSocketProtocol::CloseCodeReserved1004: return QStringLiteral("Reserved");
        case QWebSocketProtocol::CloseCodeMissingStatusCode: return QStringLiteral("No status received");
        case QWebSocketProtocol::CloseCodeAbnormalDisconnection: return QStringLiteral("Abnormal closure");
        case QWebSocketProtocol::CloseCodeWrongDatatype: return QStringLiteral("Invalid frame payload data");
        case QWebSocketProtocol::CloseCodePolicyViolated: return QStringLiteral("Policy violation");
        case QWebSocketProtocol::CloseCodeTooMuchData: return QStringLiteral("Message too big");
        case QWebSocketProtocol::CloseCodeMissingExtension: return QStringLiteral("Mandatory extension missing");
        case QWebSocketProtocol::CloseCodeBadOperation: return QStringLiteral("Internal server error");
        case QWebSocketProtocol::CloseCodeTlsHandshakeFailed: return QStringLiteral("TLS handshake failed");
    };
    return QStringLiteral("");
}
