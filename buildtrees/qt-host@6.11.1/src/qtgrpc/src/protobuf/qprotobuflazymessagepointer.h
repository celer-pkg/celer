// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFLAZYMESSAGEPOINTER_H
#define QPROTOBUFLAZYMESSAGEPOINTER_H

#include <QtProtobuf/qtprotobufexports.h>

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qtprotobuftypes.h>

QT_BEGIN_NAMESPACE
namespace QtProtobufPrivate {
class QProtobufLazyMessagePointerBase
{
    Q_DISABLE_COPY(QProtobufLazyMessagePointerBase)
    mutable QProtobufMessage *m_ptr = nullptr;

protected:
    QProtobufLazyMessagePointerBase() noexcept = default;
    explicit QProtobufLazyMessagePointerBase(QProtobufMessage *ptr) noexcept : m_ptr(ptr) { }
    QProtobufLazyMessagePointerBase(QProtobufLazyMessagePointerBase &&other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr))
    {
    }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QProtobufLazyMessagePointerBase)

    QProtobufMessage *message() const noexcept { return m_ptr; }
    void setMessage(QProtobufMessage *msg) const noexcept{ m_ptr = msg; }

    ~QProtobufLazyMessagePointerBase() { QProtobufMessageDeleter()(m_ptr); }

    void swap(QProtobufLazyMessagePointerBase &other) noexcept { qt_ptr_swap(m_ptr, other.m_ptr); }
    explicit operator bool() const noexcept { return message() != nullptr; }
};

template <typename T, QtProtobuf::if_protobuf_message<T> = true>
class QProtobufLazyMessagePointer : private QProtobufLazyMessagePointerBase
{
    T *messageAsT() const { return static_cast<T *>(message()); }

public:
    QProtobufLazyMessagePointer() noexcept = default;
    explicit QProtobufLazyMessagePointer(T *p) noexcept { setMessage(p); }
    QProtobufLazyMessagePointer(const QProtobufLazyMessagePointer &other) noexcept = default;
    QProtobufLazyMessagePointer &
    operator=(const QProtobufLazyMessagePointer &other) noexcept = default;
    QProtobufLazyMessagePointer(QProtobufLazyMessagePointer &&other) noexcept = default;
    QProtobufLazyMessagePointer &operator=(QProtobufLazyMessagePointer &&other) noexcept = default;
    ~QProtobufLazyMessagePointer() = default; // We don't own anything + base is not virtual

    T &operator*() const { return *get(); }
    T *operator->() const { return get(); }
    T *get() const
    {
        if (!*this)
            setMessage(new T);
        return messageAsT();
    }

    void reset(T *p = nullptr) noexcept
    {
        Q_ASSERT(!p || message() != p);
        QProtobufLazyMessagePointer(p).swap(*this);
    }

    void swap(QProtobufLazyMessagePointer &other) noexcept
    {
        QProtobufLazyMessagePointerBase::swap(other);
    }

    using QProtobufLazyMessagePointerBase::operator bool;

private:
    friend bool operator==(const QProtobufLazyMessagePointer &lhs,
                           const QProtobufLazyMessagePointer &rhs) noexcept
    {
        T *lhsPtr = lhs.messageAsT();
        T *rhsPtr = rhs.messageAsT();
        return (lhsPtr == nullptr && rhsPtr == nullptr)
            || (lhsPtr != nullptr && rhsPtr != nullptr && *lhsPtr == *rhsPtr);
    }
    friend bool operator!=(const QProtobufLazyMessagePointer &lhs,
                           const QProtobufLazyMessagePointer &rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }
};
} // namespace QtProtobufPrivate

QT_END_NAMESPACE
#endif // QPROTOBUFLAZYMESSAGEPOINTER_H
