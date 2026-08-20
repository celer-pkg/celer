// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROTOBUFSCALARJSONSERIALIZERS_P_H
#define PROTOBUFSCALARJSONSERIALIZERS_P_H
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

#include <QtProtobuf/qtprotobufexports.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/private/qnumeric_p.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qtconfigmacros.h>

#include <limits>

QT_BEGIN_NAMESPACE

namespace ProtobufScalarJsonSerializers {
// Tests if V is JSON compatible integer
// int32 | int64 | uint32 | sint32 | sint64 | fixed32 | sfixed32 | sfixed64
template <typename T>
constexpr inline bool IsJsonInt = false;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::int32> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::int64> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::uint32> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::sint32> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::sint64> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::fixed32> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::sfixed32> = true;
template <>
constexpr inline bool IsJsonInt<QtProtobuf::sfixed64> = true;

template <typename T>
using if_json_int = std::enable_if_t<IsJsonInt<T>, bool>;

// Tests if V is JSON incompatible 64-bit unsigned integer
// uint64 | fixed64
template <typename T>
constexpr inline bool IsJsonInt64 = false;
template <>
constexpr inline bool IsJsonInt64<QtProtobuf::uint64> = true;
template <>
constexpr inline bool IsJsonInt64<QtProtobuf::fixed64> = true;

template <typename T>
using if_json_int64 = std::enable_if_t<IsJsonInt64<T>, bool>;

// Tests if V is JSON compatible floating point number
// float | double
template <typename T>
constexpr inline bool IsJsonFloatingPoint = false;
template <>
constexpr inline bool IsJsonFloatingPoint<float> = true;
template <>
constexpr inline bool IsJsonFloatingPoint<double> = true;

template <typename T>
using if_json_floating_point = std::enable_if_t<IsJsonFloatingPoint<T>, bool>;

// Special value strings
constexpr inline QLatin1StringView Infinity("Infinity");
constexpr inline QLatin1StringView NegInfinity("-Infinity");
constexpr inline QLatin1StringView NaN("NaN");

constexpr inline QByteArrayView InfinityLower("infinity");
constexpr inline QByteArrayView NegInfinityLower("-infinity");
constexpr inline QByteArrayView NaNLower("nan");

constexpr inline QLatin1StringView True("true");
constexpr inline QLatin1StringView False("false");

template <typename T, if_json_int<T> = true>
QJsonValue serialize(T propertyValue)
{
    return QJsonValue(qint64(propertyValue));
}

template <typename T, if_json_int64<T> = true>
QJsonValue serialize(T propertyValue)
{
    return QJsonValue(QString::number(propertyValue));
}

template <typename T, if_json_floating_point<T> = true>
QJsonValue serialize(T propertyValue)
{
    if (propertyValue == -std::numeric_limits<T>::infinity())
        return QJsonValue(NegInfinity);

    if (propertyValue == std::numeric_limits<T>::infinity())
        return QJsonValue(Infinity);

    if (propertyValue != propertyValue)
        return QJsonValue(NaN);

    return QJsonValue(propertyValue);
}

inline QJsonValue serialize(bool propertyValue)
{
    return QJsonValue(propertyValue);
}

inline QJsonValue serialize(const QString &propertyValue)
{
    return QJsonValue(propertyValue);
}

inline QJsonValue serialize(const QByteArray &propertyValue)
{
    return QJsonValue(QString::fromUtf8(propertyValue.toBase64()));
}

template <typename L>
QJsonValue serializeList(const QVariant &propertyValue)
{
    QJsonArray arr;
    const L listValue = propertyValue.value<L>();
    for (const auto &value : listValue)
        arr.append(serialize(value));

    return QJsonValue(arr);
}

template <typename T>
QJsonValue serializeCommon(const QVariant &propertyValue)
{
    return serialize(propertyValue.value<T>());
}

Q_PROTOBUF_EXPORT bool validateJsonNumberString(const QString &input);

template <typename T, if_json_int<T> = true>
T deserialize(const QJsonValue &value, bool &ok)
{
    auto variantValue = value.toVariant();
    qint64 raw = 0;
    switch (variantValue.metaType().id()) {
    case QMetaType::QString:
        if (!validateJsonNumberString(variantValue.toString()))
            ok = false;
        else
            raw = variantValue.toLongLong(&ok);
        break;
    case QMetaType::LongLong:
        raw = variantValue.toLongLong(&ok);
        break;
    case QMetaType::Double: {
        double d = value.toDouble();
        ok = convertDoubleTo(d, &raw) && double(raw) == d;
    } break;
    default:
        break;
    }

    // For types that "smaller" than qint64 we need to check if the value fits its limits range
    if constexpr (sizeof(T) != sizeof(qint64)) {
        constexpr auto Limits = []() {
            if constexpr (std::is_same_v<T, QtProtobuf::sfixed32>
                          || std::is_same_v<T, QtProtobuf::int32>)
                return std::numeric_limits<qint32>{};
            else if constexpr (std::is_same_v<T, QtProtobuf::fixed32>)
                return std::numeric_limits<quint32>{};
            else
                return std::numeric_limits<T>{};
        }();
        ok = ok && (raw >= Limits.min() && raw <= Limits.max());
    }

    return ok ? T(raw) : T{};
}

template <typename T, if_json_int64<T> = true>
T deserialize(const QJsonValue &value, bool &ok)
{
    quint64 raw = 0;
    auto variantValue = value.toVariant();
    switch (variantValue.metaType().id()) {
    case QMetaType::QString:
    case QMetaType::LongLong:
        // Here we attempt converting the value to ULongLong
        raw = variantValue.toULongLong(&ok);
        break;
    case QMetaType::Double: {
        double d = value.toDouble();
        ok = convertDoubleTo(d, &raw) && double(raw) == d;
    } break;
    default:
        break;
    }
    return T(raw);
}

template <typename T, if_json_floating_point<T> = true>
T deserialize(const QJsonValue &value, bool &ok)
{
    ok = true;
    QByteArray data = value.toVariant().toByteArray();
    if (data.compare(NegInfinityLower, Qt::CaseInsensitive) == 0)
        return -std::numeric_limits<T>::infinity();

    if (data.compare(InfinityLower, Qt::CaseInsensitive) == 0)
        return std::numeric_limits<T>::infinity();

    if (data.compare(NaNLower, Qt::CaseInsensitive) == 0)
        return T(NAN);

    if constexpr (std::is_same_v<T, float>)
        return data.toFloat(&ok);
    else
        return data.toDouble(&ok);
}

template <typename T, std::enable_if_t<std::is_same_v<T, bool>, bool> = true>
bool deserialize(const QJsonValue &value, bool &ok)
{
    if (value.isBool()) {
        ok = true;
        return value.toBool();
    } else if (value.isString()) {
        if (value.toString() == True) {
            ok = true;
            return true;
        } else if (value.toString() == False) {
            ok = true;
            return false;
        }
    }
    return false;
}

template <typename T, std::enable_if_t<std::is_same_v<T, QString>, bool> = true>
QString deserialize(const QJsonValue &value, bool &ok)
{
    ok = value.isString();
    return value.toString();
}

template <typename T, std::enable_if_t<std::is_same_v<T, QByteArray>, bool> = true>
QByteArray deserialize(const QJsonValue &value, bool &ok)
{
    QByteArray data = value.toVariant().toByteArray();
    if (value.isString()) {
        ok = true;
        return QByteArray::fromBase64(data);
    }
    return {};
}

template <typename L /*list*/, typename T /*element*/>
QVariant deserializeList(const QJsonValue &value, bool &ok)
{
    if (!value.isArray()) {
        ok = false;
        return {};
    }

    L list;
    const QJsonArray array = value.toArray();
    for (const auto &arrayValue : array) {
        ok = false;
        T decoded = deserialize<T>(arrayValue, ok);
        if (!ok)
            break;
        list.append(decoded);
    }
    return QVariant::fromValue(list);
}

template <typename T>
QVariant deserializeCommon(const QJsonValue &value, bool &ok)
{
    ok = false;
    return QVariant::fromValue<T>(deserialize<T>(value, ok));
}

} // namespace ProtobufScalarJsonSerializers
QT_END_NAMESPACE

#endif // PROTOBUFSCALARJSONSERIALIZERS_P_H
