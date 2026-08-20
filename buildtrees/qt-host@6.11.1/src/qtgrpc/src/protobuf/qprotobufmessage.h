// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFMESSAGE_H
#define QPROTOBUFMESSAGE_H

#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qanystringview.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qbytearrayview.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qtmetamacros.h>

QT_BEGIN_NAMESPACE

class QAbstractProtobufSerializer;
struct QMetaObject;
class QVariant;

class QProtobufMessage;
struct QProtobufMessageDeleter {
    Q_PROTOBUF_EXPORT void operator()(QProtobufMessage *ptr) const noexcept;
};
using QProtobufMessagePointer = std::unique_ptr<QProtobufMessage, QProtobufMessageDeleter>;

namespace QtProtobufPrivate {
struct QProtobufPropertyOrdering;
template <class To, class From>
[[nodiscard]] To qprotobufmessage_cast_helper(From *from) noexcept;
}

class QProtobufMessagePrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QProtobufMessagePrivate)

class QProtobufMessage
{
    Q_GADGET_EXPORT(Q_PROTOBUF_EXPORT)
public:
    Q_PROTOBUF_EXPORT QVariant property(QAnyStringView propertyName) const;
    Q_PROTOBUF_EXPORT bool setProperty(QAnyStringView propertyName, const QVariant &value);
    Q_PROTOBUF_EXPORT bool setProperty(QAnyStringView propertyName, QVariant &&value);

    [[nodiscard]]
    Q_PROTOBUF_EXPORT static QProtobufMessagePointer constructByName(const QString &messageType);

    Q_PROTOBUF_EXPORT QList<qint32> unknownFieldNumbers() const;
    Q_PROTOBUF_EXPORT QList<QByteArray> unknownFieldData(qint32 field) const;

    Q_PROTOBUF_EXPORT QByteArray serialize(QAbstractProtobufSerializer *serializer) const;
    Q_PROTOBUF_EXPORT bool deserialize(QAbstractProtobufSerializer *serializer,
                                       QByteArrayView data);

    Q_PROTOBUF_EXPORT const QtProtobufPrivate::QProtobufPropertyOrdering *propertyOrdering() const;

protected:
    Q_PROTOBUF_EXPORT
    explicit QProtobufMessage(const QMetaObject *metaObject,
                              const QtProtobufPrivate::QProtobufPropertyOrdering *ordering);
    Q_PROTOBUF_EXPORT explicit QProtobufMessage(QProtobufMessagePrivate &dd);

    Q_PROTOBUF_EXPORT ~QProtobufMessage();
    Q_PROTOBUF_EXPORT QProtobufMessage(const QProtobufMessage &other);
    Q_PROTOBUF_EXPORT QProtobufMessage &operator=(const QProtobufMessage &other);
    QProtobufMessage(QProtobufMessage &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QProtobufMessage)

    void swap(QProtobufMessage &other) noexcept
    {
        d_ptr.swap(other.d_ptr);
    }
private:
    Q_PROTOBUF_EXPORT const QMetaObject *metaObject() const;
    friend Q_PROTOBUF_EXPORT bool comparesEqual(const QProtobufMessage &lhs,
                                                const QProtobufMessage &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QProtobufMessage);
    template <class To, class From>
    friend To QtProtobufPrivate::qprotobufmessage_cast_helper(From *from) noexcept;

    friend struct QProtobufMessageDeleter;

    Q_DECLARE_PRIVATE(QProtobufMessage)

protected:
    QExplicitlySharedDataPointer<QProtobufMessagePrivate> d_ptr;
};

class QProtobufMapEntryBasePrivate;
class QProtobufMapEntryBase : public QProtobufMessage
{
    Q_GADGET_EXPORT(Q_PROTOBUF_EXPORT)
    friend QMetaObject *buildMetaObject(QMetaType, QMetaType);
    using StaticMetaCallFn = void (*)(QObject *, QMetaObject::Call, int, void **);

protected:
    Q_PROTOBUF_EXPORT explicit QProtobufMapEntryBase(QMetaType key, QMetaType value,
                                                     StaticMetaCallFn metaCall);
    Q_PROTOBUF_EXPORT ~QProtobufMapEntryBase();

private:
    Q_DECLARE_PRIVATE(QProtobufMapEntryBase)
    Q_DISABLE_COPY_MOVE(QProtobufMapEntryBase)
};

namespace QtProtobufPrivate {

template <class To, class From>
To qprotobufmessage_cast_helper(From *from) noexcept
{
    static_assert(std::is_same_v<std::remove_cv_t<From>, QProtobufMessage>,
                  "From must always be of type QProtobufMessage");

    using ToType = std::remove_cv_t<std::remove_pointer_t<To>>;

    if (!from)
        return nullptr;

    const auto *fmo = from->metaObject();
    const auto *tmo = &ToType::staticMetaObject;

    if constexpr (std::is_final_v<ToType>) {
        if (fmo == tmo)
            return static_cast<To>(from);
    } else {
        // For safe downcast, the actual object type (fmo) must be To or inherit from To.
        if (fmo->inherits(tmo))
            return static_cast<To>(from);
    }

    return nullptr;
}

} // namespace QtProtobufPrivate

