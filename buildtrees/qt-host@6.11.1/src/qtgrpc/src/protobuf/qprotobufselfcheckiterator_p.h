// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFSELFCHECKITERATOR_P_H
#define QPROTOBUFSELFCHECKITERATOR_P_H

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

#include <QtCore/qnumeric.h>
#include <QtCore/qbytearray.h>

#include <iterator>

QT_BEGIN_NAMESPACE

class QProtobufSelfcheckIterator
{
public:
    using difference_type = QByteArray::difference_type;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = QByteArray::value_type;
    using pointer = QByteArray::pointer;
    using reference = QByteArray::reference;

    QProtobufSelfcheckIterator() = default;

    static QProtobufSelfcheckIterator fromView(QByteArrayView container)
    {
        QProtobufSelfcheckIterator iter(container);
        return iter;
    }

    QProtobufSelfcheckIterator(const QProtobufSelfcheckIterator &other) = default;
    QProtobufSelfcheckIterator &operator=(const QProtobufSelfcheckIterator &other) = default;

    explicit operator QByteArray::const_iterator &() { return m_it; }
    explicit operator QByteArray::const_iterator() const { return m_it; }

    char operator*()
    {
        Q_ASSERT(isValid());
        return *m_it;
    }

    bool isValid() const noexcept { return m_containerBegin <= m_it && m_it <= m_containerEnd; }

    QProtobufSelfcheckIterator &operator++() noexcept
    {
        if (!isValid()) {
            qWarning("Deserialization failed: Unexpected end of data.");
            return *this;
        }
        ++m_it;
        return *this;
    }

    QProtobufSelfcheckIterator operator++(int) noexcept
    {
        QProtobufSelfcheckIterator old(*this);
        operator++();
        return old;
    }

    QProtobufSelfcheckIterator &operator--() noexcept
    {
        if (!isValid()) {
            qWarning("Deserialization failed: Unexpected end of data.");
            return *this;
        }

        --m_it;
        return *this;
    }

    QProtobufSelfcheckIterator operator--(int) noexcept
    {
        QProtobufSelfcheckIterator old(*this);
        operator--();
        return old;
    }

    QProtobufSelfcheckIterator &operator+=(qsizetype count) noexcept
    {
        if (!isValid()) {
            qWarning("Deserialization failed: Unexpected end of data.");
            return *this;
        }
        m_it += count;
        return *this;
    }

    QProtobufSelfcheckIterator &operator-=(qsizetype count) noexcept
    {
        if (!isValid()) {
            qWarning("Deserialization failed: Unexpected end of data.");
            return *this;
        }
        m_it -= count;
        return *this;
    }

    const char *data() const { return m_it; }
    qsizetype bytesLeft() const { return isValid() ? std::distance(m_it, m_containerEnd) : 0; }

private:
    explicit QProtobufSelfcheckIterator(QByteArrayView container)
        : m_containerBegin(container.begin()), m_containerEnd(container.end()),
          m_it(container.begin())
    {
    }

    friend bool operator==(const QProtobufSelfcheckIterator &lhs,
                           const QProtobufSelfcheckIterator &rhs) noexcept
    {
        Q_ASSERT(lhs.m_containerBegin == rhs.m_containerBegin);
        Q_ASSERT(lhs.m_containerEnd == rhs.m_containerEnd);
        return lhs.m_it == rhs.m_it;
    }
    friend bool operator!=(const QProtobufSelfcheckIterator &lhs,
                           const QProtobufSelfcheckIterator &rhs) noexcept
    {
        Q_ASSERT(lhs.m_containerBegin == rhs.m_containerBegin);
        Q_ASSERT(lhs.m_containerEnd == rhs.m_containerEnd);
        return lhs.m_it != rhs.m_it;
    }
    friend bool operator==(const QProtobufSelfcheckIterator &lhs,
                           const QByteArray::const_iterator &other) noexcept
    {
        Q_ASSERT(lhs.m_containerBegin <= other && other <= lhs.m_containerEnd);
        return lhs.m_it == other;
    }
    friend bool operator!=(const QProtobufSelfcheckIterator &lhs,
                           const QByteArray::const_iterator &other) noexcept
    {
        Q_ASSERT(lhs.m_containerBegin <= other && other <= lhs.m_containerEnd);
        return lhs.m_it != other;
    }

    friend qint64 operator-(const QProtobufSelfcheckIterator &lhs,
                            const QProtobufSelfcheckIterator &rhs) noexcept
    {
        Q_ASSERT(lhs.m_containerBegin == rhs.m_containerBegin);
        return lhs.m_it - rhs.m_it;
    }

    QByteArrayView::const_iterator m_containerBegin;
    QByteArrayView::const_iterator m_containerEnd;
    QByteArrayView::const_iterator m_it;
};

inline QProtobufSelfcheckIterator operator+(const QProtobufSelfcheckIterator &it, qsizetype length)
{
    QProtobufSelfcheckIterator copy = it;
    copy += length;
    return copy;
}

inline QProtobufSelfcheckIterator operator-(const QProtobufSelfcheckIterator &it, qsizetype length)
{
    QProtobufSelfcheckIterator copy = it;
    copy -= length;
    return copy;
}

QT_END_NAMESPACE

#endif // QPROTOBUFSELFCHECKITERATOR_P_H
