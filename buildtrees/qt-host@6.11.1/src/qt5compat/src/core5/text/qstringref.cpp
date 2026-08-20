// Copyright (C) 2020 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// Copyright (C) 2019 Mail.ru Group.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qstringref.h"

#include <QtCore/qnumeric.h>
#include <QtCore/qlist.h>
#include <QtCore/qvarlengtharray.h>

#ifdef Q_OS_MACOS
#include <private/qcore_mac_p.h>
#endif

#include <private/qfunctions_p.h>
#include <private/qstringalgorithms_p.h>
#include <private/qstringconverter_p.h>
#include <private/qstringiterator_p.h>
#include <private/qunicodetables_p.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#ifdef Q_OS_WIN
#  include <qt_windows.h>
#endif

#ifdef truncate
#  undef truncate
#endif

QT_BEGIN_NAMESPACE

// internal
static inline qsizetype qFindChar(QStringView str, QChar ch, qsizetype from, Qt::CaseSensitivity cs) noexcept;

static inline bool qt_starts_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs);
static inline bool qt_starts_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs);
static inline bool qt_starts_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs);

// Code imported from QUnicodeTables

namespace QUnicodeTablesPrivate
{
    static const unsigned short specialCaseMap[] = {
        0x0, // placeholder
        0x1,   0x2c65, 0x1,    0x2c66, 0x1,   0x2c7e, 0x1,   0x2c7f, 0x1,    0x2c6f, 0x1,   0x2c6d,
        0x1,   0x2c70, 0x1,    0xa7ab, 0x1,   0xa7ac, 0x1,   0xa78d, 0x1,    0xa7aa, 0x1,   0xa7ae,
        0x1,   0x2c62, 0x1,    0xa7ad, 0x1,   0x2c6e, 0x1,   0x2c64, 0x1,    0xa7c5, 0x1,   0xa7b1,
        0x1,   0xa7b2, 0x1,    0xa7b0, 0x1,   0xab70, 0x1,   0xab71, 0x1,    0xab72, 0x1,   0xab73,
        0x1,   0xab74, 0x1,    0xab75, 0x1,   0xab76, 0x1,   0xab77, 0x1,    0xab78, 0x1,   0xab79,
        0x1,   0xab7a, 0x1,    0xab7b, 0x1,   0xab7c, 0x1,   0xab7d, 0x1,    0xab7e, 0x1,   0xab7f,
        0x1,   0xab80, 0x1,    0xab81, 0x1,   0xab82, 0x1,   0xab83, 0x1,    0xab84, 0x1,   0xab85,
        0x1,   0xab86, 0x1,    0xab87, 0x1,   0xab88, 0x1,   0xab89, 0x1,    0xab8a, 0x1,   0xab8b,
        0x1,   0xab8c, 0x1,    0xab8d, 0x1,   0xab8e, 0x1,   0xab8f, 0x1,    0xab90, 0x1,   0xab91,
        0x1,   0xab92, 0x1,    0xab93, 0x1,   0xab94, 0x1,   0xab95, 0x1,    0xab96, 0x1,   0xab97,
        0x1,   0xab98, 0x1,    0xab99, 0x1,   0xab9a, 0x1,   0xab9b, 0x1,    0xab9c, 0x1,   0xab9d,
        0x1,   0xab9e, 0x1,    0xab9f, 0x1,   0xaba0, 0x1,   0xaba1, 0x1,    0xaba2, 0x1,   0xaba3,
        0x1,   0xaba4, 0x1,    0xaba5, 0x1,   0xaba6, 0x1,   0xaba7, 0x1,    0xaba8, 0x1,   0xaba9,
        0x1,   0xabaa, 0x1,    0xabab, 0x1,   0xabac, 0x1,   0xabad, 0x1,    0xabae, 0x1,   0xabaf,
        0x1,   0xabb0, 0x1,    0xabb1, 0x1,   0xabb2, 0x1,   0xabb3, 0x1,    0xabb4, 0x1,   0xabb5,
        0x1,   0xabb6, 0x1,    0xabb7, 0x1,   0xabb8, 0x1,   0xabb9, 0x1,    0xabba, 0x1,   0xabbb,
        0x1,   0xabbc, 0x1,    0xabbd, 0x1,   0xabbe, 0x1,   0xabbf, 0x1,    0xa64a, 0x1,   0xa77d,
        0x1,   0xa7c6, 0x1,    0x6b,   0x1,   0xe5,   0x1,   0x26b,  0x1,    0x27d,  0x1,   0x23a,
        0x1,   0x23e,  0x1,    0x251,  0x1,   0x271,  0x1,   0x250,  0x1,    0x252,  0x1,   0x23f,
        0x1,   0x240,  0x1,    0x1d79, 0x1,   0x265,  0x1,   0x266,  0x1,    0x25c,  0x1,   0x261,
        0x1,   0x26c,  0x1,    0x26a,  0x1,   0x29e,  0x1,   0x287,  0x1,    0x29d,  0x1,   0x282,
        0x1,   0x1d8e, 0x1,    0x13a0, 0x1,   0x13a1, 0x1,   0x13a2, 0x1,    0x13a3, 0x1,   0x13a4,
        0x1,   0x13a5, 0x1,    0x13a6, 0x1,   0x13a7, 0x1,   0x13a8, 0x1,    0x13a9, 0x1,   0x13aa,
        0x1,   0x13ab, 0x1,    0x13ac, 0x1,   0x13ad, 0x1,   0x13ae, 0x1,    0x13af, 0x1,   0x13b0,
        0x1,   0x13b1, 0x1,    0x13b2, 0x1,   0x13b3, 0x1,   0x13b4, 0x1,    0x13b5, 0x1,   0x13b6,
        0x1,   0x13b7, 0x1,    0x13b8, 0x1,   0x13b9, 0x1,   0x13ba, 0x1,    0x13bb, 0x1,   0x13bc,
        0x1,   0x13bd, 0x1,    0x13be, 0x1,   0x13bf, 0x1,   0x13c0, 0x1,    0x13c1, 0x1,   0x13c2,
        0x1,   0x13c3, 0x1,    0x13c4, 0x1,   0x13c5, 0x1,   0x13c6, 0x1,    0x13c7, 0x1,   0x13c8,
        0x1,   0x13c9, 0x1,    0x13ca, 0x1,   0x13cb, 0x1,   0x13cc, 0x1,    0x13cd, 0x1,   0x13ce,
        0x1,   0x13cf, 0x1,    0x13d0, 0x1,   0x13d1, 0x1,   0x13d2, 0x1,    0x13d3, 0x1,   0x13d4,
        0x1,   0x13d5, 0x1,    0x13d6, 0x1,   0x13d7, 0x1,   0x13d8, 0x1,    0x13d9, 0x1,   0x13da,
        0x1,   0x13db, 0x1,    0x13dc, 0x1,   0x13dd, 0x1,   0x13de, 0x1,    0x13df, 0x1,   0x13e0,
        0x1,   0x13e1, 0x1,    0x13e2, 0x1,   0x13e3, 0x1,   0x13e4, 0x1,    0x13e5, 0x1,   0x13e6,
        0x1,   0x13e7, 0x1,    0x13e8, 0x1,   0x13e9, 0x1,   0x13ea, 0x1,    0x13eb, 0x1,   0x13ec,
        0x1,   0x13ed, 0x1,    0x13ee, 0x1,   0x13ef, 0x2,   0x53,   0x73,   0x2,    0x53,  0x53,
        0x2,   0x69,   0x307,  0x2,    0x46,  0x66,   0x2,   0x46,   0x46,   0x2,    0x46,  0x69,
        0x2,   0x46,   0x49,   0x2,    0x46,  0x6c,   0x2,   0x46,   0x4c,   0x3,    0x46,  0x66,
        0x69,  0x3,    0x46,   0x46,   0x49,  0x3,    0x46,  0x66,   0x6c,   0x3,    0x46,  0x46,
        0x4c,  0x2,    0x53,   0x74,   0x2,   0x53,   0x54,  0x2,    0x535,  0x582,  0x2,   0x535,
        0x552, 0x2,    0x544,  0x576,  0x2,   0x544,  0x546, 0x2,    0x544,  0x565,  0x2,   0x544,
        0x535, 0x2,    0x544,  0x56b,  0x2,   0x544,  0x53b, 0x2,    0x54e,  0x576,  0x2,   0x54e,
        0x546, 0x2,    0x544,  0x56d,  0x2,   0x544,  0x53d, 0x2,    0x2bc,  0x4e,   0x3,   0x399,
        0x308, 0x301,  0x3,    0x3a5,  0x308, 0x301,  0x2,   0x4a,   0x30c,  0x2,    0x48,  0x331,
        0x2,   0x54,   0x308,  0x2,    0x57,  0x30a,  0x2,   0x59,   0x30a,  0x2,    0x41,  0x2be,
        0x2,   0x3a5,  0x313,  0x3,    0x3a5, 0x313,  0x300, 0x3,    0x3a5,  0x313,  0x301, 0x3,
        0x3a5, 0x313,  0x342,  0x2,    0x391, 0x342,  0x2,   0x397,  0x342,  0x3,    0x399, 0x308,
        0x300, 0x2,    0x399,  0x342,  0x3,   0x399,  0x308, 0x342,  0x3,    0x3a5,  0x308, 0x300,
        0x2,   0x3a1,  0x313,  0x2,    0x3a5, 0x342,  0x3,   0x3a5,  0x308,  0x342,  0x2,   0x3a9,
        0x342, 0x2,    0x1f08, 0x399,  0x2,   0x1f09, 0x399, 0x2,    0x1f0a, 0x399,  0x2,   0x1f0b,
        0x399, 0x2,    0x1f0c, 0x399,  0x2,   0x1f0d, 0x399, 0x2,    0x1f0e, 0x399,  0x2,   0x1f0f,
        0x399, 0x2,    0x1f28, 0x399,  0x2,   0x1f29, 0x399, 0x2,    0x1f2a, 0x399,  0x2,   0x1f2b,
        0x399, 0x2,    0x1f2c, 0x399,  0x2,   0x1f2d, 0x399, 0x2,    0x1f2e, 0x399,  0x2,   0x1f2f,
        0x399, 0x2,    0x1f68, 0x399,  0x2,   0x1f69, 0x399, 0x2,    0x1f6a, 0x399,  0x2,   0x1f6b,
        0x399, 0x2,    0x1f6c, 0x399,  0x2,   0x1f6d, 0x399, 0x2,    0x1f6e, 0x399,  0x2,   0x1f6f,
        0x399, 0x2,    0x391,  0x399,  0x2,   0x397,  0x399, 0x2,    0x3a9,  0x399,  0x2,   0x1fba,
        0x345, 0x2,    0x1fba, 0x399,  0x2,   0x386,  0x345, 0x2,    0x386,  0x399,  0x2,   0x1fca,
        0x345, 0x2,    0x1fca, 0x399,  0x2,   0x389,  0x345, 0x2,    0x389,  0x399,  0x2,   0x1ffa,
        0x345, 0x2,    0x1ffa, 0x399,  0x2,   0x38f,  0x345, 0x2,    0x38f,  0x399,  0x3,   0x391,
        0x342, 0x345,  0x3,    0x391,  0x342, 0x399,  0x3,   0x397,  0x342,  0x345,  0x3,   0x397,
        0x342, 0x399,  0x3,    0x3a9,  0x342, 0x345,  0x3,   0x3a9,  0x342,  0x399,  0x1,   0xa64b
    };
}