template <class To>
[[nodiscard]] To qprotobufmessage_cast(QProtobufMessage *from)
{
    using ConstTo = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<To>>>;
    auto fromCasted = qprotobufmessage_cast<ConstTo>(static_cast<const QProtobufMessage *>(from));
    return const_cast<To>(fromCasted);
}

template <class To>
[[nodiscard]] To qprotobufmessage_cast(const QProtobufMessage *from)
{
    static_assert(std::is_pointer_v<To>,
                  "qprotobufmessage_cast requires the target to be a pointer type.");
    using ToType = std::remove_cv_t<std::remove_pointer_t<To>>;

    static_assert(sizeof(ToType) > 0,
                  "qprotobufmessage_cast requires the target type to be complete.");
    static_assert(std::is_base_of_v<QProtobufMessage, ToType>,
                  "qprotobufmessage_cast requires the target type to be a subclass of "
                  "QProtobufMessage");
    static_assert(std::is_const_v<std::remove_pointer_t<To>>,
                  "qprotobufmessage_cast cannot cast away constness (use const_cast)");

    if constexpr (std::is_same_v<QProtobufMessage, ToType>)
        return from;
    else
        return QtProtobufPrivate::qprotobufmessage_cast_helper<To>(from);
}

template <typename Key, typename Value>
class QProtobufMapEntry : public QProtobufMapEntryBase
{
    static void metaCall(QObject *object, QMetaObject::Call call, int index, void **args)
    {
        static constexpr bool needsHasValue = ResolveType<Value>::NeedsHasFunction;

        QProtobufMapEntry *entry = reinterpret_cast<QProtobufMapEntry *>(object);
        if (call == QMetaObject::ReadProperty) {
            if (index == 0)
                *reinterpret_cast<KeyType *>(args[0]) = entry->key();
            if (index == 1)
                *reinterpret_cast<ValueType *>(args[0]) = entry->value();
            if constexpr (needsHasValue) {
                if (index == 2)
                    *reinterpret_cast<bool *>(args[0]) = entry->hasValue();
            }
        } else if (call == QMetaObject::WriteProperty) {
            if (index == 0) {
                entry->setKey(*reinterpret_cast<KeyType *>(args[0]));
            } else if (index == 1 || index == 2 ) {
                entry->setValue(*reinterpret_cast<ValueType *>(args[0]));
            }
        }
    }

    template <typename T, typename = void>
    struct ResolveType {
        using type = std::remove_const_t<T>;
        static constexpr bool NeedsHasFunction = false;
    };
    template<typename T>
    struct ResolveType<T, std::enable_if_t<std::is_base_of_v<QProtobufMessage, T>>> {
        using type = std::conditional_t<std::is_const_v<T>, T const *,  T *>;
        static constexpr bool NeedsHasFunction = true;
    };

public:
    using KeyType = Key;
    using ValueType = typename ResolveType<Value>::type;

    QProtobufMapEntry()
        : QProtobufMapEntryBase(QMetaType::fromType<KeyType>(), QMetaType::fromType<ValueType>(),
                                &QProtobufMapEntry::metaCall)
    {
    }
    ~QProtobufMapEntry() {
        if constexpr (std::is_pointer_v<ValueType>) {
            if (m_ownsValue)
                delete m_value;
        }
    }

    const KeyType &key() const &
    {
        return m_key;
    }
    KeyType key() &&
    {
        return std::move(m_key);
    }

    const ValueType &value() const &
    {
        ensureValue();
        return m_value;
    }
    ValueType value() &&
    {
        ensureValue();
        m_ownsValue = false;
        return std::move(m_value);
    }
    bool hasValue() const
    {
        if constexpr (ResolveType<Value>::NeedsHasFunction)
            return m_value != nullptr;
        return true;
    }

    void setKey(const KeyType &key)
    {
        m_key = key;
    }
    void setKey(KeyType &&key)
    {
        m_key = std::move(key);
    }
    void setValue(const ValueType &value)
    {
        if (m_value == value)
            return;
        cleanupValue();
        m_value = value;
    }
    void setValue(ValueType &&value)
    {
        if (m_value == value)
            return;
        cleanupValue();
        m_value = std::move(value);
    }

private:
    void ensureValue() const
    {
        if constexpr (std::is_pointer_v<ValueType>) {
            if (!m_value) {
                m_value = new Value;
                m_ownsValue = true;
            }
        }
    }

    void cleanupValue() const
    {
        if constexpr (std::is_pointer_v<ValueType>) {
            if (m_ownsValue) {
                delete m_value;
                m_value = nullptr;
                m_ownsValue = false;
            }
        }
    }

    mutable KeyType m_key{};
    mutable ValueType m_value{};
    mutable bool m_ownsValue = false;

    Q_DISABLE_COPY_MOVE(QProtobufMapEntry)
};

QT_END_NAMESPACE

#endif // QPROTOBUFMESSAGE_H
