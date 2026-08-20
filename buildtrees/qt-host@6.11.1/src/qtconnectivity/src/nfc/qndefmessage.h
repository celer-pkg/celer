// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFMESSAGE_H
#define QNDEFMESSAGE_H

#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

// This class used to be exported exposing QList methods, see QTBUG-102367.
#if defined(QT_BUILD_NFC_LIB)
#    define Q_NFC_EXPORT_COMPAT QT6_ONLY(Q_NFC_EXPORT)
#else
#    define Q_NFC_EXPORT_COMPAT
#endif

class QNdefMessage : public QList<QNdefRecord>
{
public:
    Q_NFC_EXPORT_COMPAT
    QNdefMessage() = default;
    Q_NFC_EXPORT_COMPAT
    explicit QNdefMessage(const QNdefRecord &record) { append(record); }
    Q_NFC_EXPORT_COMPAT
    QNdefMessage(const QNdefMessage &message) = default;
    Q_NFC_EXPORT_COMPAT
    QNdefMessage(const QList<QNdefRecord> &records) : QList<QNdefRecord>(records) { }

    Q_NFC_EXPORT_COMPAT
    QNdefMessage &operator=(const QNdefMessage &other) = default;
    Q_NFC_EXPORT_COMPAT
    QNdefMessage &operator=(QNdefMessage &&other) noexcept = default;

    Q_NFC_EXPORT bool operator==(const QNdefMessage &other) const;

    Q_NFC_EXPORT QByteArray toByteArray() const;

    Q_NFC_EXPORT static QNdefMessage fromByteArray(const QByteArray &message);
};

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QNdefMessage, Q_NFC_EXPORT)

#endif // QNDEFMESSAGE_H
