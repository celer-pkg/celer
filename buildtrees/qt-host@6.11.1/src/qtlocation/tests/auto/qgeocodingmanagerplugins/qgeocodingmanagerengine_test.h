// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGEOCODINGMANAGERENGINE_TEST_H
#define QGEOCODINGMANAGERENGINE_TEST_H

#include <qgeoserviceprovider.h>
#include <qgeocodingmanagerengine.h>
#include <QLocale>
#include <qgeoaddress.h>
#include <qgeolocation.h>
#include <qgeocodereply.h>
#include <QtPositioning/QGeoCoordinate>

QT_USE_NAMESPACE

class GeocodeReplyTest : public QGeoCodeReply
{
    Q_OBJECT
public:
    GeocodeReplyTest(QObject *parent = nullptr) : QGeoCodeReply(parent) {}

    void  callAddLocation ( const QGeoLocation & location ) {addLocation(location);}
    void  callSetError ( Error error, const QString & errorString ) {setError(error, errorString);}
    void  callSetFinished ( bool finished ) {setFinished(finished);}
    void  callSetLimit ( int limit ) {setLimit(limit);}
    void  callSetOffset ( int offset ) {setOffset(offset);}
    void  callSetLocations ( const QList<QGeoLocation> & locations ) {setLocations(locations);}
    void  callSetViewport ( const QGeoShape &viewport ) {setViewport(viewport);}

};

class QGeoCodingManagerEngineTest: public QGeoCodingManagerEngine

{
Q_OBJECT
public:
    QGeoCodingManagerEngineTest(const QVariantMap &parameters,
                                QGeoServiceProvider::Error *error, QString *errorString) :
                                QGeoCodingManagerEngine(parameters)
    {
        Q_UNUSED(error);
        Q_UNUSED(errorString);
        setLocale(QLocale(QLocale::German, QLocale::Germany));
    }

    QGeoCodeReply* geocode(const QString &searchString, int limit, int offset,
                           const QGeoShape &bounds) override
    {
        GeocodeReplyTest *geocodereply = new GeocodeReplyTest();
        geocodereply->callSetLimit(limit);
        geocodereply->callSetOffset(offset);
        geocodereply->callSetViewport(bounds);
        geocodereply->callSetError(QGeoCodeReply::NoError,searchString);
        geocodereply->callSetFinished(true);
        emit(this->finished(geocodereply));

        return static_cast<QGeoCodeReply*>(geocodereply);
    }

    QGeoCodeReply* geocode (const QGeoAddress &address, const QGeoShape &bounds) override
    {
        GeocodeReplyTest *geocodereply = new GeocodeReplyTest();
        geocodereply->callSetViewport(bounds);
        geocodereply->callSetError(QGeoCodeReply::NoError,address.city());
        geocodereply->callSetFinished(true);
        emit(this->finished(geocodereply));

        return static_cast<QGeoCodeReply*>(geocodereply);
    }

    QGeoCodeReply* reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds) override
    {
        GeocodeReplyTest *geocodereply = new GeocodeReplyTest();
        geocodereply->callSetViewport(bounds);
        geocodereply->callSetError(QGeoCodeReply::NoError,coordinate.toString());
        geocodereply->callSetFinished(true);
        emit(this->finished(geocodereply));
        return static_cast<QGeoCodeReply*>(geocodereply);
    }
};

#endif