// Code imported from QChar

namespace QCharPrivate
{
    template<typename T>
    Q_DECL_CONST_FUNCTION static inline T convertCase_helper(T uc, QUnicodeTables::Case which) noexcept
    {
        const auto fold = QUnicodeTables::caseConversion(uc)[which];

        if (Q_UNLIKELY(fold.special)) {
            const ushort *specialCase = QUnicodeTablesPrivate::specialCaseMap + fold.diff;
            // so far, there are no special cases beyond BMP (guaranteed by the qunicodetables
            // generator)
            return *specialCase == 1 ? specialCase[1] : uc;
        }

        return uc + fold.diff;
    }

    [[maybe_unused]] static inline char32_t foldCase(char32_t ch, char32_t &last) noexcept
    {
        char32_t ucs4 = ch;
        if (QChar::isLowSurrogate(ucs4) && QChar::isHighSurrogate(last))
            ucs4 = QChar::surrogateToUcs4(last, ucs4);
        last = ch;
        return convertCase_helper(ucs4, QUnicodeTables::CaseFold);
    }

    static inline char16_t foldCase(char16_t ch) noexcept
    {
        return convertCase_helper(ch, QUnicodeTables::CaseFold);
    }

    static inline QChar foldCase(QChar ch) noexcept
    {
        return QChar(foldCase(ch.unicode()));
    }
}

