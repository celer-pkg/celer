// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qcoapprotocol_p.h"
#include "qcoapinternalrequest_p.h"
#include "qcoapinternalreply_p.h"
#include "qcoaprequest_p.h"
#include "qcoapconnection_p.h"
#include "qcoapnamespace_p.h"

#include <QtCore/qrandom.h>
#include <QtCore/qthread.h>
#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qnetworkdatagram.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcCoapProtocol, "qt.coap.protocol")

/*!
    \internal

    \class QCoapProtocol
    \inmodule QtCoap

    \brief The QCoapProtocol class handles the logical part of the CoAP
    protocol.

    \reentrant

    The QCoapProtocol is used by the QCoapClient class to handle the logical
    part of the protocol. It can encode requests and decode replies. It also
    handles what to do when a message is received, along with retransmission of
    lost messages.

    \sa QCoapClient
*/

/*!
    \internal

    \fn void QCoapProtocol::finished(QCoapReply *reply)

    This signal is emitted along with the \l QCoapReply::finished() signal
    whenever a CoAP reply is received, after either a success or an error.
    The \a reply parameter will contain a pointer to the reply that has just
    been received.

    \sa error(), QCoapReply::finished(), QCoapReply::error()
*/

/*!
    \internal

    \fn void QCoapProtocol::responseToMulticastReceived(QCoapReply *reply,
                                                        const QCoapMessage& message,
                                                        const QHostAddress &sender)

    This signal is emitted when a unicast response to a multicast request
    arrives. The \a reply parameter contains a pointer to the reply that has just
    been received, \a message contains the payload and the message details,
    and \a sender contains the sender address.

    \sa error(), QCoapReply::finished(), QCoapReply::error()
*/

/*!
    \internal

    \fn void QCoapProtocol::error(QCoapReply *reply, QtCoap::Error error)

    This signal is emitted whenever an error occurs. The \a reply parameter
    can be \nullptr if the error is not related to a specific QCoapReply. The
    \a error parameter contains the error code.

    \sa finished(), QCoapReply::error(), QCoapReply::finished()
*/

/*!
    \internal

    Constructs a new QCoapProtocol and sets \a parent as the parent object.
*/
QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(*new QCoapProtocolPrivate, parent)
{
    qRegisterMetaType<QCoapInternalRequest *>();
    qRegisterMetaType<QHostAddress>();
}

QCoapProtocol::~QCoapProtocol()
{
    Q_D(QCoapProtocol);

    // Clear table to avoid double deletion from QObject parenting and QSharedPointer.
    d->exchangeMap.clear();
}

