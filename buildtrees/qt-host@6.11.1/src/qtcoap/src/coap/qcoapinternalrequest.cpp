// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qcoaprequest.h"
#include "qcoapinternalrequest_p.h"

#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qloggingcategory.h>
#include <QtNetwork/QHostAddress>

QT_BEGIN_NAMESPACE

/*!
    \internal

    \class QCoapInternalRequest
    \brief The QCoapInternalRequest class contains data related to
    a message that needs to be sent.

    \reentrant

    \sa QCoapInternalMessage, QCoapInternalReply
*/

/*!
    \internal
    Constructs a new QCoapInternalRequest object and sets \a parent as
    the parent object.
*/
QCoapInternalRequest::QCoapInternalRequest(QObject *parent) :
    QCoapInternalMessage(*new QCoapInternalRequestPrivate, parent)
{
    Q_D(QCoapInternalRequest);
    d->timeoutTimer = new QTimer(this);
    connect(d->timeoutTimer, &QTimer::timeout, this, [this]() { emit timeout(this); });

    d->maxTransmitWaitTimer = new QTimer(this);
    connect(d->maxTransmitWaitTimer, &QTimer::timeout, this,
            [this]() { emit maxTransmissionSpanReached(this); });

    d->multicastExpireTimer = new QTimer(this);
    connect(d->multicastExpireTimer, &QTimer::timeout, this,
            [this]() { emit multicastRequestExpired(this); });
}

/*!
    \internal
    Constructs a new QCoapInternalRequest object with the information of
    \a request and sets \a parent as the parent object.
*/
QCoapInternalRequest::QCoapInternalRequest(const QCoapRequest &request, QObject *parent) :
    QCoapInternalRequest(parent)
{
    Q_D(QCoapInternalRequest);
    d->message = request;
    d->method = request.method();
    d->fullPayload = request.payload();

    addUriOptions(request.url(), request.proxyUrl());
}

/*!
    \internal
    Returns \c true if the request is considered valid.
*/
bool QCoapInternalRequest::isValid() const
{
    Q_D(const QCoapInternalRequest);
    return isUrlValid(d->targetUri) && d->method != QtCoap::Method::Invalid;
}

/*!
    \internal
    Initialize parameters to transform the QCoapInternalRequest into an
    empty message (RST or ACK) with the message id \a messageId.

    An empty message should contain only the \a messageId.
*/
void QCoapInternalRequest::initEmptyMessage(quint16 messageId, QCoapMessage::Type type)
{
    Q_D(QCoapInternalRequest);

    Q_ASSERT(type == QCoapMessage::Type::Acknowledgment || type == QCoapMessage::Type::Reset);

    setMethod(QtCoap::Method::Invalid);
    d->message.setType(type);
    d->message.setMessageId(messageId);
    d->message.setToken(QByteArray());
    d->message.setPayload(QByteArray());
    d->message.clearOptions();
}

/*!
    \internal
    Explicitly casts \a value to a char and appends it to the \a buffer.
*/
template<typename T>
static void appendByte(QByteArray *buffer, T value) {
    buffer->append(static_cast<char>(value));
}

/*!
    \internal
    Returns the CoAP frame corresponding to the QCoapInternalRequest into
    a QByteArray object.

    For more details, refer to section
    \l{https://tools.ietf.org/html/rfc7252#section-3}{'Message format' of RFC 7252}.
*/
//! 0                   1                   2                   3
//! 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |Ver| T |  TKL  |      Code     |          Message ID           |
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Token (if any, TKL bytes) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Options (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |1 1 1 1 1 1 1 1|    Payload (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
QByteArray QCoapInternalRequest::toQByteArray() const
{
    Q_D(const QCoapInternalRequest);
    QByteArray pdu;

    // Insert header
    appendByte(&pdu, (d->message.version()                   << 6)  // CoAP version
                   | (static_cast<quint8>(d->message.type()) << 4)  // Message type
                   |  d->message.token().size());                 // Token Length
    appendByte(&pdu,  static_cast<quint8>(d->method) & 0xFF);       // Method code
    appendByte(&pdu, (d->message.messageId() >> 8)   & 0xFF);       // Message ID
    appendByte(&pdu,  d->message.messageId()         & 0xFF);

    // Insert Token
    pdu.append(d->message.token());

    // Insert Options
    if (!d->message.options().isEmpty()) {
        const auto options = d->message.options();

        // Options should be sorted in order of their option numbers
        Q_ASSERT(std::is_sorted(d->message.options().cbegin(), d->message.options().cend(),
                                [](const QCoapOption &a, const QCoapOption &b) -> bool {
                                    return a.name() < b.name();
                 }));

        quint8 lastOptionNumber = 0;
        for (const QCoapOption &option : std::as_const(options)) {

            quint16 optionDelta = static_cast<quint16>(option.name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            // Delta value > 12 : special values
            if (optionDelta > 268) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 269);
                optionDelta = 14;
                isOptionDeltaExtended = true;
            } else if (optionDelta > 12) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 13);
                optionDelta = 13;
                isOptionDeltaExtended = true;
            }

            quint16 optionLength = static_cast<quint16>(option.length());
            bool isOptionLengthExtended = false;
            quint8 optionLengthExtended = 0;

            // Length > 12 : special values
            if (optionLength > 268) {
                optionLengthExtended = static_cast<quint8>(optionLength - 269);
                optionLength = 14;
                isOptionLengthExtended = true;
            } else if (optionLength > 12) {
                optionLengthExtended = static_cast<quint8>(optionLength - 13);
                optionLength = 13;
                isOptionLengthExtended = true;
            }

            appendByte(&pdu, (optionDelta << 4) | (optionLength & 0x0F));

            if (isOptionDeltaExtended)
                appendByte(&pdu, optionDeltaExtended);
            if (isOptionLengthExtended)
                appendByte(&pdu, optionLengthExtended);

            pdu.append(option.opaqueValue());

            lastOptionNumber = option.name();
        }
    }

    // Insert Payload
    if (!d->message.payload().isEmpty()) {
        appendByte(&pdu, 0xFF);
        pdu.append(d->message.payload());
    }

    return pdu;
}