static int qt_compare_strings(QStringView lhs, QStringView rhs, Qt::CaseSensitivity cs) noexcept
{
    return lhs.compare(rhs, cs);
}

static int qt_compare_strings(QStringView lhs, QLatin1String rhs, Qt::CaseSensitivity cs) noexcept
{
    return lhs.compare(rhs, cs);
}

static QByteArray qt_convert_to_local_8bit(QStringView string)
{
    if (string.isNull())
        return QByteArray();
    QStringEncoder fromUtf16(QStringEncoder::System, QStringEncoder::Flag::Stateless);
    return fromUtf16(string);
}

static QByteArray qt_convert_to_utf8(QStringView str)
{
    if (str.isNull())
        return QByteArray();

    return QUtf8::convertFromUnicode(str);
}

static QList<uint> qt_convert_to_ucs4(QStringView string)
{
    QList<uint> v(string.size());
    uint *a = const_cast<uint*>(v.constData());
    QStringIterator it(string);
    while (it.hasNext())
        *a++ = it.next();
    v.resize(a - v.constData());
    return v;
}

/*!
    \internal
    \since 4.5
*/
int QStringRef::compare_helper(const QChar *data1, qsizetype length1, const QChar *data2, qsizetype length2,
                            Qt::CaseSensitivity cs) noexcept
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(length2 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    Q_ASSERT(data2 || length2 == 0);
    return qt_compare_strings(QStringView(data1, length1), QStringView(data2, length2), cs);
}

/*!
    \internal
    \since 5.0
*/
int QStringRef::compare_helper(const QChar *data1, qsizetype length1, const char *data2, qsizetype length2,
                            Qt::CaseSensitivity cs)
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    if (!data2)
        return length1;
    if (Q_UNLIKELY(length2 < 0))
        length2 = qsizetype(strlen(data2));
    // ### make me nothrow in all cases
    QVarLengthArray<ushort> s2(length2);
    const auto beg = reinterpret_cast<QChar *>(s2.data());
    const auto end = QUtf8::convertToUnicode(beg, QByteArrayView(data2, length2));
    return qt_compare_strings(QStringView(data1, length1), QStringView(beg, end - beg), cs);
}

/*!
    \internal
    \since 4.5
*/
int QStringRef::compare_helper(const QChar *data1, qsizetype length1, QLatin1String s2,
                            Qt::CaseSensitivity cs) noexcept
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    return qt_compare_strings(QStringView(data1, length1), s2, cs);
}

namespace {
template<class ResultList, class StringSource>
static ResultList splitString(const StringSource &source, QStringView sep,
                              Qt::SplitBehavior behavior, Qt::CaseSensitivity cs)
{
    ResultList list;
    typename StringSource::size_type start = 0;
    typename StringSource::size_type end;
    typename StringSource::size_type extra = 0;
    while ((end = QtPrivate::findString(QStringView(source.constData(), source.size()), start + extra, sep, cs)) != -1) {
        if (start != end || behavior == Qt::KeepEmptyParts)
            list.append(source.mid(start, end - start));
        start = end + sep.size();
        extra = (sep.size() == 0 ? 1 : 0);
    }
    if (start != source.size() || behavior == Qt::KeepEmptyParts)
        list.append(source.mid(start));
    return list;
}

} // namespace

/*!
    Splits the string into substrings references wherever \a sep occurs, and
    returns the list of those strings.

    See QString::split() for how \a sep, \a behavior and \a cs interact to form
    the result.

    \note All references are valid as long this string is alive. Destroying this
    string will cause all references to be dangling pointers.

    \since 5.14
*/
QList<QStringRef> QStringRef::split(const QString &sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return splitString<QList<QStringRef>>(*this, sep, behavior, cs);
}

/*!
    \overload
    \since 5.14
*/
QList<QStringRef> QStringRef::split(QChar sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return splitString<QList<QStringRef>>(*this, QStringView(&sep, 1), behavior, cs);
}

/*!
    \class QStringRef
    \inmodule QtCore5Compat
    \since 4.3
    \brief The QStringRef class provides a thin wrapper around QString substrings.
    \reentrant
    \ingroup tools
    \ingroup string-processing

    QStringRef provides a read-only subset of the QString API.

    A string reference explicitly references a portion of a string()
    with a given size(), starting at a specific position(). Calling
    toString() returns a copy of the data as a real QString instance.

    This class is designed to improve the performance of substring
    handling when manipulating substrings obtained from existing QString
    instances. QStringRef avoids the memory allocation and reference
    counting overhead of a standard QString by simply referencing a
    part of the original string. This can prove to be advantageous in
    low level code, such as that used in a parser, at the expense of
    potentially more complex code.

    For most users, there are no semantic benefits to using QStringRef
    instead of QString since QStringRef requires attention to be paid
    to memory management issues, potentially making code more complex
    to write and maintain.

    \warning A QStringRef is only valid as long as the referenced
    string exists. If the original string is deleted, the string
    reference points to an invalid memory location.

    We suggest that you only use this class in stable code where profiling
    has clearly identified that performance improvements can be made by
    replacing standard string operations with the optimized substring
    handling provided by this class.

    \sa {Implicitly Shared Classes}
*/

/*!
    \typedef QStringRef::size_type
    \internal
*/

/*!
    \typedef QStringRef::value_type
    \internal
*/

/*!
    \typedef QStringRef::const_pointer
    \internal
*/

/*!
    \typedef QStringRef::const_reference
    \internal
*/

/*!
    \typedef QStringRef::const_iterator
    \since 5.4

    \sa QStringRef::const_reverse_iterator
*/

/*!
    \typedef QStringRef::const_reverse_iterator
    \since 5.7

    \sa QStringRef::const_iterator
*/

/*!
 \fn QStringRef::QStringRef()

 Constructs an empty string reference.
*/

/*! \fn QStringRef::QStringRef(const QString *string, int position, int length)

Constructs a string reference to the range of characters in the given
\a string specified by the starting \a position and \a length in characters.

\warning This function exists to improve performance as much as possible,
and performs no bounds checking. For program correctness, \a position and
\a length must describe a valid substring of \a string.

This means that the starting \a position must be positive or 0 and smaller
than \a string's length, and \a length must be positive or 0 but smaller than
the string's length minus the starting \a position;
i.e, 0 <= position < string->length() and
0 <= length <= string->length() - position must both be satisfied.
*/