/*!
    \internal

    Creates and sets up a new QCoapInternalRequest related to the request
    associated to the \a reply. The request will then be sent to the server
    using the given \a connection.
*/
void QCoapProtocol::sendRequest(QPointer<QCoapReply> reply, QCoapConnection *connection)
{
    Q_D(QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == thread());

    if (reply.isNull() || reply->request().method() == QtCoap::Method::Invalid
            || !QCoapRequestPrivate::isUrlValid(reply->request().url()))
        return;

    connect(reply.data(), &QCoapReply::aborted, this, [this](const QCoapToken &token) {
        Q_D(QCoapProtocol);
        d->onRequestAborted(token);
    });

    auto internalRequest = QSharedPointer<QCoapInternalRequest>::create(reply->request(), this);
    internalRequest->setMaxTransmissionWait(maximumTransmitWait());
    connect(reply.data(), &QCoapReply::finished, this, &QCoapProtocol::finished);

    if (internalRequest->isMulticast()) {
        connect(internalRequest.data(), &QCoapInternalRequest::multicastRequestExpired, this,
                [this](QCoapInternalRequest *request) {
                    Q_D(QCoapProtocol);
                    d->onMulticastRequestExpired(request);
                });
        // The timeout interval is chosen based on
        // https://tools.ietf.org/html/rfc7390#section-2.5
        internalRequest->setMulticastTimeout(nonConfirmLifetime()
                                             + maximumLatency()
                                             + maximumServerResponseDelay());
    }

    // Set a unique Message Id and Token
    QCoapMessage *requestMessage = internalRequest->message();
    internalRequest->setMessageId(d->generateUniqueMessageId());
    if (internalRequest->token().isEmpty())
        internalRequest->setToken(d->generateUniqueToken());
    internalRequest->setConnection(connection);

    d->registerExchange(requestMessage->token(), reply, internalRequest);
    QMetaObject::invokeMethod(reply, "_q_setRunning", Qt::QueuedConnection,
                              Q_ARG(QCoapToken, requestMessage->token()),
                              Q_ARG(QCoapMessageId, requestMessage->messageId()));

    // Set block size for blockwise request/replies, if specified
    if (d->blockSize > 0) {
        internalRequest->setToRequestBlock(0, d->blockSize);
        if (requestMessage->payload().size() > d->blockSize)
            internalRequest->setToSendBlock(0, d->blockSize);
    }

    if (requestMessage->type() == QCoapMessage::Type::Confirmable) {
        const auto minTimeout = minimumTimeout();
        const auto maxTimeout = maximumTimeout();
        Q_ASSERT(minTimeout <= maxTimeout);

        internalRequest->setTimeout(minTimeout == maxTimeout
                                    ? minTimeout
                                    : QtCoap::randomGenerator().bounded(minTimeout, maxTimeout));
    } else {
        internalRequest->setTimeout(maximumTimeout());
    }

    connect(internalRequest.data(), &QCoapInternalRequest::timeout, this,
            [this](QCoapInternalRequest *request) {
                    Q_D(QCoapProtocol);
                    d->onRequestTimeout(request);
            });
    connect(internalRequest.data(), &QCoapInternalRequest::maxTransmissionSpanReached, this,
            [this](QCoapInternalRequest *request) {
                    Q_D(QCoapProtocol);
                    d->onRequestMaxTransmissionSpanReached(request);
            });
    d->sendRequest(internalRequest.data());
}

/*!
    \internal

    Encodes and sends the given \a request to the server. If \a host is not empty,
    sends the request to \a host, instead of using the host address from the request.
    The \a host parameter is relevant for multicast blockwise transfers.
*/
void QCoapProtocolPrivate::sendRequest(QCoapInternalRequest *request, const QString& host) const
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    if (!request || !request->connection()) {
        qCWarning(lcCoapProtocol, "Request null or not bound to any connection: aborted.");
        return;
    }

    if (request->isMulticast())
        request->startMulticastTransmission();
    else
        request->restartTransmission();

    QByteArray requestFrame = request->toQByteArray();
    QUrl uri = request->targetUri();
    const auto& hostAddress = host.isEmpty() ? uri.host() : host;
    request->connection()->d_func()->sendRequest(requestFrame, hostAddress,
                                                 static_cast<quint16>(uri.port()));
}

/*!
    \internal

    This slot is used to send again the given \a request after a timeout or
    aborts the request and transfers a timeout error to the reply.
*/
void QCoapProtocolPrivate::onRequestTimeout(QCoapInternalRequest *request)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    if (!isRequestRegistered(request))
        return;

    if (request->message()->type() == QCoapMessage::Type::Confirmable
            && request->retransmissionCounter() < maximumRetransmitCount) {
        sendRequest(request);
    } else {
        onRequestError(request, QtCoap::Error::TimeOut);
    }
}

/*!
    \internal

    This slot is called when the maximum span for this transmission has been
    reached, and triggers a timeout error if the request is still running.
*/
void QCoapProtocolPrivate::onRequestMaxTransmissionSpanReached(QCoapInternalRequest *request)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    if (isRequestRegistered(request))
        onRequestError(request, QtCoap::Error::TimeOut);
}

/*!
    \internal

    This slot is called when the multicast request expires, meaning that no
    more responses are expected for the multicast \a request. As a result of this
    call, the request token is \e {freed up} and the \l finished() signal is emitted.
*/
void QCoapProtocolPrivate::onMulticastRequestExpired(QCoapInternalRequest *request)
{
    Q_ASSERT(request->isMulticast());

    request->stopTransmission();
    QPointer<QCoapReply> userReply = userReplyForToken(request->token());
    if (userReply) {
        QMetaObject::invokeMethod(userReply, "_q_setFinished", Qt::QueuedConnection,
                                  Q_ARG(QtCoap::Error, QtCoap::Error::Ok));
    } else {
        qCWarning(lcCoapProtocol).nospace() << "Reply for token '" << request->token()
                                            << "' is not registered, reply is null.";
    }
    forgetExchange(request);
}

