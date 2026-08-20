// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only

#ifndef QTEXTBOUNDARYFINDER_H
#define QTEXTBOUNDARYFINDER_H

#include <QtCore/qchar.h>
#include <QtCore/qstring.h>

#if QT_VERSION >= QT_VERSION_CHECK(7, 0, 0)
#  include <array>
#endif

QT_BEGIN_NAMESPACE

struct QCharAttributes;

class Q_CORE_EXPORT QTextBoundaryFinder
{
public:
    QTextBoundaryFinder();
    QTextBoundaryFinder(const QTextBoundaryFinder &other);
    QTextBoundaryFinder(QTextBoundaryFinder &&other) noexcept
        : s{std::move(other.s)},
          sv{other.sv},
          pos{other.pos},
          attributes{std::exchange(other.attributes, nullptr)}
    {
          t = other.t;
          freeBuffer = other.freeBuffer;
          QT7_ONLY(reserved = other.reserved;)
    }
    QTextBoundaryFinder &operator=(const QTextBoundaryFinder &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QTextBoundaryFinder)
    ~QTextBoundaryFinder();

    void swap(QTextBoundaryFinder &other) noexcept
    {
        std::swap(t, other.t);
        s.swap(other.s);
        std::swap(sv, other.sv);
        std::swap(pos, other.pos);
        std::swap(freeBuffer, other.freeBuffer);
        qt_ptr_swap(attributes, other.attributes);
        QT7_ONLY(reserved.swap(other.reserved);)
    }

    enum BoundaryType QT7_ONLY(: quint8) {
        Grapheme,
        Word,
        Sentence,
        Line
    };

    enum BoundaryReason {
        NotAtBoundary = 0,
        BreakOpportunity = 0x1f,
        StartOfItem = 0x20,
        EndOfItem = 0x40,
        MandatoryBreak = 0x80,
        SoftHyphen = 0x100
    };
    Q_DECLARE_FLAGS( BoundaryReasons, BoundaryReason )

    QTextBoundaryFinder(BoundaryType type, const QString &string);
    QTextBoundaryFinder(BoundaryType type, const QChar *chars, qsizetype length, unsigned char *buffer = nullptr, qsizetype bufferSize = 0)
        : QTextBoundaryFinder(type, QStringView(chars, length), buffer, bufferSize)
    {}
    QTextBoundaryFinder(BoundaryType type, QStringView str, unsigned char *buffer = nullptr, qsizetype bufferSize = 0);

    inline bool isValid() const { return attributes; }

    inline BoundaryType type() const { return t; }
    QString string() const;

    void toStart();
    void toEnd();
    qsizetype position() const;
    void setPosition(qsizetype position);

    qsizetype toNextBoundary();
    qsizetype toPreviousBoundary();

    bool isAtBoundary() const;
    BoundaryReasons boundaryReasons() const;

private:
    QT6_ONLY(BoundaryType t = Grapheme;)
    QString s;
    QStringView sv;
    qsizetype pos = 0;
    QT6_ONLY(uint freeBuffer = true;)
    QCharAttributes *attributes = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(7, 0, 0)
    bool freeBuffer = true;
    BoundaryType t = Grapheme;
    std::array<quint8, sizeof(void *) - 2> reserved = {};
#endif
};

Q_DECLARE_SHARED(QTextBoundaryFinder)

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextBoundaryFinder::BoundaryReasons)

QT_END_NAMESPACE

#endif

