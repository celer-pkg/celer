// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOMANEUVER_H
#define TST_QGEOMANEUVER_H


#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QMetaType>
#include <QSignalSpy>

#include "../utils/qlocationtestutils_p.h"
#include <qgeomaneuver.h>
#include <qgeocoordinate.h>


class tst_QGeoManeuver : public QObject
{
    Q_OBJECT

public:
    tst_QGeoManeuver();

private Q_SLOTS:
    //Start unit test for QGeoRouteManeuver
    void constructor();
    void copy_constructor();
    void direction();
    void direction_data();
    void distanceToNextInstruction();
    void instructionText();
    void position();
    void position_data();
    void timeToNextInstruction();
    void waypoint();
    void waypoint_data();
    void isValid();
    void operators();
    //End Unit Test for QGeoRouteManeuver
};

Q_DECLARE_METATYPE( QList<double>);

#endif // TST_QGEOMANEUVER_H

