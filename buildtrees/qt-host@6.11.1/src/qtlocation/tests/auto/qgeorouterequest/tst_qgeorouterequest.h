// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOROUTEREQUEST_H
#define TST_QGEOROUTEREQUEST_H

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QMetaType>

#include <qgeocoordinate.h>
#include <QtPositioning/QGeoRectangle>
#include <qgeorouterequest.h>


QT_USE_NAMESPACE

class tst_QGeoRouteRequest : public QObject
{
    Q_OBJECT

public:
    tst_QGeoRouteRequest();

private Q_SLOTS:
    //Start Unit Test for QGeoRouteRequest
    void constructor_waypoints();
    void constructor_orig_dest();
    void copy_constructor();
    void excludeAreas();
    void numberAlternativeRoutes();
    void routeOptimization();
    void routeOptimization_data();
    void segmentDetail();
    void segmentDetail_data();
    void travelModes();
    void travelModes_data();
    void waypoints();
    void waypoints_data();
    void maneuverDetail();
    void maneuverDetail_data();
    void featureWeight();
    void featureWeight_data();
    void departureTime();
    void departureTime_data();
    //End Unit Test for QGeoRouteRequest
};

Q_DECLARE_METATYPE( QList<double>);
Q_DECLARE_METATYPE( QGeoRouteRequest::RouteOptimization );
Q_DECLARE_METATYPE( QGeoRouteRequest::SegmentDetail );
Q_DECLARE_METATYPE( QGeoRouteRequest::TravelMode );
Q_DECLARE_METATYPE( QGeoRouteRequest::FeatureType);
Q_DECLARE_METATYPE( QGeoRouteRequest::FeatureWeight);
Q_DECLARE_METATYPE( QGeoRouteRequest::ManeuverDetail );

#endif // TST_QGEOROUTEREQUEST_H

