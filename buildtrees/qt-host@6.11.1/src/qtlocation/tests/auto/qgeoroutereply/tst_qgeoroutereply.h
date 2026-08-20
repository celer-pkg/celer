// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOROUTEREPLY_H
#define TST_QGEOROUTEREPLY_H

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QMetaType>
#include <QSignalSpy>

#include <qgeocoordinate.h>
#include <qgeorouterequest.h>
#include <qgeoroutereply.h>
#include <qgeoroute.h>

QT_USE_NAMESPACE
class SubRouteReply :public QGeoRouteReply
{
    Q_OBJECT
public:
    SubRouteReply(QGeoRouteRequest request):QGeoRouteReply(request) {}
    void callSetError(QGeoRouteReply::Error error, QString msg) {setError(error,msg);}
    void callSetFinished(bool finished) {setFinished(finished);}
    void callSetRoutes(const QList<QGeoRoute> & routes ) {setRoutes(routes);}
};

class tst_QGeoRouteReply :public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    //Start Unit Test for QGeoRouteReply
private slots:
    void constructor();
    void constructor_error();
    void constructor_error_data();
    void destructor();
    void destructor_data();
    void routes();
    void finished();
    void abort();
    void error();
    void error_data();
    void request();
    //End Unit Test for QGeoRouteReply


private:
    QGeoCoordinate *qgeocoordinate1;
    QGeoCoordinate *qgeocoordinate2;
    QGeoCoordinate *qgeocoordinate3;
    QGeoRouteRequest *qgeorouterequest;
    QSignalSpy *signalerror;
    QSignalSpy *signalfinished;
    QList<QGeoCoordinate> waypoints;
    SubRouteReply* reply;
};

Q_DECLARE_METATYPE( QList<QGeoRoute>);
Q_DECLARE_METATYPE( QList<QGeoCoordinate>);
Q_DECLARE_METATYPE( QList<double>);
Q_DECLARE_METATYPE( QGeoRouteReply::Error);

#endif // TST_QGEOROUTEREPLY_H