/*! \fn QStringRef::QStringRef(const QString *string)

Constructs a string reference to the given \a string.
*/

/*! \fn QStringRef::QStringRef(const QStringRef &other)

Constructs a copy of the \a other string reference.
 */
/*!
\fn QStringRef::~QStringRef()

Destroys the string reference.

Since this class is only used to refer to string data, and does not take
ownership of it, no memory is freed when instances are destroyed.
*/

/*!
    \fn int QStringRef::position() const

    Returns the starting position in the referenced string that is referred to
    by the string reference.

    \sa size(), string()
*/

/*!
    \fn int QStringRef::size() const

    Returns the number of characters referred to by the string reference.
    Equivalent to length() and count().

    \sa position(), string()
*/
/*!
    \fn int QStringRef::count() const
    Returns the number of characters referred to by the string reference.
    Equivalent to size() and length().

    \sa position(), string()
*/
/*!
    \fn int QStringRef::length() const
    Returns the number of characters referred to by the string reference.
    Equivalent to size() and count().

    \sa position(), string()
*/


/*!
    \fn bool QStringRef::isEmpty() const

    Returns \c true if the string reference has no characters; otherwise returns
    \c false.

    A string reference is empty if its size is zero.

    \sa size()
*/

/*!
    \fn bool QStringRef::isNull() const

    Returns \c true if this string reference does not reference a string or if
    the string it references is null (i.e. QString::isNull() is true).

    \sa size()
*/

/*!
    \fn const QString *QStringRef::string() const

    Returns a pointer to the string referred to by the string reference, or
    0 if it does not reference a string.

    \sa unicode()
*/


/*!
    \fn const QChar *QStringRef::unicode() const

    Returns a Unicode representation of the string reference. Since
    the data stems directly from the referenced string, it is not
    \\0'-terminated unless the string reference includes the string's
    null terminator.

    \sa string()
*/

/*!
    \fn const QChar *QStringRef::data() const

    Same as unicode().
*/

/*!
    \fn const QChar *QStringRef::constData() const

    Same as unicode().
*/

/*!
    \fn QStringRef::const_iterator QStringRef::begin() const
    \since 5.4

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the first character in
    the string.

    \sa cbegin(), constBegin(), end(), constEnd(), rbegin(), rend()
*/

/*!
    \fn QStringRef::const_iterator QStringRef::cbegin() const
    \since 5.4

    Same as begin().

    \sa begin(), constBegin(), cend(), constEnd(), rbegin(), rend()
*/

/*!
    \fn QStringRef::const_iterator QStringRef::constBegin() const
    \since 5.9

    Same as begin().

    \sa begin(), cend(), constEnd(), rbegin(), rend()
*/

/*!
    \fn QStringRef::const_iterator QStringRef::end() const
    \since 5.4

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the imaginary
    character after the last character in the list.

    \sa cbegin(), constBegin(), end(), constEnd(), rbegin(), rend()
*/

/*! \fn QStringRef::const_iterator QStringRef::cend() const
    \since 5.4

    Same as end().

    \sa end(), constEnd(), cbegin(), constBegin(), rbegin(), rend()
*/

/*! \fn QStringRef::const_iterator QStringRef::constEnd() const
    \since 5.9

    Same as end().

    \sa end(), cend(), cbegin(), constBegin(), rbegin(), rend()
*/

/*!
    \fn QStringRef::const_reverse_iterator QStringRef::rbegin() const
    \since 5.7

    Returns a const \l{STL-style iterators}{STL-style} reverse iterator pointing to the first
    character in the string, in reverse order.

    \sa begin(), crbegin(), rend()
*/

/*!
    \fn QStringRef::const_reverse_iterator QStringRef::crbegin() const
    \since 5.7

    Same as rbegin().

    \sa begin(), rbegin(), rend()
*/

/*!
    \fn QStringRef::const_reverse_iterator QStringRef::rend() const
    \since 5.7

    Returns a \l{STL-style iterators}{STL-style} reverse iterator pointing to one past
    the last character in the string, in reverse order.

    \sa end(), crend(), rbegin()
*/


/*!
    \fn QStringRef::const_reverse_iterator QStringRef::crend() const
    \since 5.7

    Same as rend().

    \sa end(), rend(), rbegin()
*/

/*!
    Returns a copy of the string reference as a QString object.

    If the string reference is not a complete reference of the string
    (meaning that position() is 0 and size() equals string()->size()),
    this function will allocate a new string to return.

    \sa string()
*/

QString QStringRef::toString() const {
    if (isNull())
        return QString();
    if (m_size && m_position == 0 && m_size == m_string->size())
        return *m_string;
    return QString(m_string->unicode() + m_position, m_size);
}


/*! \relates QStringRef

   Returns \c true if string reference \a s1 is lexically equal to string reference \a s2; otherwise
   returns \c false.
*/
bool operator==(const QStringRef &s1,const QStringRef &s2) noexcept
{
    return s1.size() == s2.size() && qt_compare_strings(s1, s2, Qt::CaseSensitive) == 0;
}

/*! \relates QStringRef

   Returns \c true if string \a s1 is lexically equal to string reference \a s2; otherwise
   returns \c false.
*/
bool operator==(const QString &s1,const QStringRef &s2) noexcept
{
    return s1.size() == s2.size() && qt_compare_strings(s1, s2, Qt::CaseSensitive) == 0;
}

/*! \relates QStringRef

   Returns \c true if string  \a s1 is lexically equal to string reference \a s2; otherwise
   returns \c false.
*/
bool operator==(QLatin1String s1, const QStringRef &s2) noexcept
{
    if (s1.size() != s2.size())
        return false;

    return qt_compare_strings(s2, s1, Qt::CaseSensitive) == 0;
}

