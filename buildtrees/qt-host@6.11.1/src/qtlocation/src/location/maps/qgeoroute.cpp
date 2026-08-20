// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoroute.h"
#include "qgeoroute_p.h"

#include "qgeorectangle.h"
#include "qgeoroutesegment.h"

#include <QDateTime>
#include <QVariantMap>

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGeoRoutePrivate)

/*!
    \class QGeoRoute
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoRoute class represents a route between two points.

    A QGeoRoute object contains high level information about a route, such
    as the length the route, the estimated travel time for the route,
    and enough information to render a basic image of the route on a map.

    The QGeoRoute object also contains a list of QGeoRouteSegment objecs which
    describe subsections of the route in greater detail.

    Routing information is normally requested using
    QGeoRoutingManager::calculateRoute(), which returns a QGeoRouteReply
    instance. If the operation is completed successfully the routing
    information can be accessed with QGeoRouteReply::routes()

    \sa QGeoRoutingManager
*/

/*!
    \qmlvaluetype route
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.5

    \brief The route type represents one geographical route.

    A route type contains high level information about a route, such
    as the length the route, the estimated travel time for the route,
    and enough information to render a basic image of the route on a map.

    The QGeoRoute object also contains a list of \l routeSegment objects which
    describe subsections of the route in greater detail.

    The primary means of acquiring route objects is \l RouteModel.

    \section1 Example

    This example shows how to display a route's maneuvers in a ListView:

    \snippet declarative/routing.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/routing.qml Route Maneuver List1
    \snippet declarative/routing.qml Route Maneuver List2
    \snippet declarative/routing.qml Route Maneuver List3

*/

/*!
    Constructs a route object.
*/
QGeoRoute::QGeoRoute()
    : d_ptr(new QGeoRoutePrivate())
{}

/*!
    Returns the private implementation.
*/
QExplicitlySharedDataPointer<QGeoRoutePrivate> &QGeoRoute::d()
{
    return d_ptr;
}

const QExplicitlySharedDataPointer<QGeoRoutePrivate> &QGeoRoute::const_d() const
{
    return d_ptr;
}

/*!
    Constructs a route object from the contents of \a other.
*/
QGeoRoute::QGeoRoute(const QGeoRoute &other) noexcept = default;

/*!
    Destroys this route object.
*/
QGeoRoute::~QGeoRoute() = default;

/*!
    Assigns the contents of \a other to this route and returns a reference to
    this route.
*/
QGeoRoute &QGeoRoute::operator=(const QGeoRoute & other) noexcept
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \fn bool QGeoRoute::operator==(const QGeoRoute &lhs, const QGeoRoute &rhs) noexcept

    Returns whether the routes \a lhs and \a rhs are equal.
*/

/*!
    \fn bool QGeoRoute::operator!=(const QGeoRoute &lhs, const QGeoRoute &rhs) noexcept

    Returns whether the routes \a lhs and \a rhs are not equal.
*/

bool QGeoRoute::isEqual(const QGeoRoute &other) const noexcept
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
            || (*d_ptr) == (*other.d_ptr));
}


/*!
    \property QGeoRoute::routeId
    \brief the identifier of this route

    Service providers which support the updating of routes commonly assign
    identifiers to routes. If this route came from such a service provider,
    then changing the identifier will probably cause route updates to stop
    working.
*/
void QGeoRoute::setRouteId(const QString &id)
{
    d_ptr->setId(id);
}

QString QGeoRoute::routeId() const
{
    return d_ptr->id();
}

/*!
    \internal
    \property QGeoRoute::request
    \brief the route request which describes the criteria used in the
           calculation of this route
*/
void QGeoRoute::setRequest(const QGeoRouteRequest &request)
{
    d_ptr->setRequest(request);
}

QGeoRouteRequest QGeoRoute::request() const
{
    return d_ptr->request();
}

/*!
    \qmlproperty geoRectangle QtLocation::route::bounds

    Read-only property which holds a bounding box which encompasses the entire route.
*/

/*!
    \property QGeoRoute::bounds
    \brief the bounding box which encompasses the entire route
*/
void QGeoRoute::setBounds(const QGeoRectangle &bounds)
{
    d_ptr->setBounds(bounds);
}

QGeoRectangle QGeoRoute::bounds() const
{
    return d_ptr->bounds();
}

/*!
    Sets the first route segment in the route to \a routeSegment.
*/
void QGeoRoute::setFirstRouteSegment(const QGeoRouteSegment &routeSegment)
{
    d_ptr->setFirstSegment(routeSegment);
}

/*!
    Returns the first route segment in the route.

    Will return an invalid route segment if there are no route segments
    associated with the route.

    The remaining route segments can be accessed sequentially with
    QGeoRouteSegment::nextRouteSegment.
*/
QGeoRouteSegment QGeoRoute::firstRouteSegment() const
{
    return d_ptr->firstSegment();
}

/*!
    \qmlmethod int QtLocation::route::segmentsCount()

    Returns the number of segments in the route

    \sa routeSegment

    \since 5.11
*/

