// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGEOROUTINGMANAGERENGINE_TEST_H
#define QGEOROUTINGMANAGERENGINE_TEST_H

#include <qgeoserviceprovider.h>
#include <qgeoroutingmanagerengine.h>
#include <qgeocoordinate.h>
#include <qgeoroutereply.h>
#include <qgeorouterequest.h>

QT_USE_NAMESPACE

class QGeoRoutingManagerEngineTest: public QGeoRoutingManagerEngine

{
Q_OBJECT
public:
    QGeoRoutingManagerEngineTest(const QVariantMap &parameters,
        QGeoServiceProvider::Error *error, QString *errorString) :
        QGeoRoutingManagerEngine(parameters)
    {
        Q_UNUSED(error);
        Q_UNUSED(errorString);
        setSupportedTravelModes(QGeoRouteRequest::PedestrianTravel);
        setSupportedFeatureTypes(QGeoRouteRequest::TollFeature);
        setSupportedFeatureWeights(QGeoRouteRequest::PreferFeatureWeight);
        setSupportedRouteOptimizations(QGeoRouteRequest::FastestRoute);
        setSupportedSegmentDetails(QGeoRouteRequest::BasicSegmentData);
        setSupportedManeuverDetails(QGeoRouteRequest::BasicManeuvers);
    }

    QGeoRouteReply* calculateRoute(const QGeoRouteRequest& request) override
    {
        Q_UNUSED(request);
        return new QGeoRouteReply(QGeoRouteReply::NoError,"no error");
    }

    QGeoRouteReply* updateRoute(const QGeoRoute &route, const QGeoCoordinate &position) override
    {
        Q_UNUSED(route);
        Q_UNUSED(position);
        return new QGeoRouteReply(QGeoRouteReply::CommunicationError,"no error");

    }


};

#endif
