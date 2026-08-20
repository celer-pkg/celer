// Copyright (C) 2016 Vlad Seryakov <vseryakov@gmail.com>
// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoroutereplymapbox.h"
#include "qgeoroutingmanagerenginemapbox.h"
#include <QtLocation/private/qgeorouteparser_p.h>
#include <QtLocation/private/qgeoroute_p.h>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtLocation/QGeoRouteSegment>
#include <QtLocation/QGeoManeuver>

QT_BEGIN_NAMESPACE

QGeoRouteReplyMapbox::QGeoRouteReplyMapbox(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
:   QGeoRouteReply(request, parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &QGeoRouteReplyMapbox::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoRouteReplyMapbox::networkReplyError);
    connect(this, &QGeoRouteReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QGeoRouteReplyMapbox::~QGeoRouteReplyMapbox()
{
}

void QGeoRouteReplyMapbox::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QGeoRoutingManagerEngineMapbox *engine = qobject_cast<QGeoRoutingManagerEngineMapbox *>(parent());
    const QGeoRouteParser *parser = engine->routeParser();

    QList<QGeoRoute> routes;
    QString errorString;

    QByteArray routeReply = reply->readAll();
    QGeoRouteReply::Error error = parser->parseReply(routes, errorString, routeReply);
    // Setting the request into the result
    for (QGeoRoute &route : routes) {
        route.setRequest(request());
        for (QGeoRoute &leg: route.routeLegs()) {
            leg.setRequest(request());
        }
    }

    QVariantMap metadata;
    metadata["osrm.reply-json"] = routeReply;

    QVariantMap extAttr;
    extAttr["engine"] = "mapbox";
    extAttr["metadata"] = metadata;

    QList<QGeoRoute> mapboxRoutes;
    for (const QGeoRoute &route : routes.mid(0, request().numberAlternativeRoutes() + 1)) {
        QGeoRoute mapboxRoute(route);
        mapboxRoute.setExtendedAttributes(extAttr);
        mapboxRoutes.append(mapboxRoute);
    }

    if (error == QGeoRouteReply::NoError) {
        setRoutes(mapboxRoutes);
        // setError(QGeoRouteReply::NoError, status);  // can't do this, or NoError is emitted and does damages
        setFinished(true);
    } else {
        setError(error, errorString);
    }
}

void QGeoRouteReplyMapbox::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoRouteReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
