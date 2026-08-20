// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoroutesegment.h"
#include "qgeoroutesegment_p.h"

#include "qgeocoordinate.h"
#include <QDateTime>

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGeoRouteSegmentPrivate)

/*!
    \class QGeoRouteSegment
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoRouteSegment class represents a segment of a route.

    A QGeoRouteSegment instance has information about the physical layout
    of the route segment, the length of the route and estimated time required
    to traverse the route segment and an optional QGeoManeuver associated with
    the beginning of the route segment.

    QGeoRouteSegment instances can be thought of as edges on a routing
    graph, with QGeoManeuver instances as optional labels attached to the
    vertices of the graph.
*/

/*!
    \qmltype routeSegment
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.5

    \brief The routeSegment type represents a segment of a Route.

    A routeSegment instance has information about the physical layout
    of the route segment, the length of the route and estimated time required
    to traverse the route segment and optional \l {routeManeuver}s associated with
    the end of the route segment.

    Instances of routeSegment can be thought of as edges on a routing
    graph, with routeManeuver instances as optional labels attached to the
    vertices of the graph.

    The primary means of acquiring Route objects is via Routes via \l RouteModel.

    \section1 Example

    The following QML snippet demonstrates how to print information about a
    route segment:

    \snippet declarative/routing.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/routing.qml routeSegment
*/

/*!
    Constructs an invalid route segment object.

    The route segment will remain invalid until one of setNextRouteSegment(),
    setTravelTime(), setDistance(), setPath() or setManeuver() is called.
*/
QGeoRouteSegment::QGeoRouteSegment()
    : d_ptr(new QGeoRouteSegmentPrivate()) {}

/*!
    Constructs a route segment object from the contents of \a other.
*/
QGeoRouteSegment::QGeoRouteSegment(const QGeoRouteSegment &other) noexcept = default;

/*!
    \internal
*/
QGeoRouteSegment::QGeoRouteSegment(QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &&dd)
    : d_ptr(dd) {}

/*!
    Destroys this route segment object.
*/
QGeoRouteSegment::~QGeoRouteSegment() = default;


/*!
    Assigns \a other to this route segment object and then returns a
    reference to this route segment object.
*/
QGeoRouteSegment &QGeoRouteSegment::operator=(const QGeoRouteSegment & other) noexcept
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \fn bool QGeoRouteSegment::operator==(const QGeoRouteSegment &lhs, const QGeoRouteSegment &rhs) noexcept
    Returns whether the route segments \a lhs and \a rhs are equal.

    The value of nextRouteSegment() is not considered in the comparison.
*/

/*!
    \fn bool QGeoRouteSegment::operator!=(const QGeoRouteSegment &lhs, const QGeoRouteSegment &rhs) noexcept

    Returns whether the route segments \a lhs and \a rhs are not equal.

    The value of nextRouteSegment() is not considered in the comparison.
*/

bool QGeoRouteSegment::isEqual(const QGeoRouteSegment &other) const noexcept
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
            || (*d_ptr) == (*other.d_ptr));
}

/*!
    Returns whether this route segment is valid or not.

    If nextRouteSegment() is called on the last route segment of a route, the
    returned value will be an invalid route segment.
*/
bool QGeoRouteSegment::isValid() const
{
    return d_ptr->valid();
}

/*!
    Returns whether this route segment is the last segment of a route leg.

    \since 5.12
*/
bool QGeoRouteSegment::isLegLastSegment() const
{
    if (!d_ptr->valid())
        return false;

    if (!d_ptr->nextRouteSegment())
        return true;
    return d_ptr->isLegLastSegment();
}

/*!
    Sets the next route segment in the route to \a routeSegment.
*/
void QGeoRouteSegment::setNextRouteSegment(const QGeoRouteSegment &routeSegment)
{
    d_ptr->setValid(true);
    d_ptr->setNextRouteSegment(routeSegment.d_ptr);
}

/*!
    Returns the next route segment in the route.

    Will return an invalid route segment if this is the last route
    segment in the route.
*/
QGeoRouteSegment QGeoRouteSegment::nextRouteSegment() const
{
    if (d_ptr->valid() && d_ptr->nextRouteSegment())
        return QGeoRouteSegment(d_ptr->nextRouteSegment());

    return QGeoRouteSegment();
}

/*!
    \qmlproperty int QtLocation::routeSegment::travelTime

    Read-only property which holds the estimated amount of time it will take to
    traverse this segment, in seconds.

*/

/*!
    \property QGeoRouteSegment::travelTime
    \brief the estimated amount of time, in seconds, that it will take to
    traverse this segment.
*/
void QGeoRouteSegment::setTravelTime(int secs)
{
    d_ptr->setValid(true);
    d_ptr->setTravelTime(secs);
}

