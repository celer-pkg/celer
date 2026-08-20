// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/private/qtprotobufserializerhelpers_p.h>
#include <QtProtobuf/private/qprotobufmessage_p.h>

QT_BEGIN_NAMESPACE

QVariant
QtProtobufSerializerHelpers::messageProperty(const QProtobufMessage *message,
                                             const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo,
                                             bool allowInitialize)
{
    const auto *metaObject = QtProtobufSerializerHelpers::messageMetaObject(message);
    int propertyIndex = fieldInfo.propertyIndex() + metaObject->propertyOffset();
    QMetaProperty metaProperty = metaObject->property(propertyIndex);

    if (!metaProperty.isValid())
        return {};

    if (fieldInfo.fieldFlags() & QtProtobufPrivate::FieldFlag::ExplicitPresence
        && !allowInitialize) {
        int hasPropertyIndex = propertyIndex + 1;
        QMetaProperty hasProperty = metaObject->property(hasPropertyIndex);
        Q_ASSERT_X(hasProperty.isValid() && hasProperty.metaType().id() == QMetaType::Bool,
                   "QProtobufMessage",
                   "The field with the explicit presence requirement doesn't have the follow 'has' "
                   "property.");
        if (!hasProperty.readOnGadget(message).toBool())
            return QVariant(metaProperty.metaType());
    }

    QVariant propertyValue = metaProperty.readOnGadget(message);
    return propertyValue;
}

bool QtProtobufSerializerHelpers::setMessageProperty(QProtobufMessage *message,
                                                     const QtProtobufPrivate::QProtobufFieldInfo
                                                         &fieldInfo,
                                                     const QVariant &value)
{
    const auto *pMessage = QProtobufMessagePrivate::get(message);
    const auto mp = pMessage->metaProperty(fieldInfo);
    if (!mp)
        return false;
    return mp->writeOnGadget(message, value);
}

bool QtProtobufSerializerHelpers::setMessageProperty(QProtobufMessage *message,
                                                     const QtProtobufPrivate::QProtobufFieldInfo
                                                         &fieldInfo,
                                                     QVariant &&value)
{
    const auto *pMessage = QProtobufMessagePrivate::get(message);
    const auto mp = pMessage->metaProperty(fieldInfo);
    if (!mp)
        return false;
    return mp->writeOnGadget(message, std::move(value));
}

const QMetaObject *QtProtobufSerializerHelpers::messageMetaObject(const QProtobufMessage *message)
{
    Q_ASSERT(message);
    const auto *pMessage = QProtobufMessagePrivate::get(message);
    Q_ASSERT(pMessage);
    return pMessage->metaObject;
}

QT_END_NAMESPACE
