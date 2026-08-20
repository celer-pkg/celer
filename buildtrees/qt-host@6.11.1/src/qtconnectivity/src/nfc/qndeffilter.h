// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFFILTER_H
#define QNDEFFILTER_H

#include <QtCore/QSharedDataPointer>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

class QNdefMessage;

class QNdefFilterPrivate;
class Q_NFC_EXPORT QNdefFilter
{
public:
    QNdefFilter();
    QNdefFilter(const QNdefFilter &other);
    ~QNdefFilter();

    void clear();

    void setOrderMatch(bool on);
    bool orderMatch() const;

    struct Record {
        QNdefRecord::TypeNameFormat typeNameFormat;
        QByteArray type;
        unsigned int minimum;
        unsigned int maximum;
    };

    template<typename T>
    bool appendRecord(unsigned int min = 1, unsigned int max = 1);
    bool appendRecord(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type,
                      unsigned int min = 1, unsigned int max = 1);
    bool appendRecord(const Record &record);

    qsizetype recordCount() const;
    Record recordAt(qsizetype i) const;

    QNdefFilter &operator=(const QNdefFilter &other);

    bool match(const QNdefMessage &message) const;

private:
    QSharedDataPointer<QNdefFilterPrivate> d;
};

template <typename T>
bool QNdefFilter::appendRecord(unsigned int min, unsigned int max)
{
    T record;

    return appendRecord(record.typeNameFormat(), record.type(), min, max);
}

QT_END_NAMESPACE

#endif // QNDEFFILTER_H