/*!
    \property QGeoRoute::segmentsCount
    \brief the number of segments in the route
*/
qsizetype QGeoRoute::segmentsCount() const
{
    return d_ptr->segmentsCount();
}

/*!
    \qmlproperty list<routeSegment> QtLocation::route::segments

    Read-only property which holds the list of \l routeSegment objects of this route.

    To access individual segments you can use standard list accessors: 'segments.length'
    indicates the number of objects and 'segments[index starting from zero]' gives
    the actual objects.

    \sa routeSegment
*/

/*!
    \property QGeoRoute::segments
    \brief the list of QGeoRouteSegment objects of this route
*/
QList<QGeoRouteSegment> QGeoRoute::segments() const
{
    return d_ptr->segments();
}

/*!
    \qmlproperty int QtLocation::route::travelTime

    Read-only property which holds the estimated amount of time it will take to
    traverse this route, in seconds.
*/

/*!
    \property QGeoRoute::travelTime
    \brief the estimated amount of time it will take to traverse this route,
           in seconds
*/
void QGeoRoute::setTravelTime(int secs)
{
    d_ptr->setTravelTime(secs);
}

int QGeoRoute::travelTime() const
{
    return d_ptr->travelTime();
}

/*!
    \qmlproperty real QtLocation::route::distance

    Read-only property which holds distance covered by this route, in meters.
*/

/*!
    \property QGeoRoute::distance
    \brief the distance covered by this route, in meters
*/
void QGeoRoute::setDistance(qreal distance)
{
    d_ptr->setDistance(distance);
}

qreal QGeoRoute::distance() const
{
    return d_ptr->distance();
}

/*!
    Sets the travel mode for this route to \a mode.

    This should be one of the travel modes returned by request().travelModes().
*/
void QGeoRoute::setTravelMode(QGeoRouteRequest::TravelMode mode)
{
    d_ptr->setTravelMode(mode);
}

/*!
    Returns the travel mode for the this route.

    This should be one of the travel modes returned by request().travelModes().
*/
QGeoRouteRequest::TravelMode QGeoRoute::travelMode() const
{
    return d_ptr->travelMode();
}

/*!
    \qmlproperty list<coordinate> QtLocation::route::path

    Read-only property which holds the geographical coordinates of this route.
    Coordinates are listed in the order in which they would be traversed by someone
    traveling along this segment of the route.

    To access individual segments you can use standard list accessors: 'path.length'
    indicates the number of objects and 'path[index starting from zero]' gives
    the actual object.

    \sa QtPositioning::coordinate
*/

/*!
    \property QGeoRoute::path
    \brief the geometric shape of the route

    The coordinates should be listed in the order in which they
    would be traversed by someone traveling along this segment of the route.
*/
void QGeoRoute::setPath(const QList<QGeoCoordinate> &path)
{
    d_ptr->setPath(path);
}


QList<QGeoCoordinate> QGeoRoute::path() const
{
    return d_ptr->path();
}

/*!
    \qmlproperty list<route> QtLocation::route::legs

    Returns the route legs associated with this route.
    Route legs are the sub-routes between each two adjacent waypoints.
    The result may be empty, if this level of detail is not supported by the
    backend.

    \since QtLocation 5.12
*/

/*!
    \property QGeoRoute::routeLegs
    \brief the route \a legs for a multi-waypoint route

    \since 5.12
*/
void QGeoRoute::setRouteLegs(const QList<QGeoRoute> &legs)
{
    d_ptr->setRouteLegs(legs);
}

QList<QGeoRoute> QGeoRoute::routeLegs() const
{
    return d_ptr->routeLegs();
}


/*!
    \qmlproperty Object route::extendedAttributes

    This property holds the extended attributes of the route and is a map.
    These attributes are plugin specific, and can be empty.

    Consult the \l {Qt Location#Plugin References and Parameters}{plugin documentation}
    for what attributes are supported and how they should be used.

    Note, due to limitations of the QQmlPropertyMap, it is not possible
    to declaratively specify the attributes in QML, assignment of attributes keys
    and values can only be accomplished by JavaScript.

    \since QtLocation 5.13
*/

/*!
    \property QGeoRoute::extendedAttributes
    \brief the extended attributes associated with this route

    \since 5.13
*/
void QGeoRoute::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    d_ptr->setExtendedAttributes(extendedAttributes);
}

QVariantMap QGeoRoute::extendedAttributes() const
{
    return d_ptr->extendedAttributes();
}

/*!
    \qmlproperty int QtLocation::route::legIndex

    Read-only property which holds the index of the leg within the containing route's
    list of QtLocation::route::legs. The index is -1 if this route is not a leg within
    an overall route.

    \sa overallRoute
*/

/*!
    \property QGeoRoute::legIndex
    \brief the leg index of this route

    The index of the leg inside the containing QGeoRoute::routeLegs list
    can be used to find the next legs.
*/
void QGeoRoute::setLegIndex(int idx)
{
    d()->setLegIndex(idx);
}