/*!
    \internal
    Initializes block parameters and creates the options needed to request the
    block \a blockNumber with a size of \a blockSize.

    \sa blockOption(), setToSendBlock()
*/
void QCoapInternalRequest::setToRequestBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    if (!checkBlockNumber(blockNumber))
        return;

    d->message.removeOption(QCoapOption::Block1);
    d->message.removeOption(QCoapOption::Block2);

    addOption(blockOption(QCoapOption::Block2, blockNumber, blockSize));
}

/*!
    \internal
    Initialize blocks parameters and creates the options needed to send the block with
    the number \a blockNumber and with a size of \a blockSize.

    \sa blockOption(), setToRequestBlock()
*/
void QCoapInternalRequest::setToSendBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    if (!checkBlockNumber(blockNumber))
        return;

    d->message.setPayload(d->fullPayload.mid(static_cast<int>(blockNumber * blockSize),
                                             static_cast<int>(blockSize)));
    d->message.removeOption(QCoapOption::Block1);

    addOption(blockOption(QCoapOption::Block1, blockNumber, blockSize));
}

/*!
    \internal
    Returns \c true if the block number is valid, \c false otherwise.
    If the block number is not valid, logs a warning message.
*/
bool QCoapInternalRequest::checkBlockNumber(uint blockNumber)
{
    if (blockNumber >> 20) {
        qCWarning(lcCoapExchange) << "Block number" << blockNumber
                                  << "is too large. It should fit in 20 bits.";
        return false;
    }

    return true;
}

/*!
    \internal
    Builds and returns a Block option.

    The \a blockSize should range from 16 to 1024 and be a power of 2,
    computed as 2^(SZX + 4), with SZX ranging from 0 to 6. For more details,
    refer to the \l{https://tools.ietf.org/html/rfc7959#section-2.2}{RFC 7959}.
*/
QCoapOption QCoapInternalRequest::blockOption(QCoapOption::OptionName name, uint blockNumber, uint blockSize) const
{
    Q_D(const QCoapInternalRequest);

    Q_ASSERT((blockSize & (blockSize - 1)) == 0); // is a power of two
    Q_ASSERT(!(blockSize >> 11)); // blockSize <= 1024

    // NUM field: the relative number of the block within a sequence of blocks
    // 4, 12 or 20 bits (as little as possible)
    Q_ASSERT(!(blockNumber >> 20)); // Fits in 20 bits
    quint32 optionData = (blockNumber << 4);

    // SZX field: the size of the block
    // 3 bits, set to "log2(blockSize) - 4"
    optionData |= (blockSize >> 7)
                  ? ((blockSize >> 10) ? 6 : (3 + (blockSize >> 8)))
                  : (blockSize >> 5);

    // M field: whether more blocks are following
    // 1 bit
    if (name == QCoapOption::Block1
            && static_cast<int>((blockNumber + 1) * blockSize) < d->fullPayload.size()) {
        optionData |= 8;
    }

    QByteArray optionValue;
    Q_ASSERT(!(optionData >> 24));
    if (optionData > 0xFFFF)
        appendByte(&optionValue, optionData >> 16);
    if (optionData > 0xFF)
        appendByte(&optionValue, (optionData >> 8) & 0xFF);
    appendByte(&optionValue, optionData & 0xFF);

    return QCoapOption(name, optionValue);
}

