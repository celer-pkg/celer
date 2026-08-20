// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:sensitive reason:trivial-impl-only

#ifndef QREGEXP_H
#define QREGEXP_H

#include <QtCore5Compat/qcore5global.h>

#include <QtCore/qglobal.h>
#include <QtCore/qcontainerfwd.h>

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE


struct QRegExpPrivate;
class QRegExp;

Q_CORE5COMPAT_EXPORT size_t qHash(const QRegExp &key, size_t seed = 0) noexcept;

class Q_CORE5COMPAT_EXPORT QRegExp
{
public:
    enum PatternSyntax {
        RegExp,
        Wildcard,
        FixedString,
        RegExp2,
        WildcardUnix,
        W3CXmlSchema11 };
    enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

    QRegExp();
    explicit QRegExp(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive,
                     PatternSyntax syntax = RegExp);
    QRegExp(const QRegExp &rx);
    ~QRegExp();
    QRegExp &operator=(const QRegExp &rx);
    QRegExp &operator=(QRegExp &&other) noexcept { swap(other); return *this; }
    void swap(QRegExp &other) noexcept { qt_ptr_swap(priv, other.priv); }

    bool operator==(const QRegExp &rx) const;
    inline bool operator!=(const QRegExp &rx) const { return !operator==(rx); }

    bool isEmpty() const;
    bool isValid() const;
    QString pattern() const;
    void setPattern(const QString &pattern);
    Qt::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(Qt::CaseSensitivity cs);
    PatternSyntax patternSyntax() const;
    void setPatternSyntax(PatternSyntax syntax);

    bool isMinimal() const;
    void setMinimal(bool minimal);

    bool exactMatch(const QString &str) const;

    operator QVariant() const;

    int indexIn(const QString &str, int offset = 0, CaretMode caretMode = CaretAtZero) const;
    int lastIndexIn(const QString &str, int offset = -1, CaretMode caretMode = CaretAtZero) const;
    int matchedLength() const;
#ifndef QT_NO_REGEXP_CAPTURE
    int captureCount() const;
    QStringList capturedTexts() const;
    QStringList capturedTexts();
    QString cap(int nth = 0) const;
    QString cap(int nth = 0);
    int pos(int nth = 0) const;
    int pos(int nth = 0);
    QString errorString() const;
    QString errorString();
#endif

    QString replaceIn(const QString &str, const QString &after) const;
    QString removeIn(const QString &str) const
    { return replaceIn(str, QString()); }
    bool containedIn(const QString &str) const
    { return indexIn(str) != -1; }
    int countIn(const QString &str) const;

    QStringList splitString(const QString &str, Qt::SplitBehavior behavior = Qt::KeepEmptyParts) const;

    int indexIn(const QStringList &list, int from) const;
    int lastIndexIn(const QStringList &list, int from) const;
    QStringList replaceIn(const QStringList &stringList, const QString &after) const;
    QStringList filterList(const QStringList &stringList) const;

    static QString escape(const QString &str);

    friend Q_CORE5COMPAT_EXPORT size_t qHash(const QRegExp &key, size_t seed) noexcept;

private:
    QRegExpPrivate *priv;
};

#ifndef QT_NO_DATASTREAM
Q_CORE5COMPAT_EXPORT QDataStream &operator<<(QDataStream &out, const QRegExp &regExp);
Q_CORE5COMPAT_EXPORT QDataStream &operator>>(QDataStream &in, QRegExp &regExp);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_CORE5COMPAT_EXPORT QDebug operator<<(QDebug, const QRegExp &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QRegExp)

#endif // QREGEXP_H
