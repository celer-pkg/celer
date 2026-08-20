// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PROTOBUFFIELDPRESENCECHECKER_P_H
#define PROTOBUFFIELDPRESENCECHECKER_P_H
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

#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qtconfigmacros.h>
#include <QtCore/qxptype_traits.h>

#include <type_traits>

QT_BEGIN_NAMESPACE

namespace ProtobufFieldPresenceChecker
{
    template <typename T>
    using HasIsEmpty = decltype(&T::isEmpty);
    template <typename T>
    using has_is_empty_method = qxp::is_detected<HasIsEmpty, T>;

    template <typename T>
    constexpr inline bool IsProtobufTrivialScalarValueType = false;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::int32> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::int64> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::sint32> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::sint64> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::uint32> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::uint64> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::fixed32> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::fixed64> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::sfixed32> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::sfixed64> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<float> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<double> = true;
    template <>
    constexpr inline bool IsProtobufTrivialScalarValueType<QtProtobuf::boolean> = true;

    using Function = bool (*)(const QVariant &);

    template <typename T, std::enable_if_t<IsProtobufTrivialScalarValueType<T>, bool> = true>
    static bool isPresent(const QVariant &value)
    {
        return value.value<T>() != T{};
    }

    template <typename T, std::enable_if_t<has_is_empty_method<T>::value, bool> = true>
    static bool isPresent(const QVariant &value)
    {
        return !value.value<T>().isEmpty();
    }
}


QT_END_NAMESPACE

#endif // PROTOBUFFIELDPRESENCECHECKER_P_H