/*!
    \internal

    Method triggered when a request fails.
*/
void QCoapProtocolPrivate::onRequestError(QCoapInternalRequest *request, QCoapInternalReply *reply)
{
    QtCoap::Error error = QtCoap::errorForResponseCode(reply->responseCode());
    onRequestError(request, error, reply);
}

/*!
    \internal

    Method triggered when a request fails.
*/
void QCoapProtocolPrivate::onRequestError(QCoapInternalRequest *request, QtCoap::Error error,
                                          QCoapInternalReply *reply)
{
    Q_Q(QCoapProtocol);
    Q_ASSERT(request);

    auto userReply = userReplyForToken(request->token());

    if (!userReply.isNull()) {
        // Set error from content, or error enum
        if (reply) {
            QMetaObject::invokeMethod(userReply.data(), "_q_setContent", Qt::QueuedConnection,
                                      Q_ARG(QHostAddress, reply->senderAddress()),
                                      Q_ARG(QCoapMessage, *reply->message()),
                                      Q_ARG(QtCoap::ResponseCode, reply->responseCode()));
        } else {
            QMetaObject::invokeMethod(userReply.data(), "_q_setError", Qt::QueuedConnection,
                                      Q_ARG(QtCoap::Error, error));
        }

        QMetaObject::invokeMethod(userReply.data(), "_q_setFinished", Qt::QueuedConnection,
                                  Q_ARG(QtCoap::Error, QtCoap::Error::Ok));
    }

    forgetExchange(request);
    emit q->error(userReply.data(), error);
}

/*!
    \internal

    Decode and process the given \a data received from the \a sender.
*/
void QCoapProtocolPrivate::onFrameReceived(const QByteArray &data, const QHostAddress &sender)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    QSharedPointer<QCoapInternalReply> reply(decode(data, sender));
    const QCoapMessage *messageReceived = reply->message();

    QCoapInternalRequest *request = nullptr;
    if (!messageReceived->token().isEmpty())
        request = requestForToken(messageReceived->token());

    if (!request) {
        request = findRequestByMessageId(messageReceived->messageId());

        // No matching request found, drop the frame.
        if (!request)
            return;
    }

    QHostAddress originalTarget(request->targetUri().host());
    if (!originalTarget.isMulticast() && !originalTarget.isEqual(sender)) {
        qCDebug(lcCoapProtocol).nospace() << "QtCoap: Answer received from incorrect host ("
                                          << sender << " instead of "
                                          << originalTarget << ")";
        return;
    }

    if (!request->isMulticast())
        request->stopTransmission();
    addReply(request->token(), reply);

    if (QtCoap::isError(reply->responseCode())) {
        onRequestError(request, reply.data());
        return;
    }

    // Reply when the server asks for an ACK
    if (request->isObserveCancelled()) {
        // Remove option to ensure that it will stop
        request->removeOption(QCoapOption::Observe);
        sendReset(request);
    } else if (messageReceived->type() == QCoapMessage::Type::Confirmable) {
        sendAcknowledgment(request);
    }

    // Send next block, ask for next block, or process the final reply
    if (reply->hasMoreBlocksToSend() && reply->nextBlockToSend() >= 0) {
        request->setToSendBlock(static_cast<uint>(reply->nextBlockToSend()), blockSize);
        request->setMessageId(generateUniqueMessageId());
        sendRequest(request);
    } else if (reply->hasMoreBlocksToReceive()) {
        request->setToRequestBlock(reply->currentBlockNumber() + 1, reply->blockSize());
        request->setMessageId(generateUniqueMessageId());
        // In case of multicast blockwise transfers, according to
        // https://tools.ietf.org/html/rfc7959#section-2.8, further blocks should be retrieved
        // via unicast requests. So instead of using the multicast request address, we need
        // to use the sender address for getting the next blocks.
        sendRequest(request, sender.toString());
    } else {
        onLastMessageReceived(request, sender);
    }
}

