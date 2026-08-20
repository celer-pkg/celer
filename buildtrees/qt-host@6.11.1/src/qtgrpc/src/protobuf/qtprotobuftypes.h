// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPROTOBUFTYPES_H
#define QTPROTOBUFTYPES_H

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qendian.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qstring.h>

#include <type_traits>

QT_BEGIN_NAMESPACE

namespace QtProtobuf {
Q_NAMESPACE_EXPORT(Q_PROTOBUF_EXPORT)

[[maybe_unused]] constexpr int InvalidFieldNumber = 0;

enum class WireTypes
{
    Unknown = -1,
    Varint = 0,
    Fixed64 = 1,
    LengthDelimited = 2,
    StartGroup = 3,
    EndGroup = 4,
    Fixed32 = 5
};
Q_ENUM_NS(WireTypes)

// The 'tag' template param exists only create a unique type
template<typename T, typename tag>
struct TransparentWrapper
{
    TransparentWrapper(T t_ = T()) : t(t_) { }
    T t;
    operator T &() { return t; }
    operator T() const { return t; }
    TransparentWrapper &operator=(T t_)
    {
        t = t_;
        return *this;
    }

    static T toType(TransparentWrapper t) { return t.t; }
    static TransparentWrapper fromType(T t) { return TransparentWrapper(t); }

    static QString toString(TransparentWrapper t) { return QString::number(t.t); }

private:
    friend constexpr TransparentWrapper qbswap(TransparentWrapper source)
    {
        return { QT_PREPEND_NAMESPACE(qbswap)(source.t) };
    }

    friend size_t qHash(TransparentWrapper key, size_t seed = 0) noexcept
    {
        return QT_PREPEND_NAMESPACE(qHash)(seed, key.t);
    }
};

using int32 = TransparentWrapper<int32_t, struct int_tag>;
using int64 = TransparentWrapper<int64_t, struct int_tag>;
using uint32 = uint32_t;
using uint64 = uint64_t;
using sint32 = int32_t;
using sint64 = int64_t;
using fixed32 = TransparentWrapper<uint32_t, struct fixed_tag>;
using fixed64 = TransparentWrapper<uint64_t, struct fixed_tag>;
using sfixed32 = TransparentWrapper<int32_t, struct fixed_tag>;
using sfixed64 = TransparentWrapper<int64_t, struct fixed_tag>;
using boolean = bool;
using int32List = QList<int32>;
using int64List = QList<int64>;
using uint32List = QList<uint32>;
using uint64List = QList<uint64>;
using sint32List = QList<sint32>;
using sint64List = QList<sint64>;
using fixed32List = QList<fixed32>;
using fixed64List = QList<fixed64>;
using sfixed32List = QList<sfixed32>;
using sfixed64List = QList<sfixed64>;
using floatList = QList<float>;
using doubleList = QList<double>;
using boolList = QList<bool>;

template<typename T>
using HasProtobufStaticPropertyOrdering = decltype(T::staticPropertyOrdering);

template <typename T>
using has_q_protobuf_object_macro = qxp::is_detected<HasProtobufStaticPropertyOrdering, T>;

template <typename T>
constexpr bool has_q_protobuf_object_macro_v = has_q_protobuf_object_macro<T>::value;

template <typename T>
inline constexpr bool IsProtobufScalarValueType = false;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::int32> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::int64> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::sint32> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::sint64> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::uint32> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::uint64> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::fixed32> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::fixed64> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::sfixed32> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::sfixed64> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<float> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<double> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QtProtobuf::boolean> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QString> = true;
template <>
constexpr inline bool IsProtobufScalarValueType<QByteArray> = true;

template <typename T>
using is_protobuf_scalar_value_type = std::bool_constant<IsProtobufScalarValueType<T>>;

template <typename T>
using is_protobuf_message = std::conjunction<std::negation<std::is_pointer<T>>,
                                             std::is_base_of<QProtobufMessage, T>,
                                             has_q_protobuf_object_macro<T>>;

template <typename T>
using if_protobuf_message = std::enable_if_t<is_protobuf_message<T>::value, bool>;

template <typename T>
using is_protobuf_non_message = std::disjunction<std::is_enum<T>,
                                                 QtProtobuf::is_protobuf_scalar_value_type<T>>;

template <typename T>
using if_protobuf_non_message = std::enable_if_t<is_protobuf_non_message<T>::value, bool>;

template <typename T>
using is_protobuf_type = std::disjunction<is_protobuf_message<T>, is_protobuf_non_message<T>>;

template <typename T>
using if_protobuf_type = std::enable_if_t<is_protobuf_type<T>::value, bool>;

template <typename T>
using is_protobuf_message_without_ordering = std::conjunction<
    std::negation<std::is_pointer<T>>, std::is_base_of<QProtobufMessage, T>,
    std::negation<has_q_protobuf_object_macro<T>>>;

template <typename T>
using if_protobuf_message_without_ordering = std::enable_if_t<
    is_protobuf_message_without_ordering<T>::value, bool>;

template <typename T>
inline constexpr bool IsProtobufMapKeyType = false;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::int32> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::int64> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::sint32> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::sint64> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::uint32> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::uint64> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::fixed32> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::fixed64> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::sfixed32> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::sfixed64> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QtProtobuf::boolean> = true;
template <>
constexpr inline bool IsProtobufMapKeyType<QString> = true;

template <typename T>
using is_protobuf_map_key = std::bool_constant<IsProtobufMapKeyType<T>>;

template <typename T>
using if_protobuf_map_key = std::enable_if_t<is_protobuf_map_key<T>::value, bool>;

template <typename K, typename V>
using if_protobuf_map = std::enable_if_t<std::conjunction_v<QtProtobuf::is_protobuf_map_key<K>,
                                                            QtProtobuf::is_protobuf_type<V>>, bool>;

} // namespace QtProtobuf

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QtProtobuf::int32)
Q_DECLARE_METATYPE(QtProtobuf::int64)
Q_DECLARE_METATYPE(QtProtobuf::fixed32)
Q_DECLARE_METATYPE(QtProtobuf::fixed64)
Q_DECLARE_METATYPE(QtProtobuf::sfixed32)
Q_DECLARE_METATYPE(QtProtobuf::sfixed64)

#endif // QTPROTOBUFTYPES_H
