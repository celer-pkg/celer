// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_qgeoroutereply.h"

QT_USE_NAMESPACE

void tst_QGeoRouteReply::initTestCase()
{
    qgeocoordinate1 = new QGeoCoordinate(43.5435 , 76.342);
    qgeocoordinate2 = new QGeoCoordinate(-43.5435 , 176.342);
    qgeocoordinate3 = new QGeoCoordinate(-13.5435 , +76.342);

    waypoints.append(*qgeocoordinate1);
    waypoints.append(*qgeocoordinate2);
    waypoints.append(*qgeocoordinate3);

    qgeorouterequest = new QGeoRouteRequest(waypoints);
    reply = new SubRouteReply(*qgeorouterequest);
}

void tst_QGeoRouteReply::cleanupTestCase()
{
    delete qgeocoordinate1;
    delete qgeocoordinate2;
    delete qgeocoordinate3;
    delete qgeorouterequest;
    delete reply;
}

void tst_QGeoRouteReply::init()
{
    qRegisterMetaType<QGeoRouteReply::Error>();
    signalerror = new QSignalSpy(reply, SIGNAL(errorOccurred(QGeoRouteReply::Error,QString)));
    signalfinished = new QSignalSpy(reply, SIGNAL(finished()));
}

void tst_QGeoRouteReply::cleanup()
{
    delete signalerror;
    delete signalfinished;
}

void tst_QGeoRouteReply::constructor()
{
    QVERIFY(!reply->isFinished());
    QCOMPARE(reply->error(), QGeoRouteReply::NoError);
    QCOMPARE(reply->request(), *qgeorouterequest);

    QVERIFY(signalerror->isValid());
    QVERIFY(signalfinished->isValid());

    QCOMPARE(signalerror->count(),0);
    QCOMPARE(signalfinished->count(),0);
}

void tst_QGeoRouteReply::constructor_error()
{
    QFETCH(QGeoRouteReply::Error,error);
    QFETCH(QString,msg);

    QVERIFY(signalerror->isValid());
    QVERIFY(signalfinished->isValid());

    QGeoRouteReply *qgeoroutereplycopy = new QGeoRouteReply(error, msg, 0);

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 0);

    QVERIFY(qgeoroutereplycopy->isFinished());
    QCOMPARE(qgeoroutereplycopy->error(), error);
    QCOMPARE(qgeoroutereplycopy->errorString(), msg);

    delete qgeoroutereplycopy;
}

void tst_QGeoRouteReply::constructor_error_data()
{
    QTest::addColumn<QGeoRouteReply::Error>("error");
    QTest::addColumn<QString>("msg");

    QTest::newRow("error1") << QGeoRouteReply::NoError << "No error.";
    QTest::newRow("error2") << QGeoRouteReply::EngineNotSetError << "Engine Not Set Error.";
    QTest::newRow("error3") << QGeoRouteReply::CommunicationError << "Communication Error.";
    QTest::newRow("error4") << QGeoRouteReply::ParseError << "Parse Error.";
    QTest::newRow("error5") << QGeoRouteReply::UnsupportedOptionError << "Unsupported Option Error.";
    QTest::newRow("error6") << QGeoRouteReply::UnknownError << "Unknown Error.";

}

void tst_QGeoRouteReply::destructor()
{
    QGeoRouteReply *qgeoroutereplycopy;

    QFETCH(QGeoRouteReply::Error, error);
    QFETCH(QString, msg);

    qgeoroutereplycopy = new QGeoRouteReply(error, msg, 0);
    delete qgeoroutereplycopy;

}

void tst_QGeoRouteReply::destructor_data()
{
    tst_QGeoRouteReply::constructor_error_data();
}

void tst_QGeoRouteReply::routes()
{
    QList<QGeoRoute> routes;
    QGeoRoute *route1 = new QGeoRoute();
    QGeoRoute *route2 = new QGeoRoute();

    route1->setDistance(15.12);
    route2->setDistance(20.12);

    routes.append(*route1);
    routes.append(*route2);

    reply->callSetRoutes(routes);

    QList<QGeoRoute> routescopy;
    routescopy = reply->routes();
    QCOMPARE(routescopy,routes);

    QCOMPARE(routescopy.at(0).distance(),route1->distance());
    QCOMPARE(routescopy.at(1).distance(),route2->distance());

    delete route1;
    delete route2;
}

void tst_QGeoRouteReply::finished()
{
    QVERIFY(signalerror->isValid());
    QVERIFY(signalfinished->isValid());

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 0);

    reply->callSetFinished(true);

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 1);

    reply->callSetFinished(false);

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 1);

    reply->callSetFinished(true);

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 2);
}

void tst_QGeoRouteReply::abort()
{
    QVERIFY(signalerror->isValid());
    QVERIFY(signalfinished->isValid());

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 0);

    reply->abort();

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 0);

    reply->abort();
    reply->callSetFinished(false);
    reply->abort();

    QCOMPARE(signalerror->count(), 0);
    QCOMPARE(signalfinished->count(), 0);
}

void tst_QGeoRouteReply::error()
{
    QFETCH(QGeoRouteReply::Error, error);
    QFETCH(QString, msg);

    QVERIFY(signalerror->isValid());
    QVERIFY(signalfinished->isValid());
    QCOMPARE(signalerror->count(), 0);

    reply->callSetError(error,msg);

    QCOMPARE(signalerror->count(), 1);
    QCOMPARE(signalfinished->count(), 1);
    QCOMPARE(reply->errorString(), msg);
    QCOMPARE(reply->error(), error);
}

void tst_QGeoRouteReply::error_data()
{
    QTest::addColumn<QGeoRouteReply::Error>("error");
    QTest::addColumn<QString>("msg");

    QTest::newRow("error1") << QGeoRouteReply::NoError << "No error.";
    QTest::newRow("error2") << QGeoRouteReply::EngineNotSetError << "Engine Not Set Error.";
    QTest::newRow("error3") << QGeoRouteReply::CommunicationError << "Communication Error.";
    QTest::newRow("error4") << QGeoRouteReply::ParseError << "Parse Error.";
    QTest::newRow("error5") << QGeoRouteReply::UnsupportedOptionError << "Unsupported Option Error.";
    QTest::newRow("error6") << QGeoRouteReply::UnknownError << "Unknown Error.";
}

void tst_QGeoRouteReply::request()
{
    SubRouteReply *rr = new SubRouteReply(*qgeorouterequest);

    QCOMPARE(rr->request(), *qgeorouterequest);

    delete rr;
}

QTEST_APPLESS_MAIN(tst_QGeoRouteReply);
