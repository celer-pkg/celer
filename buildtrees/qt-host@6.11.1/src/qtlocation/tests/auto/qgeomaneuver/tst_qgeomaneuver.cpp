// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_qgeomaneuver.h"
#include <QtLocation/private/qgeomaneuver_p.h>

tst_QGeoManeuver::tst_QGeoManeuver()
{
}

void tst_QGeoManeuver::constructor()
{
    const QString empty ="";
    QGeoManeuver qgeomaneuver;

    QVERIFY(!qgeomaneuver.isValid());
    QCOMPARE(qgeomaneuver.direction(), QGeoManeuver::NoDirection);
    QCOMPARE(qgeomaneuver.distanceToNextInstruction(), qreal(0.0));
    QCOMPARE(qgeomaneuver.instructionText(), empty);
    QCOMPARE(qgeomaneuver.timeToNextInstruction(),0);
}

void tst_QGeoManeuver::copy_constructor()
{
    QGeoManeuver qgeomaneuver;
    QGeoManeuver qgeomaneuvercopy(qgeomaneuver);

    QCOMPARE(qgeomaneuver, qgeomaneuvercopy);
}

void tst_QGeoManeuver::direction()
{
    QFETCH(QGeoManeuver::InstructionDirection,direction);

    QGeoManeuver qgeomaneuver;
    qgeomaneuver.setDirection(direction);

    QCOMPARE(qgeomaneuver.direction(),direction);
}

void tst_QGeoManeuver::direction_data()
{
    QTest::addColumn<QGeoManeuver::InstructionDirection>("direction");

    QTest::newRow("instruction1") << QGeoManeuver::NoDirection;
    QTest::newRow("instruction2") << QGeoManeuver::DirectionForward;
    QTest::newRow("instruction3") << QGeoManeuver::DirectionBearRight;
    QTest::newRow("instruction4") << QGeoManeuver::DirectionLightRight;
    QTest::newRow("instruction5") << QGeoManeuver::DirectionRight;
    QTest::newRow("instruction6") << QGeoManeuver::DirectionHardRight;
    QTest::newRow("instruction7") << QGeoManeuver::DirectionUTurnRight;
    QTest::newRow("instruction8") << QGeoManeuver::DirectionUTurnLeft;
    QTest::newRow("instruction9") << QGeoManeuver::DirectionHardLeft;
    QTest::newRow("instruction10") << QGeoManeuver::DirectionLeft;
    QTest::newRow("instruction11") << QGeoManeuver::DirectionLightLeft;
    QTest::newRow("instruction12") << QGeoManeuver::DirectionBearLeft;
}

void tst_QGeoManeuver::distanceToNextInstruction()
{
    QGeoManeuver qgeomaneuver;

    qreal distance = 0.0;
    qgeomaneuver.setDistanceToNextInstruction(distance);

    QCOMPARE (qgeomaneuver.distanceToNextInstruction(), distance);

    distance = -3423.4324;

    QVERIFY (qgeomaneuver.distanceToNextInstruction() != distance);

    qgeomaneuver.setDistanceToNextInstruction(distance);
    QCOMPARE (qgeomaneuver.distanceToNextInstruction(),distance);
}

void tst_QGeoManeuver::instructionText()
{
    QGeoManeuver qgeomaneuver;

    QString text = "After 50m turn left";

    qgeomaneuver.setInstructionText(text);

    QCOMPARE (qgeomaneuver.instructionText(),text);

    text="After 40m, turn left";
    QVERIFY (qgeomaneuver.instructionText() != text);

}

void tst_QGeoManeuver::position()
{
    QFETCH(double, latitude);
    QFETCH(double, longitude);

    QGeoCoordinate qgeocoordinate(latitude, longitude);

    QGeoManeuver qgeomaneuver;
    qgeomaneuver.setPosition(qgeocoordinate);

    QCOMPARE(qgeomaneuver.position(), qgeocoordinate);
}