/*!
   \relates QStringRef

    Returns \c true if string reference \a s1 is lexically less than
    string reference \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/
bool operator<(const QStringRef &s1,const QStringRef &s2) noexcept
{
    return qt_compare_strings(s1, s2, Qt::CaseSensitive) < 0;
}

/*!\fn bool operator<=(const QStringRef &s1,const QStringRef &s2)

   \relates QStringRef

    Returns \c true if string reference \a s1 is lexically less than
    or equal to string reference \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*!\fn bool operator>=(const QStringRef &s1,const QStringRef &s2)

   \relates QStringRef

    Returns \c true if string reference \a s1 is lexically greater than
    or equal to string reference \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*!\fn bool operator>(const QStringRef &s1,const QStringRef &s2)

   \relates QStringRef

    Returns \c true if string reference \a s1 is lexically greater than
    string reference \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/


/*!
    \fn const QChar QStringRef::at(int position) const

    Returns the character at the given index \a position in the
    string reference.

    The \a position must be a valid index position in the string
    (i.e., 0 <= \a position < size()).
*/

/*!
    \fn QChar QStringRef::operator[](int position) const
    \since 5.7

    Returns the character at the given index \a position in the
    string reference.

    The \a position must be a valid index position in the string
    reference (i.e., 0 <= \a position < size()).

    \sa at()
*/

/*!
    \fn QChar QStringRef::front() const
    \since 5.10

    Returns the first character in the string.
    Same as \c{at(0)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa back(), at(), operator[]()
*/

/*!
    \fn QChar QStringRef::back() const
    \since 5.10

    Returns the last character in the string.
    Same as \c{at(size() - 1)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa front(), at(), operator[]()
*/

/*!
    \fn void QStringRef::clear()

    Clears the contents of the string reference by making it null and empty.

    \sa isEmpty(), isNull()
*/

/*!
    \fn QStringRef &QStringRef::operator=(const QStringRef &other)

    Assigns the \a other string reference to this string reference, and
    returns the result.
*/

/*!
    \fn QStringRef &QStringRef::operator=(const QString *string)

    Constructs a string reference to the given \a string and assigns it to
    this string reference, returning the result.
*/

/*!
    \fn bool QStringRef::operator==(const char * s) const

    \overload operator==()

    The \a s byte array is converted to a QStringRef using the
    fromUtf8() function. This function stops conversion at the
    first NUL character found, or the end of the byte array.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is lexically equal to the parameter
    string \a s. Otherwise returns \c false.

    \sa QT_NO_CAST_FROM_ASCII
*/

/*!
    \fn bool QStringRef::operator!=(const char * s) const

    \overload operator!=()

    The \a s const char pointer is converted to a QStringRef using
    the fromUtf8() function.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is not lexically equal to the parameter
    string \a s. Otherwise returns \c false.

    \sa QT_NO_CAST_FROM_ASCII
*/

/*!
    \fn bool QStringRef::operator<(const char * s) const

    \overload operator<()

    The \a s const char pointer is converted to a QStringRef using
    the fromUtf8() function.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is lexically smaller than the parameter
    string \a s. Otherwise returns \c false.

    \sa QT_NO_CAST_FROM_ASCII
*/

/*!
    \fn bool QStringRef::operator<=(const char * s) const

    \overload operator<=()

    The \a s const char pointer is converted to a QStringRef using
    the fromUtf8() function.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is lexically smaller than or equal to the parameter
    string \a s. Otherwise returns \c false.

    \sa QT_NO_CAST_FROM_ASCII
*/

/*!
    \fn bool QStringRef::operator>(const char * s) const


    \overload operator>()

    The \a s const char pointer is converted to a QStringRef using
    the fromUtf8() function.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is lexically greater than the parameter
    string \a s. Otherwise returns \c false.

    \sa QT_NO_CAST_FROM_ASCII
*/

/*!
    \fn bool QStringRef::operator>= (const char * s) const

    \overload operator>=()

    The \a s const char pointer is converted to a QStringRef using
    the fromUtf8() function.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is lexically greater than or equal to the
    parameter string \a s. Otherwise returns \c false.

    \sa QT_NO_CAST_FROM_ASCII
*/

/*!  Appends the string reference to \a string, and returns a new
reference to the combined string data.
 */
QStringRef QStringRef::appendTo(QString *string) const
{
    if (!string)
        return QStringRef();
    int pos = string->size();
    string->insert(pos, unicode(), size());
    return QStringRef(string, pos, size());
}

/*!
    \fn int QStringRef::compare(const QStringRef &s1, const QString &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
    \since 4.5

    Compares \a s1 with \a s2 and returns a negative
    integer if \a s1 is less than \a s2, a positive integer if it is greater
    than \a s2, and zero if they are equal.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \fn int QStringRef::compare(const QStringRef &s1, const QStringRef &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
    \since 4.5
    \overload

    Compares \a s1 with \a s2 and returns a negative
    integer if \a s1 is less than \a s2, a positive integer if it is greater
    than \a s2, and zero if they are equal.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \fn int QStringRef::compare(const QStringRef &s1, QLatin1String s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
    \since 4.5
    \overload

    Compares \a s1 with \a s2 and returns a negative
    integer if \a s1 is less than \a s2, a positive integer if it is greater
    than \a s2, and zero if they are equal.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \overload
    \fn int QStringRef::compare(const QString &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.5

    Compares \c {*this} with \a other and returns a negative integer if
    \c {*this} is less than \a other, a positive integer if it is greater
    than \a other, and zero if they are equal.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.
*/

/*!
    \overload
    \fn int QStringRef::compare(const QStringRef &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.5

    Compares \c {*this} with \a other and returns a negative integer if
    \c {*this} is less than \a other, a positive integer if it is greater
    than \a other, and zero if they are equal.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.
*/

/*!
    \overload
    \fn int QStringRef::compare(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 5.14

    Compares \c {*this} with \a ch and returns a negative integer if \c {*this}
    is less than \a ch, a positive integer if it is greater than \a ch, and zero
    if they are equal. Here, \a ch interpreted as a string of length one.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \overload
    \fn int QStringRef::compare(QLatin1String other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.5

    Compares \c {*this} with \a other and returns a negative integer if \c {*this}
    is less than \a other, a positive integer if it is greater than \a other, and
    zero if they are equal.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.
*/

