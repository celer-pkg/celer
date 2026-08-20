// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/qtprotobuftypes.h>

QT_BEGIN_NAMESPACE


/*!
    \enum QtProtobuf::WireTypes
    \brief The WireTypes enumeration reflects protobuf default wiretypes.

    The following table shows the values in the enumeration and their
    corresponding types:

    \value Unknown          Invalid wire type
    \value Varint           int32, int64, uint32, uint64, sint32, sint64, bool, enum
    \value Fixed64          fixed64, sfixed64, double
    \value LengthDelimited  string, bytes, embedded messages, packed repeated fields
    \value StartGroup       groups. Deprecated in proto syntax 3. Not supported by Qt Protobuf.
    \value EndGroup         groups. Deprecated in proto syntax 3. Not supported by Qt Protobuf.
    \value Fixed32          fixed32, sfixed32, float

    \sa {https://protobuf.dev/programming-guides/encoding} {encoding}
*/

/*!
    \class QProtobufFieldInfo
    \inmodule QtProtobuf
    \internal
    \brief Holds a property's index in the property system, and the json_name.

    This class is used by the QAbstractProtobufSerializer to help serialize/
    deserialize protobuf messages.

    \sa QProtobufPropertyOrdering
*/

/*!
    \typealias QProtobufPropertyOrdering
    \internal

    A map between the property field index and an instance of
    QProtobufFieldInfo.

    \sa Q_PROTOBUF_OBJECT
*/

/*!
    \namespace QtProtobuf
    \brief The QtProtobuf namespace contains type aliases and classes needed to support Qt Protobuf.
    \inmodule QtProtobuf
*/

/*!
    \struct QtProtobuf::transparent
    \inmodule QtProtobuf
    \internal
    \brief Only used to create new, unique types for numeric types.
*/

/*!
    \typealias QtProtobuf::int32

    int32 is a regular signed 32-bit integer that is represented in protobuf as
    a variable size integer, an alias for WireTypes::Varint.
*/

/*!
    \typealias QtProtobuf::int64

    int64 is a regular signed 64-bit integer that is represented in protobuf as
    a variable size integer, an alias for WireTypes::Varint.
*/

/*!
    \typealias QtProtobuf::uint32

    uint32 is an unsigned 32-bit integer that is represented in protobuf as
    variable size integer, an alias for WireTypes::Varint.
*/

/*!
    \typealias QtProtobuf::uint64

    uint64 is an unsigned 64-bit integer that is represented in protobuf as
    variable size integer, an alias for WireTypes::Varint.
*/

/*!
    \typealias QtProtobuf::sint32

    sint32 is a 32-bit integer with forced sign marker that is represented in
    protobuf as variable size integer, an alias for WireTypes::Varint.
    sint32 is serialized using ZigZag conversion to reduce size of negative
    numbers.

    \sa {https://protobuf.dev/programming-guides/encoding/#signed-ints} {signed-integers}
*/

/*!
    \typealias QtProtobuf::sint64

    sint64 is a 64-bit integer with forced sign marker that is represented in
    protobuf as variable size integer, an alias for WireTypes::Varint.
    sint64 is serialized using ZigZag conversion to reduce size of negative numbers.

    \sa {https://protobuf.dev/programming-guides/encoding/#signed-ints} {signed-integers}
*/

/*!
    \typealias QtProtobuf::fixed32

    fixed32 is an unsigned 32-bit integer that is represented in protobuf as a
    fixed size 32-bit field, an alias for WireTypes::Fixed32.
*/

/*!
    \typealias QtProtobuf::fixed64

    fixed64 is an unsigned 64-bit integer that is represented in protobuf as a
    fixed size 64-bit field, an alias for WireTypes::Fixed64.
*/

/*!
    \typealias QtProtobuf::sfixed32

    sfixed32 is a signed 32-bit integer that is represented in protobuf as a
    fixed size 32-bit field, an alias for WireTypes::Fixed32.
*/

/*!
    \typealias QtProtobuf::sfixed64

    sfixed64 is a signed 64-bit integer that is represented in protobuf as a
    fixed size 64-bit field, an alias for WireTypes::Fixed64.
*/

/*!
    \typealias QtProtobuf::int32List

    Alias for a list of QtProtobuf::int32.
*/

/*!
    \typealias QtProtobuf::int64List

    Alias for a list of QtProtobuf::int64.
*/

/*!
    \typealias QtProtobuf::uint32List

    Alias for a list of QtProtobuf::uint32.
*/

/*!
    \typealias QtProtobuf::uint64List

    Alias for a list of QtProtobuf::uint64.
*/

/*!
    \typealias QtProtobuf::sint32List

    Alias for a list of QtProtobuf::sint32.
*/

/*!
    \typealias QtProtobuf::sint64List

    Alias for a list of QtProtobuf::sint64.
*/

/*!
    \typealias QtProtobuf::fixed32List

    Alias for a list of QtProtobuf::fixed32.
*/

/*!
    \typealias QtProtobuf::fixed64List

    Alias for a list of QtProtobuf::fixed64.
*/

/*!
    \typealias QtProtobuf::sfixed32List

    Alias for a list of QtProtobuf::sfixed32.
*/

/*!
    \typealias QtProtobuf::sfixed64List

    Alias for a list of QtProtobuf::sfixed64.
*/

/*!
    \typealias QtProtobuf::floatList

    Alias for a list of float.
*/

/*!
    \typealias QtProtobuf::doubleList

    Alias for a list of double.
*/

/*!
    \typealias QtProtobuf::boolList

    Alias for a list of bool.
*/

/*!
    \typealias QtProtobuf::RegisterFunction
    \internal
*/

/*!
    \fn template <typename T> struct QtProtobuf::ProtoTypeRegistrar
    \internal

    Used in the type registration process.
*/

QT_END_NAMESPACE

#include "moc_qtprotobuftypes.cpp"