/*!
    \internal

    Returns the internal request for the given \a token.
*/
QCoapInternalRequest *QCoapProtocolPrivate::requestForToken(const QCoapToken &token) const
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return it->request.data();

    return nullptr;
}

/*!
    \internal

    Returns the QCoapReply instance of the given \a token.
*/
QPointer<QCoapReply> QCoapProtocolPrivate::userReplyForToken(const QCoapToken &token) const
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return it->userReply;

    return nullptr;
}

/*!
    \internal

    Returns the replies for the exchange identified by \a token.
*/
QList<QSharedPointer<QCoapInternalReply>>
QCoapProtocolPrivate::repliesForToken(const QCoapToken &token) const
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return it->replies;

    return {};
}

/*!
    \internal

    Returns the last reply for the exchange identified by \a token.
*/
QCoapInternalReply *QCoapProtocolPrivate::lastReplyForToken(const QCoapToken &token) const
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return it->replies.last().data();

    return nullptr;
}

/*!
    \internal

    Finds an internal request matching the given \a reply.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findRequestByUserReply(const QCoapReply *reply) const
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if (it->userReply == reply)
            return it->request.data();
    }

    return nullptr;
}

/*!
    \internal

    Finds an internal request containing the message id \a messageId.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findRequestByMessageId(quint16 messageId) const
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if (it->request->message()->messageId() == messageId)
            return it->request.data();
    }

    return nullptr;
}

/*!
    \internal

    Handles what to do when we received the last block of a reply.

    Merges all blocks, removes the request from the map, updates the
    associated QCoapReply and emits the
    \l{QCoapProtocol::finished(QCoapReply*)}{finished(QCoapReply*)} signal.
*/
void QCoapProtocolPrivate::onLastMessageReceived(QCoapInternalRequest *request,
                                                 const QHostAddress &sender)
{
    Q_ASSERT(request);
    if (!request || !isRequestRegistered(request))
        return;

    auto replies = repliesForToken(request->token());
    Q_ASSERT(!replies.isEmpty());

    //! TODO: Change QPointer<QCoapReply> into something independent from
    //! User. QSharedPointer(s)?
    QPointer<QCoapReply> userReply = userReplyForToken(request->token());
    if (userReply.isNull() || replies.isEmpty()
            || (request->isObserve() && request->isObserveCancelled())) {
        forgetExchange(request);
        return;
    }

    auto lastReply = replies.last();
    // Ignore empty ACK messages
    if (lastReply->message()->type() == QCoapMessage::Type::Acknowledgment
            && lastReply->responseCode() == QtCoap::ResponseCode::EmptyMessage) {
        exchangeMap[request->token()].replies.takeLast();
        return;
    }

    // Merge payloads for blockwise transfers
    if (replies.size() > 1) {

        // In multicast case, multiple hosts will reply to the same multicast request.
        // We are interested only in replies coming from the sender.
        if (request->isMulticast()) {
            replies.erase(std::remove_if(replies.begin(), replies.end(),
                                         [sender](QSharedPointer<QCoapInternalReply> reply) {
                                            return reply->senderAddress() != sender;
                                         }), replies.end());
        }

        std::stable_sort(std::begin(replies), std::end(replies),
        [](QSharedPointer<QCoapInternalReply> a, QSharedPointer<QCoapInternalReply> b) -> bool {
            return (a->currentBlockNumber() < b->currentBlockNumber());
        });

        QByteArray finalPayload;
        int lastBlockNumber = -1;
        for (auto reply : std::as_const(replies)) {
            int currentBlock = static_cast<int>(reply->currentBlockNumber());
            QByteArray replyPayload = reply->message()->payload();
            if (replyPayload.isEmpty() || currentBlock <= lastBlockNumber)
                continue;

            finalPayload.append(replyPayload);
            lastBlockNumber = currentBlock;
        }

        lastReply->message()->setPayload(finalPayload);
    }

    // Forward the answer
    QMetaObject::invokeMethod(userReply, "_q_setContent", Qt::QueuedConnection,
                              Q_ARG(QHostAddress, lastReply->senderAddress()),
                              Q_ARG(QCoapMessage, *lastReply->message()),
                              Q_ARG(QtCoap::ResponseCode, lastReply->responseCode()));

    if (request->isObserve()) {
        QMetaObject::invokeMethod(userReply, "_q_setNotified", Qt::QueuedConnection);
        forgetExchangeReplies(request->token());
    } else if (request->isMulticast()) {
        Q_Q(QCoapProtocol);
        emit q->responseToMulticastReceived(userReply, *lastReply->message(), sender);
    } else {
        QMetaObject::invokeMethod(userReply, "_q_setFinished", Qt::QueuedConnection,
                                  Q_ARG(QtCoap::Error, QtCoap::Error::Ok));
        forgetExchange(request);
    }
}

