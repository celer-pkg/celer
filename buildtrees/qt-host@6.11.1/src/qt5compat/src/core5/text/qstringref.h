// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2019 Intel Corporation.
// Copyright (C) 2019 Mail.ru Group.
// Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Marc Mutz <marc.mutz@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser
// ### TODO: de-inline non-trivial stuff

#ifndef QSTRINGREF_H
#define QSTRINGREF_H

#if defined(QT_NO_CAST_FROM_ASCII) && defined(QT_RESTRICTED_CAST_FROM_ASCII)
#error QT_NO_CAST_FROM_ASCII and QT_RESTRICTED_CAST_FROM_ASCII must not be defined at the same time
#endif

#include <QtCore/qchar.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qarraydata.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qhashfunctions.h>
#include <QtCore/qstringliteral.h>
#include <QtCore/qstringalgorithms.h>
#include <QtCore/qstringview.h>
#include <QtCore/qstringtokenizer.h>

#include <QtCore5Compat/qcore5global.h>

#include <iterator>

#ifdef truncate
#error qstringref.h must be included before any header file that defines truncate
#endif

QT_BEGIN_NAMESPACE

class Q_CORE5COMPAT_EXPORT QStringRef
{
    const QString *m_string;
    int m_position;
    int m_size;
public:
    typedef QString::size_type size_type;
    typedef QString::value_type value_type;
    typedef const QChar *const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef QString::const_pointer const_pointer;
    typedef QString::const_reference const_reference;

    constexpr QStringRef() noexcept
        : m_string(nullptr), m_position(0), m_size(0) { }
    inline QStringRef(const QString *string, int position, int size);
    inline QStringRef(const QString *string);

    inline const QString *string() const { return m_string; }
    inline int position() const { return m_position; }
    inline int size() const { return m_size; }
    inline int count() const { return m_size; }
    inline int length() const { return m_size; }

