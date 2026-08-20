// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qndefnfctextrecord.h>

#include <QtCore/QStringConverter>
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

/*!
    \class QNdefNfcTextRecord
    \brief The QNdefNfcTextRecord class provides an NFC RTD-Text.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    RTD-Text encapsulates a user displayable text record.
*/

/*!
    \enum QNdefNfcTextRecord::Encoding

    This enum describes the text encoding standard used.

    \value Utf8     The text is encoded with UTF-8.
    \value Utf16    The text is encoding with UTF-16.
*/

/*!
    \fn QNdefNfcTextRecord::QNdefNfcTextRecord()

    Constructs an empty NFC text record of type \l QNdefRecord::NfcRtd.
*/

/*!
    \fn QNdefNfcTextRecord::QNdefNfcTextRecord(const QNdefRecord& other)

    Constructs a new NFC text record that is a copy of \a other.
*/

/*!
    Returns the locale of the text record.
*/
QString QNdefNfcTextRecord::locale() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    quint8 status = p.at(0);

    quint8 codeLength = status & 0x3f;

    return QString::fromLatin1(p.constData() + 1, codeLength);
}

/*!
    Sets the locale of the text record to \a locale.
*/
void QNdefNfcTextRecord::setLocale(const QString &locale)
{
    QByteArray p = payload();

    quint8 status = p.isEmpty() ? 0 : p.at(0);

    quint8 codeLength = status & 0x3f;

    quint8 newStatus = (status & 0xd0) | locale.size();

    p[0] = newStatus;
    p.replace(1, codeLength, locale.toLatin1());

    setPayload(p);
}

/*!
    Returns the contents of the text record as a string.
*/
QString QNdefNfcTextRecord::text() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    quint8 status = p.at(0);
    bool utf16 = status & 0x80;
    quint8 codeLength = status & 0x3f;

    auto toUnicode = QStringDecoder(
        utf16 ? QStringDecoder::Encoding::Utf16BE : QStringDecoder::Encoding::Utf8,
        QStringDecoder::Flag::Stateless);

    return toUnicode(QByteArrayView(p.constData() + 1 + codeLength, p.size() - 1 - codeLength));
}

/*!
    Sets the contents of the text record to \a text.
*/
void QNdefNfcTextRecord::setText(const QString text)
{
    if (payload().isEmpty())
        setLocale(QLocale().name());

    QByteArray p = payload();

    quint8 status = p.at(0);

    bool utf16 = status & 0x80;
    quint8 codeLength = status & 0x3f;

    p.truncate(1 + codeLength);

    auto fromUnicode = QStringEncoder(
        utf16? QStringEncoder::Encoding::Utf16BE : QStringEncoder::Encoding::Utf8,
        QStringEncoder::Flag::Stateless|QStringEncoder::Flag::WriteBom);

    p += fromUnicode(text);

    setPayload(p);
}

/*!
    Returns the encoding of the contents.
*/
QNdefNfcTextRecord::Encoding QNdefNfcTextRecord::encoding() const
{
    if (payload().isEmpty())
        return Utf8;

    QByteArray p = payload();

    quint8 status = p.at(0);

    bool utf16 = status & 0x80;

    if (utf16)
        return Utf16;
    else
        return Utf8;
}

/*!
    Sets the enconding of the contents to \a encoding.
*/
void QNdefNfcTextRecord::setEncoding(Encoding encoding)
{
    QByteArray p = payload();

    quint8 status = p.isEmpty() ? 0 : p.at(0);

    QString string = text();

    if (encoding == Utf8)
        status &= ~0x80;
    else
        status |= 0x80;

    p[0] = status;

    setPayload(p);

    setText(string);
}

QT_END_NAMESPACE