/*!
    \internal

    Sends an internal request acknowledging the given \a request, reusing its
    URI and connection.
*/
void QCoapProtocolPrivate::sendAcknowledgment(QCoapInternalRequest *request) const
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    QCoapInternalRequest ackRequest;
    ackRequest.setTargetUri(request->targetUri());

    auto internalReply = lastReplyForToken(request->token());
    ackRequest.initEmptyMessage(internalReply->message()->messageId(),
                                QCoapMessage::Type::Acknowledgment);
    ackRequest.setConnection(request->connection());
    sendRequest(&ackRequest);
}

/*!
    \internal

    Sends a Reset message (RST), reusing the details of the given
    \a request. A Reset message indicates that a specific message has been
    received, but cannot be properly processed.
*/
void QCoapProtocolPrivate::sendReset(QCoapInternalRequest *request) const
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    QCoapInternalRequest resetRequest;
    resetRequest.setTargetUri(request->targetUri());

    auto lastReply = lastReplyForToken(request->token());
    resetRequest.initEmptyMessage(lastReply->message()->messageId(), QCoapMessage::Type::Reset);
    resetRequest.setConnection(request->connection());
    sendRequest(&resetRequest);
}

/*!
    \internal

    Cancels resource observation. The QCoapReply::notified() signal will not
    be emitted after cancellation.

    A Reset (RST) message will be sent at the reception of the next message.
*/
void QCoapProtocol::cancelObserve(QPointer<QCoapReply> reply) const
{
    Q_D(const QCoapProtocol);

    if (reply.isNull())
        return;

    QCoapInternalRequest *request = d->requestForToken(reply->request().token());
    if (request) {
        // Stop here if already cancelled
        if (!request->isObserve() || request->isObserveCancelled())
            return;

        request->setObserveCancelled();
    }

    // Set as cancelled even if request is not tracked anymore
    QMetaObject::invokeMethod(reply, "_q_setObserveCancelled", Qt::QueuedConnection);
}

/*!
    \internal

    Cancels resource observation for the given \a url. The QCoapReply::notified()
    signal will not be emitted after cancellation.

    A Reset (RST) message will be sent at the reception of the next message.
*/
void QCoapProtocol::cancelObserve(const QUrl &url) const
{
    Q_D(const QCoapProtocol);

    for (const auto &exchange : d->exchangeMap) {
        Q_ASSERT(exchange.userReply);
        if (exchange.userReply->url() == url)
            cancelObserve(exchange.userReply);
    }
}

/*!
    \internal

    Returns a currently unused message Id.
*/
quint16 QCoapProtocolPrivate::generateUniqueMessageId() const
{
    // TODO: Optimize message id generation for large sets
    // TODO: Store used message id for the period specified by CoAP spec
    quint16 id = 0;
    while (isMessageIdRegistered(id))
        id = static_cast<quint16>(QtCoap::randomGenerator().bounded(0x10000));

    return id;
}

/*!
    \internal

    Returns a currently unused token.
*/
QCoapToken QCoapProtocolPrivate::generateUniqueToken() const
{
    // TODO: Optimize token generation for large sets
    // TODO: Store used token for the period specified by CoAP spec
    QCoapToken token;
    while (isTokenRegistered(token)) {
        quint8 length = static_cast<quint8>(QtCoap::randomGenerator().bounded(minimumTokenSize, 9));
        token.resize(length);
        quint8 *tokenData = reinterpret_cast<quint8 *>(token.data());
        for (int i = 0; i < token.size(); ++i)
            tokenData[i] = static_cast<quint8>(QtCoap::randomGenerator().bounded(256));
    }

    return token;
}

