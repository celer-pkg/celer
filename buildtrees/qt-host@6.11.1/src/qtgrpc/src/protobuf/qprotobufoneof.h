// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFONEOF_H
#define QPROTOBUFONEOF_H

#if 0
#  pragma qt_class(QProtobufOneof)
#endif

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qmetatype.h>
#include <QtCore/qtclasshelpermacros.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

namespace QtProtobufPrivate {

class QProtobufOneofPrivate;
class QProtobufOneof final
{
    template <typename T>
    using is_oneof_value_type = std::disjunction<
        QtProtobuf::is_protobuf_message_without_ordering<T>,
        QtProtobuf::is_protobuf_non_message<T>>;
    template <typename T>
    using if_oneof_value_type = std::enable_if_t<is_oneof_value_type<T>::value, bool>;

    template <typename T>
    using is_oneof_compatible = std::disjunction<QtProtobuf::is_protobuf_message<T>,
                                                 is_oneof_value_type<T>>;
    template <typename T>
    using if_oneof_compatible = std::enable_if_t<is_oneof_compatible<T>::value, bool>;

public:
    Q_PROTOBUF_EXPORT  QProtobufOneof();
    Q_PROTOBUF_EXPORT  ~QProtobufOneof();
    Q_PROTOBUF_EXPORT  QProtobufOneof(const QProtobufOneof &other);
    Q_PROTOBUF_EXPORT  QProtobufOneof &operator=(const QProtobufOneof &other);
    QProtobufOneof(QProtobufOneof &&other) noexcept : d_ptr(std::exchange(other.d_ptr, {})) { }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QProtobufOneof)
    void swap(QProtobufOneof &other) noexcept
    {
        qt_ptr_swap(d_ptr, other.d_ptr);
    }

    template <typename T, if_oneof_compatible<T> = true>
    void setValue(const T &value, int fieldNumber)
    {
        setValue(QVariant::fromValue<T>(value), fieldNumber);
    }

    template<typename T, QtProtobuf::if_protobuf_message<T> = true>
    void setValue(T &&value, int fieldNumber)
    {
        setValue(QVariant::fromValue<T>(std::forward<T>(value)), fieldNumber);
    }

    template <typename T, if_oneof_value_type<T> = true>
    T value() const
    {
        ensureMetaType(QMetaType::fromType<T>(), rawValue().metaType());
        return rawValue().value<T>();
    }

    template <typename T, QtProtobuf::if_protobuf_message<T> = true>
    T *message()
    {
        ensureRawValue(QMetaType::fromType<T>());
        return static_cast<T *>(rawValue().data());
    }

    template <typename T, QtProtobuf::if_protobuf_message<T> = true>
    const T *message() const
    {
        ensureMetaType(QMetaType::fromType<T>(), rawValue().metaType());
        return static_cast<const T *>(rawValue().data());
    }

    template <typename T, if_oneof_value_type<T> = true>
    bool isEqual(const T &otherValue, int fieldNumber) const
    {
        return this->fieldNumber() == fieldNumber
                && QMetaType::fromType<T>() == rawValue().metaType() && value<T>() == otherValue;
    }

    template <typename T, QtProtobuf::if_protobuf_message<T> = true>
    bool isEqual(const T &otherValue, int fieldNumber) const
    {
        if (this->fieldNumber() != fieldNumber
            || QMetaType::fromType<T>() != rawValue().metaType()) {
            return false;
        }

        const auto *messagePtr = message<T>();
        return messagePtr && *messagePtr == otherValue;
    }

    Q_PROTOBUF_EXPORT int fieldNumber() const;
    Q_PROTOBUF_EXPORT bool holdsField(int fieldNumber) const;

private:
    Q_ALWAYS_INLINE static void ensureMetaType(QMetaType lhs, QMetaType rhs)
    {
        Q_ASSERT(lhs == rhs);
    }

    Q_PROTOBUF_EXPORT void ensureRawValue(QMetaType metaType);

    friend Q_PROTOBUF_EXPORT bool comparesEqual(const QProtobufOneof &lhs,
                                                const QProtobufOneof &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QProtobufOneof)

    friend class QT_PREPEND_NAMESPACE(QProtobufMessage);

    Q_PROTOBUF_EXPORT void setValue(const QVariant &value, int fieldNumber);
    Q_PROTOBUF_EXPORT const QVariant &rawValue() const;
    QVariant &rawValue()
    {
        return const_cast<QVariant &>(std::as_const(*this).rawValue());
    }

    QProtobufOneofPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QProtobufOneof)
};
} // namespace QtProtobufPrivate

QT_END_NAMESPACE

#endif // QPROTOBUFONEOF_H
