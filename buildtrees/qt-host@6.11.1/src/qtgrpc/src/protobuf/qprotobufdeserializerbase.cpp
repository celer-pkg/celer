// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/private/qprotobufdeserializerbase_p.h>

#include <QtProtobuf/private/qprotobufregistration_p.h>
#include <QtProtobuf/private/qtprotobuflogging_p.h>
#include <QtProtobuf/private/qtprotobufserializerhelpers_p.h>

QT_BEGIN_NAMESPACE

QProtobufDeserializerBase::QProtobufDeserializerBase()
    = default;

QProtobufDeserializerBase::~QProtobufDeserializerBase()
    = default;

bool QProtobufDeserializerBase::deserializeMessageField(QProtobufMessage *message)
{
    Q_ASSERT(message != nullptr);

    auto prevCachedRepeatedIterator = std::move(m_cachedRepeatedIterator);
    auto prevCachedPropertyValue = m_cachedPropertyValue;
    auto prevCachedIndex = m_cachedIndex;
    clearCachedValue();
    bool result = deserializeMessage(message);
    m_cachedPropertyValue = prevCachedPropertyValue;
    m_cachedIndex = prevCachedIndex;
    m_cachedRepeatedIterator = std::move(prevCachedRepeatedIterator);

    return result;
}

bool QProtobufDeserializerBase::deserializeMessage(QProtobufMessage *message)
{
    Q_ASSERT(message != nullptr);

    const auto *ordering = message->propertyOrdering();
    for (int fieldIndex = nextFieldIndex(message); fieldIndex >= 0;
         fieldIndex = nextFieldIndex(message)) {
        QtProtobufPrivate::QProtobufFieldInfo fieldInfo(*ordering, fieldIndex);
        if (m_cachedIndex != fieldIndex) {
            if (!storeCachedValue(message)) {
                setError(QAbstractProtobufSerializer::Error::InvalidFormat,
                         "Unable to store the property in message");
                return false;
            }

            m_cachedPropertyValue = QtProtobufSerializerHelpers::messageProperty(message, fieldInfo,
                                                                                 true);
            m_cachedIndex = fieldIndex;
        }

        QMetaType metaType = m_cachedPropertyValue.metaType();
        if (metaType.flags().testFlag(QMetaType::IsPointer)) {
            if (!deserializeMessageField(m_cachedPropertyValue.value<QProtobufMessage *>()))
                return false;

            continue;
        }

        const auto fieldFlags = fieldInfo.fieldFlags();
        if ((fieldFlags.testFlags(RepeatedMessageFlags)
             || fieldFlags.testFlags({ QtProtobufPrivate::FieldFlag::Map }))
            && m_cachedPropertyValue.canView(QMetaType::fromType<QProtobufRepeatedIterator>())) {
            if (!m_cachedRepeatedIterator.isValid())
                m_cachedRepeatedIterator = m_cachedPropertyValue.view<QProtobufRepeatedIterator>();

            if (!deserializeMessageField(m_cachedRepeatedIterator.addNext()))
                return false;

            m_cachedRepeatedIterator.push();
            continue;
        }

        if (fieldFlags.testFlag(QtProtobufPrivate::FieldFlag::Enum)) {
            if (!deserializeEnum(m_cachedPropertyValue, fieldInfo)) {
                setError(QAbstractProtobufSerializer::Error::UnknownType,
                         "Unable to covert enum field to compatible serialization format");
                return false;
            }
            continue;
        }

        if (deserializeScalarField(m_cachedPropertyValue, fieldInfo)) {
            if (!m_cachedPropertyValue.isValid())
                return false;

            continue;
        }

        auto handler = QtProtobufPrivate::findHandler(metaType);
        if (!handler.deserializer) {
            qProtoWarning() << "No deserializer for type" << metaType.name();
            setError(QAbstractProtobufSerializer::Error::UnknownType,
                     QString::fromUtf8("No deserializer is registered for type %1")
                         .arg(QString::fromUtf8(metaType.name())));
            return false;
        }

        handler.deserializer([this](QProtobufMessage *
                                        message) { return this->deserializeMessageField(message); },
                             m_cachedPropertyValue.data());
    }

    if (!storeCachedValue(message)) {
        setError(QAbstractProtobufSerializer::Error::InvalidFormat,
                 "Unable to store the property in message");
        return false;
    }

    return true;
}

bool QProtobufDeserializerBase::storeCachedValue(QProtobufMessage *message)
{
    bool ok = true;
    if (m_cachedIndex >= 0 && !m_cachedPropertyValue.isNull()) {
        const auto *ordering = message->propertyOrdering();
        QtProtobufPrivate::QProtobufFieldInfo fieldInfo(*ordering, m_cachedIndex);
        ok = QtProtobufSerializerHelpers::setMessageProperty(message, fieldInfo,
                                                             m_cachedPropertyValue);

        clearCachedValue();
    }
    return ok;
}

void QProtobufDeserializerBase::clearCachedValue()
{
    m_cachedPropertyValue.clear();
    m_cachedIndex = -1;
    m_cachedRepeatedIterator = {};
}

QT_END_NAMESPACE
