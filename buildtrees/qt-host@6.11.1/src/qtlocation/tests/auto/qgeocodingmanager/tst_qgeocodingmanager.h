// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#ifndef TST_QGEOCODINGMANAGER_H
#define TST_QGEOCODINGMANAGER_H

#include <QLocale>
#include <QtTest/QtTest>
#include <QSignalSpy>

#include <qgeoserviceprovider.h>
#include <qgeocodingmanager.h>
#include <qgeocodereply.h>
#include <QtPositioning/QGeoRectangle>
#include <qgeoaddress.h>
#include <qgeocoordinate.h>


QT_USE_NAMESPACE

class tst_QGeoCodingManager: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void locale();
    void name();
    void version();
    void search();
    void geocode();
    void reverseGeocode();

private:
    QGeoServiceProvider *qgeoserviceprovider;
    QGeoCodingManager *qgeocodingmanager;
    QSignalSpy *signalerror;
    QSignalSpy *signalfinished;
    void loadGeocodingManager();

};
Q_DECLARE_METATYPE(QGeoCodeReply*);
Q_DECLARE_METATYPE(QGeoCodeReply::Error);

#endif

