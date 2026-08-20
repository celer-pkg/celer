// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGEOCODINGMANAGERENGINE_TEST_H
#define QGEOCODINGMANAGERENGINE_TEST_H

#include <qgeoserviceprovider.h>
#include <qgeocodingmanagerengine.h>
#include <QLocale>
#include <QtPositioning/qgeoaddress.h>
#include <QtPositioning/qgeolocation.h>
#include <qgeocodereply.h>
#include <QtLocation/private/qgeocodereply_p.h>
#include <QtPositioning/QGeoShape>

#include <QTimer>
#include <QDebug>
#include <QTimerEvent>
#include <QVariantMap>

QT_USE_NAMESPACE

class GeocodeReplyTest :public QGeoCodeReply
{
    Q_OBJECT
public:
    using QGeoCodeReply::QGeoCodeReply;

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
                                QGeoCodingManagerEngine(parameters),
                                validateWellKnownValues_(false),
                                finishRequestImmediately_(true),
                                supported_(true),
                                geocodeReply_(0),
                                timerId_(0),
                                errorCode_(QGeoCodeReply::NoError)
    {
        Q_UNUSED(error);
        Q_UNUSED(errorString);
        if (parameters.contains("supported"))
            supported_ = qvariant_cast<bool>(parameters.value("supported"));
        if (parameters.contains("finishRequestImmediately"))
            finishRequestImmediately_ = qvariant_cast<bool>(parameters.value("finishRequestImmediately"));
        if (parameters.contains("validateWellKnownValues"))
            validateWellKnownValues_ = qvariant_cast<bool>(parameters.value("validateWellKnownValues"));
        if (parameters.contains("includeExtendedData")) {
            includeExtendedData_ = qvariant_cast<bool>(parameters.value("includeExtendedData"));
            extendedLocationData_["QGeoCodingManagerEngineTest_locationExtendedAttribute"] = 42;
            extendedReplyData_["QGeoCodingManagerEngineTest_extraData"] = 43;
        }

        setLocale(QLocale (QLocale::German, QLocale::Germany));
    }

    QGeoCodeReply* geocode(const QString &searchString,
                           int limit = -1,
                           int offset = 0,
                           const QGeoShape &bounds = QGeoShape()) override
    {
        geocodeReply_ = new GeocodeReplyTest();
        connect(geocodeReply_, SIGNAL(aborted()), this, SLOT(requestAborted()));
        geocodeReply_->callSetViewport(bounds);

        if (searchString.length() == 1) {
            errorString_ = searchString;
            errorCode_ = (QGeoCodeReply::Error)searchString.toInt();
        } else {
            errorString_ = "";
            errorCode_ = QGeoCodeReply::NoError;
        }

        if (errorCode_ == QGeoCodeReply::NoError)
            setLocations(geocodeReply_, searchString, limit, offset);

        if (finishRequestImmediately_) {
            // check if we should finish with error
            if (errorCode_) {
                geocodeReply_->callSetError(errorCode_, errorString_);
            } else {
                geocodeReply_->callSetFinished(true);
            }
        } else {
            // we only allow serialized requests in QML - previous must have been aborted
            Q_ASSERT(timerId_ == 0);
            timerId_ = startTimer(200);
        }
        return static_cast<QGeoCodeReply*>(geocodeReply_);
    }

