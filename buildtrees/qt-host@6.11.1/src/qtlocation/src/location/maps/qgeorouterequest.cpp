// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeorouterequest.h"
#include "qgeorouterequest_p.h"

#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoRectangle>

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGeoRouteRequestPrivate)

/*!
    \class QGeoRouteRequest
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoRouteRequest class represents the parameters and restrictions
    which define a request for routing information.

    The default state of a QGeoRouteRequest instance will result in a request
    for basic route segment and navigation maneuvers describing the fastest
    route by car which covers the given waypoints.

    There may be significant variation in the features supported by different
    providers of routing information, or even in the features supported by
    the same provider if different levels of authorization are used.

    There are several functions in QGeoRoutingManager which can be used to
    check which features are supported with the current provider and
    authorization level.
    \sa QGeoRoutingManager
*/

/*
    DESIGN NOTE

    There are plans to make this extensible by allowing the user to set a
    list of QGeoTransportOptions (or QGeoTransitOptions).  We don't have any
    subclasses for that just yet, otherwise they'd be present.

    A QGeoPublicTransportOption subclass would allow users to specify things
    like cost limits, the maximum number of changes of vehicle, the maximum
    walking time / distance between changes of vehicle.

    There's Nokia / Navteq support for specifying things like Truck attributes
    so you can route around various road restrictions and conditions which
    effect trucks.

    A QGeoTrafficAwareTransportOption is probably also a good place to put the
    inputs for the various traffic / time aware bits of information. A
    departure / arrrival time could be set, and the presence of this transport
    options subclass (and the fact that user auth said that the user had
    support) would mean we could provide better values for the estimated
    travel times and so on.

    This all relies on at least one service making this data available to us,
    which would probably be tied to token based authorization.  It could be
    some time before this becomes available.
*/

/*!
    \enum QGeoRouteRequest::TravelMode

    Defines modes of travel to be used for a route.

    \value CarTravel
        The route will be optimized for someone who is driving a car.
    \value PedestrianTravel
        The route will be optimized for someone who is walking.
    \value BicycleTravel
        The route will be optimized for someone who is riding a bicycle.
    \value PublicTransitTravel
        The route will be optimized for someone who is making use of public transit.
    \value TruckTravel
        The route will be optimized for someone who is driving a truck.
*/

/*!
    \enum QGeoRouteRequest::FeatureType

    Defines a feature which is important to the planning of a route.

    These values will be used in combination with
    QGeoRouteRequest::FeatureWeight to determine if they should or should
    not be part of the route.

    \value NoFeature
        Used by QGeoRoutingManager::supportedFeatureTypes() to indicate that
        no features will be taken into account when planning the route.
    \value TollFeature
        Consdier tollways when planning the route.
    \value HighwayFeature
        Consider highways when planning the route.
    \value PublicTransitFeature
        Consider public transit when planning the route.
    \value FerryFeature
        Consider ferries when planning the route.
    \value TunnelFeature
        Consider tunnels when planning the route.
    \value DirtRoadFeature
        Consider dirt roads when planning the route.
    \value ParksFeature
        Consider parks when planning the route.
    \value MotorPoolLaneFeature
        Consider motor pool lanes when planning the route.
    \value TrafficFeature
        Consider the current traffic situation when planning the route. Since QtLocation 5.10
*/

/*!
    \enum QGeoRouteRequest::FeatureWeight

    Defines the weight to associate with a feature during  the
    planning of a route.

    These values will be used in combination with
    QGeoRouteRequest::Feature to determine if they should or should
    not be part of the route.

    \value NeutralFeatureWeight
        The presence or absence of the feature will not affect the
        planning of the route.
    \value PreferFeatureWeight
        Routes which contain the feature will be preferred over those that do
        not.
    \value RequireFeatureWeight
        Only routes which contain the feature will be considered, otherwise
        no route will be returned.
    \value AvoidFeatureWeight
        Routes which do not contain the feature will be preferred over those
        that do.
    \value DisallowFeatureWeight
        Only routes which do not contain the feature will be considered,
        otherwise no route will be returned.
*/

// TODO improve description of MostScenicRoute
/*!
    \enum QGeoRouteRequest::RouteOptimization

    Defines the type of optimization which is applied to the planning of the route.

    \value ShortestRoute
        Minimize the length of the journey.
    \value FastestRoute
        Minimize the traveling time for the journey.
    \value MostEconomicRoute
        Minimize the cost of the journey.
    \value MostScenicRoute
        Maximize the scenic potential of the journey.
*/

