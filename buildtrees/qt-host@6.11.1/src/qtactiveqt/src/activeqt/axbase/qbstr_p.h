// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QBSTR_P_H
#define QBSTR_P_H

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

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <utility>
#include <oaidl.h>

class QBStr
{
public:
    QBStr() = default;

    ~QBStr() noexcept
    {
        free();
    }

    explicit QBStr(LPCOLESTR str) noexcept
    {
        if (str)
            m_str = ::SysAllocString(str);
        Q_ASSERT(m_str);
    }

    explicit QBStr(const QString &str) noexcept
    {
        m_str = ::SysAllocStringLen(reinterpret_cast<const OLECHAR *>(str.unicode()),
                                    static_cast<UINT>(str.length()));
        Q_ASSERT(m_str);
    }

    QBStr(const QBStr &str) noexcept : m_str{ str.copy() } { }

    QBStr(QBStr &&str) noexcept : m_str{ std::exchange(str.m_str, nullptr) } { }

    QBStr &operator=(const QBStr &rhs) noexcept
    {
        if (this != std::addressof(rhs))
            reset(rhs.copy());

        return *this;
    }

    QBStr &operator=(QBStr &&rhs) noexcept
    {
        if (this != std::addressof(rhs))
            reset(std::exchange(rhs.m_str, nullptr));

        return *this;
    }

    const BSTR &bstr() const noexcept
    {
        return m_str;
    }

    QString str() const
    {
        return QString::fromWCharArray(m_str);
    }

    [[nodiscard]] BSTR release() noexcept
    {
        return std::exchange(m_str, nullptr);
    }

    [[nodiscard]] BSTR *operator&() noexcept // NOLINT(google-runtime-operator)
    {
        Q_ASSERT(!m_str);
        return &m_str;
    }

    explicit operator bool() const noexcept
    {
        return m_str != nullptr;
    }

private:
    void free() noexcept
    {
        if (m_str)
            ::SysFreeString(m_str);
        m_str = nullptr;
    }

    void reset(BSTR str) noexcept
    {
        free();
        m_str = str;
    }

    [[nodiscard]] BSTR copy() const noexcept
    {
        if (!m_str)
            return nullptr;

        return ::SysAllocStringByteLen(reinterpret_cast<const char *>(m_str),
                                       ::SysStringByteLen(m_str));
    }

    BSTR m_str = nullptr;
};

#endif