    QGeoCodeReply* geocode(const QGeoAddress & address, const QGeoShape &bounds) override
    {
        geocodeReply_ = new GeocodeReplyTest();
        connect(geocodeReply_, SIGNAL(aborted()), this, SLOT(requestAborted()));
        geocodeReply_->callSetViewport(bounds);

        if (address.street().startsWith("error")) {
            errorString_ = address.street();
            errorCode_ = (QGeoCodeReply::Error)address.county().toInt();
        } else {
            errorString_ = "";
            errorCode_ = QGeoCodeReply::NoError;
        }
        // 1. Check if we are to validate values
        if (validateWellKnownValues_) {
            if (address.street() != "wellknown street") {
                 geocodeReply_->callSetError(QGeoCodeReply::EngineNotSetError, address.street());
            } else {
                geocodeReply_->callSetError(QGeoCodeReply::NoError,address.street());
            }
        }

        // 2. Set the locations into the reply
        setLocations(geocodeReply_, address);

        // 3. Finish the request
        if (finishRequestImmediately_) {
            // check if we should finish with error
            if (errorCode_) {
                geocodeReply_->callSetError(errorCode_, errorString_);
            } else {
                geocodeReply_->callSetFinished(true);
            }
        } else {
            // we only allow serialized requests in QML - previous must have been aborted
            Q_ASSERT(timerId_ == 0);
            timerId_ = startTimer(200);
        }
        return static_cast<QGeoCodeReply*>(geocodeReply_);
    }

public Q_SLOTS:
    void requestAborted()
    {
        if (timerId_) {
            killTimer(timerId_);
            timerId_ = 0;
        }
        errorString_ = "";
        errorCode_ = QGeoCodeReply::NoError;
    }

public:
    void setLocations(GeocodeReplyTest* reply, const QString searchString, int limit, int offset)
    {
        if (limit < 0)
            limit = 0;
        for (int i = 0; i < limit; ++i) {
            QGeoLocation location;
            QGeoAddress address;
            address.setStreet(searchString);
            address.setCounty(QString::number(offset));
            location.setAddress(address);
            if (includeExtendedData_)
                injectExtra(location, extendedLocationData_);
            reply->callAddLocation(location);
        }
    }

    void setLocations(GeocodeReplyTest* reply, const QGeoAddress& address)
    {
        int count = address.county().toInt();

        for (int i = 0; i < count; ++i) {
            QGeoLocation location;
            location.setAddress(address);
            if (includeExtendedData_)
                injectExtra(location, extendedLocationData_);
            reply->callAddLocation(location);
        }
    }

    void setLocations(GeocodeReplyTest* reply, const QGeoCoordinate & coordinate)
    {
        for (int i = 0; i < coordinate.longitude(); ++i) {
            QGeoLocation location;
            location.setCoordinate(coordinate);
            if (includeExtendedData_)
                injectExtra(location, extendedLocationData_);
            reply->callAddLocation(location);
        }
    }

    QGeoCodeReply* reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds) override
    {
        geocodeReply_ = new GeocodeReplyTest();
        connect(geocodeReply_, SIGNAL(aborted()), this, SLOT(requestAborted()));

        setLocations(geocodeReply_, coordinate);
        geocodeReply_->callSetViewport(bounds);

        if (coordinate.latitude() > 70) {
            errorString_ = "error";
            errorCode_ = (QGeoCodeReply::Error) (qRound(coordinate.latitude() - 70));
        } else {
            errorString_ = "";
            errorCode_ = QGeoCodeReply::NoError;
        }
        if (finishRequestImmediately_) {
            if (errorCode_) {
                geocodeReply_->callSetError(errorCode_, errorString_);
            } else {
                geocodeReply_->callSetError(QGeoCodeReply::NoError,coordinate.toString());
                geocodeReply_->callSetFinished(true);
            }
        } else {
            // we only allow serialized requests in QML - previous must have been aborted or finished
            Q_ASSERT(timerId_ == 0);
            timerId_ = startTimer(200);
        }
        return static_cast<QGeoCodeReply*>(geocodeReply_);
    }

protected:
     void timerEvent(QTimerEvent *event) override
     {
         Q_UNUSED(event);
         Q_ASSERT(timerId_ == event->timerId());
         Q_ASSERT(geocodeReply_);
         killTimer(timerId_);
         timerId_ = 0;
         if (errorCode_) {
             geocodeReply_->callSetError(errorCode_, errorString_);
             emit errorOccurred(geocodeReply_, errorCode_, errorString_);
        } else {
             geocodeReply_->callSetError(QGeoCodeReply::NoError, "no error");
             geocodeReply_->callSetFinished(true);
         }
         emit finished(geocodeReply_);
     }

     static void injectExtra(QGeoLocation &location, const QVariantMap &extra)
     {
        location.setExtendedAttributes(extra);
     }

private:
    bool includeExtendedData_ = false;
    bool validateWellKnownValues_;
    bool finishRequestImmediately_;
    bool supported_;
    GeocodeReplyTest* geocodeReply_;
    int timerId_;
    QGeoCodeReply::Error errorCode_;
    QString errorString_;
    QVariantMap extendedLocationData_;
    QVariantMap extendedReplyData_;
};

#endif