/*!
    \enum QGeoRouteRequest::SegmentDetail

    Defines the amount of route segment information that should be included
    with the route.

    \value NoSegmentData
        No segment data should be included with the route.  A route requested
        with this level of segment detail will initialize
        QGeoRouteSegment::path() as a straight line between the positions of
        the previous and next QGeoManeuver instances.

    \value BasicSegmentData
        Basic segment data will be included with the route.  This will include
        QGeoRouteSegment::path().
*/

/*!
    \enum QGeoRouteRequest::ManeuverDetail

    Defines the amount of maneuver information that should be included with
    the route.

    \value NoManeuvers
        No maneuvers should be included with the route.

    \value BasicManeuvers
        Basic manevuers will be included with the route. This will
        include QGeoManeuver::instructionText().
*/

/*!
    Constructs a request to calculate a route through the coordinates \a waypoints.

    The route will traverse the objects of \a waypoints in order.
*/
QGeoRouteRequest::QGeoRouteRequest(const QList<QGeoCoordinate> &waypoints)
    : d_ptr(new QGeoRouteRequestPrivate())
{
    d_ptr->waypoints = waypoints;
}

/*!
    Constructs a request to calculate a route between \a origin and
    \a destination.
*/
QGeoRouteRequest::QGeoRouteRequest(const QGeoCoordinate &origin, const QGeoCoordinate &destination)
    : d_ptr(new QGeoRouteRequestPrivate())
{
    d_ptr->waypoints.append(origin);
    d_ptr->waypoints.append(destination);
}

/*!
    Constructs a route request object from the contents of \a other.
*/
QGeoRouteRequest::QGeoRouteRequest(const QGeoRouteRequest &other) noexcept = default;

/*!
    Destroys the request.
*/
QGeoRouteRequest::~QGeoRouteRequest() = default;

/*!
    Assigns \a other to this route request object and then returns a reference
    to this route request object.
*/
QGeoRouteRequest &QGeoRouteRequest::operator=(const QGeoRouteRequest & other) noexcept
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \fn bool QGeoRouteRequest::operator==(const QGeoRouteRequest &lhs, const QGeoRouteRequest &rhs) noexcept

    Returns whether the route requests \a lhs and \a rhs are equal.
*/

/*!
    \fn bool QGeoRouteRequest::operator!=(const QGeoRouteRequest &lhs, const QGeoRouteRequest &rhs) noexcept

    Returns whether the route requests \a lhs and \a rhs are not equal.
*/

bool QGeoRouteRequest::isEqual(const QGeoRouteRequest &other) const noexcept
{
    return ((d_ptr.constData() == other.d_ptr.constData())
                        || (*d_ptr) == (*other.d_ptr));
}

/*!
    Sets \a waypoints as the waypoints that the route should pass through.

    The waypoints should be given in order from origin to destination.

    This request will be invalid until the waypoints have been set to a
    list containing two or more coordinates.
*/
void QGeoRouteRequest::setWaypoints(const QList<QGeoCoordinate> &waypoints)
{
    d_ptr->waypoints = waypoints;
}

/*!
    Returns the waypoints that the route will pass through.
*/
QList<QGeoCoordinate> QGeoRouteRequest::waypoints() const
{
    return d_ptr->waypoints;
}

/*!
    Sets \a areas as excluded areas that the route must not cross.
*/
void QGeoRouteRequest::setExcludeAreas(const QList<QGeoRectangle> &areas)
{
    d_ptr->excludeAreas = areas;
}

/*!
    Returns areas the route must not cross.
*/
QList<QGeoRectangle> QGeoRouteRequest::excludeAreas() const
{
    return d_ptr->excludeAreas;
}

/*!
    Sets the number of alternative routes to request to \a alternatives. If \a alternatives is
    negative the number of alternative routes is set to 0.

    The default value is 0.
*/
void QGeoRouteRequest::setNumberAlternativeRoutes(int alternatives)
{
    d_ptr->numberAlternativeRoutes = qMax(0, alternatives);
}

/*!
    Returns the number of alternative routes which will be requested.
*/
int QGeoRouteRequest::numberAlternativeRoutes() const
{
    return d_ptr->numberAlternativeRoutes;
}

/*!
    Sets the travel modes which should be considered during the planning of the
    route to \a travelModes.

    The default value is QGeoRouteRequest::CarTravel.
*/
void QGeoRouteRequest::setTravelModes(QGeoRouteRequest::TravelModes travelModes)
{
    d_ptr->travelModes = travelModes;
}

