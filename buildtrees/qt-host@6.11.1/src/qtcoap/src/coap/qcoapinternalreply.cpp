// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qcoapinternalreply_p.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \internal

    \class QCoapInternalReply
    \brief The QCoapInternalReply class contains data related to
    a received message.

    It is a subclass of QCoapInternalMessage.

    \reentrant

    \sa QCoapInternalMessage, QCoapInternalRequest
*/

/*!
    \internal
    Constructs a new QCoapInternalReply with \a parent as the parent object.
*/
QCoapInternalReply::QCoapInternalReply(QObject *parent) :
    QCoapInternalMessage(*new QCoapInternalReplyPrivate, parent)
{
}

/*!
    \internal
    Creates a QCoapInternalReply from the CoAP \a reply frame.

    For more details, refer to section
    \l{https://tools.ietf.org/html/rfc7252#section-3}{'Message format' of RFC 7252}.
*/
//!  0                   1                   2                   3
//!  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |Ver| T |  TKL  |      Code     |          Message ID           |
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Token (if any, TKL bytes) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Options (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |1 1 1 1 1 1 1 1|    Payload (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
QCoapInternalReply *QCoapInternalReply::createFromFrame(const QByteArray &reply, QObject *parent)
{
    QCoapInternalReply *internalReply = new QCoapInternalReply(parent);
    QCoapInternalReplyPrivate *d = internalReply->d_func();

    const quint8 *pduData = reinterpret_cast<const quint8 *>(reply.data());

    // Parse Header and Token
    d->message.setVersion((pduData[0] >> 6) & 0x03);
    d->message.setType(QCoapMessage::Type((pduData[0] >> 4) & 0x03));
    quint8 tokenLength = (pduData[0]) & 0x0F;
    d->responseCode = static_cast<QtCoap::ResponseCode>(pduData[1]);
    d->message.setMessageId(static_cast<quint16>((static_cast<quint16>(pduData[2]) << 8)
                                                 | static_cast<quint16>(pduData[3])));
    d->message.setToken(reply.mid(4, tokenLength));

    // Parse Options
    int i = 4 + tokenLength;
    quint16 lastOptionNumber = 0;
    while (i != reply.size() && pduData[i] != 0xFF) {
        quint16 optionDelta = ((pduData[i] >> 4) & 0x0F);
        quint16 optionLength = (pduData[i] & 0x0F);

        // Delta value > 12 : special values
        if (optionDelta == 13) {
            ++i;
            optionDelta = pduData[i] + 13;
        } else if (optionDelta == 14) {
            ++i;
            optionDelta = pduData[i] + 269;
        }

        // Delta length > 12 : special values
        if (optionLength == 13) {
            ++i;
            optionLength = pduData[i] + 13;
        } else if (optionLength == 14) {
            ++i;
            optionLength = pduData[i] + 269;
        }

        quint16 optionNumber = lastOptionNumber + optionDelta;
        internalReply->addOption(QCoapOption::OptionName(optionNumber),
                                 reply.mid(i + 1, optionLength));
        lastOptionNumber = optionNumber;
        i += 1 + optionLength;
    }

    // Parse Payload
    if (i < reply.size() && pduData[i] == 0xFF) {
        // -1 because of 0xFF at the beginning
        QByteArray currentPayload = reply.mid(i + 1);
        d->message.setPayload(d->message.payload().append(currentPayload));
    }

    return internalReply;
}

/*!
    \internal
    Appends the given \a data byte array to the current payload.
*/
void QCoapInternalReply::appendData(const QByteArray &data)
{
    Q_D(QCoapInternalReply);
    d->message.setPayload(d->message.payload().append(data));
}

/*!
    \internal
    Adds the given CoAP \a option and sets block parameters if needed.
*/
void QCoapInternalReply::addOption(const QCoapOption &option)
{
    if (option.name() == QCoapOption::Block2)
        setFromDescriptiveBlockOption(option);

    QCoapInternalMessage::addOption(option);
}

/*!
    \internal
    Sets the sender address.
*/
void QCoapInternalReply::setSenderAddress(const QHostAddress &address)
{
    Q_D(QCoapInternalReply);
    d->senderAddress = address;
}

/*!
    \internal
    Returns the number of the next block, if there is another block to come,
    otherwise -1.
    For more details, refer to the
    \l{https://tools.ietf.org/html/rfc7959#section-2.2}{RFC 7959}.
*/
int QCoapInternalReply::nextBlockToSend() const
{
    Q_D(const QCoapInternalReply);

    QCoapOption option = d->message.option(QCoapOption::Block1);
    if (!option.isValid())
        return -1;

    const auto value = option.opaqueValue();
    const quint8 *optionData = reinterpret_cast<const quint8 *>(value.data());
    const quint8 lastByte = optionData[option.length() - 1];

    // M field
    bool hasNextBlock = ((lastByte & 0x8) == 0x8);
    if (!hasNextBlock)
        return -1;

    // NUM field
    quint32 blockNumber = 0;
    for (int i = 0; i < option.length() - 1; ++i)
        blockNumber = (blockNumber << 8) | optionData[i];
    blockNumber = (blockNumber << 4) | (lastByte >> 4);
    return static_cast<int>(blockNumber) + 1;
}

/*!
    \internal
    Returns \c true if the client has one or more blocks to send.
*/
bool QCoapInternalReply::hasMoreBlocksToSend() const
{
    return nextBlockToSend() >= 0;
}

/*!
    \internal
    Returns the response code of the reply.
*/
QtCoap::ResponseCode QCoapInternalReply::responseCode() const
{
    Q_D(const QCoapInternalReply);
    return d->responseCode;
}

/*!
    \internal
    Returns the host address from which the reply was received.
*/
QHostAddress QCoapInternalReply::senderAddress() const
{
    Q_D(const QCoapInternalReply);
    return d->senderAddress;
}

QT_END_NAMESPACE