/*!
    \overload
    \fn int QStringRef::compare(const QByteArray &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 5.8

    Compares \c {*this} with \a other and returns a negative integer if \c {*this}
    is less than \a other, a positive integer if it is greater than \a other, and
     zero if they are equal. The contents of \a other is interpreted as UTF-8.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QStringRef &s1, const QString & s2)
    \since 4.5

    Compares \a s1 with \a s2 and returns a negative
    integer if \a s1 is less than \a s2, a positive integer if it is greater
    than \a s2, and zero if they are equal.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa compare(), QLocale, {Comparing Strings}
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QStringRef &s1, const QStringRef & s2)
    \since 4.5
    \overload

    Compares \a s1 with \a s2 and returns a negative
    integer if \a s1 is less than \a s2, a positive integer if it is greater
    than \a s2, and zero if they are equal.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa {Comparing Strings}
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QString &other) const
    \since 4.5
    \overload

    Compares \c {*this} with \a other and returns a negative integer if \c {*this}
    is less than \a other, a positive integer if it is greater than \a other, and
    zero if they are equal.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa {Comparing Strings}
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QStringRef &other) const
    \since 4.5
    \overload

    Compares \c {*this} with \a other and returns a negative integer if \c {*this}
    is less than \a other, a positive integer if it is greater than \a other, and
    zero if they are equal.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa {Comparing Strings}
*/

/*!
    \fn QStringRef::left(int n) const
    \since 5.2

    Returns a substring reference to the \a n leftmost characters
    of the string.

    If \a n is greater than or equal to size(), or less than zero,
    a reference to the entire string is returned.

    \sa right(), mid(), startsWith(), chopped(), chop(), truncate()
*/
QStringRef QStringRef::left(int n) const
{
    if (size_t(n) >= size_t(m_size))
        return *this;
    return QStringRef(m_string, m_position, n);
}

/*!
    \fn QStringRef::right(int n) const
    \since 5.2

    Returns a substring reference to the \a n rightmost characters
    of the string.

    If \a n is greater than or equal to size(), or less than zero,
    a reference to the entire string is returned.

    \sa left(), mid(), endsWith(), chopped(), chop(), truncate()
*/
QStringRef QStringRef::right(int n) const
{
    if (size_t(n) >= size_t(m_size))
        return *this;
    return QStringRef(m_string, m_size - n + m_position, n);
}

/*!
    \fn QStringRef QStringRef::mid(int position, int n = -1) const
    \since 5.2

    Returns a substring reference to \a n characters of this string,
    starting at the specified \a position.

    If the \a position exceeds the length of the string, a null
    reference is returned.

    If there are less than \a n characters available in the string,
    starting at the given \a position, or if \a n is -1 (default), the
    function returns all characters from the specified \a position
    onwards.

    \sa left(), right(), chopped(), chop(), truncate()
*/
QStringRef QStringRef::mid(int pos, int n) const
{
    qsizetype p = pos;
    qsizetype l = n;
    using namespace QtPrivate;
    switch (QContainerImplHelper::mid(m_size, &p, &l)) {
    case QContainerImplHelper::Null:
        return QStringRef();
    case QContainerImplHelper::Empty:
        return QStringRef(m_string, 0, 0);
    case QContainerImplHelper::Full:
        return *this;
    case QContainerImplHelper::Subset:
        return QStringRef(m_string, p + m_position, l);
    }
    Q_UNREACHABLE_RETURN(QStringRef());
}

/*!
    \fn QStringRef QStringRef::chopped(int len) const
    \since 5.10

    Returns a substring reference to the size() - \a len leftmost characters
    of this string.

    \note The behavior is undefined if \a len is negative or greater than size().

    \sa endsWith(), left(), right(), mid(), chop(), truncate()
*/

/*!
    \fn void QStringRef::truncate(int position)
    \since 5.6

    Truncates the string at the given \a position index.

    If the specified \a position index is beyond the end of the
    string, nothing happens.

    If \a position is negative, it is equivalent to passing zero.

    \sa QString::truncate()
*/

/*!
    \fn void QStringRef::chop(int n)
    \since 5.8

    Removes \a n characters from the end of the string.

    If \a n is greater than or equal to size(), the result is an
    empty string; if \a n is negative, it is equivalent to passing zero.

    \sa QString::chop(), truncate()
*/

/*!
  \since 4.8

  Returns the index position of the first occurrence of the string \a
  str in this string reference, searching forward from index position
  \a from. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  If \a from is -1, the search starts at the last character; if it is
  -2, at the next to last character and so on.

  \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(const QString &str, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::findString(QStringView(unicode(), length()), from, QStringView(str.unicode(), str.size()), cs));
}

/*!
    \fn int QStringRef::indexOf(QStringView str, int from, Qt::CaseSensitivity cs) const
    \since 5.14
    \overload indexOf()

    Returns the index position of the first occurrence of the string view \a str
    in this string reference, searching forward from index position \a from.
    Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    If \a from is -1, the search starts at the last character; if it is
    -2, at the next to last character and so on.

    \sa QString::indexOf(), QStringView::indexOf(), lastIndexOf(), contains(), count()
*/

/*!
    \since 4.8
    \overload indexOf()

    Returns the index position of the first occurrence of the
    character \a ch in the string reference, searching forward from
    index position \a from. Returns -1 if \a ch could not be found.

    \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(QChar ch, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(qFindChar(QStringView(unicode(), length()), ch, from, cs));
}

/*!
  \since 4.8

  Returns the index position of the first occurrence of the string \a
  str in this string reference, searching forward from index position
  \a from. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  If \a from is -1, the search starts at the last character; if it is
  -2, at the next to last character and so on.

  \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(QLatin1String str, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::findString(QStringView(unicode(), size()), from, str, cs));
}

/*!
    \since 4.8

    \overload indexOf()

    Returns the index position of the first occurrence of the string
    reference \a str in this string reference, searching forward from
    index position \a from. Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(const QStringRef &str, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::findString(QStringView(unicode(), size()), from, QStringView(str.unicode(), str.size()), cs));
}

/*!
  \since 4.8

  Returns the index position of the last occurrence of the string \a
  str in this string reference, searching backward from index position
  \a from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(const QString &str, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::lastIndexOf(*this, from, str, cs));
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the character
  \a ch, searching backward from position \a from.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(QChar ch, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QStringView{ *this }.lastIndexOf(ch, from, cs));
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string \a
  str in this string reference, searching backward from index position
  \a from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(QLatin1String str, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::lastIndexOf(*this, from, str, cs));
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string
  reference \a str in this string reference, searching backward from
  index position \a from. If \a from is -1 (default), the search
  starts at the last character; if \a from is -2, at the next to last
  character and so on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(const QStringRef &str, int from, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::lastIndexOf(*this, from, str, cs));
}

/*!
  \fn int QStringRef::lastIndexOf(QStringView str, int from, Qt::CaseSensitivity cs) const
  \since 5.14
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string view \a
  str in this string, searching backward from index position \a
  from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa indexOf(), contains(), count()
*/

