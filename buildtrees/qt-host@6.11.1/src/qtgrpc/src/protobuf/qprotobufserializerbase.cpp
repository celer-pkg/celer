// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/private/qprotobufserializerbase_p.h>

#include <QtProtobuf/private/qprotobufregistration_p.h>
#include <QtProtobuf/private/qtprotobufdefs_p.h>
#include <QtProtobuf/private/qtprotobuflogging_p.h>
#include <QtProtobuf/private/qtprotobufserializerhelpers_p.h>

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qprotobufpropertyordering.h>
#include <QtProtobuf/qprotobufrepeatediterator.h>
#include <QtProtobuf/qtprotobuftypes.h>

QT_BEGIN_NAMESPACE

QProtobufSerializerBase::QProtobufSerializerBase() = default;
QProtobufSerializerBase::~QProtobufSerializerBase() = default;

void QProtobufSerializerBase::serializeMessage(const QProtobufMessage *message)
{
    Q_ASSERT(message != nullptr);

    const auto *ordering = message->propertyOrdering();
    Q_ASSERT(ordering != nullptr);

    for (int index = 0; index < ordering->fieldCount(); ++index) {
        int fieldNumber = ordering->fieldNumber(index);
        Q_ASSERT_X(fieldNumber <= ProtobufFieldNumMax && fieldNumber >= ProtobufFieldNumMin, "",
                   "Protobuf field number is out of range");
        QtProtobufPrivate::QProtobufFieldInfo fieldInfo(*ordering, index);
        QVariant value = QtProtobufSerializerHelpers::messageProperty(message, fieldInfo);
        QMetaType metaType = value.metaType();

        // Empty value
        if (metaType.id() == QMetaType::UnknownType || value.isNull())
            continue;

        if (metaType.flags().testFlag(QMetaType::IsPointer)) {
            serializeMessageField(value.value<QProtobufMessage *>(), fieldInfo);
            continue;
        }

        if (value.canView(QMetaType::fromType<QProtobufRepeatedIterator>())) {
            QProtobufRepeatedIterator propertyIt = value.view<QProtobufRepeatedIterator>();
            while (propertyIt.hasNext())
                serializeMessageField(propertyIt.next(), fieldInfo);
            continue;
        }

        QtProtobufPrivate::FieldFlags flags = fieldInfo.fieldFlags();
        // Convert enums to something known by scalar type serializers
        if (flags.testFlag(QtProtobufPrivate::FieldFlag::Enum)) {
            if (!serializeEnum(value, fieldInfo))
                qProtoWarning() << metaType << " is not registered as protobuf enum";
            continue;
        }

        if (serializeScalarField(value, fieldInfo))
            continue;

        auto handler = QtProtobufPrivate::findHandler(metaType);
        if (!handler.serializer) {
            qProtoWarning("No serializer found for type %s", value.typeName());
            continue;
        }

        handler.serializer([this](const QProtobufMessage *message,
                                  const QtProtobufPrivate::QProtobufFieldInfo
                                      &fieldInfo) { serializeMessageField(message, fieldInfo); },
                           value.constData(), fieldInfo);
    }
}

void QProtobufSerializerBase::serializeMessageField(const QProtobufMessage *message,
                                                    const QtProtobufPrivate::QProtobufFieldInfo
                                                        &fieldInfo)
{
    serializeMessageFieldBegin();
    serializeMessage(message);
    serializeMessageFieldEnd(message, fieldInfo);
}

QT_END_NAMESPACE