/*!
    \internal
    Sets the request's message id.
*/
void QCoapInternalRequest::setMessageId(quint16 id)
{
    Q_D(QCoapInternalRequest);
    d->message.setMessageId(id);
}

/*!
    \internal
    Sets the request's token.
*/
void QCoapInternalRequest::setToken(const QCoapToken &token)
{
    Q_D(QCoapInternalRequest);
    d->message.setToken(token);
}

/*!
    \internal
    Adds the given CoAP \a option and sets block parameters if needed.
*/
void QCoapInternalRequest::addOption(const QCoapOption &option)
{
    if (option.name() == QCoapOption::Block1)
        setFromDescriptiveBlockOption(option);

    QCoapInternalMessage::addOption(option);
}

/*!
    \internal
    Adds the CoAP options related to the target and proxy with the given \a uri
    and \a proxyUri. Returns \c true upon success, \c false if an error
    occurred.

    Numbers refer to step numbers from CoAP
    \l{RFC 7252}{https://tools.ietf.org/html/rfc7252#section-6.4}.
*/
bool QCoapInternalRequest::addUriOptions(QUrl uri, const QUrl &proxyUri)
{
    Q_D(QCoapInternalRequest);
    // Set to an invalid state
    d->targetUri = QUrl();

    // When using a proxy uri, we SHOULD NOT include Uri-Host/Port/Path/Query
    // options.
    if (!proxyUri.isEmpty()) {
        if (!isUrlValid(proxyUri))
            return false;

        addOption(QCoapOption(QCoapOption::ProxyUri, proxyUri.toString()));
        d->targetUri = proxyUri;
        return true;
    }

    uri = uri.adjusted(QUrl::NormalizePathSegments);

    // 1/3/4. Fails if URL is relative, has no 'coap' scheme or has a fragment
    if (!isUrlValid(uri))
        return false;

    // 2. Ensure encoding matches CoAP standard (i.e. uri is in ASCII encoding)
    const auto uriStr = uri.toString();
    bool isAscii = std::all_of(uriStr.cbegin(), uriStr.cend(),
                               [](const QChar &ch) {
                                   return (ch.unicode() < 128);
                               });
    if (!isAscii)
        return false;

    // 5. Add Uri-Host option if not a plain IP
    QCoapOption uriHost = uriHostOption(uri);
    if (uriHost.isValid())
        addOption(uriHost);

    // 6. Port should be set at this point
    Q_ASSERT(uri.port() != -1);

    // 7. Add port to options if it is not the default port
    if (uri.port() != QtCoap::DefaultPort && uri.port() != QtCoap::DefaultSecurePort)
        addOption(QCoapOption::UriPort, static_cast<quint32>(uri.port()));

    // 8. Add path segments to options
    const auto path = uri.path();
    const auto listPath = QStringView{path}.split(QLatin1Char('/'));
    for (auto pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption(QCoapOption::UriPath, pathPart.toString()));
    }

    // 9. Add queries to options
    QString query = uri.query();
    const auto listQuery = QStringView{query}.split(QLatin1Char('&'));
    for (auto queryElement : listQuery) {
        if (!queryElement.isEmpty())
            addOption(QCoapOption(QCoapOption::UriQuery, queryElement.toString()));
    }

    d->targetUri = uri;
    return true;
}

/*!
    \internal
    Returns the token of the request.
*/
QCoapToken QCoapInternalRequest::token() const
{
    return message()->token();
}

/*!
    \internal
    Used to mark the transmission as "in progress", when starting or retrying
    to transmit a message. This method manages the retransmission counter,
    the transmission timeout and the exchange timeout.
*/
void QCoapInternalRequest::restartTransmission()
{
    Q_D(QCoapInternalRequest);

    if (!d->transmissionInProgress) {
        d->transmissionInProgress = true;
        d->maxTransmitWaitTimer->start();
    } else {
        d->retransmissionCounter++;
        d->timeout *= 2;
    }

    if (d->timeout > 0)
        d->timeoutTimer->start(static_cast<int>(d->timeout));
}

/*!
    \internal

    Starts the timer for keeping the multicast request \e alive.
*/
void QCoapInternalRequest::startMulticastTransmission()
{
    Q_ASSERT(isMulticast());

    Q_D(QCoapInternalRequest);
    d->multicastExpireTimer->start();
}

/*!
    \internal
    Marks the transmission as not running, after a successful reception or an
    error. It resets the retransmission count if needed and stops all timeout timers.
*/
void QCoapInternalRequest::stopTransmission()
{
    Q_D(QCoapInternalRequest);
    if (isMulticast()) {
        d->multicastExpireTimer->stop();
    } else {
        d->transmissionInProgress = false;
        d->retransmissionCounter = 0;
        d->maxTransmitWaitTimer->stop();
        d->timeoutTimer->stop();
    }
}