/*!
    \since 4.8
    Returns the number of (potentially overlapping) occurrences of
    the string \a str in this string reference.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::count(), contains(), indexOf()
*/
int QStringRef::count(const QString &str, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::count(QStringView(unicode(), size()), QStringView(str.unicode(), str.size()), cs));
}

/*!
    \since 4.8
    \overload count()

    Returns the number of occurrences of the character \a ch in the
    string reference.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::count(), contains(), indexOf()
*/
int QStringRef::count(QChar ch, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::count(QStringView(unicode(), size()), ch, cs));
}

/*!
    \since 4.8
    \overload count()

    Returns the number of (potentially overlapping) occurrences of the
    string reference \a str in this string reference.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::count(), contains(), indexOf()
*/
int QStringRef::count(const QStringRef &str, Qt::CaseSensitivity cs) const
{
    // ### Qt6: qsizetype
    return int(QtPrivate::count(QStringView(unicode(), size()), QStringView(str.unicode(), str.size()), cs));
}

/*!
    \since 5.9

    Returns \c true if the string is read right to left.

    \sa QString::isRightToLeft()
*/
bool QStringRef::isRightToLeft() const
{
    return QtPrivate::isRightToLeft(QStringView(unicode(), size()));
}

/*!
    \since 4.8

    Returns \c true if the string reference starts with \a str; otherwise
    returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(const QString &str, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, str, cs);
}

/*!
    \since 4.8
    \overload startsWith()
    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(QLatin1String str, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, str, cs);
}

/*!
    \fn bool QStringRef::startsWith(QStringView str, Qt::CaseSensitivity cs) const
    \since 5.10
    \overload startsWith()
    \sa QString::startsWith(), endsWith()
*/

/*!
    \since 4.8
    \overload startsWith()
    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(const QStringRef &str, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, str, cs);
}

/*!
    \since 4.8
    \overload startsWith()

    Returns \c true if the string reference starts with \a ch; otherwise
    returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(QChar ch, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, ch, cs);
}

/*!
    \since 4.8
    Returns \c true if the string reference ends with \a str; otherwise
    returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::endsWith(), startsWith()
*/
bool QStringRef::endsWith(const QString &str, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, str, cs);
}

/*!
    \since 4.8
    \overload endsWith()

    Returns \c true if the string reference ends with \a ch; otherwise
    returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::endsWith(), endsWith()
*/
bool QStringRef::endsWith(QChar ch, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, ch, cs);
}

/*!
    \since 4.8
    \overload endsWith()
    \sa QString::endsWith(), endsWith()
*/
bool QStringRef::endsWith(QLatin1String str, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, str, cs);
}

/*!
    \fn bool QStringRef::endsWith(QStringView str, Qt::CaseSensitivity cs) const
    \since 5.10
    \overload endsWith()
    \sa QString::endsWith(), startsWith()
*/

/*!
    \since 4.8
    \overload endsWith()
    \sa QString::endsWith(), endsWith()
*/
bool QStringRef::endsWith(const QStringRef &str, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, str, cs);
}

/*! \fn bool QStringRef::contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \since 4.8
    Returns \c true if this string reference contains an occurrence of
    the string \a str; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

/*! \fn bool QStringRef::contains(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \overload contains()
    \since 4.8

    Returns \c true if this string contains an occurrence of the
    character \a ch; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

*/

/*! \fn bool QStringRef::contains(const QStringRef &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \overload contains()
    \since 4.8

    Returns \c true if this string reference contains an occurrence of
    the string reference \a str; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

/*! \fn bool QStringRef::contains(QLatin1String str, Qt::CaseSensitivity cs) const
    \since 4.8
    \overload contains()

    Returns \c true if this string reference contains an occurrence of
    the string \a str; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

/*! \fn bool QStringRef::contains(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 5.14
    \overload contains()

    Returns \c true if this string reference contains an occurrence of
    the string view \a str; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

template <typename Haystack, typename Needle>
bool qt_starts_with_impl(Haystack haystack, Needle needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.isNull())
        return needle.isNull(); // historical behavior, consider changing in ### Qt 6.
    const auto haystackLen = haystack.size();
    const auto needleLen = needle.size();
    if (haystackLen == 0)
        return needleLen == 0;
    if (needleLen > haystackLen)
        return false;

    return qt_compare_strings(haystack.left(needleLen), needle, cs) == 0;
}

static inline bool qt_starts_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs)
{
    return qt_starts_with_impl(haystack, needle, cs);
}

static inline bool qt_starts_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs)
{
    return qt_starts_with_impl(haystack, needle, cs);
}

static inline bool qt_starts_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs)
{
    return haystack.size()
           && (cs == Qt::CaseSensitive ? haystack.front() == needle
                                       : QCharPrivate::foldCase(haystack.front()) == QCharPrivate::foldCase(needle));
}

template <typename Haystack, typename Needle>
bool qt_ends_with_impl(Haystack haystack, Needle needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.isNull())
        return needle.isNull(); // historical behavior, consider changing in ### Qt 6.
    const auto haystackLen = haystack.size();
    const auto needleLen = needle.size();
    if (haystackLen == 0)
        return needleLen == 0;
    if (haystackLen < needleLen)
        return false;

    return qt_compare_strings(haystack.right(needleLen), needle, cs) == 0;
}

static inline bool qt_ends_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs)
{
    return qt_ends_with_impl(haystack, needle, cs);
}

static inline bool qt_ends_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs)
{
    return qt_ends_with_impl(haystack, needle, cs);
}

static inline bool qt_ends_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs)
{
    return haystack.size()
           && (cs == Qt::CaseSensitive ? haystack.back() == needle
                                       : QCharPrivate::foldCase(haystack.back()) == QCharPrivate::foldCase(needle));
}

/*!
    \internal

    Returns the index position of the first occurrence of the
    character \a ch in the string given by \a str and \a len,
    searching forward from index
    position \a from. Returns -1 if \a ch could not be found.
*/

