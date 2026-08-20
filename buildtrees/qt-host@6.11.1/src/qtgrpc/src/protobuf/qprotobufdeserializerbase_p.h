// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QPROTOBUFDESERIALIZERBASE_P_H
#define QPROTOBUFDESERIALIZERBASE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtProtobuf/qabstractprotobufserializer.h>
#include <QtProtobuf/qprotobufpropertyordering.h>
#include <QtProtobuf/qprotobufrepeatediterator.h>

#include <QtCore/qtconfigmacros.h>

QT_BEGIN_NAMESPACE

class QVariant;
class QProtobufMessage;

class QProtobufDeserializerBase
{
public:
    QProtobufDeserializerBase();

    bool deserializeMessage(QProtobufMessage *message);
    void clearCachedValue();

protected:
    ~QProtobufDeserializerBase();

    virtual void setError(QAbstractProtobufSerializer::Error error, QAnyStringView errorString) = 0;

    virtual bool deserializeMessageField(QProtobufMessage *message);

private:
    virtual bool deserializeEnum(QVariant &value,
                                 const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) = 0;
    virtual int nextFieldIndex(QProtobufMessage *message) = 0;
    virtual bool deserializeScalarField(QVariant &value,
                                        const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo) = 0;

    bool storeCachedValue(QProtobufMessage *message);

    QVariant m_cachedPropertyValue;
    QProtobufRepeatedIterator m_cachedRepeatedIterator;
    int m_cachedIndex = -1;

    Q_DISABLE_COPY_MOVE(QProtobufDeserializerBase)

public:
    static constexpr QtProtobufPrivate::FieldFlags RepeatedMessageFlags{
        QtProtobufPrivate::FieldFlag::Message, QtProtobufPrivate::FieldFlag::Repeated
    };
};

QT_END_NAMESPACE

#endif // QPROTOBUFDESERIALIZERBASE_P_H
