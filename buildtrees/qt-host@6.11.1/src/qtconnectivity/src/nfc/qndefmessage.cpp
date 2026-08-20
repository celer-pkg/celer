// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qndefmessage.h"
#include "qndefrecord_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QNdefMessage)

/*!
    \class QNdefMessage
    \brief The QNdefMessage class provides an NFC NDEF message.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since Qt 5.2

    A QNdefMessage is a collection of 0 or more QNdefRecords. QNdefMessage inherits from
    QList<QNdefRecord> and therefore the standard QList functions can be used to manipulate the
    NDEF records in the message.

    NDEF messages can be parsed from a byte array conforming to the NFC Data Exchange Format
    technical specification by using the fromByteArray() static function. Conversely QNdefMessages
    can be converted into a byte array with the toByteArray() function.
*/

/*!
    \fn QNdefMessage::QNdefMessage()

    Constructs a new empty NDEF message.
*/

/*!
    \fn QNdefMessage::QNdefMessage(const QNdefRecord &record)

    Constructs a new NDEF message containing a single record \a record.
*/

/*!
    \fn QNdefMessage::QNdefMessage(const QNdefMessage &message)

    Constructs a new NDEF message that is a copy of \a message.
*/

/*!
    \fn QNdefMessage::QNdefMessage(const QList<QNdefRecord> &records)

    Constructs a new NDEF message that contains all of the records in \a records.
*/

/*!
    Returns an NDEF message parsed from the contents of \a message.

    The \a message parameter is interpreted as the raw message format defined in the NFC Data
    Exchange Format technical specification.

    If a parse error occurs an empty NDEF message is returned.
*/
QNdefMessage QNdefMessage::fromByteArray(const QByteArray &message)
{
    QNdefMessage result;

    bool seenMessageBegin = false;
    bool seenMessageEnd = false;

    QByteArray partialChunk;
    QNdefRecord record;

    qsizetype idx = 0;
    while (idx < message.size()) {
        quint8 flags = message.at(idx);

        const bool messageBegin = flags & 0x80;
        const bool messageEnd = flags & 0x40;

        const bool cf = flags & 0x20;
        const bool sr = flags & 0x10;
        const bool il = flags & 0x08;
        const quint8 typeNameFormat = flags & 0x07;

        if (messageBegin && seenMessageBegin) {
            qWarning("Got message begin but already parsed some records");
            return QNdefMessage();
        } else if (!messageBegin && !seenMessageBegin) {
            qWarning("Haven't got message begin yet");
            return QNdefMessage();
        } else if (messageBegin && !seenMessageBegin) {
            seenMessageBegin = true;
        }
        if (messageEnd && seenMessageEnd) {
            qWarning("Got message end but already parsed final record");
            return QNdefMessage();
        } else if (messageEnd && !seenMessageEnd) {
            seenMessageEnd = true;
        }
        // TNF must be 0x06 even for the last chunk, when cf == 0.
        if ((typeNameFormat != 0x06) && !partialChunk.isEmpty()) {
            qWarning("Partial chunk not empty, but TNF not 0x06 as expected");
            return QNdefMessage();
        }

        int headerLength = 1;
        headerLength += (sr) ? 1 : 4;
        headerLength += (il) ? 1 : 0;

        if (idx + headerLength >= message.size()) {
            qWarning("Unexpected end of message");
            return QNdefMessage();
        }

        const quint8 typeLength = message.at(++idx);

        if ((typeNameFormat == 0x06) && (typeLength != 0)) {
            qWarning("Invalid chunked data, TYPE_LENGTH != 0");
            return QNdefMessage();
        }

        quint32 payloadLength;
        if (sr)
            payloadLength = quint8(message.at(++idx));
        else {
            payloadLength = quint8(message.at(++idx)) << 24;
            payloadLength |= quint8(message.at(++idx)) << 16;
            payloadLength |= quint8(message.at(++idx)) << 8;
            payloadLength |= quint8(message.at(++idx)) << 0;
        }

        quint8 idLength;
        if (il)
            idLength = message.at(++idx);
        else
            idLength = 0;

        // On 32-bit systems this can overflow
        const qsizetype convertedPayloadLength = static_cast<qsizetype>(payloadLength);
        const qsizetype contentLength = convertedPayloadLength + typeLength + idLength;

        // On a 32 bit platform the payload can theoretically exceed the max.
        // size of a QByteArray. This will never happen in practice with correct
        // data because there are no NFC tags that can store such data sizes,
        // but still can be possible if the data is corrupted.
        if ((contentLength < 0) || (convertedPayloadLength < 0)
            || ((std::numeric_limits<qsizetype>::max() - idx) < contentLength)) {
            qWarning("Payload can't fit into QByteArray");
            return QNdefMessage();
        }

        if (idx + contentLength >= message.size()) {
            qWarning("Unexpected end of message");
            return QNdefMessage();
        }

        if ((typeNameFormat == 0x06) && il) {
            qWarning("Invalid chunked data, IL != 0");
            return QNdefMessage();
        }

        if (typeNameFormat != 0x06)
            record.setTypeNameFormat(QNdefRecord::TypeNameFormat(typeNameFormat));

        if (typeLength > 0) {
            QByteArray type(&message.constData()[++idx], typeLength);
            record.setType(type);
            idx += typeLength - 1;
        }

        if (idLength > 0) {
            QByteArray id(&message.constData()[++idx], idLength);
            record.setId(id);
            idx += idLength - 1;
        }

        if (payloadLength > 0) {
            QByteArray payload(&message.constData()[++idx], payloadLength);

            if (cf) {
                // chunked payload, except last
                partialChunk.append(payload);
            } else if (typeNameFormat == 0x06) {
                // last chunk of chunked payload
                record.setPayload(partialChunk + payload);
                partialChunk.clear();
            } else {
                // non-chunked payload
                record.setPayload(payload);
            }

            idx += payloadLength - 1;
        }

        if (!cf) {
            result.append(record);
            record = QNdefRecord();
        }

        if (!cf && seenMessageEnd)
            break;

        // move to start of next record
        ++idx;
    }

    if (!seenMessageBegin || !seenMessageEnd) {
        qWarning("Malformed NDEF Message, missing begin or end");
        return QNdefMessage();
    }

    return result;
}