    int indexOf(const QString &str, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int indexOf(const QStringRef &str, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    Q_REQUIRED_RESULT int indexOf(QStringView s, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept
    { return int(QtPrivate::findString(*this, from, s, cs)); } // ### Qt6: qsizetype
    int indexOf(QChar ch, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int indexOf(QLatin1String str, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(const QStringRef &str, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(const QString &str, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(QChar ch, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(QLatin1String str, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    Q_REQUIRED_RESULT int lastIndexOf(QStringView s, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept
    { return int(QtPrivate::lastIndexOf(*this, from, s, cs)); } // ### Qt6: qsizetype

    inline bool contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline bool contains(const QStringRef &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline bool contains(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline bool contains(QLatin1String str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline bool contains(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept;

    int count(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(const QStringRef &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    Q_REQUIRED_RESULT
    QList<QStringRef> split(const QString &sep, Qt::SplitBehavior behavior = Qt::KeepEmptyParts,
                            Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    Q_REQUIRED_RESULT
    QList<QStringRef> split(QChar sep, Qt::SplitBehavior behavior = Qt::KeepEmptyParts,
                            Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    Q_REQUIRED_RESULT QStringRef left(int n) const;
    Q_REQUIRED_RESULT QStringRef right(int n) const;
    Q_REQUIRED_RESULT QStringRef mid(int pos, int n = -1) const;
    Q_REQUIRED_RESULT QStringRef chopped(int n) const
    { Q_ASSERT(n >= 0); Q_ASSERT(n <= size()); return left(size() - n); }

    void truncate(int pos) noexcept { m_size = qBound(0, pos, m_size); }
    void chop(int n) noexcept
    {
        if (n >= m_size)
            m_size = 0;
        else if (n > 0)
            m_size -= n;
    }

    bool isRightToLeft() const;

    Q_REQUIRED_RESULT bool startsWith(QStringView s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept
    { return QtPrivate::startsWith(*this, s, cs); }
    bool startsWith(QLatin1String s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool startsWith(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool startsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool startsWith(const QStringRef &c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    Q_REQUIRED_RESULT bool endsWith(QStringView s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept
    { return QtPrivate::endsWith(*this, s, cs); }
    bool endsWith(QLatin1String s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QStringRef &c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    inline operator QStringView() const {
        if (isNull())
            return {};
        return QStringView(m_string->data() + m_position, m_size);
    }

#ifdef QSTRINGVIEW_REFUSES_QSTRINGREF
    operator QAnyStringView() const noexcept { return QStringView{*this}; }
#else
    operator QAnyStringView() const noexcept { return operator QStringView(); }
#endif

    inline QStringRef &operator=(const QString *string);

    inline const QChar *unicode() const
    {
        static const char16_t _empty = 0;
        if (!m_string)
            return reinterpret_cast<const QChar *>(&_empty);
        return m_string->unicode() + m_position;
    }
    inline const QChar *data() const { return unicode(); }
    inline const QChar *constData() const {  return unicode(); }

    inline const_iterator begin() const { return unicode(); }
    inline const_iterator cbegin() const { return unicode(); }
    inline const_iterator constBegin() const { return unicode(); }
    inline const_iterator end() const { return unicode() + size(); }
    inline const_iterator cend() const { return unicode() + size(); }
    inline const_iterator constEnd() const { return unicode() + size(); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    inline const_reverse_iterator crbegin() const { return rbegin(); }
    inline const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    inline const_reverse_iterator crend() const { return rend(); }

    Q_REQUIRED_RESULT QByteArray toLatin1() const;
    Q_REQUIRED_RESULT QByteArray toUtf8() const;
    Q_REQUIRED_RESULT QByteArray toLocal8Bit() const;
    Q_REQUIRED_RESULT QList<uint> toUcs4() const;

    inline void clear() { m_string = nullptr; m_position = m_size = 0; }
    QString toString() const;
    inline bool isEmpty() const { return m_size == 0; }
    inline bool isNull() const { return m_string == nullptr || m_string->isNull(); }

    QStringRef appendTo(QString *string) const;

    inline const QChar at(int i) const
        { Q_ASSERT(uint(i) < uint(size())); return m_string->at(i + m_position); }
    QChar operator[](int i) const { return at(i); }
    Q_REQUIRED_RESULT QChar front() const { return at(0); }
    Q_REQUIRED_RESULT QChar back() const { return at(size() - 1); }

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    // ASCII compatibility
    QT_ASCII_CAST_WARN inline bool operator==(const char *s) const;
    QT_ASCII_CAST_WARN inline bool operator!=(const char *s) const;
    QT_ASCII_CAST_WARN inline bool operator<(const char *s) const;
    QT_ASCII_CAST_WARN inline bool operator<=(const char *s) const;
    QT_ASCII_CAST_WARN inline bool operator>(const char *s) const;
    QT_ASCII_CAST_WARN inline bool operator>=(const char *s) const;
#endif

    int compare(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept;
    int compare(const QStringRef &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept;
    int compare(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept
    { return QtPrivate::compareStrings(*this, QStringView(&c, 1), cs); }
    int compare(QLatin1String s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept;
#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    int compare(const QByteArray &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    { return QStringRef::compare_helper(unicode(), size(), s.data(), qstrnlen(s.data(), s.size()), cs); }
#endif
    static int compare(const QStringRef &s1, const QString &s2,
                       Qt::CaseSensitivity = Qt::CaseSensitive) noexcept;
    static int compare(const QStringRef &s1, const QStringRef &s2,
                       Qt::CaseSensitivity = Qt::CaseSensitive) noexcept;
    static int compare(const QStringRef &s1, QLatin1String s2,
                       Qt::CaseSensitivity cs = Qt::CaseSensitive) noexcept;

    int localeAwareCompare(const QString &s) const;
    int localeAwareCompare(const QStringRef &s) const;
    static int localeAwareCompare(const QStringRef &s1, const QString &s2);
    static int localeAwareCompare(const QStringRef &s1, const QStringRef &s2);

    Q_REQUIRED_RESULT QStringRef trimmed() const;
    short  toShort(bool *ok = nullptr, int base = 10) const;
    ushort toUShort(bool *ok = nullptr, int base = 10) const;
    int toInt(bool *ok = nullptr, int base = 10) const;
    uint toUInt(bool *ok = nullptr, int base = 10) const;
    long toLong(bool *ok = nullptr, int base = 10) const;
    ulong toULong(bool *ok = nullptr, int base = 10) const;
    qlonglong toLongLong(bool *ok = nullptr, int base = 10) const;
    qulonglong toULongLong(bool *ok = nullptr, int base = 10) const;
    float toFloat(bool *ok = nullptr) const;
    double toDouble(bool *ok = nullptr) const;

    friend inline bool operator==(QChar, const QStringRef &) noexcept;
    friend inline bool operator<(QChar, const QStringRef &) noexcept;
    friend inline bool operator>(QChar, const QStringRef &) noexcept;

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    friend inline bool operator==(const char *s1, const QStringRef &s2);
    friend inline bool operator!=(const char *s1, const QStringRef &s2);
    friend inline bool operator<(const char *s1, const QStringRef &s2);
    friend inline bool operator>(const char *s1, const QStringRef &s2);
    friend inline bool operator<=(const char *s1, const QStringRef &s2);
    friend inline bool operator>=(const char *s1, const QStringRef &s2);
#endif

private:
    static int compare_helper(const QChar *data1, qsizetype length1,
                              const QChar *data2, qsizetype length2,
                              Qt::CaseSensitivity cs = Qt::CaseSensitive) noexcept;
    static int compare_helper(const QChar *data1, qsizetype length1,
                              const char *data2, qsizetype length2,
                              Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static int compare_helper(const QChar *data1, qsizetype length1,
                              QLatin1String s2,
                              Qt::CaseSensitivity cs = Qt::CaseSensitive) noexcept;
};
Q_DECLARE_TYPEINFO(QStringRef, Q_PRIMITIVE_TYPE);

namespace QtPrivate {
namespace Tok {
    template <> struct ViewForImpl<QStringRef>  : ViewForImpl<QStringView> {};
    } // namespace Tok
} // namespace QtPrivate

inline Q_DECL_PURE_FUNCTION size_t qHash(const QStringRef &key, size_t seed = 0) noexcept
{
    return qHash(QStringView { key }, seed);
}
QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF(QStringRef)

inline QStringRef &QStringRef::operator=(const QString *aString)
{ m_string = aString; m_position = 0; m_size = aString ? int(aString->size()) : 0; return *this; }

inline QStringRef::QStringRef(const QString *aString, int aPosition, int aSize)
        :m_string(aString), m_position(aPosition), m_size(aSize){}

inline QStringRef::QStringRef(const QString *aString)
    :m_string(aString), m_position(0), m_size(aString ? int(aString->size()) : 0){}

// QStringRef <> QStringRef
Q_CORE5COMPAT_EXPORT bool operator==(const QStringRef &s1, const QStringRef &s2) noexcept;
inline bool operator!=(const QStringRef &s1, const QStringRef &s2) noexcept
{ return !(s1 == s2); }
Q_CORE5COMPAT_EXPORT bool operator<(const QStringRef &s1, const QStringRef &s2) noexcept;
inline bool operator>(const QStringRef &s1, const QStringRef &s2) noexcept
{ return s2 < s1; }
inline bool operator<=(const QStringRef &s1, const QStringRef &s2) noexcept
{ return !(s1 > s2); }
inline bool operator>=(const QStringRef &s1, const QStringRef &s2) noexcept
{ return !(s1 < s2); }

// QString <> QStringRef
Q_CORE5COMPAT_EXPORT bool operator==(const QString &lhs, const QStringRef &rhs) noexcept;
inline bool operator!=(const QString &lhs, const QStringRef &rhs) noexcept { return lhs.compare(rhs) != 0; }
inline bool operator< (const QString &lhs, const QStringRef &rhs) noexcept { return lhs.compare(rhs) <  0; }
inline bool operator> (const QString &lhs, const QStringRef &rhs) noexcept { return lhs.compare(rhs) >  0; }
inline bool operator<=(const QString &lhs, const QStringRef &rhs) noexcept { return lhs.compare(rhs) <= 0; }
inline bool operator>=(const QString &lhs, const QStringRef &rhs) noexcept { return lhs.compare(rhs) >= 0; }

inline bool operator==(const QStringRef &lhs, const QString &rhs) noexcept { return rhs == lhs; }
inline bool operator!=(const QStringRef &lhs, const QString &rhs) noexcept { return rhs != lhs; }
inline bool operator< (const QStringRef &lhs, const QString &rhs) noexcept { return rhs >  lhs; }
inline bool operator> (const QStringRef &lhs, const QString &rhs) noexcept { return rhs <  lhs; }
inline bool operator<=(const QStringRef &lhs, const QString &rhs) noexcept { return rhs >= lhs; }
inline bool operator>=(const QStringRef &lhs, const QString &rhs) noexcept { return rhs <= lhs; }

inline int QStringRef::compare(const QString &s, Qt::CaseSensitivity cs) const noexcept
{ return QStringRef::compare_helper(constData(), length(), s.constData(), s.size(), cs); }
inline int QStringRef::compare(const QStringRef &s, Qt::CaseSensitivity cs) const noexcept
{ return QStringRef::compare_helper(constData(), length(), s.constData(), s.length(), cs); }
inline int QStringRef::compare(QLatin1String s, Qt::CaseSensitivity cs) const noexcept
{ return QStringRef::compare_helper(constData(), length(), s, cs); }
inline int QStringRef::compare(const QStringRef &s1, const QString &s2, Qt::CaseSensitivity cs) noexcept
{ return QStringRef::compare_helper(s1.constData(), s1.length(), s2.constData(), s2.size(), cs); }
inline int QStringRef::compare(const QStringRef &s1, const QStringRef &s2, Qt::CaseSensitivity cs) noexcept
{ return QStringRef::compare_helper(s1.constData(), s1.length(), s2.constData(), s2.length(), cs); }
inline int QStringRef::compare(const QStringRef &s1, QLatin1String s2, Qt::CaseSensitivity cs) noexcept
{ return QStringRef::compare_helper(s1.constData(), s1.length(), s2, cs); }

// QLatin1String <> QStringRef
Q_CORE5COMPAT_EXPORT bool operator==(QLatin1String lhs, const QStringRef &rhs) noexcept;
inline bool operator!=(QLatin1String lhs, const QStringRef &rhs) noexcept { return rhs.compare(lhs) != 0; }
inline bool operator< (QLatin1String lhs, const QStringRef &rhs) noexcept { return rhs.compare(lhs) >  0; }
inline bool operator> (QLatin1String lhs, const QStringRef &rhs) noexcept { return rhs.compare(lhs) <  0; }
inline bool operator<=(QLatin1String lhs, const QStringRef &rhs) noexcept { return rhs.compare(lhs) >= 0; }
inline bool operator>=(QLatin1String lhs, const QStringRef &rhs) noexcept { return rhs.compare(lhs) <= 0; }

inline bool operator==(const QStringRef &lhs, QLatin1String rhs) noexcept { return rhs == lhs; }
inline bool operator!=(const QStringRef &lhs, QLatin1String rhs) noexcept { return rhs != lhs; }
inline bool operator< (const QStringRef &lhs, QLatin1String rhs) noexcept { return rhs >  lhs; }
inline bool operator> (const QStringRef &lhs, QLatin1String rhs) noexcept { return rhs <  lhs; }
inline bool operator<=(const QStringRef &lhs, QLatin1String rhs) noexcept { return rhs >= lhs; }
inline bool operator>=(const QStringRef &lhs, QLatin1String rhs) noexcept { return rhs <= lhs; }

// QChar <> QStringRef
inline bool operator==(QChar lhs, const QStringRef &rhs) noexcept
{ return rhs.size() == 1 && lhs == rhs.front(); }
inline bool operator< (QChar lhs, const QStringRef &rhs) noexcept
{ return QStringRef::compare_helper(&lhs, 1, rhs.data(), rhs.size()) <  0; }
inline bool operator> (QChar lhs, const QStringRef &rhs) noexcept
{ return QStringRef::compare_helper(&lhs, 1, rhs.data(), rhs.size()) >  0; }

inline bool operator!=(QChar lhs, const QStringRef &rhs) noexcept { return !(lhs == rhs); }
inline bool operator<=(QChar lhs, const QStringRef &rhs) noexcept { return !(lhs >  rhs); }
inline bool operator>=(QChar lhs, const QStringRef &rhs) noexcept { return !(lhs <  rhs); }

inline bool operator==(const QStringRef &lhs, QChar rhs) noexcept { return   rhs == lhs; }
inline bool operator!=(const QStringRef &lhs, QChar rhs) noexcept { return !(rhs == lhs); }
inline bool operator< (const QStringRef &lhs, QChar rhs) noexcept { return   rhs >  lhs; }
inline bool operator> (const QStringRef &lhs, QChar rhs) noexcept { return   rhs <  lhs; }
inline bool operator<=(const QStringRef &lhs, QChar rhs) noexcept { return !(rhs <  lhs); }
inline bool operator>=(const QStringRef &lhs, QChar rhs) noexcept { return !(rhs >  lhs); }

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
// QStringRef <> QByteArray
QT_ASCII_CAST_WARN inline bool operator==(const QStringRef &lhs, const QByteArray &rhs) { return lhs.compare(rhs) == 0; }
QT_ASCII_CAST_WARN inline bool operator!=(const QStringRef &lhs, const QByteArray &rhs) { return lhs.compare(rhs) != 0; }
QT_ASCII_CAST_WARN inline bool operator< (const QStringRef &lhs, const QByteArray &rhs) { return lhs.compare(rhs) <  0; }
QT_ASCII_CAST_WARN inline bool operator> (const QStringRef &lhs, const QByteArray &rhs) { return lhs.compare(rhs) >  0; }
QT_ASCII_CAST_WARN inline bool operator<=(const QStringRef &lhs, const QByteArray &rhs) { return lhs.compare(rhs) <= 0; }
QT_ASCII_CAST_WARN inline bool operator>=(const QStringRef &lhs, const QByteArray &rhs) { return lhs.compare(rhs) >= 0; }

QT_ASCII_CAST_WARN inline bool operator==(const QByteArray &lhs, const QStringRef &rhs) { return rhs.compare(lhs) == 0; }
QT_ASCII_CAST_WARN inline bool operator!=(const QByteArray &lhs, const QStringRef &rhs) { return rhs.compare(lhs) != 0; }
QT_ASCII_CAST_WARN inline bool operator< (const QByteArray &lhs, const QStringRef &rhs) { return rhs.compare(lhs) >  0; }
QT_ASCII_CAST_WARN inline bool operator> (const QByteArray &lhs, const QStringRef &rhs) { return rhs.compare(lhs) <  0; }
QT_ASCII_CAST_WARN inline bool operator<=(const QByteArray &lhs, const QStringRef &rhs) { return rhs.compare(lhs) >= 0; }
QT_ASCII_CAST_WARN inline bool operator>=(const QByteArray &lhs, const QStringRef &rhs) { return rhs.compare(lhs) <= 0; }

// QStringRef <> const char *
QT_ASCII_CAST_WARN inline bool QStringRef::operator==(const char *s) const
{ return QStringRef::compare_helper(constData(), size(), s, -1) == 0; }
QT_ASCII_CAST_WARN inline bool QStringRef::operator!=(const char *s) const
{ return QStringRef::compare_helper(constData(), size(), s, -1) != 0; }
QT_ASCII_CAST_WARN inline bool QStringRef::operator<(const char *s) const
{ return QStringRef::compare_helper(constData(), size(), s, -1) < 0; }
QT_ASCII_CAST_WARN inline bool QStringRef::operator<=(const char *s) const
{ return QStringRef::compare_helper(constData(), size(), s, -1) <= 0; }
QT_ASCII_CAST_WARN inline bool QStringRef::operator>(const char *s) const
{ return QStringRef::compare_helper(constData(), size(), s, -1) > 0; }
QT_ASCII_CAST_WARN inline bool QStringRef::operator>=(const char *s) const
{ return QStringRef::compare_helper(constData(), size(), s, -1) >= 0; }

QT_ASCII_CAST_WARN inline bool operator==(const char *s1, const QStringRef &s2)
{ return QStringRef::compare_helper(s2.constData(), s2.size(), s1, -1) == 0; }
QT_ASCII_CAST_WARN inline bool operator!=(const char *s1, const QStringRef &s2)
{ return QStringRef::compare_helper(s2.constData(), s2.size(), s1, -1) != 0; }
QT_ASCII_CAST_WARN inline bool operator<(const char *s1, const QStringRef &s2)
{ return QStringRef::compare_helper(s2.constData(), s2.size(), s1, -1) > 0; }
QT_ASCII_CAST_WARN inline bool operator<=(const char *s1, const QStringRef &s2)
{ return QStringRef::compare_helper(s2.constData(), s2.size(), s1, -1) >= 0; }
QT_ASCII_CAST_WARN inline bool operator>(const char *s1, const QStringRef &s2)
{ return QStringRef::compare_helper(s2.constData(), s2.size(), s1, -1) < 0; }
QT_ASCII_CAST_WARN inline bool operator>=(const char *s1, const QStringRef &s2)
{ return QStringRef::compare_helper(s2.constData(), s2.size(), s1, -1) <= 0; }
#endif // !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)

inline int QStringRef::localeAwareCompare(const QString &s) const
{ return QString::localeAwareCompare(QStringView{ *this }, QStringView{ s }); }
inline int QStringRef::localeAwareCompare(const QStringRef &s) const
{ return QString::localeAwareCompare(QStringView{ *this }, QStringView{ s }); }
inline int QStringRef::localeAwareCompare(const QStringRef &s1, const QString &s2)
{ return QString::localeAwareCompare(QStringView{ s1 }, QStringView{ s2 }); }
inline int QStringRef::localeAwareCompare(const QStringRef &s1, const QStringRef &s2)
{ return QString::localeAwareCompare(QStringView{ s1 }, QStringView{ s2 }); }

inline bool QStringRef::contains(const QString &s, Qt::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool QStringRef::contains(const QStringRef &s, Qt::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool QStringRef::contains(QLatin1String s, Qt::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool QStringRef::contains(QChar c, Qt::CaseSensitivity cs) const
{ return indexOf(c, 0, cs) != -1; }
inline bool QStringRef::contains(QStringView s, Qt::CaseSensitivity cs) const noexcept
{ return indexOf(s, 0, cs) != -1; }

#if !defined(QT_USE_FAST_OPERATOR_PLUS) && !defined(QT_USE_QSTRINGBUILDER)
inline QString operator+(const QString &s1, const QStringRef &s2)
{ QString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline QString operator+(const QStringRef &s1, const QString &s2)
{ QString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline QString operator+(const QStringRef &s1, QLatin1String s2)
{ QString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline QString operator+(QLatin1String s1, const QStringRef &s2)
{ QString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline QString operator+(const QStringRef &s1, const QStringRef &s2)
{ QString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline QString operator+(const QStringRef &s1, QChar s2)
{ QString t; t.reserve(s1.size() + 1); t += s1; t += s2; return t; }
inline QString operator+(QChar s1, const QStringRef &s2)
{ QString t; t.reserve(1 + s2.size()); t += s1; t += s2; return t; }
#endif // !(QT_USE_FAST_OPERATOR_PLUS || QT_USE_QSTRINGBUILDER)

QT_END_NAMESPACE

#if defined(QT_USE_FAST_OPERATOR_PLUS) || defined(QT_USE_QSTRINGBUILDER)

#include <QtCore/qstringbuilder.h>

QT_BEGIN_NAMESPACE

template <> struct QConcatenable<QStringRef>
{
    typedef QStringRef type;
    typedef QString ConvertTo;
    enum { ExactSize = true };
    static int size(const QStringRef &a) { return a.size(); }
    static inline void appendTo(const QStringRef &a, QChar *&out)
    {
        const int n = a.size();
        if (n)
            memcpy(out, reinterpret_cast<const char*>(a.constData()), sizeof(QChar) * n);
        out += n;
    }
};

QT_END_NAMESPACE

#endif

#endif // QSTRINGREF_H
