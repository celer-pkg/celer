// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFNFCTEXTRECORD_H
#define QNDEFNFCTEXTRECORD_H

#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcTextRecord : public QNdefRecord
{
public:
#ifndef Q_QDOC
    Q_DECLARE_NDEF_RECORD(QNdefNfcTextRecord, QNdefRecord::NfcRtd, "T", QByteArray(1, char(0)))
#else
    QNdefNfcTextRecord();
    QNdefNfcTextRecord(const QNdefRecord& other);
#endif

    QString locale() const;
    void setLocale(const QString &locale);

    QString text() const;
    void setText(const QString text);

    enum Encoding {
        Utf8,
        Utf16
    };

    Encoding encoding() const;
    void setEncoding(Encoding encoding);
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcTextRecord, QNdefRecord::NfcRtd, "T")

#endif // QNDEFNFCTEXTRECORD_H
