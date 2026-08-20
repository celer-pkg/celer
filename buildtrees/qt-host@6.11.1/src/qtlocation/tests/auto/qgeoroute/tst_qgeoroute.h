// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOROUTE_H
#define TST_QGEOROUTE_H

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QMetaType>

#include <qgeoroute.h>
#include <QtPositioning/QGeoRectangle>
#include <qgeocoordinate.h>
#include <qgeorouterequest.h>
#include <qgeoroutesegment.h>


QT_USE_NAMESPACE

class tst_QGeoRoute : public QObject
{
    Q_OBJECT

public:
    tst_QGeoRoute();

private Q_SLOTS:
    //Start unit test for QGeoRoute
    void constructor();
    void copy_constructor();
    void bounds();
    void distance();
    void path();
    void path_data();
    void request();
    void routeId();
    void firstrouteSegments();
    void travelMode();
    void travelMode_data();
    void travelTime();
    void operators();
    //End Unit Test for QGeoRoute
};

Q_DECLARE_METATYPE( QList<double>);
Q_DECLARE_METATYPE( QList<QGeoCoordinate>);
Q_DECLARE_METATYPE( QGeoRouteRequest::TravelMode);


#endif // TST_QGEOROUTE_H