int QGeoRoute::legIndex() const
{
    return const_d()->legIndex();
}

/*!
    \qmlproperty Route QtLocation::route::overallRoute

    Read-only property which holds the route that contains this leg.
*/

/*!
    \property QGeoRoute::overallRoute
    \brief the route that contains this route leg

    This this route is not a leg within an overall route, then this property
    holds an empty route.
*/
void QGeoRoute::setOverallRoute(const QGeoRoute &route)
{
    d()->setContainingRoute(route);
}

QGeoRoute QGeoRoute::overallRoute() const
{
    return const_d()->containingRoute();
}

/*******************************************************************************
*******************************************************************************/

bool QGeoRoutePrivate::operator ==(const QGeoRoutePrivate &other) const
{
    return equals(other);
}

bool QGeoRoutePrivate::equals(const QGeoRoutePrivate &other) const
{
    // here both routes are of type QGeoRoutePrivate
    QGeoRouteSegment s1 = firstSegment();
    QGeoRouteSegment s2 = other.firstSegment();

    while (true) {
        if (s1.isValid() != s2.isValid())
            return false;
        if (!s1.isValid())
            break;
        if (s1 != s2)
            return false;
        s1 = s1.nextRouteSegment();
        s2 = s2.nextRouteSegment();
    }

    return id() == other.id()
        && request() == other.request()
        && bounds() == other.bounds()
        && travelTime() == other.travelTime()
        && distance() == other.distance()
        && travelMode() == other.travelMode()
        && path() == other.path()
        && routeLegs() == other.routeLegs()
        && extendedAttributes() == other.extendedAttributes();
}

void QGeoRoutePrivate::setId(const QString &id)
{
    m_id = id;
}

QString QGeoRoutePrivate::id() const
{
    return m_id;
}

void QGeoRoutePrivate::setRequest(const QGeoRouteRequest &request)
{
    m_request = request;
}

QGeoRouteRequest QGeoRoutePrivate::request() const
{
    return m_request;
}

void QGeoRoutePrivate::setBounds(const QGeoRectangle &bounds)
{
    m_bounds = bounds;
}

QGeoRectangle QGeoRoutePrivate::bounds() const
{
    return m_bounds;
}

void QGeoRoutePrivate::setTravelTime(int travelTime)
{
    m_travelTime = travelTime;
}

int QGeoRoutePrivate::travelTime() const
{
    return m_travelTime;
}

void QGeoRoutePrivate::setDistance(qreal distance)
{
    m_distance = distance;
}

qreal QGeoRoutePrivate::distance() const
{
    return m_distance;
}

void QGeoRoutePrivate::setTravelMode(QGeoRouteRequest::TravelMode mode)
{
    m_travelMode = mode;
}

QGeoRouteRequest::TravelMode QGeoRoutePrivate::travelMode() const
{
    return m_travelMode;
}

void QGeoRoutePrivate::setPath(const QList<QGeoCoordinate> &path)
{
    m_path = path;
}

QList<QGeoCoordinate> QGeoRoutePrivate::path() const
{
    return m_path;
}

void QGeoRoutePrivate::setFirstSegment(const QGeoRouteSegment &firstSegment)
{
    m_firstSegment = firstSegment;
    m_numSegments = -1;
}

QGeoRouteSegment QGeoRoutePrivate::firstSegment() const
{
    return m_firstSegment;
}

int QGeoRoutePrivate::segmentsCount() const
{
    if (m_numSegments >= 0)
        return m_numSegments;

    int count = 0;
    forEachSegment([&count](const QGeoRouteSegment &){
        ++count;
    });
    m_numSegments = count;
    return count;
}

QList<QGeoRouteSegment> QGeoRoutePrivate::segments() const
{
    QList<QGeoRouteSegment> segments;
    forEachSegment([&segments](const QGeoRouteSegment &segment){
        segments.append(segment);
    });
    return segments;
}

void QGeoRoutePrivate::setRouteLegs(const QList<QGeoRoute> &legs)
{
    m_legs = legs;
}

QList<QGeoRoute> QGeoRoutePrivate::routeLegs() const
{
    return m_legs;
}

void QGeoRoutePrivate::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    m_extendedAttributes = extendedAttributes;
}

QVariantMap QGeoRoutePrivate::extendedAttributes() const
{
    return m_extendedAttributes;
}

void QGeoRoutePrivate::setLegIndex(int idx)
{
    if (idx >= 0)
        m_legIndex = idx;
}

int QGeoRoutePrivate::legIndex() const
{
    return m_legIndex;
}

void QGeoRoutePrivate::setContainingRoute(const QGeoRoute &route)
{
    m_containingRoute.reset(new QGeoRoute(route));
}

QGeoRoute QGeoRoutePrivate::containingRoute() const
{
    if (m_containingRoute)
        return *m_containingRoute;
    return QGeoRoute();
}

QT_END_NAMESPACE

#include "moc_qgeoroute.cpp"
