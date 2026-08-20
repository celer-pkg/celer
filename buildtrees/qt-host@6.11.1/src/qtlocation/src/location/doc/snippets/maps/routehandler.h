// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [RouteHandler]
class RouteHandler : public QObject
{
    Q_OBJECT
public:
    RouteHandler(QGeoRoutingManager *routingManager,
                   const QGeoCoordinate &origin,
                   const QGeoCoordinate &destination) {

        QGeoRouteRequest request(origin, destination);

        // The request defaults to the fastest route by car, which is
        // equivalent to:
        // request.setTravelMode(QGeoRouteRequest::CarTravel);
        // request.setRouteOptimization(QGeoRouteRequest::FastestRoute);

        request.setAvoidFeatureTypes(QGeoRouteRequest::AvoidTolls);
        request.setAvoidFeatureTypes(QGeoRouteRequest::AvoidMotorPoolLanes);

        QGeoRouteRequest::AvoidFeaturesTypes avoidableFeatures = routingManager->supportedAvoidFeatureTypes();

        if (!(avoidableFeatures & request.avoidFeatureTypes())) {
            // ... inform the user that the routing manager does not
            // provide support for avoiding tolls and/or motor pool lanes ...
            return;
        }

        QGeoRouteReply *reply = routingManager->calculateRoute(request);

        if (reply->isFinished()) {
            if (reply->error() == QGeoRouteReply::NoError) {
                routeCalculated(reply);
            } else {
                routeError(reply, reply->error(), reply->errorString());
            }
            return;
        }

        connect(routingManager, &QGeoRoutingManager::finished,
                this, &RouteHandler::routeCalculated);

        connect(routingManager, &QGeoRoutingManager::errorOccurred,
                this, &RouteHandler::routeError);
    }

private slots:
    void routeCalculated(QGeoRouteReply *reply)
    {
        // A route request can ask for several alternative routes ...
        if (reply->routes().size() != 0) {

            // ... but by default it will only get a single route
            QGeoRoute route = reply->routes().at(0);

            //... now we have to make use of the route ...
        }

        reply->deleteLater();
    }

    void routeError(QGeoRouteReply *reply, QGeoRouteReply:Error error, const QString &errorString)
    {
        // ... inform the user that an error has occurred ...
        reply->deleteLater();
    }
};
//! [RouteHandler]
