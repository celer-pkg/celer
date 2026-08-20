// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeoroutemodel_p.h"
#include "error_messages_p.h"

#include <QtCore/QCoreApplication>
#include <QtQml/QQmlEngine>
#include <QtQml/qqmlinfo.h>
#include <QtLocation/QGeoRoutingManager>
#include <QtPositioning/QGeoRectangle>

QT_BEGIN_NAMESPACE

/*!
    \qmltype RouteModel
    \nativetype QDeclarativeGeoRouteModel
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.5

    \brief The RouteModel type provides access to routes.

    The RouteModel type is used as part of a model/view grouping to retrieve
    geographic routes from a backend provider. Routes include data about driving
    directions between two points, walking directions with multiple waypoints,
    and various other similar concepts. It functions much like other Model
    types in QML (see for example \l {Models and Views in Qt Quick#Models}{ListModel}),
    and interacts with views such as \l MapItemView, and \l{ListView}.

    Like \l Map and \l GeocodeModel, all the data for a RouteModel to work comes
    from a services plugin. This is contained in the \l{plugin} property, and
    this must be set before the RouteModel can do any useful work.

    Once the plugin is set, create a \l RouteQuery with the appropriate
    waypoints and other settings, and set the RouteModel's \l{query}
    property. If \l autoUpdate is enabled, the update will being automatically.
    Otherwise, the \l{update} method may be used. By default, autoUpdate is
    disabled.

    The data stored and returned in the RouteModel consists of \l route objects,
    as a list with the role name "routeData". See the documentation for \l route
    for further details on its structure and contents.

    \section2 Example Usage

    The following snippet is two-part, showing firstly the declaration of
    objects, and secondly a short piece of procedural code using it. We set
    the routeModel's \l{autoUpdate} property to false, and call \l{update} once
    the query is set up, to avoid useless extra requests halfway through the
    set up of the query.

    \code
    Plugin {
        id: aPlugin
        name: "osm"
    }

    RouteQuery {
        id: aQuery
    }

    RouteModel {
        id: routeModel
        plugin: aPlugin
        query: aQuery
        autoUpdate: false
    }
    \endcode

    \code
    {
        aQuery.addWaypoint(...)
        aQuery.addWaypoint(...)
        aQuery.travelModes = ...
        routeModel.update()
    }
    \endcode

*/

QDeclarativeGeoRouteModel::QDeclarativeGeoRouteModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

/*!
    \qmlproperty int QtLocation::RouteModel::count

    This property holds how many routes the model currently has.
    Amongst other uses, you can use this value when accessing routes
    via the QtLocation::RouteModel::get -method.
*/

int QDeclarativeGeoRouteModel::count() const
{
    return routes_.count();
}

/*!
    \qmlmethod void QtLocation::RouteModel::reset()

    Resets the model. All route data is cleared, any outstanding requests
    are aborted and possible errors are cleared. Model status will be set
    to RouteModel.Null
*/

void QDeclarativeGeoRouteModel::reset()
{
    if (!routes_.isEmpty()) {
        beginResetModel();
        routes_.clear();
        emit countChanged();
        emit routesChanged();
        endResetModel();
    }

    emit abortRequested();
    setError(NoError, QString());
    setStatus(QDeclarativeGeoRouteModel::Null);
}

/*!
    \qmlmethod void QtLocation::RouteModel::cancel()

    Cancels any outstanding requests and clears errors.  Model status will be set to either
    RouteModel.Null or RouteModel.Ready.
*/
void QDeclarativeGeoRouteModel::cancel()
{
    emit abortRequested();
    setError(NoError, QString());
    setStatus(routes_.isEmpty() ? Null : Ready);
}

/*!
    \qmlmethod route QtLocation::RouteModel::get(int index)

    Returns the route at the specified \a index. Use the \l count
    property to check the amount of routes available. The routes
    are indexed from zero, so the accessible range is 0...(count - 1).

    If you access out of bounds, an empty route is returned and
    a warning is issued.
*/

QGeoRoute QDeclarativeGeoRouteModel::get(int index)
{
    if (index < 0 || index >= routes_.count()) {
        qmlWarning(this) << QStringLiteral("Index '%1' out of range").arg(index);
        return QGeoRoute();
    }
    return routes_.at(index);
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::componentComplete()
{
    complete_ = true;
    if (autoUpdate_) {
        update();
    }
}

/*!
    \internal
*/
int QDeclarativeGeoRouteModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return routes_.count();
}