/*!
    \internal

    Returns a new unmanaged QCoapInternalReply based on \a data and \a sender.
*/
QCoapInternalReply *QCoapProtocolPrivate::decode(const QByteArray &data, const QHostAddress &sender)
{
    Q_Q(QCoapProtocol);
    QCoapInternalReply *reply = QCoapInternalReply::createFromFrame(data, q);
    reply->setSenderAddress(sender);

    return reply;
}

/*!
    \internal

    Aborts the request corresponding to the given \a reply. It is triggered
    by the destruction of the QCoapReply object or a call to
    QCoapReply::abortRequest().
*/
void QCoapProtocolPrivate::onRequestAborted(const QCoapToken &token)
{
    QCoapInternalRequest *request = requestForToken(token);
    if (!request)
        return;

    request->stopTransmission();
    forgetExchange(request);
}

/*!
    \internal

    Triggered in case of a connection error.
*/
void QCoapProtocolPrivate::onConnectionError(QAbstractSocket::SocketError socketError)
{
    Q_Q(QCoapProtocol);

    QtCoap::Error coapError;
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError :
        coapError = QtCoap::Error::HostNotFound;
        break;
    case QAbstractSocket::AddressInUseError :
        coapError = QtCoap::Error::AddressInUse;
        break;
    default:
        coapError = QtCoap::Error::Unknown;
        break;
    }

    emit q->error(nullptr, coapError);
}

/*!
    \internal

    Registers a new CoAP exchange using \a token.
*/
void QCoapProtocolPrivate::registerExchange(const QCoapToken &token, QCoapReply *reply,
                                            QSharedPointer<QCoapInternalRequest> request)
{
    CoapExchangeData data = { reply, request,
                              QList<QSharedPointer<QCoapInternalReply> >()
                            };

    exchangeMap.insert(token, data);
}

/*!
    \internal

    Adds \a reply to the list of replies of the exchange identified by
    \a token.
    Returns \c true if the reply was successfully added. This method will fail
    and return \c false if no exchange is associated with the \a token
    provided.
*/
bool QCoapProtocolPrivate::addReply(const QCoapToken &token,
                                    QSharedPointer<QCoapInternalReply> reply)
{
    if (!isTokenRegistered(token) || !reply) {
        qCWarning(lcCoapProtocol).nospace() << "Reply token '" << token
                                            << "' not registered, or reply is null.";
        return false;
    }

    exchangeMap[token].replies.push_back(reply);
    return true;
}

/*!
    \internal

    Remove the exchange identified by its \a token. This is
    typically done when finished or aborted.
    It will delete the QCoapInternalRequest and QCoapInternalReplies
    associated with the exchange.

    Returns \c true if the exchange was found and removed, \c false otherwise.
*/
bool QCoapProtocolPrivate::forgetExchange(const QCoapToken &token)
{
    return exchangeMap.remove(token) > 0;
}

/*!
    \internal

    Remove the exchange using a request.

    \sa forgetExchange(const QCoapToken &)
*/
bool QCoapProtocolPrivate::forgetExchange(const QCoapInternalRequest *request)
{
    return forgetExchange(request->token());
}

/*!
    \internal

    Remove all replies for the exchange corresponding to \a token.
*/
bool QCoapProtocolPrivate::forgetExchangeReplies(const QCoapToken &token)
{
    auto it = exchangeMap.find(token);
    if (it == exchangeMap.end())
        return false;

    it->replies.clear();
    return true;
}

/*!
    \internal

    Returns \c true if the \a token is reserved or in use; returns \c false if
    this token can be used to identify a new exchange.
*/
bool QCoapProtocolPrivate::isTokenRegistered(const QCoapToken &token) const
{
    // Reserved for empty messages and uninitialized tokens
    if (token == QByteArray())
        return true;

    return exchangeMap.contains(token);
}

