// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEREQUEST_H
#define QGEOROUTEREQUEST_H

#include <QtCore/QList>
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QDateTime>

#include <QtLocation/qlocationglobal.h>
#include <QtPositioning/QGeoCoordinate>

QT_BEGIN_NAMESPACE

class QGeoRectangle;
class QGeoRouteRequestPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoRouteRequestPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QGeoRouteRequest
{
public:
    enum TravelMode {
        CarTravel = 0x0001,
        PedestrianTravel = 0x0002,
        BicycleTravel = 0x0004,
        PublicTransitTravel = 0x0008,
        TruckTravel = 0x0010
    };
    Q_DECLARE_FLAGS(TravelModes, TravelMode)

    enum FeatureType {
        NoFeature = 0x00000000,
        TollFeature = 0x00000001,
        HighwayFeature = 0x00000002,
        PublicTransitFeature = 0x00000004,
        FerryFeature = 0x00000008,
        TunnelFeature = 0x00000010,
        DirtRoadFeature = 0x00000020,
        ParksFeature = 0x00000040,
        MotorPoolLaneFeature = 0x00000080,
        TrafficFeature = 0x00000100
    };
    Q_DECLARE_FLAGS(FeatureTypes, FeatureType)

    enum FeatureWeight {
        NeutralFeatureWeight = 0x00000000,
        PreferFeatureWeight = 0x00000001,
        RequireFeatureWeight = 0x00000002,
        AvoidFeatureWeight = 0x00000004,
        DisallowFeatureWeight = 0x00000008
    };
    Q_DECLARE_FLAGS(FeatureWeights, FeatureWeight)

    enum RouteOptimization {
        ShortestRoute = 0x0001,
        FastestRoute = 0x0002,
        MostEconomicRoute = 0x0004,
        MostScenicRoute = 0x0008
    };
    Q_DECLARE_FLAGS(RouteOptimizations, RouteOptimization)

    enum SegmentDetail {
        NoSegmentData = 0x0000,
        BasicSegmentData = 0x0001
    };
    Q_DECLARE_FLAGS(SegmentDetails, SegmentDetail)

    enum ManeuverDetail {
        NoManeuvers = 0x0000,
        BasicManeuvers = 0x0001
    };
    Q_DECLARE_FLAGS(ManeuverDetails, ManeuverDetail)

    explicit QGeoRouteRequest(const QList<QGeoCoordinate> &waypoints = QList<QGeoCoordinate>());
    QGeoRouteRequest(const QGeoCoordinate &origin,
                     const QGeoCoordinate &destination);
    QGeoRouteRequest(const QGeoRouteRequest &other) noexcept;
    QGeoRouteRequest(QGeoRouteRequest &&other) noexcept = default;

    ~QGeoRouteRequest();

    QGeoRouteRequest &operator= (const QGeoRouteRequest &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoRouteRequest)

    void swap(QGeoRouteRequest &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QGeoRouteRequest &lhs,
                                  const QGeoRouteRequest &rhs) noexcept
    {
        return lhs.isEqual(rhs);
    }
    friend inline bool operator!=(const QGeoRouteRequest &lhs,
                                  const QGeoRouteRequest &rhs) noexcept
    {
        return !lhs.isEqual(rhs);
    }

    void setWaypoints(const QList<QGeoCoordinate> &waypoints);
    QList<QGeoCoordinate> waypoints() const;

    void setExcludeAreas(const QList<QGeoRectangle> &areas);
    QList<QGeoRectangle> excludeAreas() const;

    // defaults to 0
    void setNumberAlternativeRoutes(int alternatives);
    int numberAlternativeRoutes() const;

    // defaults to TravelByCar
    void setTravelModes(TravelModes travelModes);
    TravelModes travelModes() const;

    void setFeatureWeight(FeatureType featureType, FeatureWeight featureWeight);
    FeatureWeight featureWeight(FeatureType featureType) const;
    QList<FeatureType> featureTypes() const;

    // defaults to OptimizeFastest
    void setRouteOptimization(RouteOptimizations optimization);
    RouteOptimizations routeOptimization() const;

    // defaults to BasicSegmentData
    void setSegmentDetail(SegmentDetail segmentDetail);
    SegmentDetail segmentDetail() const;

    // defaults to BasicManeuvers
    void setManeuverDetail(ManeuverDetail maneuverDetail);
    ManeuverDetail maneuverDetail() const;

    // defaults to invalid datetime
    void setDepartureTime(const QDateTime &departureTime);
    QDateTime departureTime() const;

private:
    QExplicitlySharedDataPointer<QGeoRouteRequestPrivate> d_ptr;

    bool isEqual(const QGeoRouteRequest &other) const noexcept;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoRouteRequest::TravelModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoRouteRequest::FeatureTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoRouteRequest::FeatureWeights)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoRouteRequest::RouteOptimizations)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoRouteRequest::SegmentDetails)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoRouteRequest::ManeuverDetails)

QT_END_NAMESPACE

#endif