/*!
    \internal
*/
QVariant QDeclarativeGeoRouteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qmlWarning(this) << QStringLiteral("Error in indexing route model's data (invalid index).");
        return QVariant();
    }

    if (index.row() >= routes_.count()) {
        qmlWarning(this) << QStringLiteral("Fatal error in indexing route model's data (index overflow).");
        return QVariant();
    }

    if (role == RouteRole)
        return QVariant::fromValue(routes_.at(index.row()));
    return QVariant();
}

QHash<int, QByteArray> QDeclarativeGeoRouteModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames.insert(RouteRole, "routeData");
    return roleNames;
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (plugin_ == plugin)
        return;

    reset(); // reset the model

    if (plugin_) {
        disconnect(plugin_, &QDeclarativeGeoServiceProvider::localesChanged,
                   this, &QDeclarativeGeoRouteModel::measurementSystemChanged);
    }
    if (plugin) {
        connect(plugin, &QDeclarativeGeoServiceProvider::localesChanged,
                this, &QDeclarativeGeoRouteModel::measurementSystemChanged);
    }

    plugin_ = plugin;

    if (complete_)
        emit pluginChanged();

    if (!plugin)
        return;

    if (plugin_->isAttached()) {
        pluginReady();
    } else {
        connect(plugin_, &QDeclarativeGeoServiceProvider::attached,
                this, &QDeclarativeGeoRouteModel::pluginReady);
    }
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::pluginReady()
{
    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    QGeoRoutingManager *routingManager = serviceProvider->routingManager();

    if (serviceProvider->routingError() != QGeoServiceProvider::NoError) {
        QDeclarativeGeoRouteModel::RouteError newError = UnknownError;
        switch (serviceProvider->routingError()) {
        case QGeoServiceProvider::NotSupportedError:
            newError = EngineNotSetError; break;
        case QGeoServiceProvider::UnknownParameterError:
            newError = UnknownParameterError; break;
        case QGeoServiceProvider::MissingRequiredParameterError:
            newError = MissingRequiredParameterError; break;
        case QGeoServiceProvider::ConnectionError:
            newError = CommunicationError; break;
        default:
            break;
        }

        setError(newError, serviceProvider->routingErrorString());
        return;
    }

    if (!routingManager) {
        setError(EngineNotSetError, tr("Plugin does not support routing."));
        return;
    }

    connect(routingManager, &QGeoRoutingManager::finished,
            this, &QDeclarativeGeoRouteModel::routingFinished);
    connect(routingManager, &QGeoRoutingManager::errorOccurred,
            this, &QDeclarativeGeoRouteModel::routingError);
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::queryDetailsChanged()
{
    if (autoUpdate_ && complete_)
        update();
}

/*!
    \qmlproperty Plugin QtLocation::RouteModel::plugin

    This property holds the plugin that providers the actual
    routing service. Note that all plugins do not necessarily
    provide routing (could for example provide only geocoding or maps).

    A valid plugin must be set before the RouteModel can perform any useful
    operations.

    \sa Plugin
*/

QDeclarativeGeoServiceProvider *QDeclarativeGeoRouteModel::plugin() const
{
    return plugin_;
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::setQuery(QDeclarativeGeoRouteQuery *query)
{
    if (!query || query == routeQuery_)
        return;
    if (routeQuery_)
        routeQuery_->disconnect(this);
    routeQuery_ = query;
    connect(query, &QDeclarativeGeoRouteQuery::queryDetailsChanged,
            this, &QDeclarativeGeoRouteModel::queryDetailsChanged);
    if (complete_) {
        emit queryChanged();
        if (autoUpdate_)
            update();
    }
}

/*!
    \qmlproperty RouteQuery QtLocation::RouteModel::query

    This property holds the data of the route request.
    The primary data are the waypoint coordinates and possible further
    preferences (means of traveling, things to avoid on route etc).
*/

QDeclarativeGeoRouteQuery *QDeclarativeGeoRouteModel::query() const
{
    return routeQuery_;
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::setAutoUpdate(bool autoUpdate)
{
    if (autoUpdate_ == autoUpdate)
        return;
    autoUpdate_ = autoUpdate;
    if (complete_)
        emit autoUpdateChanged();
}

/*!
    \qmlproperty bool QtLocation::RouteModel::autoUpdate

    This property controls whether the Model automatically updates in response
    to changes in its attached RouteQuery. The default value of this property
    is false.

    If setting this value to 'true', note that any change at all in
    the RouteQuery object set in the \l{query} property will trigger a new
    request to be sent. If you are adjusting many properties of the RouteQuery
    with autoUpdate enabled, this can generate large numbers of useless (and
    later discarded) requests.
*/

bool QDeclarativeGeoRouteModel::autoUpdate() const
{
    return autoUpdate_;
}

/*!
    \qmlproperty Locale::MeasurementSystem QtLocation::RouteModel::measurementSystem

    This property holds the measurement system which will be used when calculating the route. This
    property is changed when the \l {QtLocation::Plugin::locales}{Plugin::locales} property of
    \l {QtLocation::RouteModel::plugin}{plugin} changes.

    If setting this property it must be set after the \l {QtLocation::RouteModel::plugin}{plugin}
    property is set.
*/
void QDeclarativeGeoRouteModel::setMeasurementSystem(QLocale::MeasurementSystem ms)
{
    if (!plugin_)
        return;

    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    if (!serviceProvider)
        return;

    QGeoRoutingManager *routingManager = serviceProvider->routingManager();
    if (!routingManager)
        return;

    if (routingManager->measurementSystem() == ms)
        return;

    routingManager->setMeasurementSystem(ms);
    emit measurementSystemChanged();
}

QLocale::MeasurementSystem QDeclarativeGeoRouteModel::measurementSystem() const
{
    if (!plugin_)
        return QLocale().measurementSystem();

    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    if (!serviceProvider) {
        if (plugin_->locales().isEmpty())
            return QLocale().measurementSystem();

        return QLocale(plugin_->locales().first()).measurementSystem();
    }

    QGeoRoutingManager *routingManager = serviceProvider->routingManager();
    if (!routingManager) {
        if (plugin_->locales().isEmpty())
            return QLocale().measurementSystem();

        return QLocale(plugin_->locales().first()).measurementSystem();
    }

    return routingManager->measurementSystem();
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::setStatus(QDeclarativeGeoRouteModel::Status status)
{
    if (status_ == status)
        return;

    status_ = status;

    if (complete_)
        emit statusChanged();
}

/*!
    \qmlproperty enumeration QtLocation::RouteModel::status

    This read-only property holds the current status of the model.

    \list
    \li RouteModel.Null - No route requests have been issued or \l reset has been called.
    \li RouteModel.Ready - Route request(s) have finished successfully.
    \li RouteModel.Loading - Route request has been issued but not yet finished
    \li RouteModel.Error - Routing error has occurred, details are in \l error and \l errorString
    \endlist
*/

QDeclarativeGeoRouteModel::Status QDeclarativeGeoRouteModel::status() const
{
    return status_;
}

/*!
    \qmlproperty string QtLocation::RouteModel::errorString

    This read-only property holds the textual presentation of the latest routing error.
    If no error has occurred or the model has been reset, an empty string is returned.

    An empty string may also be returned if an error occurred which has no associated
    textual representation.
*/

QString QDeclarativeGeoRouteModel::errorString() const
{
    return errorString_;
}

/*!
    \qmlproperty enumeration QtLocation::RouteModel::error

    This read-only property holds the latest error value of the routing request.

    \list
    \li RouteModel.NoError - No error has occurred.
    \li RouteModel.CommunicationError - An error occurred while communicating with the service provider.
    \li RouteModel.EngineNotSetError - The model's plugin property was not set or there is no routing manager associated with the plugin.
    \li RouteModel.MissingRequiredParameterError - A required parameter was not specified.
    \li RouteModel.ParseError - The response from the service provider was in an unrecognizable format.
    \li RouteModel.UnknownError - An error occurred which does not fit into any of the other categories.
    \li RouteModel.UnknownParameterError - The plugin did not recognize one of the parameters it was given.
    \li RouteModel.UnsupportedOptionError - The requested operation is not supported by the routing provider.
                                            This may happen when the loaded engine does not support a particular
                                            type of routing request.
    \endlist
*/

QDeclarativeGeoRouteModel::RouteError QDeclarativeGeoRouteModel::error() const
{
    return error_;
}

void QDeclarativeGeoRouteModel::setError(RouteError error, const QString& errorString)
{
    if (error_ == error && errorString_ == errorString)
        return;
    error_ = error;
    errorString_ = errorString;
    emit errorChanged();
}

/*!
    \qmlmethod void QtLocation::RouteModel::update()

    Instructs the RouteModel to update its data. This is most useful
    when \l autoUpdate is disabled, to force a refresh when the query
    has been changed.
*/
void QDeclarativeGeoRouteModel::update()
{
    if (!complete_)
        return;

    if (!plugin_) {
        setError(EngineNotSetError, tr("Cannot route, plugin not set."));
        return;
    }

    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    if (!serviceProvider)
        return;

    QGeoRoutingManager *routingManager = serviceProvider->routingManager();
    if (!routingManager) {
        setError(EngineNotSetError, tr("Cannot route, route manager not set."));
        return;
    }
    if (!routeQuery_) {
        setError(ParseError, tr("Cannot route, valid query not set."));
        return;
    }
    emit abortRequested(); // Clear previous requests
    QGeoRouteRequest request = routeQuery_->routeRequest();
    if (request.waypoints().count() < 2) {
        setError(ParseError,tr("Not enough waypoints for routing."));
        return;
    }

    setError(NoError, QString());

    QGeoRouteReply *reply = routingManager->calculateRoute(request);
    setStatus(QDeclarativeGeoRouteModel::Loading);
    if (!reply->isFinished()) {
        connect(this, &QDeclarativeGeoRouteModel::abortRequested, reply, &QGeoRouteReply::abort);
    } else {
        if (reply->error() == QGeoRouteReply::NoError) {
            routingFinished(reply);
        } else {
            routingError(reply, reply->error(), reply->errorString());
        }
    }
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::routingFinished(QGeoRouteReply *reply)
{
    if (!reply)
        return;
    reply->deleteLater();
    if (reply->error() != QGeoRouteReply::NoError)
        return;

    beginResetModel();
    const int oldCount = routes_.count();
    routes_ = reply->routes();
    endResetModel();

    setError(NoError, QString());
    setStatus(QDeclarativeGeoRouteModel::Ready);

    if (oldCount != 0 || routes_.count() != 0)
        emit routesChanged();
    if (oldCount != routes_.count())
        emit countChanged();
}

/*!
    \internal
*/
void QDeclarativeGeoRouteModel::routingError(QGeoRouteReply *reply,
                                               QGeoRouteReply::Error error,
                                               const QString &errorString)
{
    if (!reply)
        return;
    reply->deleteLater();
    setError(static_cast<QDeclarativeGeoRouteModel::RouteError>(error), errorString);
    setStatus(QDeclarativeGeoRouteModel::Error);
}


/*!
    \qmltype RouteQuery
    \nativetype QDeclarativeGeoRouteQuery
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.5

    \brief The RouteQuery type is used to provide query parameters to a
           RouteModel.

    A RouteQuery is used to pack all the parameters necessary to make a request
    to a routing service, which can then populate the contents of a RouteModel.

    These parameters describe key details of the route, such as \l waypoints to
    pass through, \l excludedAreas to avoid, the \l travelModes in use, as well
    as detailed preferences on how to optimize the route and what features
    to prefer or avoid along the path (such as toll roads, highways, etc).

    RouteQuery objects are used exclusively to fill out the value of a
    RouteModel's \l{RouteModel::query}{query} property, which can then begin
    the retrieval process to populate the model.

    Some plugins might allow or require specific parameters to operate.
    In order to specify these plugin-specific parameters, MapParameter elements
    can be nested inside a RouteQuery.

    \section2 Example Usage

    The following snipped shows an incomplete example of creating a RouteQuery
    object and setting it as the value of a RouteModel's \l{RouteModel::query}{query}
    property.

    \code
    RouteQuery {
        id: aQuery
    }

    RouteModel {
        query: aQuery
        autoUpdate: false
    }
    \endcode

    For a more complete example, see the documentation for the \l{RouteModel}
    type, and the Mapviewer example.

    \sa RouteModel

*/

QDeclarativeGeoRouteQuery::QDeclarativeGeoRouteQuery(QObject *parent)
    : QObject(parent)
{
}

QDeclarativeGeoRouteQuery::QDeclarativeGeoRouteQuery(const QGeoRouteRequest &request, QObject *parent)
    : QObject(parent), request_(request)
{
    // Extra params assumed to be already set in the request.
    // Init waypoints
    m_waypoints = request_.waypoints();
}

QDeclarativeGeoRouteQuery::~QDeclarativeGeoRouteQuery()
{
}

/*!
    \internal
*/
void QDeclarativeGeoRouteQuery::componentComplete()
{
    complete_ = true;
}

/*!
    \qmlproperty QList<FeatureType> RouteQuery::featureTypes

    List of features that will be considered when planning the
    route. Features with a weight of NeutralFeatureWeight will not be returned.

    \list
    \li RouteQuery.NoFeature - No features will be taken into account when planning the route
    \li RouteQuery.TollFeature - Consider tollways when planning the route
    \li RouteQuery.HighwayFeature - Consider highways when planning the route
    \li RouteQuery.PublicTransitFeature - Consider public transit when planning the route
    \li RouteQuery.FerryFeature - Consider ferries when planning the route
    \li RouteQuery.TunnelFeature - Consider tunnels when planning the route
    \li RouteQuery.DirtRoadFeature - Consider dirt roads when planning the route
    \li RouteQuery.ParksFeature - Consider parks when planning the route
    \li RouteQuery.MotorPoolLaneFeature - Consider motor pool lanes when planning the route
    \li RouteQuery.TrafficFeature - Consider traffic when planning the route
    \endlist

    \sa setFeatureWeight, featureWeight
*/

QList<int> QDeclarativeGeoRouteQuery::featureTypes() const
{
    QList<int> list;

    const auto featureTypes = request_.featureTypes();
    for (const auto &featureType : featureTypes)
        list.append(static_cast<int>(featureType));
    return list;
}

/*!
    \qmlproperty int RouteQuery::numberAlternativeRoutes

    The number of alternative routes requested when requesting routes.
    The default value is 0.
*/


int QDeclarativeGeoRouteQuery::numberAlternativeRoutes() const
{
    return request_.numberAlternativeRoutes();
}

void QDeclarativeGeoRouteQuery::setNumberAlternativeRoutes(int numberAlternativeRoutes)
{
    if (numberAlternativeRoutes == request_.numberAlternativeRoutes())
        return;

    request_.setNumberAlternativeRoutes(numberAlternativeRoutes);

    if (complete_) {
        emit numberAlternativeRoutesChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlproperty list<coordinate> RouteQuery::waypoints

    The coordinates of the waypoints for the desired route.
    The waypoints should be given in order from origin to destination.
    Two or more coordinates are needed.

    Waypoints can be set as part of the RouteQuery type declaration or
    dynamically with the functions provided.

    \sa addWaypoint, removeWaypoint, clearWaypoints
*/

QList<QGeoCoordinate> QDeclarativeGeoRouteQuery::waypoints() const
{
    return m_waypoints;
}

void QDeclarativeGeoRouteQuery::setWaypoints(const QList<QGeoCoordinate> &value)
{
    if (m_waypoints == value)
        return;

    m_waypoints = value;
    waypointChanged();
}

/*!
    \qmlproperty list<geoRectangle> RouteQuery::excludedAreas

    Areas that the route must not cross.

    Excluded areas can be set as part of the \l RouteQuery type declaration or
    dynamically with the functions provided.

    \sa addExcludedArea, removeExcludedArea, clearExcludedAreas
*/
QList<QGeoRectangle> QDeclarativeGeoRouteQuery::excludedAreas() const
{
    return request_.excludeAreas();
}

void QDeclarativeGeoRouteQuery::setExcludedAreas(const QList<QGeoRectangle> &value)
{
    if (request_.excludeAreas() == value)
        return;

    request_.setExcludeAreas(value);

    if (complete_) {
        emit excludedAreasChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlmethod void QtLocation::RouteQuery::addExcludedArea(geoRectangle area)

    Adds the specified geoRectangle \a area to the excluded areas
    (areas that the route must not cross).
    The same area can only be added once.

    \sa removeExcludedArea, clearExcludedAreas
*/


void QDeclarativeGeoRouteQuery::addExcludedArea(const QGeoRectangle &area)
{
    if (!area.isValid())
        return;

    QList<QGeoRectangle> excludedAreas = request_.excludeAreas();

    if (excludedAreas.contains(area))
        return;

    excludedAreas.append(area);

    request_.setExcludeAreas(excludedAreas);

    if (complete_) {
        emit excludedAreasChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlmethod void QtLocation::RouteQuery::removeExcludedArea(geoRectangle area)

    Removes the given \a area from excluded areas (areas that the route must not cross).

    \sa addExcludedArea, clearExcludedAreas
*/

void QDeclarativeGeoRouteQuery::removeExcludedArea(const QGeoRectangle &area)
{
    if (!area.isValid())
        return;

    QList<QGeoRectangle> excludedAreas = request_.excludeAreas();

    int index = excludedAreas.lastIndexOf(area);
    if (index == -1) {
        qmlWarning(this) << QStringLiteral("Cannot remove nonexistent area.");
        return;
    }
    excludedAreas.removeAt(index);
    request_.setExcludeAreas(excludedAreas);

    if (complete_) {
        emit excludedAreasChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlmethod void QtLocation::RouteQuery::clearExcludedAreas()

    Clears all excluded areas (areas that the route must not cross).

    \sa addExcludedArea, removeExcludedArea
*/

void QDeclarativeGeoRouteQuery::clearExcludedAreas()
{
    if (request_.excludeAreas().isEmpty())
        return;

    request_.setExcludeAreas(QList<QGeoRectangle>());

    if (complete_) {
        emit excludedAreasChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlmethod void QtLocation::RouteQuery::addWaypoint(coordinate)

    Appends the given \a coordinate to the list of waypoints. The same
    coordinate can be set multiple times.

    \sa removeWaypoint, clearWaypoints
*/
void QDeclarativeGeoRouteQuery::addWaypoint(const QGeoCoordinate &waypoint)
{
    if (!waypoint.isValid()) {
        qmlWarning(this) << QStringLiteral("Invalid coordinate as waypoint");
        return;
    }

    m_waypoints << waypoint;
    waypointChanged();
}

/*!
    \qmlmethod void QtLocation::RouteQuery::removeWaypoint(coordinate)

    Removes the given \a coordinate from the list of waypoints. If the
    same coordinate appears multiple times, the most recently added
    coordinate instance is removed.

    \sa addWaypoint, clearWaypoints
*/
void QDeclarativeGeoRouteQuery::removeWaypoint(const QGeoCoordinate &waypoint)
{
    if (!waypoint.isValid()) {
        qmlWarning(this) << QStringLiteral("Invalid coordinate as waypoint");
        return;
    }

    if (qsizetype idx = m_waypoints.lastIndexOf(waypoint); idx >= 0) {
        m_waypoints.remove(idx);
        waypointChanged();
    } else {
        qmlWarning(this) << QStringLiteral("Cannot remove nonexistent waypoint.");
    }
}

/*!
    \qmlmethod void QtLocation::RouteQuery::clearWaypoints()

    Clears all waypoints.

    \sa removeWaypoint, addWaypoint
*/
void QDeclarativeGeoRouteQuery::clearWaypoints()
{
    if (m_waypoints.isEmpty())
        return;

    m_waypoints.clear();
    waypointChanged();
}

/*!
    \qmlmethod void QtLocation::RouteQuery::setFeatureWeight(FeatureType feature, FeatureWeight weight)

    Defines the \a weight  to associate with a \a feature during the planning
    of a route.

    Following lists the possible feature weights:

    \value RouteQuery.NeutralFeatureWeight
    The presence or absence of the feature does not affect the planning of the
    route

    \value RouteQuery.PreferFeatureWeight
    Routes which contain the feature are preferred over those that do not

    \value RouteQuery.RequireFeatureWeight
    Only routes which contain the feature are considered, otherwise no
    route will be returned

    \value RouteQuery.AvoidFeatureWeight
    Routes which do not contain the feature are preferred over those that
    do

    \value RouteQuery.DisallowFeatureWeight
    Only routes which do not contain the feature are considered, otherwise
    no route will be returned

    \sa featureTypes, resetFeatureWeights, featureWeight

*/

void QDeclarativeGeoRouteQuery::setFeatureWeight(FeatureType featureType, FeatureWeight featureWeight)
{
    if (featureType == NoFeature && !request_.featureTypes().isEmpty()) {
        resetFeatureWeights();
        return;
    }

    // Check if the weight changes, as we need to signal it
    FeatureWeight originalWeight = static_cast<FeatureWeight>(request_.featureWeight(static_cast<QGeoRouteRequest::FeatureType>(featureType)));
    if (featureWeight == originalWeight)
        return;

    request_.setFeatureWeight(static_cast<QGeoRouteRequest::FeatureType>(featureType),
                              static_cast<QGeoRouteRequest::FeatureWeight>(featureWeight));
    if (complete_ && ((originalWeight == NeutralFeatureWeight) || (featureWeight == NeutralFeatureWeight))) {
        // featureTypes should now give a different list, because the original and new weight
        // were not same, and other one was neutral weight
        emit featureTypesChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlmethod void QtLocation::RouteQuery::resetFeatureWeights()

    Resets all feature weights to their default state (NeutralFeatureWeight).

    \sa featureTypes, setFeatureWeight, featureWeight
*/
void QDeclarativeGeoRouteQuery::resetFeatureWeights()
{
    // reset all feature types.
    const auto featureTypes = request_.featureTypes();
    for (const auto &featureType : featureTypes)
        request_.setFeatureWeight(featureType, QGeoRouteRequest::NeutralFeatureWeight);
    if (complete_) {
        emit featureTypesChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \qmlmethod FeatureWeight QtLocation::RouteQuery::featureWeight(FeatureType featureType)

    Gets the weight for the \a featureType.

    \sa featureTypes, setFeatureWeight, resetFeatureWeights
*/

int QDeclarativeGeoRouteQuery::featureWeight(FeatureType featureType)
{
    return request_.featureWeight(static_cast<QGeoRouteRequest::FeatureType>(featureType));
}

/*!
    \internal
*/
void QDeclarativeGeoRouteQuery::setTravelModes(QDeclarativeGeoRouteQuery::TravelModes travelModes)
{
    QGeoRouteRequest::TravelModes reqTravelModes;

    if (travelModes & QDeclarativeGeoRouteQuery::CarTravel)
        reqTravelModes |= QGeoRouteRequest::CarTravel;
    if (travelModes & QDeclarativeGeoRouteQuery::PedestrianTravel)
        reqTravelModes |= QGeoRouteRequest::PedestrianTravel;
    if (travelModes & QDeclarativeGeoRouteQuery::BicycleTravel)
        reqTravelModes |= QGeoRouteRequest::BicycleTravel;
    if (travelModes & QDeclarativeGeoRouteQuery::PublicTransitTravel)
        reqTravelModes |= QGeoRouteRequest::PublicTransitTravel;
    if (travelModes & QDeclarativeGeoRouteQuery::TruckTravel)
        reqTravelModes |= QGeoRouteRequest::TruckTravel;

    if (reqTravelModes == request_.travelModes())
        return;

    request_.setTravelModes(reqTravelModes);

    if (complete_) {
        emit travelModesChanged();
        emit queryDetailsChanged();
    }
}


/*!
    \qmlproperty enumeration RouteQuery::segmentDetail

    The level of detail which will be used in the representation of routing segments.

    \value RouteQuery.NoSegmentData
    No segment data should be included with the route

    \value RouteQuery.BasicSegmentData
    Basic segment data will be included with the route

    The default value is \c {RouteQuery.BasicSegmentData}.
*/

void QDeclarativeGeoRouteQuery::setSegmentDetail(SegmentDetail segmentDetail)
{
    if (static_cast<QGeoRouteRequest::SegmentDetail>(segmentDetail) == request_.segmentDetail())
        return;
    request_.setSegmentDetail(static_cast<QGeoRouteRequest::SegmentDetail>(segmentDetail));
    if (complete_) {
        emit segmentDetailChanged();
        emit queryDetailsChanged();
    }
}

QDeclarativeGeoRouteQuery::SegmentDetail QDeclarativeGeoRouteQuery::segmentDetail() const
{
    return static_cast<QDeclarativeGeoRouteQuery::SegmentDetail>(request_.segmentDetail());
}

/*!
    \qmlproperty enumeration RouteQuery::maneuverDetail

    The level of detail which will be used in the representation of routing maneuvers.

    \value RouteQuery.NoManeuvers
    No maneuvers should be included with the route

    \value RouteQuery.BasicManeuvers
    Basic maneuvers will be included with the route

    The default value is \c {RouteQuery.BasicManeuvers}.
*/

void QDeclarativeGeoRouteQuery::setManeuverDetail(ManeuverDetail maneuverDetail)
{
    if (static_cast<QGeoRouteRequest::ManeuverDetail>(maneuverDetail) == request_.maneuverDetail())
        return;
    request_.setManeuverDetail(static_cast<QGeoRouteRequest::ManeuverDetail>(maneuverDetail));
    if (complete_) {
        emit maneuverDetailChanged();
        emit queryDetailsChanged();
    }
}

QDeclarativeGeoRouteQuery::ManeuverDetail QDeclarativeGeoRouteQuery::maneuverDetail() const
{
    return static_cast<QDeclarativeGeoRouteQuery::ManeuverDetail>(request_.maneuverDetail());
}

/*!
    \qmlproperty enumeration RouteQuery::travelModes

    The travel modes which should be considered during the planning of the route.
    Values can be combined with OR ('|') -operator.

    \value RouteQuery.CarTravel
    The route will be optimized for someone who is driving a car

    \value RouteQuery.PedestrianTravel
    The route will be optimized for someone who is walking

    \value RouteQuery.BicycleTravel
    The route will be optimized for someone who is riding a bicycle

    \value RouteQuery.PublicTransit
    Travel The route will be optimized for someone who is making use of public transit

    \value RouteQuery.TruckTravel
    The route will be optimized for someone who is driving a truck

    The default value is \c {RouteQuery.CarTravel}.
*/

QDeclarativeGeoRouteQuery::TravelModes QDeclarativeGeoRouteQuery::travelModes() const
{
    QGeoRouteRequest::TravelModes reqTravelModes = request_.travelModes();
    QDeclarativeGeoRouteQuery::TravelModes travelModes;

    if (reqTravelModes & QGeoRouteRequest::CarTravel)
        travelModes |= QDeclarativeGeoRouteQuery::CarTravel;
    if (reqTravelModes & QGeoRouteRequest::PedestrianTravel)
        travelModes |= QDeclarativeGeoRouteQuery::PedestrianTravel;
    if (reqTravelModes & QGeoRouteRequest::BicycleTravel)
        travelModes |= QDeclarativeGeoRouteQuery::BicycleTravel;
    if (reqTravelModes & QGeoRouteRequest::PublicTransitTravel)
        travelModes |= QDeclarativeGeoRouteQuery::PublicTransitTravel;
    if (reqTravelModes & QGeoRouteRequest::TruckTravel)
        travelModes |= QDeclarativeGeoRouteQuery::TruckTravel;

    return travelModes;
}

/*!
    \qmlproperty enumeration RouteQuery::routeOptimizations

    The route optimizations which should be considered during the planning of the route.
    Values can be combined with OR ('|') -operator.


    \value RouteQuery.ShortestRoute
    Minimize the length of the journey

    \value RouteQuery.FastestRoute
    Minimize the traveling time for the journey

    \value RouteQuery.MostEconomicRoute
    Minimize the cost of the journey

    \value RouteQuery.MostScenicRoute
    Maximize the scenic potential of the journey

    The default value is \c {RouteQuery.FastestRoute}.
*/

QDeclarativeGeoRouteQuery::RouteOptimizations QDeclarativeGeoRouteQuery::routeOptimizations() const
{
    QGeoRouteRequest::RouteOptimizations reqOptimizations = request_.routeOptimization();
    QDeclarativeGeoRouteQuery::RouteOptimizations optimization;

    if (reqOptimizations & QGeoRouteRequest::ShortestRoute)
        optimization |= QDeclarativeGeoRouteQuery::ShortestRoute;
    if (reqOptimizations & QGeoRouteRequest::FastestRoute)
        optimization |= QDeclarativeGeoRouteQuery::FastestRoute;
    if (reqOptimizations & QGeoRouteRequest::MostEconomicRoute)
        optimization |= QDeclarativeGeoRouteQuery::MostEconomicRoute;
    if (reqOptimizations & QGeoRouteRequest::MostScenicRoute)
        optimization |= QDeclarativeGeoRouteQuery::MostScenicRoute;

    return optimization;
}

/*!
    \qmlproperty date RouteQuery::departureTime

    The departure time to be used when querying for the route.
    The default value is an invalid date, meaning no departure time will be used in the query.

    \since 5.13
*/
void QDeclarativeGeoRouteQuery::setDepartureTime(const QDateTime &departureTime)
{
    if (departureTime == request_.departureTime())
        return;

    request_.setDepartureTime(departureTime);
    if (complete_) {
        emit departureTimeChanged();
        emit queryDetailsChanged();
    }
}

QDateTime QDeclarativeGeoRouteQuery::departureTime() const
{
    return request_.departureTime();
}

void QDeclarativeGeoRouteQuery::setRouteOptimizations(QDeclarativeGeoRouteQuery::RouteOptimizations optimization)
{
    QGeoRouteRequest::RouteOptimizations reqOptimizations;

    if (optimization & QDeclarativeGeoRouteQuery::ShortestRoute)
        reqOptimizations |= QGeoRouteRequest::ShortestRoute;
    if (optimization & QDeclarativeGeoRouteQuery::FastestRoute)
        reqOptimizations |= QGeoRouteRequest::FastestRoute;
    if (optimization & QDeclarativeGeoRouteQuery::MostEconomicRoute)
        reqOptimizations |= QGeoRouteRequest::MostEconomicRoute;
    if (optimization & QDeclarativeGeoRouteQuery::MostScenicRoute)
        reqOptimizations |= QGeoRouteRequest::MostScenicRoute;

    if (reqOptimizations == request_.routeOptimization())
        return;

    request_.setRouteOptimization(reqOptimizations);

    if (complete_) {
        emit routeOptimizationsChanged();
        emit queryDetailsChanged();
    }
}

/*!
    \internal
*/
QGeoRouteRequest QDeclarativeGeoRouteQuery::routeRequest() const
{
    if (m_waypointsChanged) {
        m_waypointsChanged = false;
        // Update waypoints and metadata into request
        request_.setWaypoints(m_waypoints);
    }
    return request_;
}

void QDeclarativeGeoRouteQuery::excludedAreaCoordinateChanged()
{
    if (!m_excludedAreaCoordinateChanged) {
        m_excludedAreaCoordinateChanged = true;
        QMetaObject::invokeMethod(this, "doCoordinateChanged", Qt::QueuedConnection);
    }
}

void QDeclarativeGeoRouteQuery::waypointChanged()
{
    m_waypointsChanged = true;
    if (complete_) {
        emit waypointsChanged();
        emit queryDetailsChanged();
    }
}

void QDeclarativeGeoRouteQuery::doCoordinateChanged()
{
    m_excludedAreaCoordinateChanged = false;
    if (complete_)
        emit queryDetailsChanged();
}

QT_END_NAMESPACE
