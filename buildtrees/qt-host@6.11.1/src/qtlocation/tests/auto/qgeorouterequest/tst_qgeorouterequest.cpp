// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_qgeorouterequest.h"

#include <QtPositioning/QGeoRectangle>

QT_USE_NAMESPACE

tst_QGeoRouteRequest::tst_QGeoRouteRequest()
{
}

void tst_QGeoRouteRequest::constructor_waypoints()
{
    QGeoCoordinate qgeocoord1(43.5435, 76.342);
    QGeoCoordinate qgeocoord2(-43.5435, 176.342);
    QGeoCoordinate qgeocoord3(-13.5435, +76.342);

    QList<QGeoCoordinate> waypoints;
    waypoints.append(qgeocoord1);
    waypoints.append(qgeocoord2);
    waypoints.append(qgeocoord3);

    QGeoRouteRequest copy(waypoints);

    QCOMPARE(copy.waypoints(), waypoints);
    QCOMPARE(copy.numberAlternativeRoutes(), 0);
    QCOMPARE(copy.routeOptimization(), QGeoRouteRequest::FastestRoute);
    QCOMPARE(copy.segmentDetail(), QGeoRouteRequest::BasicSegmentData);
    QCOMPARE(copy.travelModes(), QGeoRouteRequest::CarTravel);
    QCOMPARE(copy.maneuverDetail(), QGeoRouteRequest::BasicManeuvers);
}

void tst_QGeoRouteRequest::constructor_orig_dest()
{
    QGeoCoordinate qgeocoord1(43.5435, 76.342);
    QGeoCoordinate qgeocoord2(-43.5435, 176.342);

    QGeoRouteRequest copy(qgeocoord1, qgeocoord2);

    QList<QGeoCoordinate> waypoints;
    waypoints.append(qgeocoord1);
    waypoints.append(qgeocoord2);

    QCOMPARE(copy.waypoints(), waypoints);
    QCOMPARE(copy.numberAlternativeRoutes(), 0);
    QCOMPARE(copy.routeOptimization(), QGeoRouteRequest::FastestRoute);
    QCOMPARE(copy.segmentDetail(), QGeoRouteRequest::BasicSegmentData);
    QCOMPARE(copy.travelModes(), QGeoRouteRequest::CarTravel);
    QCOMPARE(copy.maneuverDetail(), QGeoRouteRequest::BasicManeuvers);

}

void tst_QGeoRouteRequest::copy_constructor()
{
    QGeoRouteRequest qgeorouterequest;

    QGeoRouteRequest qgeorouterequestcopy(qgeorouterequest);
    QCOMPARE(qgeorouterequest, qgeorouterequestcopy);
}

void tst_QGeoRouteRequest::excludeAreas()
{
    QGeoRouteRequest qgeorouterequest;

    QGeoCoordinate qgeocoordinate;
    qgeocoordinate.setLatitude(13.3851);
    qgeocoordinate.setLongitude(52.5312);

    QGeoCoordinate qgeocoordinatecopy(34.324 , -110.32);

    QGeoRectangle qgeoboundingboxcopy(qgeocoordinate, 0.4, 0.4);
    QGeoRectangle qgeoboundingboxcopy2(qgeocoordinatecopy, 1.2, 0.9);
    QList<QGeoRectangle> areas;
    areas.append(qgeoboundingboxcopy);
    areas.append(qgeoboundingboxcopy2);

    qgeorouterequest.setExcludeAreas(areas);

    QCOMPARE(qgeorouterequest.excludeAreas(), areas);
}

void tst_QGeoRouteRequest::numberAlternativeRoutes()
{
    QGeoRouteRequest qgeorouterequest;
    qgeorouterequest.setNumberAlternativeRoutes(0);
    QCOMPARE(qgeorouterequest.numberAlternativeRoutes(), 0);

    qgeorouterequest.setNumberAlternativeRoutes(24);
    QCOMPARE(qgeorouterequest.numberAlternativeRoutes(), 24);

    qgeorouterequest.setNumberAlternativeRoutes(-12);

    QCOMPARE(qgeorouterequest.numberAlternativeRoutes(), 0);
}

