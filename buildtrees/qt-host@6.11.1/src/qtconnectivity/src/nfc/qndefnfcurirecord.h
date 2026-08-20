// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFNFCURIRECORD_H
#define QNDEFNFCURIRECORD_H

#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_FORWARD_DECLARE_CLASS(QUrl)

QT_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcUriRecord : public QNdefRecord
{
public:
#ifndef Q_QDOC
    Q_DECLARE_NDEF_RECORD(QNdefNfcUriRecord, QNdefRecord::NfcRtd, "U", QByteArray(0, char(0)))
#else
    QNdefNfcUriRecord();
    QNdefNfcUriRecord(const QNdefRecord& other);
#endif

    QUrl uri() const;
    void setUri(const QUrl &uri);
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcUriRecord, QNdefRecord::NfcRtd, "U")

#endif // QNDEFNFCURIRECORD_H
