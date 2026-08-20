// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOROUTEMODEL_H
#define QDECLARATIVEGEOROUTEMODEL_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QAbstractListModel>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoRectangle>
#include <qgeorouterequest.h>
#include <qgeoroutereply.h>
#include <QtQml/qqml.h>
#include <QtQml/QQmlParserStatus>

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qdeclarativegeoserviceprovider_p.h>

#include <QtPositioning/private/qgeocoordinateobject_p.h>

QT_BEGIN_NAMESPACE

class QGeoServiceProvider;
class QGeoRoutingManager;
class QDeclarativeGeoRoute;
class QDeclarativeGeoRouteQuery;

class Q_LOCATION_EXPORT QDeclarativeGeoRouteModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(RouteModel)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QDeclarativeGeoRouteQuery *query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(RouteError error READ error NOTIFY errorChanged)
    Q_PROPERTY(QLocale::MeasurementSystem measurementSystem READ measurementSystem WRITE setMeasurementSystem NOTIFY measurementSystemChanged)

    Q_INTERFACES(QQmlParserStatus)

public:
    enum Roles {
        RouteRole = Qt::UserRole + 500
    };

    enum Status {
        Null,
        Ready,
        Loading,
        Error
    };
    Q_ENUM(Status)

    enum RouteError {
        NoError = QGeoRouteReply::NoError,
        EngineNotSetError = QGeoRouteReply::EngineNotSetError,
        CommunicationError = QGeoRouteReply::CommunicationError,
        ParseError = QGeoRouteReply::ParseError,
        UnsupportedOptionError = QGeoRouteReply::UnsupportedOptionError,
        UnknownError = QGeoRouteReply::UnknownError,
        //we leave gap for future QGeoRouteReply errors

        //QGeoServiceProvider related errors start here
        UnknownParameterError = 100,
        MissingRequiredParameterError
    };
    Q_ENUM(RouteError)

    explicit QDeclarativeGeoRouteModel(QObject *parent = nullptr);

    // From QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;

    // From QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    void setPlugin(QDeclarativeGeoServiceProvider *plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    void setQuery(QDeclarativeGeoRouteQuery *query);
    QDeclarativeGeoRouteQuery *query() const;

    void setAutoUpdate(bool autoUpdate);
    bool autoUpdate() const;

    void setMeasurementSystem(QLocale::MeasurementSystem ms);
    QLocale::MeasurementSystem measurementSystem() const;

    Status status() const;
    QString errorString() const;
    RouteError error() const;

    int count() const;
    Q_INVOKABLE QGeoRoute get(int index);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void cancel();

Q_SIGNALS:
    void countChanged();
    void pluginChanged();
    void queryChanged();
    void autoUpdateChanged();
    void statusChanged();
    void errorChanged(); //emitted also for errorString notification
    void routesChanged();
    void measurementSystemChanged();
    void abortRequested();

public Q_SLOTS:
    void update();

private Q_SLOTS:
    void routingFinished(QGeoRouteReply *reply);
    void routingError(QGeoRouteReply *reply,
                      QGeoRouteReply::Error error,
                      const QString &errorString);
    void queryDetailsChanged();
    void pluginReady();

private:
    void setStatus(Status status);
    void setError(RouteError error, const QString &errorString);

    bool complete_ = false;

    QDeclarativeGeoServiceProvider *plugin_ = nullptr;
    QDeclarativeGeoRouteQuery *routeQuery_ = nullptr;

    QList<QGeoRoute> routes_;
    bool autoUpdate_ = false;
    Status status_ = QDeclarativeGeoRouteModel::Null;
    QString errorString_;
    RouteError error_ = QDeclarativeGeoRouteModel::NoError;
};

class Q_LOCATION_EXPORT QDeclarativeGeoRouteQuery : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(RouteQuery)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(int numberAlternativeRoutes READ numberAlternativeRoutes WRITE setNumberAlternativeRoutes NOTIFY numberAlternativeRoutesChanged)
    Q_PROPERTY(TravelModes travelModes READ travelModes WRITE setTravelModes NOTIFY travelModesChanged)
    Q_PROPERTY(RouteOptimizations routeOptimizations READ routeOptimizations WRITE setRouteOptimizations NOTIFY routeOptimizationsChanged)
    Q_PROPERTY(SegmentDetail segmentDetail READ segmentDetail WRITE setSegmentDetail NOTIFY segmentDetailChanged)
    Q_PROPERTY(ManeuverDetail maneuverDetail READ maneuverDetail WRITE setManeuverDetail NOTIFY maneuverDetailChanged)
    Q_PROPERTY(QList<QGeoCoordinate> waypoints READ waypoints WRITE setWaypoints NOTIFY waypointsChanged)
    Q_PROPERTY(QList<QGeoRectangle> excludedAreas READ excludedAreas WRITE setExcludedAreas NOTIFY excludedAreasChanged)
    Q_PROPERTY(QList<int> featureTypes READ featureTypes NOTIFY featureTypesChanged)
    Q_PROPERTY(QDateTime departureTime READ departureTime WRITE setDepartureTime NOTIFY departureTimeChanged REVISION(5, 13))
    Q_INTERFACES(QQmlParserStatus)

