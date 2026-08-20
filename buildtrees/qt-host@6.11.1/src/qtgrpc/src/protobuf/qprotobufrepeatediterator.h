// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFREPEATEDITERATOR_H
#define QPROTOBUFREPEATEDITERATOR_H

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qtprotobuftypes.h>
#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QProtobufMessage;
class QProtobufRepeatedIterator;

namespace QtProtobufPrivate {
template <typename T, QtProtobuf::if_protobuf_message<T> = true>
class ListIterator
{
public:
    ~ListIterator() = default;

    bool hasNext() const noexcept { return m_it != m_list->end(); }
    QProtobufMessage *next()
    {
        Q_ASSERT(m_it != m_list->end());
        return &*m_it++;
    }

    QProtobufMessage *addNext() { return &m_list->emplace_back(); }

    /* protobuf allows having elements in the repeated fields that are failed to deserialize */
    void push() noexcept { }

private:
    Q_DISABLE_COPY_MOVE(ListIterator)

    friend class QT_PREPEND_NAMESPACE(QProtobufRepeatedIterator);

    explicit ListIterator(QList<T> &list) : m_list(&list), m_it(m_list->begin()) { }
    QList<T> *m_list = nullptr;
    typename QList<T>::Iterator m_it;
};

template <typename K, typename V, QtProtobuf::if_protobuf_map<K, V> = true>
class MapIterator
{
    using MapEntry = QProtobufMapEntry<K, V>;
    static_assert(!std::is_pointer_v<typename MapEntry::KeyType>, "Map key must not be message");

public:
    ~MapIterator() { delete m_mapEntry; }

    bool hasNext() const noexcept { return m_it != m_hash->end(); }
    QProtobufMessage *next()
    {
        Q_ASSERT(m_it != m_hash->end());
        m_mapEntry->setKey(m_it.key());
        if constexpr (std::is_pointer_v<typename MapEntry::ValueType>)
            m_mapEntry->setValue(&m_it.value());
        else
            m_mapEntry->setValue(m_it.value());
        ++m_it;
        return m_mapEntry;
    }

    QProtobufMessage *addNext()
    {
        m_mapEntry->setKey({});
        m_mapEntry->setValue({});
        return m_mapEntry;
    }
    void push()
    {
        auto it = m_hash->emplace(m_mapEntry->key());
        if constexpr (std::is_pointer_v<typename MapEntry::ValueType>)
            *it = std::move(*m_mapEntry->value());
        else
            *it = m_mapEntry->value();
    }

private:
    Q_DISABLE_COPY_MOVE(MapIterator)

    friend class QT_PREPEND_NAMESPACE(QProtobufRepeatedIterator);

    explicit MapIterator(QHash<K, V> &hash)
        : m_hash(&hash), m_it(m_hash->begin()), m_mapEntry(new MapEntry)
    {
    }
    QHash<K, V> *m_hash = nullptr;
    typename QHash<K, V>::Iterator m_it;
    MapEntry *m_mapEntry;
};
}

class QProtobufRepeatedIterator
{
    enum class Operation { HasNext, Next, AddNext, Push, Deleter };
    using ImplFn = void (*)(Operation op, void *data, void **args, void *ret);

    template <typename T>
    static constexpr ImplFn MakeImpl()
    {
        return [](Operation op, void *data, void **, void *ret) {
            switch (op) {
            case Operation::HasNext:
                *static_cast<bool *>(ret) = static_cast<const T *>(data)->hasNext();
                break;
            case Operation::Next:
                *static_cast<QProtobufMessage **>(ret) = static_cast<T *>(data)->next();
                break;
            case Operation::AddNext:
                *static_cast<QProtobufMessage **>(ret) = static_cast<T *>(data)->addNext();
                break;
            case Operation::Push:
                static_cast<T *>(data)->push();
                break;
            case Operation::Deleter:
                static_cast<T *>(data)->~T();
                break;
            }
        };
    }
public:
    QProtobufRepeatedIterator() = default;
    ~QProtobufRepeatedIterator()
    {
        if (m_impl)
            m_impl(Operation::Deleter, &m_data, nullptr, nullptr);
    }

    QProtobufRepeatedIterator(QProtobufRepeatedIterator &&other) noexcept
        : m_data(std::exchange(other.m_data, {})), m_impl(std::exchange(other.m_impl, nullptr))
    {
    }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QProtobufRepeatedIterator)
    void swap(QProtobufRepeatedIterator &other) noexcept
    {
        std::swap(m_data, other.m_data);
        qt_ptr_swap(m_impl, other.m_impl);
    };

    [[nodiscard]] bool isValid() const noexcept { return m_impl != nullptr; }

    [[nodiscard]] bool hasNext() const noexcept
    {
        bool ret = false;
        if (m_impl)
            m_impl(Operation::HasNext, &m_data, nullptr, &ret);
        return ret;
    }

    [[nodiscard]] QProtobufMessage *next()
    {
        Q_ASSERT(m_impl);

        QProtobufMessage *ret;
        m_impl(Operation::Next, &m_data, nullptr, &ret);
        return ret;
    }

    [[nodiscard]] QProtobufMessage *addNext()
    {
        Q_ASSERT(m_impl);

        QProtobufMessage *ret;
        m_impl(Operation::AddNext, &m_data, nullptr, &ret);
        return ret;
    }

    void push()
    {
        Q_ASSERT(m_impl);

        m_impl(Operation::Push, &m_data, nullptr, nullptr);
    }

    template<typename T>
    static QProtobufRepeatedIterator fromList(QList<T> &list)
    {
        return QProtobufRepeatedIterator(list);
    }

    template<typename K, typename V>
    static QProtobufRepeatedIterator fromHash(QHash<K, V> &hash)
    {
        return QProtobufRepeatedIterator(hash);
    }

private:
    Q_DISABLE_COPY(QProtobufRepeatedIterator)

    template<typename K, typename V, typename Iterator = QtProtobufPrivate::MapIterator<K, V>>
    explicit QProtobufRepeatedIterator(QHash<K, V> &data) noexcept : m_impl(MakeImpl<Iterator>())
    {
        ::new (static_cast<void *>(&m_data)) Iterator(data);
    }

    template<typename T, typename Iterator = QtProtobufPrivate::ListIterator<T>>
    explicit QProtobufRepeatedIterator(QList<T> &data) noexcept : m_impl(MakeImpl<Iterator>())
    {
        ::new (static_cast<void *>(&m_data)) Iterator(data);
    }

    mutable struct { alignas(sizeof(void *)) unsigned char data[sizeof(void *) * 4]; } m_data;
    ImplFn m_impl = nullptr;

    static_assert(sizeof(QHash<int, int>::const_iterator) <= sizeof(m_data));
    static_assert(alignof(decltype(m_data)) == alignof(void *));
};

QT_END_NAMESPACE

#endif // QPROTOBUFREPEATEDITERATOR_H
