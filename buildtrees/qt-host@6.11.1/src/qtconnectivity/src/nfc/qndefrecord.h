// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNDEFRECORD_H
#define QNDEFRECORD_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QByteArray>
#include <QtNfc/qtnfcglobal.h>

QT_BEGIN_NAMESPACE

class QNdefRecordPrivate;

class Q_NFC_EXPORT QNdefRecord
{
public:
    enum TypeNameFormat {
        Empty = 0x00,
        NfcRtd = 0x01,
        Mime = 0x02,
        Uri = 0x03,
        ExternalRtd = 0x04,
        Unknown = 0x05
    };

    QNdefRecord();
    ~QNdefRecord();

    QNdefRecord(const QNdefRecord &other);
    QNdefRecord &operator=(const QNdefRecord &other);

    void setTypeNameFormat(TypeNameFormat typeNameFormat);
    TypeNameFormat typeNameFormat() const;

    void setType(const QByteArray &type);
    QByteArray type() const;

    void setId(const QByteArray &id);
    QByteArray id() const;

    void setPayload(const QByteArray &payload);
    QByteArray payload() const;

    bool isEmpty() const;

    template <typename T>
    inline bool isRecordType() const
    {
        T dummy;
        return (typeNameFormat() == dummy.typeNameFormat() && type() == dummy.type());
    }

    bool operator==(const QNdefRecord &other) const;
    inline bool operator!=(const QNdefRecord &other) const { return !operator==(other); }

    void clear();

protected:
    QNdefRecord(const QNdefRecord &other, TypeNameFormat typeNameFormat, const QByteArray &type);
    QNdefRecord(const QNdefRecord &other, TypeNameFormat typeNameFormat);
    QNdefRecord(TypeNameFormat typeNameFormat, const QByteArray &type);

private:
    QSharedDataPointer<QNdefRecordPrivate> d;
};

#define Q_DECLARE_NDEF_RECORD(className, typeNameFormat, type, initialPayload) \
    className() : QNdefRecord(typeNameFormat, type) { setPayload(initialPayload); } \
    className(const QNdefRecord &other) : QNdefRecord(other, typeNameFormat, type) { }

#define Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(className, typeNameFormat_, type_) \
    QT_BEGIN_NAMESPACE \
    template<> inline bool QNdefRecord::isRecordType<className>() const\
    { \
        return (typeNameFormat() == typeNameFormat_ && type() == type_); \
    } \
    QT_END_NAMESPACE

Q_NFC_EXPORT size_t qHash(const QNdefRecord &key);

QT_END_NAMESPACE

#endif // QNDEFRECORD_H