public:

    explicit QDeclarativeGeoRouteQuery(QObject *parent = nullptr);
    QDeclarativeGeoRouteQuery(const QGeoRouteRequest &request, QObject *parent = nullptr); // init from request. For instances intended to be read only
    ~QDeclarativeGeoRouteQuery();

    // From QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;

    QGeoRouteRequest routeRequest() const;

    enum TravelMode {
        CarTravel = QGeoRouteRequest::CarTravel,
        PedestrianTravel = QGeoRouteRequest::PedestrianTravel,
        BicycleTravel = QGeoRouteRequest::BicycleTravel,
        PublicTransitTravel = QGeoRouteRequest::PublicTransitTravel,
        TruckTravel = QGeoRouteRequest::TruckTravel
    };
    Q_ENUM(TravelMode)
    Q_DECLARE_FLAGS(TravelModes, TravelMode)
    Q_FLAG(TravelModes)

    enum FeatureType {
        NoFeature = QGeoRouteRequest::NoFeature,
        TollFeature = QGeoRouteRequest::TollFeature,
        HighwayFeature = QGeoRouteRequest::HighwayFeature,
        PublicTransitFeature = QGeoRouteRequest::PublicTransitFeature,
        FerryFeature = QGeoRouteRequest::FerryFeature,
        TunnelFeature = QGeoRouteRequest::TunnelFeature,
        DirtRoadFeature = QGeoRouteRequest::DirtRoadFeature,
        ParksFeature = QGeoRouteRequest::ParksFeature,
        MotorPoolLaneFeature = QGeoRouteRequest::MotorPoolLaneFeature,
        TrafficFeature = QGeoRouteRequest::TrafficFeature
    };
    Q_DECLARE_FLAGS(FeatureTypes, FeatureType)
    Q_FLAG(FeatureTypes);

    enum FeatureWeight {
        NeutralFeatureWeight = QGeoRouteRequest::NeutralFeatureWeight,
        PreferFeatureWeight = QGeoRouteRequest::PreferFeatureWeight,
        RequireFeatureWeight = QGeoRouteRequest::RequireFeatureWeight,
        AvoidFeatureWeight = QGeoRouteRequest::AvoidFeatureWeight,
        DisallowFeatureWeight = QGeoRouteRequest::DisallowFeatureWeight
    };
    Q_DECLARE_FLAGS(FeatureWeights, FeatureWeight)
    Q_FLAG(FeatureWeight)

    enum RouteOptimization {
        ShortestRoute = QGeoRouteRequest::ShortestRoute,
        FastestRoute = QGeoRouteRequest::FastestRoute,
        MostEconomicRoute = QGeoRouteRequest::MostEconomicRoute,
        MostScenicRoute = QGeoRouteRequest::MostScenicRoute
    };
    Q_ENUM(RouteOptimization)
    Q_DECLARE_FLAGS(RouteOptimizations, RouteOptimization)
    Q_FLAG(RouteOptimizations)

    enum SegmentDetail {
        NoSegmentData = 0x0000,
        BasicSegmentData = 0x0001
    };
    Q_ENUM(SegmentDetail)
    Q_DECLARE_FLAGS(SegmentDetails, SegmentDetail)
    Q_FLAG(SegmentDetails)

    enum ManeuverDetail {
        NoManeuvers = 0x0000,
        BasicManeuvers = 0x0001
    };
    Q_ENUM(ManeuverDetail)
    Q_DECLARE_FLAGS(ManeuverDetails, ManeuverDetail)
    Q_FLAG(ManeuverDetails)

    void setNumberAlternativeRoutes(int numberAlternativeRoutes);
    int numberAlternativeRoutes() const;

    //QList<FeatureType> featureTypes();
    QList<int> featureTypes() const;


    QList<QGeoCoordinate> waypoints() const;
    void setWaypoints(const QList<QGeoCoordinate> &value);

    QList<QGeoRectangle> excludedAreas() const;
    void setExcludedAreas(const QList<QGeoRectangle> &value);

    Q_INVOKABLE void addWaypoint(const QGeoCoordinate &w);
    Q_INVOKABLE void removeWaypoint(const QGeoCoordinate &waypoint);
    Q_INVOKABLE void clearWaypoints();

    Q_INVOKABLE void addExcludedArea(const QGeoRectangle &area);
    Q_INVOKABLE void removeExcludedArea(const QGeoRectangle &area);
    Q_INVOKABLE void clearExcludedAreas();

    Q_INVOKABLE void setFeatureWeight(FeatureType featureType, FeatureWeight featureWeight);
    Q_INVOKABLE int featureWeight(FeatureType featureType);
    Q_INVOKABLE void resetFeatureWeights();

    /*
    feature weights
    */

    void setTravelModes(TravelModes travelModes);
    TravelModes travelModes() const;

    void setSegmentDetail(SegmentDetail segmentDetail);
    SegmentDetail segmentDetail() const;

    void setManeuverDetail(ManeuverDetail maneuverDetail);
    ManeuverDetail maneuverDetail() const;

    void setRouteOptimizations(RouteOptimizations optimization);
    RouteOptimizations routeOptimizations() const;

    void setDepartureTime(const QDateTime &departureTime);
    QDateTime departureTime() const;

Q_SIGNALS:
    void numberAlternativeRoutesChanged();
    void travelModesChanged();
    void routeOptimizationsChanged();

    void waypointsChanged();
    void excludedAreasChanged();

    void featureTypesChanged();
    void maneuverDetailChanged();
    void segmentDetailChanged();

    void queryDetailsChanged();
    void departureTimeChanged();

private Q_SLOTS:
    void excludedAreaCoordinateChanged();
    void waypointChanged();

private:
    Q_INVOKABLE void doCoordinateChanged();

    mutable QGeoRouteRequest request_;
    bool complete_ = false;
    bool m_excludedAreaCoordinateChanged = false;
    mutable bool m_waypointsChanged = false;
    QList<QGeoCoordinate> m_waypoints;
};

QT_END_NAMESPACE

#endif