/*!
    \internal

    Returns \c true if the \a request is present in a currently registered
    exchange.
*/
bool QCoapProtocolPrivate::isRequestRegistered(const QCoapInternalRequest *request) const
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if (it->request.data() == request)
            return true;
    }

    return false;
}

/*!
    \internal

    Returns \c true if a request has a message id equal to \a id, or if \a id
    is reserved.
*/
bool QCoapProtocolPrivate::isMessageIdRegistered(quint16 id) const
{
    // Reserved for uninitialized message Id
    if (id == 0)
        return true;

    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if (it->request->message()->messageId() == id)
            return true;
    }

    return false;
}

/*!
    \internal

    Returns the ACK_TIMEOUT value in milliseconds.
    The default is 2000.

    \sa minimumTimeout(), setAckTimeout()
*/
uint QCoapProtocol::ackTimeout() const
{
    Q_D(const QCoapProtocol);
    return d->ackTimeout;
}

/*!
    \internal

    Returns the ACK_RANDOM_FACTOR value.
    The default is 1.5.

    \sa setAckRandomFactor()
*/
double QCoapProtocol::ackRandomFactor() const
{
    Q_D(const QCoapProtocol);
    return d->ackRandomFactor;
}

/*!
    \internal

    Returns the MAX_RETRANSMIT value. This is the maximum number of
    retransmissions of a message, before notifying a timeout error.
    The default is 4.

    \sa setMaximumRetransmitCount()
*/
uint QCoapProtocol::maximumRetransmitCount() const
{
    Q_D(const QCoapProtocol);
    return d->maximumRetransmitCount;
}

/*!
    \internal

    Returns the maximum block size wanted.
    The default is 0, which invites the server to choose the block size.

    \sa setBlockSize()
*/
quint16 QCoapProtocol::blockSize() const
{
    Q_D(const QCoapProtocol);
    return d->blockSize;
}

/*!
    \internal

    Returns the MAX_TRANSMIT_SPAN in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}.

    It is the maximum time from the first transmission of a Confirmable
    message to its last retransmission.
*/
uint QCoapProtocol::maximumTransmitSpan() const
{
    return static_cast<uint>(ackTimeout()
                             * ((1u << maximumRetransmitCount()) - 1)
                             * ackRandomFactor());
}

/*!
    \internal

    Returns the MAX_TRANSMIT_WAIT in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}.

    It is the maximum time from the first transmission of a Confirmable
    message to the time when the sender gives up on receiving an
    acknowledgment or reset.
*/
uint QCoapProtocol::maximumTransmitWait() const
{
    return static_cast<uint>(ackTimeout() * ((1u << (maximumRetransmitCount() + 1)) - 1)
                             * ackRandomFactor());
}

/*!
    \internal

    Returns the MAX_LATENCY in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}. This
    value is arbitrarily set to 100 seconds by the standard.

    It is the maximum time a datagram is expected to take from the start of
    its transmission to the completion of its reception.
*/
uint QCoapProtocol::maximumLatency() const
{
    return 100 * 1000;
}

/*!
    \internal

    Returns the minimum duration for messages timeout. The timeout is defined
    as a random value between minimumTimeout() and maximumTimeout(). This is a
    convenience method identical to ackTimeout().

    \sa ackTimeout(), setAckTimeout()
*/
uint QCoapProtocol::minimumTimeout() const
{
    Q_D(const QCoapProtocol);
    return d->ackTimeout;
}

/*!
    \internal

    Returns the maximum duration for messages timeout in milliseconds.

    \sa maximumTimeout(), setAckTimeout(), setAckRandomFactor()
*/
uint QCoapProtocol::maximumTimeout() const
{
    Q_D(const QCoapProtocol);
    return static_cast<uint>(d->ackTimeout * d->ackRandomFactor);
}

/*!
    \internal

    Returns the \c NON_LIFETIME in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}.

    It is the time from sending a non-confirmable message to the time its
    message ID can be safely reused.
*/
uint QCoapProtocol::nonConfirmLifetime() const
{
    return maximumTransmitSpan() + maximumLatency();
}