/*!
    Returns the travel modes which this request specifies should be considered
    during the planning of the route.
*/
QGeoRouteRequest::TravelModes QGeoRouteRequest::travelModes() const
{
    return d_ptr->travelModes;
}

/*!
    Assigns the weight \a featureWeight to the feature \a featureType during
    the planning of the route.

    By default all features are assigned a weight of NeutralFeatureWeight.

    It is impossible to assign a weight to QGeoRouteRequest::NoFeature.
*/
void QGeoRouteRequest::setFeatureWeight(QGeoRouteRequest::FeatureType featureType, QGeoRouteRequest::FeatureWeight featureWeight)
{
    if (featureWeight != QGeoRouteRequest::NeutralFeatureWeight) {
        if (featureType != QGeoRouteRequest::NoFeature)
            d_ptr->featureWeights[featureType] = featureWeight;
    } else {
        d_ptr->featureWeights.remove(featureType);
    }
}

/*!
    Returns the weight assigned to \a featureType in the planning of the route.

    If no feature weight has been specified for \a featureType then
    NeutralFeatureWeight will be returned.
*/
QGeoRouteRequest::FeatureWeight QGeoRouteRequest::featureWeight(QGeoRouteRequest::FeatureType featureType) const
{
    return d_ptr->featureWeights.value(featureType, QGeoRouteRequest::NeutralFeatureWeight);
}

/*!
    Returns the list of features that will be considered when planning the
    route.  Features with a weight of NeutralFeatureWeight will not be returned.
*/
QList<QGeoRouteRequest::FeatureType> QGeoRouteRequest::featureTypes() const
{
    return d_ptr->featureWeights.keys();
}

/*!
    Sets the optimization criteria to use while planning the route to
    \a optimization.

    The default value is QGeoRouteRequest::FastestRoute.
*/
void QGeoRouteRequest::setRouteOptimization(QGeoRouteRequest::RouteOptimizations optimization)
{
    d_ptr->routeOptimization = optimization;
}

/*!
    Returns the optimization criteria which this request specifies should be
    used while planning the route.
*/
QGeoRouteRequest::RouteOptimizations QGeoRouteRequest::routeOptimization() const
{
    return d_ptr->routeOptimization;
}

/*!
    Sets the level of detail to use when representing routing segments to
    \a segmentDetail.
*/
void QGeoRouteRequest::setSegmentDetail(QGeoRouteRequest::SegmentDetail segmentDetail)
{
    d_ptr->segmentDetail = segmentDetail;
}

/*!
    Returns the level of detail which will be used in the representation of
    routing segments.
*/
QGeoRouteRequest::SegmentDetail QGeoRouteRequest::segmentDetail() const
{
    return d_ptr->segmentDetail;
}

/*!
    Sets the level of detail to use when representing routing maneuvers to
    \a maneuverDetail.

    The default value is QGeoRouteRequest::BasicManeuvers.
*/
void QGeoRouteRequest::setManeuverDetail(QGeoRouteRequest::ManeuverDetail maneuverDetail)
{
    d_ptr->maneuverDetail = maneuverDetail;
}

/*!
    Returns the level of detail which will be used in the representation of
    routing maneuvers.
*/
QGeoRouteRequest::ManeuverDetail QGeoRouteRequest::maneuverDetail() const
{
    return d_ptr->maneuverDetail;
}

/*!
    Sets the departure time \a departureTime for the route calculation. This
    information can be used by the backend to calculate a faster route, for
    example, by avoiding traffic congestion during rush hour.

    The default value is an invalid QDateTime.

    \since 5.13
*/
void QGeoRouteRequest::setDepartureTime(const QDateTime &departureTime)
{
    d_ptr->departureTime = departureTime;
}

/*!
    Returns the departure time in the request.

    \since 5.13
*/
QDateTime QGeoRouteRequest::departureTime() const
{
    return d_ptr->departureTime;
}

/*******************************************************************************
*******************************************************************************/

bool QGeoRouteRequestPrivate::operator==(const QGeoRouteRequestPrivate &other) const
{
    return ((waypoints == other.waypoints)
            && (excludeAreas == other.excludeAreas)
            && (numberAlternativeRoutes == other.numberAlternativeRoutes)
            && (travelModes == other.travelModes)
            && (featureWeights == other.featureWeights)
            && (routeOptimization == other.routeOptimization)
            && (segmentDetail == other.segmentDetail)
            && (maneuverDetail == other.maneuverDetail));
}

QT_END_NAMESPACE