/*!
    \internal
    Returns the target uri.

    \sa setTargetUri()
*/
QUrl QCoapInternalRequest::targetUri() const
{
    Q_D(const QCoapInternalRequest);
    return d->targetUri;
}

/*!
    \internal
    Returns the connection used to send this request.

    \sa setConnection()
*/
QCoapConnection *QCoapInternalRequest::connection() const
{
    Q_D(const QCoapInternalRequest);
    return d->connection;
}

/*!
    \internal
    Returns the method of the request.

    \sa setMethod()
*/
QtCoap::Method QCoapInternalRequest::method() const
{
    Q_D(const QCoapInternalRequest);
    return d->method;
}

/*!
    \internal
    Returns true if the request is an Observe request.

*/
bool QCoapInternalRequest::isObserve() const
{
    Q_D(const QCoapInternalRequest);
    return d->message.hasOption(QCoapOption::Observe);
}

/*!
    \internal
    Returns true if the observe request needs to be cancelled.

    \sa setCancelObserve()
*/
bool QCoapInternalRequest::isObserveCancelled() const
{
    Q_D(const QCoapInternalRequest);
    return d->observeCancelled;
}

/*!
    \internal

    Returns \c true if the request is multicast, returns \c false otherwise.
*/
bool QCoapInternalRequest::isMulticast() const
{
    const QHostAddress hostAddress(targetUri().host());
    return hostAddress.isMulticast();
}

/*!
    \internal
    Returns the value of the retransmission counter.
*/
uint QCoapInternalRequest::retransmissionCounter() const
{
    Q_D(const QCoapInternalRequest);
    return d->retransmissionCounter;
}

/*!
    \internal
    Sets the method of the request to the given \a method.

    \sa method()
*/
void QCoapInternalRequest::setMethod(QtCoap::Method method)
{
    Q_D(QCoapInternalRequest);
    d->method = method;
}

/*!
    \internal
    Sets the connection to use to send this request to the given \a connection.

    \sa connection()
*/
void QCoapInternalRequest::setConnection(QCoapConnection *connection)
{
    Q_D(QCoapInternalRequest);
    d->connection = connection;
}

/*!
    \internal
    Marks the observe request as cancelled.

    \sa isObserveCancelled()
*/
void QCoapInternalRequest::setObserveCancelled()
{
    Q_D(QCoapInternalRequest);
    d->observeCancelled = true;
}

/*!
    \internal
    Sets the target uri to the given \a targetUri.

    \sa targetUri()
*/
void QCoapInternalRequest::setTargetUri(QUrl targetUri)
{
    Q_D(QCoapInternalRequest);
    d->targetUri = targetUri;
}

/*!
    \internal
    Sets the timeout to the given \a timeout value in milliseconds. Timeout is
    used for reliable transmission of Confirmable messages.

    When such request times out, its timeout value will double.
*/
void QCoapInternalRequest::setTimeout(uint timeout)
{
    Q_D(QCoapInternalRequest);
    d->timeout = timeout;
}

/*!
    \internal
    Sets the maximum transmission span for the request. If the request is
    not finished at the end of the transmission span, the request will timeout.
*/
void QCoapInternalRequest::setMaxTransmissionWait(uint duration)
{
    Q_D(QCoapInternalRequest);
    d->maxTransmitWaitTimer->setInterval(static_cast<int>(duration));
}

/*!
    \internal

    Sets the timeout interval in milliseconds for keeping the multicast request
    \e alive.

    In the unicast case, receiving a response means that the request is finished.
    In the multicast case it is not known how many responses will be received, so
    the response, along with its token, will be kept for
    NON_LIFETIME + MAX_LATENCY + MAX_SERVER_RESPONSE_DELAY time, as suggested
    in \l {RFC 7390 - Section 2.5}.
*/
void QCoapInternalRequest::setMulticastTimeout(uint responseDelay)
{
    Q_D(QCoapInternalRequest);
    d->multicastExpireTimer->setInterval(static_cast<int>(responseDelay));
}

/*!
    \internal
    Decode the \a uri provided and returns a QCoapOption.
*/
QCoapOption QCoapInternalRequest::uriHostOption(const QUrl &uri) const
{
    QHostAddress address(uri.host());

    // No need for Uri-Host option with an IPv4 or IPv6 address
    if (!address.isNull())
        return QCoapOption();

    return QCoapOption(QCoapOption::UriHost, uri.host());
}

QT_END_NAMESPACE