int QGeoRouteSegment::travelTime() const
{
    return d_ptr->travelTime();
}

/*!
    \qmlproperty real QtLocation::routeSegment::distance

    Read-only property which holds the distance covered by this segment of
    the route, in meters.
*/

/*!
    \property QGeoRouteSegment::distance
    \brief the distance covered by this segment of the route, in meters.
*/
void QGeoRouteSegment::setDistance(qreal distance)
{
    d_ptr->setValid(true);
    d_ptr->setDistance(distance);
}

qreal QGeoRouteSegment::distance() const
{
    return d_ptr->distance();
}

/*!
    \qmlproperty list<coordinate> QtLocation::routeSegment::path

    Read-only property which holds the geographical coordinates of this segment.
    Coordinates are listed in the order in which they would be traversed by someone
    traveling along this segment of the route.

    To access individual segments you can use standard list accessors: 'path.length'
    indicates the number of objects and 'path[index starting from zero]' gives
    the actual object.

    \sa QtPositioning::coordinate
*/

/*!
    \property QGeoRouteSegment::path
    \brief the geometric shape of this route segment of the route.

    The coordinates should be listed in the order in which they
    would be traversed by someone traveling along this segment of the route.
*/
void QGeoRouteSegment::setPath(const QList<QGeoCoordinate> &path)
{
    d_ptr->setValid(true);
    d_ptr->setPath(path);
}

QList<QGeoCoordinate> QGeoRouteSegment::path() const
{
    return d_ptr->path();
}

/*!
    \qmlproperty RouteManeuver QtLocation::routeSegment::maneuver

    Read-only property which holds the maneuver for this route segment.

    Will return invalid maneuver if no information has been attached to the endpoint
    of this route segment.
*/

/*!
    \property QGeoRouteSegment::maneuver
    \brief the maneuver for this route segment.

    Holds an invalid QGeoManeuver if no information has been attached
    to the starting point of this route segment.
*/
void QGeoRouteSegment::setManeuver(const QGeoManeuver &maneuver)
{
    d_ptr->setValid(true);
    d_ptr->setManeuver(maneuver);
}

QGeoManeuver QGeoRouteSegment::maneuver() const
{
    return d_ptr->maneuver();
}

/*******************************************************************************
*******************************************************************************/

QGeoRouteSegmentPrivate::QGeoRouteSegmentPrivate() = default;

bool operator==(const QGeoRouteSegmentPrivate &lhs, const QGeoRouteSegmentPrivate &rhs)
{
    return lhs.m_valid == rhs.m_valid
        && lhs.m_travelTime == rhs.m_travelTime
        && lhs.m_distance == rhs.m_distance
        && lhs.m_path == rhs.m_path
        && lhs.m_maneuver == rhs.m_maneuver;
}

bool QGeoRouteSegmentPrivate::valid() const
{
    return m_valid;
}

void QGeoRouteSegmentPrivate::setValid(bool valid)
{
    m_valid = valid;
}

bool QGeoRouteSegmentPrivate::isLegLastSegment() const
{
    return m_legLastSegment;
}

void QGeoRouteSegmentPrivate::setLegLastSegment(bool lastSegment)
{
    m_legLastSegment = lastSegment;
}

int QGeoRouteSegmentPrivate::travelTime() const
{
    return m_travelTime;
}

void QGeoRouteSegmentPrivate::setTravelTime(int travelTime)
{
    m_travelTime = travelTime;
}

qreal QGeoRouteSegmentPrivate::distance() const
{
    return m_distance;
}

void QGeoRouteSegmentPrivate::setDistance(qreal distance)
{
    m_distance = distance;
}

QList<QGeoCoordinate> QGeoRouteSegmentPrivate::path() const
{
    return m_path;
}

void QGeoRouteSegmentPrivate::setPath(const QList<QGeoCoordinate> &path)
{
    m_path = path;
}

QGeoManeuver QGeoRouteSegmentPrivate::maneuver() const
{
    return m_maneuver;
}

void QGeoRouteSegmentPrivate::setManeuver(const QGeoManeuver &maneuver)
{
    m_maneuver = maneuver;
}

QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> QGeoRouteSegmentPrivate::nextRouteSegment() const
{
    return m_nextSegment;
}

void QGeoRouteSegmentPrivate::setNextRouteSegment(const QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &next)
{
    m_nextSegment = next;
}

QGeoRouteSegmentPrivate *QGeoRouteSegmentPrivate::get(QGeoRouteSegment &segment)
{
    return segment.d_ptr.data();
}

QT_END_NAMESPACE

#include "moc_qgeoroutesegment.cpp"
