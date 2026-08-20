// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QABSTRACTPROTOBUFSERIALIZER_H
#define QABSTRACTPROTOBUFSERIALIZER_H

#include <QtProtobuf/qtprotobufexports.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qbytearrayview.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QProtobufMessage;

class Q_PROTOBUF_EXPORT QAbstractProtobufSerializer
{
public:
    enum class Error : uint8_t {
        None,
        InvalidHeader,
        UnknownType,
        UnexpectedEndOfStream,
        InvalidFormat,
    };

    QByteArray serialize(const QProtobufMessage *message) const;
    bool deserialize(QProtobufMessage *message, QByteArrayView data) const;

    virtual ~QAbstractProtobufSerializer();

    virtual Error lastError() const = 0;
    virtual QString lastErrorString() const = 0;

private:
    virtual QByteArray serializeMessage(const QProtobufMessage *message) const = 0;
    virtual bool deserializeMessage(QProtobufMessage *message, QByteArrayView data) const = 0;
};

QT_END_NAMESPACE
#endif // QABSTRACTPROTOBUFSERIALIZER_H
