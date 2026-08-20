// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGEOCODEREPLY_H
#define TST_QGEOCODEREPLY_H

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QMetaType>
#include <QSignalSpy>

#include "../utils/qlocationtestutils_p.h"

#include <qgeocodereply.h>
#include <qgeolocation.h>
#include <qgeoaddress.h>
#include <qgeocoordinate.h>
#include <QtPositioning/QGeoRectangle>

QT_USE_NAMESPACE
class SubGeocodeReply : public QGeoCodeReply
{
    Q_OBJECT
public:
    SubGeocodeReply() : QGeoCodeReply() {}

    void  callAddLocation ( const QGeoLocation & location ) {addLocation(location);}
    void  callSetError ( Error error, const QString & errorString ) {setError(error, errorString);}
    void  callSetFinished ( bool finished ) {setFinished(finished);}
    void  callSetLimit ( int limit ) {setLimit(limit);}
    void  callSetOffset ( int offset ) {setOffset(offset);}
    void  callSetLocations ( const QList<QGeoLocation> & locations ) {setLocations(locations);}
    void  callSetViewport ( const QGeoShape &viewport ) {setViewport(viewport);}

};

class tst_QGeoCodeReply :public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    //Start Unit Test for QGeoCodeReply
private slots:
    void constructor();
    void constructor_error();
    void constructor_error_data();
    void destructor();
    void destructor_data();
    void abort();
    void error();
    void error_data();
    void finished();
    void limit();
    void offset();
    void locations();
    void viewport();

    //End Unit Test for QGeoCodeReply



private:
    QSignalSpy *signalerror;
    QSignalSpy *signalfinished;
    SubGeocodeReply* reply;
    QGeoLocation *qgeolocation;
    QGeoRectangle *qgeoboundingbox;
};

Q_DECLARE_METATYPE(QList<double>);
Q_DECLARE_METATYPE(QGeoCodeReply::Error);

#endif // TST_QGEOCODEREPLY_H

