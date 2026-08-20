// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGEOROUTINGMANAGERENGINE_TEST_H
#define QGEOROUTINGMANAGERENGINE_TEST_H

#include <qgeoserviceprovider.h>
#include <qgeoroutingmanagerengine.h>
#include <QLocale>
#include <qgeoaddress.h>
#include <qgeoroutereply.h>
#include <QtLocation/private/qgeoroute_p.h>

#include <QDebug>
#include <QTimer>
#include <QTimerEvent>

QT_USE_NAMESPACE

class RouteReplyTest :public QGeoRouteReply
{
    Q_OBJECT

public:
    RouteReplyTest(QObject *parent=0) :QGeoRouteReply (QGeoRouteRequest(), parent)
    {}
    void  callSetError ( Error error, const QString & errorString ) {setError(error, errorString);}
    void  callSetFinished ( bool finished ) {setFinished(finished);}
    void  callSetRoutes(const QList<QGeoRoute> &routes) {setRoutes(routes);}
};

class QGeoRoutingManagerEngineTest: public QGeoRoutingManagerEngine
{
    Q_OBJECT
    RouteReplyTest* routeReply_;
    bool finishRequestImmediately_;
    int timerId_;
    QGeoRouteReply::Error errorCode_;
    QString errorString_;
    bool alternateGeoRouteImplementation_;

public:
    QGeoRoutingManagerEngineTest(const QVariantMap &parameters,
        QGeoServiceProvider::Error *error, QString *errorString) :
        QGeoRoutingManagerEngine(parameters),
        routeReply_(0),
        finishRequestImmediately_(true),
        timerId_(0),
        errorCode_(QGeoRouteReply::NoError),
        alternateGeoRouteImplementation_(false)
    {
        Q_UNUSED(error);
        Q_UNUSED(errorString);

        if (parameters.contains("gc_finishRequestImmediately")) {
            finishRequestImmediately_ = qvariant_cast<bool>(parameters.value("gc_finishRequestImmediately"));
        }

        if (parameters.contains("gc_alternateGeoRoute")) {
            alternateGeoRouteImplementation_ = qvariant_cast<bool>(parameters.value("gc_alternateGeoRoute"));
        }

        setLocale(QLocale (QLocale::German, QLocale::Germany));
        setSupportedFeatureTypes (
                    QGeoRouteRequest::NoFeature | QGeoRouteRequest::TollFeature |
                    QGeoRouteRequest::HighwayFeature | QGeoRouteRequest::PublicTransitFeature |
                    QGeoRouteRequest::FerryFeature | QGeoRouteRequest::TunnelFeature |
                    QGeoRouteRequest::DirtRoadFeature | QGeoRouteRequest::ParksFeature |
                    QGeoRouteRequest::MotorPoolLaneFeature );
        setSupportedFeatureWeights (
                    QGeoRouteRequest::NeutralFeatureWeight | QGeoRouteRequest::PreferFeatureWeight |
                    QGeoRouteRequest::RequireFeatureWeight | QGeoRouteRequest::AvoidFeatureWeight |
                    QGeoRouteRequest::DisallowFeatureWeight );
        setSupportedManeuverDetails (
                    QGeoRouteRequest::NoManeuvers | QGeoRouteRequest::BasicManeuvers);
        setSupportedRouteOptimizations (
                    QGeoRouteRequest::ShortestRoute | QGeoRouteRequest::FastestRoute |
                    QGeoRouteRequest::MostEconomicRoute | QGeoRouteRequest::MostScenicRoute);
        setSupportedSegmentDetails (
                    QGeoRouteRequest::NoSegmentData | QGeoRouteRequest::BasicSegmentData );
        setSupportedTravelModes (
                    QGeoRouteRequest::CarTravel | QGeoRouteRequest::PedestrianTravel |
                    QGeoRouteRequest::BicycleTravel | QGeoRouteRequest::PublicTransitTravel |
                    QGeoRouteRequest::TruckTravel );
    }

    QGeoRouteReply* calculateRoute(const QGeoRouteRequest& request) override
    {
        routeReply_ = new RouteReplyTest();
        connect(routeReply_, SIGNAL(aborted()), this, SLOT(requestAborted()));

        if (request.numberAlternativeRoutes() > 70) {
            errorCode_ = (QGeoRouteReply::Error)(request.numberAlternativeRoutes() - 70);
            errorString_ = "error";
        } else {
            errorCode_ = QGeoRouteReply::NoError;
            errorString_ = "";
        }
        setRoutes(request, routeReply_);
        if (finishRequestImmediately_) {
            if (errorCode_) {
                routeReply_->callSetError(errorCode_, errorString_);
            } else {
                routeReply_->callSetError(QGeoRouteReply::NoError, "no error");
                routeReply_->callSetFinished(true);
            }
        } else {
            // we only allow serialized requests in QML - previous must have been aborted or finished
            Q_ASSERT(timerId_ == 0);
            timerId_ = startTimer(200);
        }
        return static_cast<QGeoRouteReply*>(routeReply_);
    }

    void setRoutes(const QGeoRouteRequest& request, RouteReplyTest* reply)
    {
        QList<QGeoRoute> routes;
        int travelTime = 0;

        for (int i = 0; i < request.numberAlternativeRoutes(); ++i) {
            QGeoRoute route;
            route.setPath(request.waypoints());
            route.setTravelTime(travelTime);
            if (alternateGeoRouteImplementation_)
                route.setTravelTime(123456);

            if (request.departureTime().isValid()) {
                QVariantMap extendedAttributes = route.extendedAttributes();
                extendedAttributes["tst_departureTime"] = request.departureTime();
                route.setExtendedAttributes(extendedAttributes);
            }

            routes.append(route);
        }
        reply->callSetRoutes(routes);
    }

public Q_SLOTS:
    void requestAborted()
    {
        if (timerId_) {
            killTimer(timerId_);
            timerId_ = 0;
        }
        errorCode_ = QGeoRouteReply::NoError;
        errorString_ = "";
    }

protected:
     void timerEvent(QTimerEvent *event) override
     {
         Q_UNUSED(event);
         Q_ASSERT(timerId_ == event->timerId());
         Q_ASSERT(routeReply_);
         killTimer(timerId_);
         timerId_ = 0;
         if (errorCode_) {
             routeReply_->callSetError(errorCode_, errorString_);
             emit errorOccurred(routeReply_, errorCode_, errorString_);
         } else {
             routeReply_->callSetError(QGeoRouteReply::NoError, "no error");
             routeReply_->callSetFinished(true);
             emit finished(routeReply_);
         }
     }
};

#endif