void tst_QGeoRouteRequest::routeOptimization()
{
    QFETCH(QGeoRouteRequest::RouteOptimization, optimization);

    QGeoRouteRequest qgeorouterequest;
    QCOMPARE(qgeorouterequest.routeOptimization(),QGeoRouteRequest::FastestRoute);

    qgeorouterequest.setRouteOptimization(optimization);
    QCOMPARE(qgeorouterequest.routeOptimization(), optimization);
}

void tst_QGeoRouteRequest::routeOptimization_data()
{
    QTest::addColumn<QGeoRouteRequest::RouteOptimization>("optimization");

    QTest::newRow("optimization1") << QGeoRouteRequest::FastestRoute;
    QTest::newRow("optimization2") << QGeoRouteRequest::ShortestRoute;
    QTest::newRow("optimization3") << QGeoRouteRequest::MostEconomicRoute;
    QTest::newRow("optimization4") << QGeoRouteRequest::MostScenicRoute;

}

void tst_QGeoRouteRequest::segmentDetail()
{
    QFETCH(QGeoRouteRequest::SegmentDetail, detail);

    QGeoRouteRequest qgeorouterequest;
    QCOMPARE(qgeorouterequest.segmentDetail(), QGeoRouteRequest::BasicSegmentData);

    qgeorouterequest.setSegmentDetail(detail);
    QCOMPARE(qgeorouterequest.segmentDetail(), detail);
}

void tst_QGeoRouteRequest::segmentDetail_data()
{
    QTest::addColumn<QGeoRouteRequest::SegmentDetail>("detail");

    QTest::newRow("detail1") << QGeoRouteRequest::NoSegmentData;
    QTest::newRow("detail2") << QGeoRouteRequest::BasicSegmentData;
}

void tst_QGeoRouteRequest::travelModes()
{
    QFETCH(QGeoRouteRequest::TravelMode,mode);

    QGeoRouteRequest qgeorouterequest;
    QCOMPARE(qgeorouterequest.travelModes(), QGeoRouteRequest::CarTravel);

    qgeorouterequest.setTravelModes(mode);
    QCOMPARE(qgeorouterequest.travelModes(), mode);
}

void tst_QGeoRouteRequest::travelModes_data()
{
    QTest::addColumn<QGeoRouteRequest::TravelMode>("mode");

    QTest::newRow("mode1") << QGeoRouteRequest::CarTravel;
    QTest::newRow("mode2") << QGeoRouteRequest::PedestrianTravel;
    QTest::newRow("mode3") << QGeoRouteRequest::BicycleTravel;
    QTest::newRow("mode4") << QGeoRouteRequest::PublicTransitTravel;
    QTest::newRow("mode5") << QGeoRouteRequest::TruckTravel;
}

void tst_QGeoRouteRequest::waypoints()
{
    QFETCH(QList<double>, coordinates);

    QList<QGeoCoordinate> waypoints;

    for (int i = 0; i < coordinates.size(); i += 2)
        waypoints.append(QGeoCoordinate(coordinates.at(i), coordinates.at(i+1)));

    QGeoRouteRequest qgeorouterequest;
    qgeorouterequest.setWaypoints(waypoints);

    QList<QGeoCoordinate> waypointsRetrieved = qgeorouterequest.waypoints();
    QCOMPARE(waypointsRetrieved, waypoints);
}

void tst_QGeoRouteRequest::waypoints_data()
{
    QTest::addColumn<QList<double> >("coordinates");

    QList<double> coordinates;

    coordinates << 0.0 << 0.0;
    QTest::newRow("path1") << coordinates ;

    coordinates << -23.23 << 54.45345;
    QTest::newRow("path2") << coordinates ;

    coordinates << -85.4324 << -121.343;
    QTest::newRow("path3") << coordinates ;

    coordinates << 1.323 << 12.323;
    QTest::newRow("path4") << coordinates ;

    coordinates << 1324.323 << 143242.323;
    QTest::newRow("path5") << coordinates ;
}

