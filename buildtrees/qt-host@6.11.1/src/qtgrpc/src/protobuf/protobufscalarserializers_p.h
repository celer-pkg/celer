// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#ifndef PROTOBUFSCALARSERIALIZERS_P_H
#define PROTOBUFSCALARSERIALIZERS_P_H
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

#include <QtProtobuf/private/qprotobufselfcheckiterator_p.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qtconfigmacros.h>

#include <type_traits>

QT_BEGIN_NAMESPACE

namespace ProtobufScalarSerializers {
// The below type trait structures help to determine the required encoding method for protobuf
// types.
// See https://protobuf.dev/programming-guides/encoding for details.

// Tests if V is a Varint-compatible unsigned integer type.
// uint32 | uint64
template <typename V>
using is_unsigned_int = std::conjunction<std::is_integral<V>, std::is_unsigned<V>>;

template <typename V>
using if_unsigned_int = std::enable_if_t<is_unsigned_int<V>::value, bool>;

// Tests if V is a Varint-compatible signed integer type.
// int32 | int64
template <typename V>
constexpr inline bool IsSignedInt = false;
template <>
constexpr inline bool IsSignedInt<QtProtobuf::int32> = true;
template <>
constexpr inline bool IsSignedInt<QtProtobuf::int64> = true;

template <typename V>
using is_signed_int = std::integral_constant<bool, IsSignedInt<V>>;

template <typename V>
using if_signed_int = std::enable_if_t<is_signed_int<V>::value, bool>;

// Tests if V is a Varint-compatible signed integer type, with ZigZag encoding support.
// sint32 | sint64
template <typename V>
using is_zigzag_int = std::conjunction<std::is_integral<V>, std::is_signed<V>>;

template <typename V>
using if_zigzag_int = std::enable_if_t<is_zigzag_int<V>::value, bool>;

// Tests if V is a Varint-compatible integer type.
// int32 | int64 | uint32 | uint64 | sint32 | sint64
template <typename V>
using is_int = std::disjunction<std::is_integral<V>, is_signed_int<V>>;

// Tests if V is a 32-bit integer without the need for encoding.
// sfixed32 | fixed32
template <typename V>
constexpr inline bool IsFixed32Int = false;
template <>
constexpr inline bool IsFixed32Int<QtProtobuf::fixed32> = true;
template <>
constexpr inline bool IsFixed32Int<QtProtobuf::sfixed32> = true;

template <typename V>
using is_fixed32_int = std::integral_constant<bool, IsFixed32Int<V>>;

// Tests if V is a 64-bit integer without the need for encoding.
// sfixed64 | fixed64
template <typename V>
constexpr inline bool IsFixed64Int = false;
template <>
constexpr inline bool IsFixed64Int<QtProtobuf::fixed64> = true;
template <>
constexpr inline bool IsFixed64Int<QtProtobuf::sfixed64> = true;

template <typename V>
using is_fixed64_int = std::integral_constant<bool, IsFixed64Int<V>>;

// Tests if V is a 32-bit type without the need for encoding.
// sfixed32 | fixed32 | float
template <typename V>
using is_i32 = std::disjunction<is_fixed32_int<V>, std::is_same<V, float>>;

// Tests if V is a 64-bit type without the need for encoding.
// sfixed64 | fixed64 | double
template <typename V>
using is_i64 = std::disjunction<is_fixed64_int<V>, std::is_same<V, double>>;

// Tests if V is a type without the need for encoding.
// sfixed32 | fixed32 | float | sfixed64 | fixed64 | double
template <typename V>
using is_i32_or_i64 = std::disjunction<is_i32<V>, is_i64<V>>;

template <typename V>
using if_i32_or_i64 = std::enable_if_t<is_i32_or_i64<V>::value, bool>;

// Tests if V is the length-delimited non-message, non-list type.
// QString | QByteArray
template <typename V>
constexpr inline bool IsLengthDelimited = false;
template <>
constexpr inline bool IsLengthDelimited<QByteArray> = true;
template <>
constexpr inline bool IsLengthDelimited<QString> = true;

template <typename V>
using is_length_delimited = std::integral_constant<bool, IsLengthDelimited<V>>;

template <typename V>
using if_length_delimited = std::enable_if_t<is_length_delimited<V>::value, bool>;

template <typename V>
using if_not_length_delimited = std::enable_if_t<std::negation_v<is_length_delimited<V>>, bool>;

// Test if V can be stored in non-packed repeated field
template <typename V>
using is_non_packed_compatible = std::disjunction<is_int<V>, is_i32_or_i64<V>,
                                                  is_length_delimited<V>>;

template <typename V>
using if_non_packed_compatible = std::enable_if_t<is_non_packed_compatible<V>::value, bool>;

[[nodiscard]] QByteArray prependLengthDelimitedSize(const QByteArray &data);
[[nodiscard]] std::optional<QByteArray> deserializeLengthDelimited(QProtobufSelfcheckIterator &it);

// ###########################################################################
//                                Serializers
// ###########################################################################

[[nodiscard]] QByteArray serializeVarintCommonImpl(quint64 value);

template <typename V, if_unsigned_int<V> = true>
[[nodiscard]] QByteArray serializeVarintCommon(V value)
{
    return serializeVarintCommonImpl(quint64(value));
}

//---------------Integral and floating point types serializers---------------
/*
    Serialization of unsigned integral types

  Use Varint encoding[0]:
  "Varints are a method of serializing integers using one or more bytes. Smaller numbers
  [regardless its type] take a smaller number of bytes."

  [0]: https://protobuf.dev/programming-guides/encoding

  value: Value to serialize
  Returns a byte array with 'value' encoded
*/
template <typename V, if_unsigned_int<V> = true>
[[nodiscard]] QByteArray serializeBasic(const V &value)
{
    return serializeVarintCommon(value);
}

/*
    Serialization of fixed-length primitive types

    Natural layout of bits is used: value is encoded in a byte array same way as it is located in
    memory

    value: Value to serialize
    returns a byte array with 'value' encoded
 */
template <typename V, if_i32_or_i64<V> = true>
[[nodiscard]] QByteArray serializeBasic(const V &value)
{
    // Reserve required number of bytes
    QByteArray result(sizeof(V), Qt::Uninitialized);
    qToUnaligned(qToLittleEndian(value), result.data());
    return result;
}

/*
    Serialization of signed integral types
    Uses ZigZag encoding[0] first then apply serialization as for unsigned integral types

     [0]: https://protobuf.dev/programming-guides/encoding

     value: Value to serialize
     Returns a byte array with 'value' encoded
 */
template <typename V, if_zigzag_int<V> = true>
[[nodiscard]] QByteArray serializeBasic(const V &value)
{
    using UV = std::make_unsigned_t<V>;

    // Use ZigZag convertion first and apply unsigned variant next
    V zigZagValue = (value << 1) ^ (value >> (sizeof(UV) * 8 - 1));
    return serializeBasic(static_cast<UV>(zigZagValue));
}

template <typename V, if_signed_int<V> = true>
[[nodiscard]] QByteArray serializeBasic(const V &value)
{
    // Non-ZigZag signed integers should always be (de)serialized as the
    // QtProtobuf::uint64
    return serializeBasic(static_cast<QtProtobuf::uint64>(value));
}

//------------------QString and QByteArray types serializers-----------------
template <typename V, if_length_delimited<V> = true>
[[nodiscard]] QByteArray serializeBasic(const V &value)
{
    // Varint serialize field size and apply result as starting point
    if constexpr (std::is_same<V, QString>::value)
        return prependLengthDelimitedSize(value.toUtf8());
    else
        return prependLengthDelimitedSize(value);
}

//--------------------------List types serializers---------------------------
template <typename V, if_not_length_delimited<V> = true>
[[nodiscard]] QByteArray serializeListType(const QList<V> &listValue)
{
    QByteArray serializedList;
    if (listValue.isEmpty())
        return serializedList;

    for (auto &value : listValue)
        serializedList.append(serializeBasic<V>(value));

    // If internal field type is not LengthDelimited, exact amount of fields to be specified
    serializedList = prependLengthDelimitedSize(serializedList);
    return serializedList;
}

template <typename V, if_non_packed_compatible<V> = true>
[[nodiscard]] QByteArray serializeNonPackedList(const QList<V> &listValue, const QByteArray &header)
{
    QByteArray serializedList;
    for (auto &value : listValue) {
        serializedList.append(header);
        serializedList.append(serializeBasic<V>(value));
    }
    return serializedList;
}

// ###########################################################################
//                                Deserializers
// ###########################################################################

template <typename V, if_unsigned_int<V> = true>
[[nodiscard]] std::optional<V> deserializeVarintCommon(QProtobufSelfcheckIterator &it)
{
    quint64 value = 0;
    int k = 0;
    while (true) {
        if (it.bytesLeft() == 0)
            return std::nullopt;
        quint64 byte = quint64(static_cast<unsigned char>(*it) & 0b01111111);
        value += byte << k;
        k += 7;
        if (((*it++) & 0b10000000) == 0)
            break;
    }
    return { V(value) };
}

//-------------Integral and floating point types deserializers---------------
template <typename V, if_i32_or_i64<V> = false>
[[nodiscard]] bool deserializeBasic(QProtobufSelfcheckIterator &it, QVariant &variantValue)
{
    constexpr qsizetype size = sizeof(V);
    if (it.bytesLeft() < size)
        return false;
    variantValue = QVariant::fromValue(qFromLittleEndian(qFromUnaligned<V>(it.data())));
    it += size;
    return true;
}

template <typename V, if_unsigned_int<V> = false>
[[nodiscard]] bool deserializeBasic(QProtobufSelfcheckIterator &it, QVariant &variantValue)
{
    auto opt = deserializeVarintCommon<V>(it);
    if (!opt)
        return false;
    variantValue = QVariant::fromValue(*opt);
    return true;
}

template <typename V, if_zigzag_int<V> = false>
[[nodiscard]] bool deserializeBasic(QProtobufSelfcheckIterator &it, QVariant &variantValue)
{
    using UV = std::make_unsigned_t<V>;
    auto opt = deserializeVarintCommon<UV>(it);
    if (!opt)
        return false;
    UV unsignedValue = *opt;
    V value = (unsignedValue >> 1) ^ (-1 * (unsignedValue & 1));
    variantValue = QVariant::fromValue<V>(value);
    return true;
}

template <typename V, if_signed_int<V> = false>
[[nodiscard]] bool deserializeBasic(QProtobufSelfcheckIterator &it, QVariant &variantValue)
{
    // Non-ZigZag signed integers should always be (de)serialized as the
    // QtProtobuf::uint64
    auto opt = deserializeVarintCommon<QtProtobuf::uint64>(it);
    if (!opt)
        return false;
    variantValue = QVariant::fromValue(static_cast<V>(*opt));
    return true;
}

//-----------------QString and QByteArray types deserializers----------------
template <typename V, if_length_delimited<V> = false>
[[nodiscard]] bool deserializeBasic(QProtobufSelfcheckIterator &it, QVariant &variantValue)
{
    std::optional<QByteArray> result = deserializeLengthDelimited(it);
    if (!result) {
        variantValue = QVariant();
        return false;
    }
    if constexpr (std::is_same<QString, V>::value)
        variantValue = QVariant::fromValue(QString::fromUtf8(*result));
    else
        variantValue = QVariant::fromValue(*result);
    return true;
}

//-------------------------List types deserializers--------------------------
template <typename V>
[[nodiscard]] bool deserializeList(QProtobufSelfcheckIterator &it, QVariant &previousValue)
{
    static constexpr auto
        MaxSafeCount = static_cast<quint64>(std::numeric_limits<qsizetype>::max());

    const auto opt = deserializeVarintCommon<QtProtobuf::uint64>(it);
    if (!opt)
        return false;
    const quint64 count = *opt;
    if (count > MaxSafeCount)
        return false;
    const auto safeCount = static_cast<qsizetype>(count);
    const QProtobufSelfcheckIterator lastVarint = it + safeCount;
    if (!lastVarint.isValid())
        return false;

    QList<V> out;
    out.reserve(safeCount);

    while (it != lastVarint) {
        QVariant variantValue;
        if (!deserializeBasic<V>(it, variantValue))
            return false;
        out.append(variantValue.value<V>());
    }

    previousValue.setValue(std::move(out));
    return true;
}

template <typename V>
[[nodiscard]] bool deserializeNonPackedList(QProtobufSelfcheckIterator &it, QVariant &previousValue)
{
    Q_ASSERT_X(previousValue.metaType() == QMetaType::fromType<QList<V>>() && previousValue.data(),
               "QProtobufSerializerPrivate::deserializeNonPackedList",
               "Previous value metatype doesn't match the list metatype");
    QVariant variantValue;
    if (deserializeBasic<V>(it, variantValue)) {
        QList<V> *out = reinterpret_cast<QList<V> *>(previousValue.data());
        out->append(variantValue.value<V>());
        return true;
    }
    return false;
}

} // namespace ProtobufScalarSerializers

QT_END_NAMESPACE

#endif // PROTOBUFSCALARSERIALIZERS_P_H
