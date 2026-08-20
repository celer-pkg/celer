// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qndefnfcurirecord.h"

#include <QtCore/QString>
#include <QtCore/QUrl>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

/*!
    \class QNdefNfcUriRecord
    \brief The QNdefNfcUriRecord class provides an NFC RTD-URI.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    RTD-URI encapsulates a URI.
*/

/*!
    \fn QNdefNfcUriRecord::QNdefNfcUriRecord()

    Constructs an empty NFC uri record.
*/

/*!
    \fn QNdefNfcUriRecord::QNdefNfcUriRecord(const QNdefRecord& other)

    Constructs a new NFC uri record that is a copy of \a other.
*/
static const char * const abbreviations[] = {
    0,
    "http://www.",
    "https://www.",
    "http://",
    "https://",
    "tel:",
    "mailto:",
    "ftp://anonymous:anonymous@",
    "ftp://ftp.",
    "ftps://",
    "sftp://",
    "smb://",
    "nfs://",
    "ftp://",
    "dav://",
    "news:",
    "telnet://",
    "imap:",
    "rtsp://",
    "urn:",
    "pop:",
    "sip:",
    "sips:",
    "tftp:",
    "btspp://",
    "btl2cap://",
    "btgoep://",
    "tcpobex://",
    "irdaobex://",
    "file://",
    "urn:epc:id:",
    "urn:epc:tag:",
    "urn:epc:pat:",
    "urn:epc:raw:",
    "urn:epc:",
    "urn:nfc:",
};

/*!
    Returns the URI of this URI record.
*/
QUrl QNdefNfcUriRecord::uri() const
{
    QByteArray p = payload();

    if (p.isEmpty())
        return QUrl();

    quint8 code = p.at(0);
    if (code >= sizeof(abbreviations) / sizeof(*abbreviations))
        code = 0;
    p.remove(0, 1);
    if (const char *abbreviation = abbreviations[code])
        p.insert(0, abbreviation);
    return QUrl(QString::fromUtf8(p));
}

/*!
    Sets the URI of this URI record to \a uri.
*/
void QNdefNfcUriRecord::setUri(const QUrl &uri)
{
    int abbrevs = sizeof(abbreviations) / sizeof(*abbreviations);

    for (int i = 1; i < abbrevs; ++i) {
        if (uri.toString().startsWith(QLatin1String(abbreviations[i]))) {
            QByteArray p(1, i);

            p += uri.toString().mid(qstrlen(abbreviations[i])).toUtf8();

            setPayload(p);

            return;
        }
    }

    QByteArray p(1, 0);
    p += uri.toString().toUtf8();

    setPayload(p);
}

QT_END_NAMESPACE