void tst_QGeoRouteRequest::maneuverDetail()
{
    QFETCH(QGeoRouteRequest::ManeuverDetail,maneuver);

    QGeoRouteRequest qgeorouterequest;
    QCOMPARE(qgeorouterequest.maneuverDetail(), QGeoRouteRequest::BasicManeuvers);

    qgeorouterequest.setManeuverDetail(maneuver);
    QCOMPARE(qgeorouterequest.maneuverDetail(), maneuver);
}

void tst_QGeoRouteRequest::maneuverDetail_data()
{
    QTest::addColumn<QGeoRouteRequest::ManeuverDetail>("maneuver");

    QTest::newRow("maneuver1") << QGeoRouteRequest::NoManeuvers;
    QTest::newRow("maneuver2") << QGeoRouteRequest::BasicManeuvers;

}

void tst_QGeoRouteRequest::featureWeight_data()
{
    QTest::addColumn<QGeoRouteRequest::FeatureType>("type");
    QTest::addColumn<bool>("checkTypes");
    QTest::addColumn<QGeoRouteRequest::FeatureWeight>("initialWeight");
    QTest::addColumn<QGeoRouteRequest::FeatureWeight>("newWeight");
    QTest::addColumn<QGeoRouteRequest::FeatureWeight>("expectWeight");

    QTest::newRow("NoFeature") << QGeoRouteRequest::NoFeature << false
                               << QGeoRouteRequest::NeutralFeatureWeight
                               << QGeoRouteRequest::PreferFeatureWeight
                               << QGeoRouteRequest::NeutralFeatureWeight;
    QTest::newRow("TollFeature") << QGeoRouteRequest::TollFeature << true
                                 << QGeoRouteRequest::NeutralFeatureWeight
                                 << QGeoRouteRequest::PreferFeatureWeight
                                 << QGeoRouteRequest::PreferFeatureWeight;
    QTest::newRow("HighwayFeature") << QGeoRouteRequest::HighwayFeature << true
                                    << QGeoRouteRequest::NeutralFeatureWeight
                                    << QGeoRouteRequest::RequireFeatureWeight
                                    << QGeoRouteRequest::RequireFeatureWeight;
}

void tst_QGeoRouteRequest::featureWeight()
{
    QFETCH(QGeoRouteRequest::FeatureType, type);
    QFETCH(bool, checkTypes);
    QFETCH(QGeoRouteRequest::FeatureWeight, initialWeight);
    QFETCH(QGeoRouteRequest::FeatureWeight, newWeight);
    QFETCH(QGeoRouteRequest::FeatureWeight, expectWeight);

    QGeoRouteRequest qgeorouterequest;
    QCOMPARE(qgeorouterequest.featureWeight(type), initialWeight);
    qgeorouterequest.setFeatureWeight(type, newWeight);
    QCOMPARE(qgeorouterequest.featureWeight(type), expectWeight);

    if (checkTypes)
        QVERIFY(qgeorouterequest.featureTypes().contains(type));
}

void tst_QGeoRouteRequest::departureTime_data()
{
    QTest::addColumn<QDateTime>("departureTime");

    QTest::newRow("Invalid") << QDateTime();
    QTest::newRow("date1") << QDateTime(QDate(2012, 7, 6), QTime(23, 55, 0));
    QTest::newRow("date2") << QDateTime(QDate(2012, 7, 7), QTime(0, 5, 0));
}

void tst_QGeoRouteRequest::departureTime()
{
    QGeoRouteRequest qgeorouterequest;
    QFETCH(QDateTime , departureTime);
    qgeorouterequest.setDepartureTime(departureTime);
    QCOMPARE(qgeorouterequest.departureTime(), departureTime);
}

QTEST_APPLESS_MAIN(tst_QGeoRouteRequest);
