// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#ifndef TST_QGEOROUTINGMANAGER_H
#define TST_QGEOROUTINGMANAGER_H

#include <QLocale>
#include <QtTest/QtTest>
#include <qgeoserviceprovider.h>
#include <qgeoroutingmanager.h>
#include <qgeorouterequest.h>
#include <qgeoroutereply.h>
#include <qgeocoordinate.h>
#include <qgeoroute.h>

QT_USE_NAMESPACE

class tst_QGeoRoutingManager: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void supports();
    void locale();
    void name();
    void version();
    void calculate();
    void update();

private:
    QGeoServiceProvider *qgeoserviceprovider;
    QGeoRoutingManager *qgeoroutingmanager;
    QGeoRouteRequest *request;
    QGeoRouteReply *reply;
    QGeoCoordinate *origin;
    QGeoCoordinate *destination;
    QGeoRoute *route;
    QGeoCoordinate *position;
    void loadRoutingManager();

};

#endif
