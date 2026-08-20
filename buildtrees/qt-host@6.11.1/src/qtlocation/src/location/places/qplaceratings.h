// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACERATINGS_H
#define QPLACERATINGS_H

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>
#include <QtLocation/qlocationglobal.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QPlaceRatingsPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceRatingsPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceRatings
{
    Q_GADGET
    QML_VALUE_TYPE(ratings)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(qreal average READ average WRITE setAverage)
    Q_PROPERTY(qreal maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int count READ count WRITE setCount)

public:
    QPlaceRatings();
    QPlaceRatings(const QPlaceRatings &other) noexcept;
    QPlaceRatings(QPlaceRatings &&other) noexcept = default;
    ~QPlaceRatings();

    QPlaceRatings &operator=(const QPlaceRatings &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceRatings)

    void swap(QPlaceRatings &other) noexcept { d.swap(other.d); }

    friend inline bool operator==(const QPlaceRatings &lhs, const QPlaceRatings &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QPlaceRatings &lhs, const QPlaceRatings &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    qreal average() const;
    void setAverage(qreal average);

    int count() const;
    void setCount(int count);

    qreal maximum() const;
    void setMaximum(qreal max);

    bool isEmpty() const;

private:
    QSharedDataPointer<QPlaceRatingsPrivate> d;

    bool isEqual(const QPlaceRatings &other) const noexcept;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceRatings)

#endif
