// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoroutereplyosm.h"
#include "qgeoroutingmanagerengineosm.h"

QT_BEGIN_NAMESPACE

QGeoRouteReplyOsm::QGeoRouteReplyOsm(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
:   QGeoRouteReply(request, parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &QGeoRouteReplyOsm::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoRouteReplyOsm::networkReplyError);
    connect(this, &QGeoRouteReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QGeoRouteReplyOsm::~QGeoRouteReplyOsm()
{
}

void QGeoRouteReplyOsm::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QGeoRoutingManagerEngineOsm *engine = qobject_cast<QGeoRoutingManagerEngineOsm *>(parent());
    const QGeoRouteParser *parser = engine->routeParser();

    QList<QGeoRoute> routes;
    QString errorString;
    QGeoRouteReply::Error error = parser->parseReply(routes, errorString, reply->readAll());
    // Setting the request into the result
    for (QGeoRoute &route : routes) {
        route.setRequest(request());
        for (QGeoRoute &leg: route.routeLegs()) {
            leg.setRequest(request());
        }
    }

    if (error == QGeoRouteReply::NoError) {
        setRoutes(routes.mid(0, request().numberAlternativeRoutes() + 1));
        // setError(QGeoRouteReply::NoError, status);  // can't do this, or NoError is emitted and does damages
        setFinished(true);
    } else {
        setError(error, errorString);
    }
}

void QGeoRouteReplyOsm::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoRouteReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
