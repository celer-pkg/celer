// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOROUTESEGMENT_H
#define TST_QGEOROUTESEGMENT_H

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QMetaType>

#include <qgeocoordinate.h>
#include <qgeoroutesegment.h>
#include <qgeomaneuver.h>

QT_USE_NAMESPACE

class tst_QGeoRouteSegment : public QObject
{
    Q_OBJECT

public:
    tst_QGeoRouteSegment();

private Q_SLOTS:
    //Start unit test for QGeoRouteSegment
    void constructor();
    void copy_constructor();
    void travelTime();
    void travelTime_data();
    void distance();
    void distance_data();
    void path();
    void path_data();
    void maneuver();
    void nextroutesegment();
    void operators();
    void operators_data();
    //End Unit Test for QGeoRouteSegment
};

Q_DECLARE_METATYPE( QList<double>);

#endif // TST_GEOROUTESEGMENT_H