/*!
    \internal

    Returns the \c MAX_SERVER_RESPONSE_DELAY in milliseconds, as defined in
    \l {RFC 7390 - Section 2.5}.

    It is the expected maximum response delay over all servers that the client
    can send a multicast request to.

    \sa setMaximumServerResponseDelay()
*/
uint QCoapProtocol::maximumServerResponseDelay() const
{
    Q_D(const QCoapProtocol);
    return d->maximumServerResponseDelay;
}

/*!
    \internal

    Sets the ACK_TIMEOUT value to \a ackTimeout in milliseconds.
    The default is 2000 ms.

    Timeout only applies to Confirmable message. The actual timeout for
    reliable transmissions is a random value between ackTimeout() and
    ackTimeout() * ackRandomFactor().

    \sa ackTimeout(), setAckRandomFactor(), minimumTimeout(), maximumTimeout()
*/
void QCoapProtocol::setAckTimeout(uint ackTimeout)
{
    Q_D(QCoapProtocol);
    d->ackTimeout = ackTimeout;
}

/*!
    \internal

    Sets the ACK_RANDOM_FACTOR value to \a ackRandomFactor. This value
    should be greater than or equal to 1.
    The default is 1.5.

    \sa ackRandomFactor(), setAckTimeout()
*/
void QCoapProtocol::setAckRandomFactor(double ackRandomFactor)
{
    Q_D(QCoapProtocol);
    if (ackRandomFactor < 1)
        qCWarning(lcCoapProtocol, "The acknowledgment random factor should be >= 1");

    d->ackRandomFactor = qMax(1., ackRandomFactor);
}

/*!
    \internal

    Sets the MAX_RETRANSMIT value to \a maximumRetransmitCount, but never
    to more than 25.
    The default is 4.

    \sa maximumRetransmitCount()
*/
void QCoapProtocol::setMaximumRetransmitCount(uint maximumRetransmitCount)
{
    Q_D(QCoapProtocol);

    if (maximumRetransmitCount > 25) {
        qCWarning(lcCoapProtocol, "Maximum retransmit count is capped at 25.");
        maximumRetransmitCount = 25;
    }

    d->maximumRetransmitCount = maximumRetransmitCount;
}

/*!
    \internal

    Sets the maximum block size wanted to \a blockSize.

    The \a blockSize should be zero, or range from 16 to 1024 and be a
    power of 2. A size of 0 invites the server to choose the block size.

    \sa blockSize()
*/
void QCoapProtocol::setBlockSize(quint16 blockSize)
{
    Q_D(QCoapProtocol);

    if ((blockSize & (blockSize - 1)) != 0) {
        qCWarning(lcCoapProtocol, "Block size should be a power of 2");
        return;
    }

    if (blockSize != 0 && (blockSize < 16 || blockSize > 1024)) {
        qCWarning(lcCoapProtocol, "Block size should be set to zero,"
                                  "or to a power of 2 from 16 through 1024");
        return;
    }

    d->blockSize = blockSize;
}

/*!
    \internal

    Sets the \c MAX_SERVER_RESPONSE_DELAY value to \a responseDelay in milliseconds.
    The default is 250 seconds.

    As defined in \l {RFC 7390 - Section 2.5}, \c MAX_SERVER_RESPONSE_DELAY is the expected
    maximum response delay over all servers that the client can send a multicast request to.

    \sa maximumServerResponseDelay()
*/
void QCoapProtocol::setMaximumServerResponseDelay(uint responseDelay)
{
    Q_D(QCoapProtocol);
    d->maximumServerResponseDelay = responseDelay;
}

/*!
    \internal

    Sets the minimum token size to \a tokenSize in bytes. For security reasons it is
    recommended to use tokens with a length of at least 4 bytes. The default value for
    this parameter is 4 bytes.
*/
void QCoapProtocol::setMinimumTokenSize(int tokenSize)
{
    Q_D(QCoapProtocol);

    if (tokenSize > 0 && tokenSize <= 8) {
        d->minimumTokenSize = tokenSize;
    } else {
        qCWarning(lcCoapProtocol,
                  "Failed to set the minimum token size,"
                  "it should not be more than 8 bytes and cannot be 0.");
    }
}

QT_END_NAMESPACE