static inline qsizetype qFindChar(QStringView str, QChar ch, qsizetype from, Qt::CaseSensitivity cs) noexcept
{
    if (from < 0)
        from = qMax(from + str.size(), qsizetype(0));
    if (from < str.size()) {
        const char16_t *s = str.utf16();
        char16_t c = ch.unicode();
        const char16_t *n = s + from;
        const char16_t *e = s + str.size();
        if (cs == Qt::CaseSensitive) {
            n = QtPrivate::qustrchr(QStringView(n, e), c);
            if (n != e)
                return n - s;
        } else {
            c = QCharPrivate::foldCase(c);
            --n;
            while (++n != e)
                if (QCharPrivate::foldCase(*n) == c)
                    return n - s;
        }
    }
    return -1;
}

/*!
    \since 4.8

    Returns a Latin-1 representation of the string as a QByteArray.

    The returned byte array is undefined if the string contains non-Latin1
    characters. Those characters may be suppressed or replaced with a
    question mark.

    \sa toUtf8(), toLocal8Bit(), QStringEncoder
*/
QByteArray QStringRef::toLatin1() const
{
    return QStringView{ *this }.toLatin1();
}

/*!
    \since 4.8

    Returns the local 8-bit representation of the string as a
    QByteArray. The returned byte array is undefined if the string
    contains characters not supported by the local 8-bit encoding.

    On Unix systems this is equivalent to toUtf8(), on Windows the systems
    current code page is being used.

    If this string contains any characters that cannot be encoded in the
    locale, the returned byte array is undefined. Those characters may be
    suppressed or replaced by another.

    \sa toLatin1(), toUtf8(), QStringEncoder
*/
QByteArray QStringRef::toLocal8Bit() const
{
    return qt_convert_to_local_8bit(*this);
}

/*!
    \since 4.8

    Returns a UTF-8 representation of the string as a QByteArray.

    UTF-8 is a Unicode codec and can represent all characters in a Unicode
    string like QString.

    \sa toLatin1(), toLocal8Bit(), QStringEncoder
*/
QByteArray QStringRef::toUtf8() const
{
    return qt_convert_to_utf8(*this);
}

/*!
    \since 4.8

    Returns a UCS-4/UTF-32 representation of the string as a QList<uint>.

    UCS-4 is a Unicode codec and therefore it is lossless. All characters from
    this string will be encoded in UCS-4. Any invalid sequence of code units in
    this string is replaced by the Unicode's replacement character
    (QChar::ReplacementCharacter, which corresponds to \c{U+FFFD}).

    The returned list is not \\0'-terminated.

    \sa toUtf8(), toLatin1(), toLocal8Bit(), QStringEncoder
*/
QList<uint> QStringRef::toUcs4() const
{
    return qt_convert_to_ucs4(*this);
}

/*!
    Returns a string that has whitespace removed from the start and
    the end.

    Whitespace means any character for which QChar::isSpace() returns
    \c true. This includes the ASCII characters '\\t', '\\n', '\\v',
    '\\f', '\\r', and ' '.

    Unlike QString::simplified(), trimmed() leaves internal whitespace alone.

    \since 5.1

    \sa QString::trimmed()
*/
QStringRef QStringRef::trimmed() const
{
    auto trimmed = QStringView(*this).trimmed();
    auto begin = trimmed.cbegin();
    auto end = trimmed.cend();
    if (begin == cbegin() && end == cend())
        return *this;
    int position = m_position + (begin - cbegin());
    return QStringRef(m_string, position, end - begin);
}

/*!
    Returns the string converted to a \c{long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toLongLong()

    \sa QString::toLongLong()

    \since 5.1
*/
qint64 QStringRef::toLongLong(bool *ok, int base) const
{
    return QStringView{ *this }.toLongLong(ok, base);
}

/*!
    Returns the string converted to an \c{unsigned long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toULongLong()

    \sa QString::toULongLong()

    \since 5.1
*/
quint64 QStringRef::toULongLong(bool *ok, int base) const
{
    return QStringView{ *this }.toULongLong(ok, base);
}

/*!
    \fn long QStringRef::toLong(bool *ok, int base) const

    Returns the string converted to a \c long using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toLong()

    \sa QString::toLong()

    \since 5.1
*/
long QStringRef::toLong(bool *ok, int base) const
{
    return QStringView{ *this }.toLong(ok, base);
}

/*!
    \fn ulong QStringRef::toULong(bool *ok, int base) const

    Returns the string converted to an \c{unsigned long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toULongLong()

    \sa QString::toULong()

    \since 5.1
*/
ulong QStringRef::toULong(bool *ok, int base) const
{
   return QStringView{ *this }.toULong(ok, base);
}

/*!
    Returns the string converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toInt()

    \sa QString::toInt()

    \since 5.1
*/
int QStringRef::toInt(bool *ok, int base) const
{
    return QStringView{ *this }.toInt(ok, base);
}

/*!
    Returns the string converted to an \c{unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toUInt()

    \sa QString::toUInt()

    \since 5.1
*/
uint QStringRef::toUInt(bool *ok, int base) const
{
    return QStringView{ *this }.toUInt(ok, base);
}

/*!
    Returns the string converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toShort()

    \sa QString::toShort()

    \since 5.1
*/
short QStringRef::toShort(bool *ok, int base) const
{
    return QStringView{ *this }.toShort(ok, base);
}

/*!
    Returns the string converted to an \c{unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toUShort()

    \sa QString::toUShort()

    \since 5.1
*/
ushort QStringRef::toUShort(bool *ok, int base) const
{
    return QStringView{ *this }.toUShort(ok, base);
}

/*!
    Returns the string converted to a \c double value.

    Returns an infinity if the conversion overflows or 0.0 if the
    conversion fails for other reasons (e.g. underflow).

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toDouble()

    For historic reasons, this function does not handle
    thousands group separators. If you need to convert such numbers,
    use QLocale::toDouble().

    \sa QString::toDouble()

    \since 5.1
*/
double QStringRef::toDouble(bool *ok) const
{
    return QStringView{ *this }.toDouble(ok);
}

/*!
    Returns the string converted to a \c float value.

    Returns an infinity if the conversion overflows or 0.0 if the
    conversion fails for other reasons (e.g. underflow).

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toFloat()

    \sa QString::toFloat()

    \since 5.1
*/
float QStringRef::toFloat(bool *ok) const
{
    return QStringView{ *this }.toFloat(ok);
}

/*! \fn size_t qHash(const QStringRef &key, size_t seed = 0)
    \relates QHash
    \since 5.0

    Returns the hash value for the \a key, using \a seed to seed the calculation.
*/

QT_END_NAMESPACE
