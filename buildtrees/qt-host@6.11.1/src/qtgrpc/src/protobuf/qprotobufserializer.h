// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFSERIALIZER_H
#define QPROTOBUFSERIALIZER_H

#include <QtProtobuf/qabstractprotobufserializer.h>
#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qbytearrayview.h>
#include <QtCore/qstring.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QProtobufMessage;
class QProtobufSerializerPrivate;
class Q_PROTOBUF_EXPORT QProtobufSerializer final : public QAbstractProtobufSerializer
{
    Q_DISABLE_COPY_MOVE(QProtobufSerializer)
public:
    QProtobufSerializer();
    ~QProtobufSerializer() override;

    Error lastError() const override;
    QString lastErrorString() const override;

    void shouldPreserveUnknownFields(bool preserveUnknownFields);

private:
    QByteArray serializeMessage(const QProtobufMessage *message) const override;
    bool deserializeMessage(QProtobufMessage *message, QByteArrayView data) const override;

    std::unique_ptr<QProtobufSerializerPrivate> d_ptr;
};

QT_END_NAMESPACE
#endif // QPROTOBUFSERIALIZER_H
