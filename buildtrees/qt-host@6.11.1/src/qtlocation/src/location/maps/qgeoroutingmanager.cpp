// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoroutingmanager.h"
#include "qgeoroutingmanager_p.h"
#include "qgeoroutingmanagerengine.h"

#include <QLocale>

QT_BEGIN_NAMESPACE

/*!
    \class QGeoRoutingManager
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoRoutingManager class provides support for geographic routing
    operations.

    The calculateRoute() and updateRoute() methods function QGeoRouteReply
    objects, which manage these operations and report on the result of the
    operations and any errors which may have occurred.

    The calculateRoute() function is used to find a route (or routes) that
    follows a set of waypoints and matches various other criteria.  The
    QGeoRouteRequest class is used to specify this information.

    If supportsRouteUpdates() returns true then the QGeoRoutingManager
    supports updating route information based on position updates.  This
    will cause the travel time and distance estimates to be updated, and
    any QGeoRouteSegments already traversed to be removed from the route.

    The updates can be triggered with the updateRoute() function, which makes
    use of the QGeoPositionInfo instances emitted as position updates by
    QGeoPositionInfoSource.

    Instances of QGeoRoutingManager can be accessed with
    QGeoServiceProvider::routingManager().

    A small example of the usage of QGeoRoutingManager and QGeoRouteRequests
    follows:
    \snippet maps/routehandler.h RouteHandler
*/

/*!
    Constructs a new manager with the specified \a parent and with the
    implementation provided by \a engine.

    This constructor is used internally by QGeoServiceProviderFactory. Regular
    users should acquire instances of QGeoRoutingManager with
    QGeoServiceProvider::routingManager();
*/
QGeoRoutingManager::QGeoRoutingManager(QGeoRoutingManagerEngine *engine, QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoRoutingManagerPrivate())
{
    d_ptr->engine.reset(engine);
    if (d_ptr->engine) {
        d_ptr->engine->setParent(this);

        connect(d_ptr->engine.get(), &QGeoRoutingManagerEngine::finished,
                this, &QGeoRoutingManager::finished);

        connect(d_ptr->engine.get(), &QGeoRoutingManagerEngine::errorOccurred,
                this, &QGeoRoutingManager::errorOccurred);
    } else {
        qFatal("The routing manager engine that was set for this routing manager was NULL.");
    }
}

/*!
    Destroys this manager.
*/
QGeoRoutingManager::~QGeoRoutingManager()
{
    delete d_ptr;
}

/*!
    Returns the name of the engine which implements the behaviour of this
    routing manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
QString QGeoRoutingManager::managerName() const
{
    return d_ptr->engine->managerName();
}

/*!
    Returns the version of the engine which implements the behaviour of this
    routin manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
int QGeoRoutingManager::managerVersion() const
{
    return d_ptr->engine->managerVersion();
}

/*!
    Begins the calculation of the route specified by \a request.

    A QGeoRouteReply object will be returned, which can be used to manage the
    routing operation and to return the results of the operation.

    This manager and the returned QGeoRouteReply object will emit signals
    indicating if the operation completes or if errors occur.

    Once the operation has completed, QGeoRouteReply::routes can be used to
    retrieve the calculated route or routes.

    If \a request includes features which are not supported by this manager, as
    reported by the methods in this manager, then a
    QGeoRouteReply::UnsupportedOptionError will occur.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoRoutingManager::finished(),
    QGeoRoutingManager::errorOccurred(), QGeoRouteReply::finished() or
    QGeoRouteReply::errorOccurred() with deleteLater().
*/
QGeoRouteReply *QGeoRoutingManager::calculateRoute(const QGeoRouteRequest &request)
{
    return d_ptr->engine->calculateRoute(request);
}

/*!
    Begins the process of updating \a route based on the current position \a
    position.

    A QGeoRouteReply object will be returned, which can be used to manage the
    routing operation and to return the results of the operation.

    This manager and the returned QGeoRouteReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsRouteUpdates() returns false an
    QGeoRouteReply::UnsupportedOptionError will occur.

    Once the operation has completed, QGeoRouteReply::routes can be used to
    retrieve the updated route.

    The returned route could be entirely different to the original route,
    especially if \a position is far away from the initial route.
    Otherwise the route will be similar, although the remaining time and
    distance will be updated and any segments of the original route which
    have been traversed will be removed.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoRoutingManager::finished(),
    QGeoRoutingManager::errorOccurred(), QGeoRouteReply::finished() or
    QGeoRouteReply::errorOccurred() with deleteLater().
*/
QGeoRouteReply *QGeoRoutingManager::updateRoute(const QGeoRoute &route, const QGeoCoordinate &position)
{
    return d_ptr->engine->updateRoute(route, position);
}