void tst_QGeoManeuver::position_data()
{
    QTest::addColumn<double>("latitude");
    QTest::addColumn<double>("longitude");

    QTest::newRow("invalid0") << -12220.0 << 0.0;
    QTest::newRow("invalid1") << 0.0 << 181.0;

    QTest::newRow("correct0") << 0.0 << 0.0;
    QTest::newRow("correct1") << 90.0 << 0.0;
    QTest::newRow("correct2") << 0.0 << 180.0;
    QTest::newRow("correct3") << -90.0 << 0.0;
    QTest::newRow("correct4") << 0.0 << -180.0;
    QTest::newRow("correct5") << 45.0 << 90.0;
}

void tst_QGeoManeuver::timeToNextInstruction()
{
    QGeoManeuver qgeomaneuver;

    int time = 0;
    qgeomaneuver.setTimeToNextInstruction(time);

    QCOMPARE(qgeomaneuver.timeToNextInstruction(), time);

    time = 35;

    QVERIFY(qgeomaneuver.timeToNextInstruction() != time);

    qgeomaneuver.setTimeToNextInstruction(time);
    QCOMPARE(qgeomaneuver.timeToNextInstruction(), time);
}

void tst_QGeoManeuver::waypoint()
{
    QFETCH(double, latitude);
    QFETCH(double, longitude);

    QGeoCoordinate qgeocoordinate(latitude, longitude);
    QGeoManeuver qgeomaneuver;

    qgeomaneuver.setWaypoint(qgeocoordinate);

    QCOMPARE(qgeomaneuver.waypoint(), qgeocoordinate);

    qgeocoordinate.setLatitude(30.3);
    QVERIFY(qgeomaneuver.waypoint() != qgeocoordinate);
}

void tst_QGeoManeuver::waypoint_data()
{
    QTest::addColumn<double>("latitude");
    QTest::addColumn<double>("longitude");

    QTest::newRow("invalid0") << -12220.0 << 0.0;
    QTest::newRow("invalid1") << 0.0 << 181.0;

    QTest::newRow("correct0") << 0.0 << 0.0;
    QTest::newRow("correct1") << 90.0 << 0.0;
    QTest::newRow("correct2") << 0.0 << 180.0;
    QTest::newRow("correct3") << -90.0 << 0.0;
    QTest::newRow("correct4") << 0.0 << -180.0;
    QTest::newRow("correct5") << 45.0 << 90.0;
}

void tst_QGeoManeuver::isValid()
{
    QGeoManeuver qgeomaneuver;
    QVERIFY(!qgeomaneuver.isValid());
    qgeomaneuver.setDirection(QGeoManeuver::DirectionBearLeft);
    QVERIFY(qgeomaneuver.isValid());
}

void tst_QGeoManeuver::operators()
{
    QGeoManeuver qgeomaneuver;
    QGeoManeuver qgeomaneuvercopy(qgeomaneuver);

    QVERIFY(qgeomaneuver == qgeomaneuvercopy);
    QVERIFY(!(qgeomaneuver != qgeomaneuvercopy));

    qgeomaneuver.setDirection(QGeoManeuver::DirectionBearLeft);
    qgeomaneuver.setInstructionText("Turn left in 50m");
    qgeomaneuver.setTimeToNextInstruction(60);
    qgeomaneuver.setDistanceToNextInstruction(560.45);

    qgeomaneuvercopy.setDirection(QGeoManeuver::DirectionForward);
    qgeomaneuvercopy.setInstructionText("Turn left in 80m");
    qgeomaneuvercopy.setTimeToNextInstruction(70);
    qgeomaneuvercopy.setDistanceToNextInstruction(56065.45);

   QVERIFY(!(qgeomaneuver == qgeomaneuvercopy));
   QVERIFY(qgeomaneuver != qgeomaneuvercopy);

    qgeomaneuvercopy = qgeomaneuver;
    QVERIFY(qgeomaneuver == qgeomaneuvercopy);
    QVERIFY(!(qgeomaneuver != qgeomaneuvercopy));
}

QTEST_APPLESS_MAIN(tst_QGeoManeuver);
