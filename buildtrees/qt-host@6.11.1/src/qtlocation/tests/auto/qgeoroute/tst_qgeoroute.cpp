// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_qgeoroute.h"
#include "../geotestplugin/qgeoroutingmanagerengine_test.h"


tst_QGeoRoute::tst_QGeoRoute()
{
}

void tst_QGeoRoute::constructor()
{
    QGeoRoute qgeoroute;

    QCOMPARE(qgeoroute.bounds(), QGeoRectangle());
    QCOMPARE(qgeoroute.distance(), qreal(0.0));
    QCOMPARE(qgeoroute.path().size(), 0);
    QCOMPARE(qgeoroute.routeId(), QString());
    QCOMPARE(qgeoroute.travelTime(), 0);
}

void tst_QGeoRoute::copy_constructor()
{
    QGeoRoute qgeoroute;

    QGeoRoute qgeoroutecopy(qgeoroute);
    QCOMPARE(qgeoroute, qgeoroutecopy);

    // CoW
    qreal distance = qgeoroute.distance();
    qgeoroutecopy.setDistance(distance + 10.0);

    QVERIFY(qgeoroute == qgeoroutecopy); // QGeoRoute uses a QExplicitlySharedDataPointer. no implicit detach()
}

void tst_QGeoRoute::bounds()
{
    QGeoRoute qgeoroute;

    QGeoCoordinate qgeocoordinate;
    qgeocoordinate.setLatitude(13.3851);
    qgeocoordinate.setLongitude(52.5312);

    QGeoRectangle qgeoboundingbox(qgeocoordinate, 0.4, 0.4);

    qgeoroute.setBounds(qgeoboundingbox);

    QCOMPARE(qgeoroute.bounds(), qgeoboundingbox);

    qgeoboundingbox.setWidth(23.1);

    QVERIFY(qgeoroute.bounds().width() != qgeoboundingbox.width());
}

void tst_QGeoRoute::distance()
{
    QGeoRoute qgeoroute;

    qreal distance = 0.0;

    qgeoroute.setDistance(distance);
    QCOMPARE(qgeoroute.distance(), distance);

    distance = 34.4324;
    QVERIFY(qgeoroute.distance() != distance);

    qgeoroute.setDistance(distance);
    QCOMPARE(qgeoroute.distance(), distance);
}

void tst_QGeoRoute::path()
{
    QFETCH(QList<double>, coordinates);

    QList<QGeoCoordinate> path;

    for (int i = 0; i < coordinates.size(); i += 2)
        path.append(QGeoCoordinate(coordinates.at(i),coordinates.at(i+1)));

    QGeoRoute qgeoroute;
    qgeoroute.setPath(path);

    QList<QGeoCoordinate> pathRetrieved = qgeoroute.path();
    QCOMPARE(pathRetrieved, path);
}

void tst_QGeoRoute::path_data()
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

void tst_QGeoRoute::request()
{
    QGeoRoute qgeoroute;
    QGeoCoordinate qgeocoordinate;

    qgeocoordinate.setLatitude(65.654);
    qgeocoordinate.setLongitude(0.4324);

    QGeoCoordinate qgeocoordinatecopy(34.54 , -21.32);

    QList<QGeoCoordinate> path;
    path.append(qgeocoordinate);
    path.append(qgeocoordinatecopy);

    QGeoRouteRequest qgeorouterequest(path);

    qgeoroute.setRequest(qgeorouterequest);

    QCOMPARE(qgeoroute.request(), qgeorouterequest);

    QGeoCoordinate qgeocoordinatecopy2(4.7854 , -121.32);
    path.append(qgeocoordinatecopy2);

    QGeoRouteRequest qgeorouterequestcopy(path);

    QVERIFY(qgeoroute.request() != qgeorouterequestcopy);
}

void tst_QGeoRoute::routeId()
{
    const QString text = "routeId 4504";

    QGeoRoute qgeoroute;
    qgeoroute.setRouteId(text);

    QCOMPARE(qgeoroute.routeId(), text);
}

void tst_QGeoRoute::firstrouteSegments()
{
    QGeoRouteSegment qgeoroutesegment;
    qgeoroutesegment.setDistance(35.453);
    qgeoroutesegment.setTravelTime(56);

    QGeoRoute qgeoroute;
    qgeoroute.setFirstRouteSegment(qgeoroutesegment);

    QCOMPARE(qgeoroute.firstRouteSegment(), qgeoroutesegment);

    QGeoRouteSegment qgeoroutesegmentcopy;
    qgeoroutesegmentcopy.setDistance(435.432);
    qgeoroutesegmentcopy.setTravelTime(786);

    QVERIFY(qgeoroute.firstRouteSegment() != qgeoroutesegmentcopy);
}

void tst_QGeoRoute::travelMode()
{
    QFETCH(QGeoRouteRequest::TravelMode, mode);

    QGeoRoute qgeoroute;
    qgeoroute.setTravelMode(mode);
    QCOMPARE(qgeoroute.travelMode(), mode);
}
void tst_QGeoRoute::travelMode_data()
{
    QTest::addColumn<QGeoRouteRequest::TravelMode>("mode");

    QTest::newRow("mode1") << QGeoRouteRequest::CarTravel;
    QTest::newRow("mode2") << QGeoRouteRequest::PedestrianTravel;
    QTest::newRow("mode3") << QGeoRouteRequest::BicycleTravel;
    QTest::newRow("mode4") << QGeoRouteRequest::PublicTransitTravel;
    QTest::newRow("mode5") << QGeoRouteRequest::TruckTravel;
}

void tst_QGeoRoute::travelTime()
{
    QGeoRoute qgeoroute;
    int time = 0;
    qgeoroute.setTravelTime(time);

    QCOMPARE (qgeoroute.travelTime(), time);

    time = 35;

    QVERIFY (qgeoroute.travelTime() != time);

    qgeoroute.setTravelTime(time);
    QCOMPARE (qgeoroute.travelTime(), time);
}

void tst_QGeoRoute::operators()
{
    QGeoRoute qgeoroute;
    QGeoRoute qgeoroutecopy(qgeoroute);

    QVERIFY(qgeoroute == qgeoroutecopy);
    QVERIFY(!(qgeoroute != qgeoroutecopy));

    qgeoroute.setDistance(543.324); // QExplicitlySharedDataPointer does not detach implicitly.
    qgeoroute.setRouteId("RouteId 111");
    qgeoroute.setTravelMode(QGeoRouteRequest::PedestrianTravel);
    qgeoroute.setTravelTime(10);

    qgeoroutecopy.setDistance(12.21);
    qgeoroutecopy.setRouteId("RouteId 666");
    qgeoroutecopy.setTravelMode(QGeoRouteRequest::BicycleTravel);
    qgeoroutecopy.setTravelTime(99);

    QEXPECT_FAIL("", "QGeoRoute equality operators broken", Continue);
    QVERIFY(!(qgeoroute == qgeoroutecopy));
    QEXPECT_FAIL("", "QGeoRoute equality operators broken", Continue);
    QVERIFY(qgeoroute != qgeoroutecopy);

    qgeoroutecopy = qgeoroute;
    QVERIFY(qgeoroute == qgeoroutecopy);
    QVERIFY(!(qgeoroute != qgeoroutecopy));


    QGeoRoute r;
    QCOMPARE(r.travelTime(), 0);
    QGeoRoute r2;
    r2.setTravelTime(123456);
    r = r2;
    QCOMPARE(r.travelTime(), 123456);
}



QTEST_APPLESS_MAIN(tst_QGeoRoute);
