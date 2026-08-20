// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplaceresult.h"
#include "qplaceresult_p.h"
#include <QtCore/qnumeric.h>

QT_USE_NAMESPACE

bool QPlaceResultPrivate::compare(const QPlaceSearchResultPrivate *other) const
{
    const QPlaceResultPrivate *od = static_cast<const QPlaceResultPrivate *>(other);
    return QPlaceSearchResultPrivate::compare(other)
           && ((qIsNaN(distance) && qIsNaN(od->distance))
                || QtPrivate::fuzzyCompare(distance, od->distance))
           && place == od->place
           && sponsored == od->sponsored;
}

/*!
    \class QPlaceResult
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since 5.6

    \brief The QPlaceResult class represents a search result containing a place.

    The PlaceResult holds the distance to the place from the center of the search request,
    an instance of the place and an indication of whether the result is
    sponsored or \l {http://en.wikipedia.org/wiki/Organic_search}{organic}.

    The intended usage is that a QPlaceSearchResult can be converted into a QPlaceResult
    like so:

    \snippet places/requesthandler.h Convert search result

    The implementation is handled in such a way that object slicing is not an issue.

    \sa QPlaceSearchResult
*/

/*!
    Constructs a new place result object.
*/
QPlaceResult::QPlaceResult()
:   QPlaceSearchResult(new QPlaceResultPrivate)
{
}

/*!
    Destructor.
*/
QPlaceResult::~QPlaceResult()
{
}

/*!
    \fn QPlaceResult::QPlaceResult(const QPlaceSearchResult &other)
    Constructs a copy of \a other if possible, otherwise constructs a default place result.
*/
Q_IMPLEMENT_SEARCHRESULT_COPY_CTOR(QPlaceResult)

Q_IMPLEMENT_SEARCHRESULT_D_FUNC(QPlaceResult)

/*!
    Returns the distance of the place to the search center.  This
    field is only relevant provided the search request contained
    a search area with a search center.  Otherwise,
    the distance is NaN indicating an undefined distance.  The default value
    for distance is NaN.
*/
qreal QPlaceResult::distance() const
{
    Q_D(const QPlaceResult);
    return d->distance;
}

/*!
    Set the \a distance of the search result's place from a search center.
*/
void QPlaceResult::setDistance(qreal distance)
{
    Q_D(QPlaceResult);
    d->distance = distance;
}

/*!
    Returns the place of the search result.
*/
QPlace QPlaceResult::place() const
{
    Q_D(const QPlaceResult);
    return d->place;
}

/*!
    Sets the \a place that this result refers to.
*/
void QPlaceResult::setPlace(const QPlace &place)
{
    Q_D(QPlaceResult);
    d->place = place;
}

/*!
    Returns true if the result is a sponsored result.

    \sa setSponsored()
*/
bool QPlaceResult::isSponsored() const
{
    Q_D(const QPlaceResult);
    return d->sponsored;
}

/*!
    Sets whether the result is a \a sponsored result or not.

    \sa isSponsored()
*/
void QPlaceResult::setSponsored(bool sponsored)
{
    Q_D(QPlaceResult);
    d->sponsored = sponsored;
}