/*!
    Returns the travel modes supported by this manager.
*/
QGeoRouteRequest::TravelModes QGeoRoutingManager::supportedTravelModes() const
{
    return d_ptr->engine->supportedTravelModes();
}

/*!
    Returns the types of features that this manager can take into account
    during route planning.
*/
QGeoRouteRequest::FeatureTypes QGeoRoutingManager::supportedFeatureTypes() const
{
    return d_ptr->engine->supportedFeatureTypes();
}

/*!
    Returns the weightings which this manager can apply to different features
    during route planning.
*/
QGeoRouteRequest::FeatureWeights QGeoRoutingManager::supportedFeatureWeights() const
{
    return d_ptr->engine->supportedFeatureWeights();
}

/*!
    Returns the route optimizations supported by this manager.
*/
QGeoRouteRequest::RouteOptimizations QGeoRoutingManager::supportedRouteOptimizations() const
{
    return d_ptr->engine->supportedRouteOptimizations();
}

/*!
    Returns the levels of detail for routing segments which can be requested
    with this manager.
*/
QGeoRouteRequest::SegmentDetails QGeoRoutingManager::supportedSegmentDetails() const
{
    return d_ptr->engine->supportedSegmentDetails();
}

/*!
    Returns the levels of detail for navigation maneuvers which can be
    requested by this manager.
*/
QGeoRouteRequest::ManeuverDetails QGeoRoutingManager::supportedManeuverDetails() const
{
    return d_ptr->engine->supportedManeuverDetails();
}

/*!
    Sets the locale to be used by this manager to \a locale.

    If this routing manager supports returning addresses and instructions
    in different languages, they will be returned in the language of \a locale.

    The locale used defaults to the system locale if this is not set.
*/
void QGeoRoutingManager::setLocale(const QLocale &locale)
{
    d_ptr->engine->setLocale(locale);
}

/*!
    Returns the locale used to hint to this routing manager about what
    language to use for addresses and instructions.
*/
QLocale QGeoRoutingManager::locale() const
{
    return d_ptr->engine->locale();
}

/*!
    Sets the measurement system used by this manager to \a system.

    The measurement system can be set independently of the locale. Both setLocale() and this
    function set the measurement system. The value set by the last function called will be used.

    \sa measurementSystem(), locale(), setLocale()
*/
void QGeoRoutingManager::setMeasurementSystem(QLocale::MeasurementSystem system)
{
    d_ptr->engine->setMeasurementSystem(system);
}

/*!
    Returns the measurement system used by this manager.

    If setMeasurementSystem() has been called then the value returned by this function may be
    different to that returned by locale().\l {QLocale::measurementSystem()}{measurementSystem()}.
    In which case the value returned by this function is what will be used by the manager.

    \sa setMeasurementSystem(), setLocale()
*/
QLocale::MeasurementSystem QGeoRoutingManager::measurementSystem() const
{
    return d_ptr->engine->measurementSystem();
}

/*!
\fn void QGeoRoutingManager::finished(QGeoRouteReply *reply)

This signal is emitted when \a reply has finished processing.

If reply::error() equals QGeoRouteReply::NoError then the processing
finished successfully.

This signal and QGeoRouteReply::finished() will be emitted at the same time.

\note Do not delete the \a reply object in the slot connected to this signal.
Use deleteLater() instead.
*/

/*!
\fn void QGeoRoutingManager::errorOccurred(QGeoRouteReply *reply, QGeoRouteReply::Error error, const QString &errorString)

This signal is emitted when an error has been detected in the processing of
\a reply.  The QGeoRoutingManager::finished() signal will probably follow.

The error will be described by the error code \a error.  If \a errorString is
not empty it will contain a textual description of the error.

This signal and QGeoRouteReply::errorOccurred() will be emitted at the same time.

\note Do not delete the \a reply object in the slot connected to this signal.
Use deleteLater() instead.
*/

/*******************************************************************************
*******************************************************************************/

QT_END_NAMESPACE
