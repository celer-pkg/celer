// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/qabstractprotobufserializer.h>

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qprotobufpropertyordering.h>

#include <QtProtobuf/private/qtprotobufserializerhelpers_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractProtobufSerializer
    \inmodule QtProtobuf
    \since 6.5
    \brief The QAbstractProtobufSerializer class is interface that represents
           basic functions for serialization/deserialization.
    \reentrant

    The QProtobufSerializer class registers serializers/deserializers for
    classes implementing a protobuf message, inheriting \l QProtobufMessage. These
    classes are generated automatically, based on a \c{.proto} file, using the CMake
    function \l qt_add_protobuf or by running
    \l {The qtprotobufgen Tool} {qtprotobufgen} directly.

    This class should be used as a base for specific serializers. The handlers
    property contains all message-specific serializers and should be used while
    serialization/deserialization. Inherited classes should reimplement scope of
    virtual methods that used by registered message
    serialization/deserialization functions.
*/

/*!
    \enum QAbstractProtobufSerializer::Error
    \since 6.8

    This enum contains possible errors that can occur during deserialization.
    When an error occurs, call lastErrorString() to get a
    human-readable error message.

    \value None                         No error occurred.
    \value InvalidHeader                Something went wrong while attempting to
                                        decode a header in the message.
    \value UnknownType                  While serializing or deserializing a
                                        message, no deserializer was found
                                        for a message field.
    \value UnexpectedEndOfStream        While deserializing a message, the
                                        stream ended unexpectedly.
    \value InvalidFormat                The data has invalid format. For example
                                        the JSON value doesn't match the field type.
*/

/*!
   \fn QAbstractProtobufSerializer::Error QAbstractProtobufSerializer::lastError() const
   \since 6.8

   Returns the last error for the serializer instance.

   \sa lastErrorString()
*/

/*!
   \fn QString QAbstractProtobufSerializer::lastErrorString() const
   \since 6.8

   Returns the last error string for the serializer instance.

   \sa lastError()
*/

/*!
    Destroys this QAbstractProtobufSerializer.
*/
QAbstractProtobufSerializer::~QAbstractProtobufSerializer() = default;

/*!
    \fn QByteArray QAbstractProtobufSerializer::serializeMessage(const QProtobufMessage *message) const

    This is called by serialize() to serialize a registered Protobuf \a message.
    \a message must not be \nullptr.
    Returns a QByteArray containing the serialized message.
*/

/*!
    \fn bool QAbstractProtobufSerializer::deserializeMessage(QProtobufMessage *message,
        QByteArrayView data) const

    This is called by deserialize() to deserialize a registered Protobuf
    \a message from a QByteArrayView \a data. \a message can be
    assumed to not be \nullptr.
    Returns \c true if deserialization was successful, otherwise \c false.
*/

/*!
    Serializes a registered Protobuf message \a message into a QByteArray.
    \a message must not be \nullptr.

    \sa deserialize()
*/
QByteArray QAbstractProtobufSerializer::serialize(const QProtobufMessage *message) const
{
    Q_ASSERT(message != nullptr && message->propertyOrdering() != nullptr
             && message->propertyOrdering()->data != nullptr);
    return serializeMessage(message);
}

/*!
    Deserializes a registered Protobuf message \a message from a QByteArray
    \a data. \a message must not be \nullptr.
    Returns \c true if deserialization was successful, otherwise \c false.

    Unexpected/unknown properties in the \a data are skipped.

    \sa serialize()
*/
bool QAbstractProtobufSerializer::deserialize(QProtobufMessage *message, QByteArrayView data) const
{
    Q_ASSERT(message != nullptr && message->propertyOrdering() != nullptr
             && message->propertyOrdering()->data != nullptr);
    // Wipe the message by reconstructing it in place.
    const auto mtype = QtProtobufSerializerHelpers::messageMetaObject(message)->metaType();
    mtype.destruct(message);
    mtype.construct(message);
    return deserializeMessage(message, data);
}

QT_END_NAMESPACE
