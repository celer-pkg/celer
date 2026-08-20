// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplaceratings.h"
#include "qplaceratings_p.h"

QT_USE_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QPlaceRatingsPrivate)

bool QPlaceRatingsPrivate::operator==(const QPlaceRatingsPrivate &other) const
{
    return average == other.average && maximum == other.maximum && count == other.count;
}

bool QPlaceRatingsPrivate::isEmpty() const
{
    return count == 0 && average == 0 && maximum == 0;
}

/*!
    \class QPlaceRatings
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlaceRatings class holds rating information about a place.

    Rating information is used to describe how good a place is conceived to be.
    Typically this information is visualized as a number of stars.
    The average() function returns an aggregated ratings value out of a possible
    maximum as given by the maximum() function.

    \snippet places/requesthandler.h Ratings
*/

/*!
    \qmlvaluetype ratings
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-data
    \since QtLocation 6.5

    \brief The ratings type holds place rating information.

    Rating information is used to describe how \e good a place is conceived to be.  Typically this
    information is visualized as a number of stars.  The \l {ratings::}{average} property gives an aggregated
    ratings value out of a possible maximum as given by the \l {ratings::maximum} property.
*/

/*!
    Constructs a new ratings object.
*/
QPlaceRatings::QPlaceRatings()
    : d(new QPlaceRatingsPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QPlaceRatings::QPlaceRatings(const QPlaceRatings &other) noexcept = default;

/*!
    Destroys the ratings object.
*/
QPlaceRatings::~QPlaceRatings() = default;

/*!
    Assigns \a other to this ratings object and returns
    a reference to this ratings object.
*/
QPlaceRatings &QPlaceRatings::operator=(const QPlaceRatings &other) noexcept
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

/*!
    \fn bool QPlaceRatings::operator==(const QPlaceRatings &lhs, const QPlaceRatings &rhs) noexcept

    Returns true if \a lhs is equal to \a rhs, otherwise returns false.
*/

/*!
    \fn bool QPlaceRatings::operator!=(const QPlaceRatings &lhs, const QPlaceRatings &rhs) noexcept

    Returns true if \a lhs is not equal to \a rhs, otherwise returns false.
*/

bool QPlaceRatings::isEqual(const QPlaceRatings &other) const noexcept
{
    return (*(d.constData()) == *(other.d.constData()));
}

/*!
    \qmlproperty real ratings::average

    This property holds the average of the individual ratings.

    \sa maximum
*/

/*!
    \property QPlaceRatings::average
    \brief the average value of individual ratings.
*/
qreal QPlaceRatings::average() const
{
    return d->average;
}

void QPlaceRatings::setAverage(qreal average)
{
    d->average = average;
}

/*!
    \qmlproperty real ratings::maximum

    This property holds the maximum rating value.
*/

/*!
    \property QPlaceRatings::maximum
    \brief the maximum possible rating value.
*/
qreal QPlaceRatings::maximum() const
{
    return d->maximum;
}

void QPlaceRatings::setMaximum(qreal max)
{
    d->maximum = max;
}

/*!
    \qmlproperty int ratings::count

    This property holds the total number of individual user ratings
    used in determining the overall ratings \l average.
*/

/*!
    \property QPlaceRatings::count
    \brief the total number of individual ratings.
*/
int QPlaceRatings::count() const
{
    return d->count;
}

void QPlaceRatings::setCount(int count)
{
    d->count = count;
}

/*!
    Returns true if all fields of the place ratings are 0; otherwise returns false.
*/
bool QPlaceRatings::isEmpty() const
{
    return d->isEmpty();
}

#include "moc_qplaceratings.cpp"