/*!
    \fn QNdefMessage &QNdefMessage::operator=(const QNdefMessage &other)
    \overload
    \since 6.2

    Copy assignment operator from QList<QNdefRecord>. Assigns the
    \a other list of NDEF records to this NDEF record list.

    After the operation, \a other and \c *this will be equal.
*/

/*!
    \fn QNdefMessage &QNdefMessage::operator=(QNdefMessage &&other)
    \overload
    \since 6.2

    Move assignment operator from QList<QNdefRecord>. Moves the
    \a other list of NDEF records to this NDEF record list.

    After the operation, \a other will be empty.
*/

/*!
    Returns \c true if this NDEF message is equivalent to \a other; otherwise
    returns \c false.

    An empty message (i.e. isEmpty() returns \c true) is equivalent to a NDEF
    message containing a single record of type \l QNdefRecord::Empty.
*/
bool QNdefMessage::operator==(const QNdefMessage &other) const
{
    // both records are empty
    if (isEmpty() && other.isEmpty())
        return true;

    // compare empty to really empty
    if (isEmpty() && other.size() == 1 && other.first().typeNameFormat() == QNdefRecord::Empty)
        return true;
    if (other.isEmpty() && size() == 1 && first().typeNameFormat() == QNdefRecord::Empty)
        return true;

    if (size() != other.size())
        return false;

    for (qsizetype i = 0; i < size(); ++i) {
        if (at(i) != other.at(i))
            return false;
    }

    return true;
}

/*!
    Returns the NDEF message as a byte array.

    The return value of this function conforms to the format defined in the NFC Data Exchange
    Format technical specification.
*/
QByteArray QNdefMessage::toByteArray() const
{
    // An empty message is treated as a message containing a single empty record.
    if (isEmpty())
        return QNdefMessage(QNdefRecord()).toByteArray();

    QByteArray m;

    for (qsizetype i = 0; i < size(); ++i) {
        const QNdefRecord &record = at(i);

        quint8 flags = record.typeNameFormat();

        if (i == 0)
            flags |= 0x80;
        if (i == size() - 1)
            flags |= 0x40;

        // cf (chunked records) not supported yet

        if (record.payload().size() < 255)
            flags |= 0x10;

        if (!record.id().isEmpty())
            flags |= 0x08;

        m.append(flags);
        m.append(record.type().size());

        if (flags & 0x10) {
            m.append(quint8(record.payload().size()));
        } else {
            quint32 length = record.payload().size();
            m.append(length >> 24);
            m.append(length >> 16);
            m.append(length >> 8);
            m.append(length & 0x000000ff);
        }

        if (flags & 0x08)
            m.append(record.id().size());

        if (!record.type().isEmpty())
            m.append(record.type());

        if (!record.id().isEmpty())
            m.append(record.id());

        if (!record.payload().isEmpty())
            m.append(record.payload());
    }

    return m;
}

QT_END_NAMESPACE
