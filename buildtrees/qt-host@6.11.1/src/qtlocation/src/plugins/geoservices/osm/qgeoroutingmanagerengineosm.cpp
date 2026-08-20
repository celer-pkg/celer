// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoroutingmanagerengineosm.h"
#include "qgeoroutereplyosm.h"
#include "qgeorouteparserosrmv4_p.h"
#include "QtLocation/private/qgeorouteparserosrmv5_p.h"

#include <QtCore/QUrlQuery>

#include <QtCore/QDebug>

QGeoRoutingManagerEngineOsm::QGeoRoutingManagerEngineOsm(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString)
:   QGeoRoutingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("osm.useragent")))
        m_userAgent = parameters.value(QStringLiteral("osm.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("osm.routing.host")))
        m_urlPrefix = parameters.value(QStringLiteral("osm.routing.host")).toString().toLatin1();
    else
        m_urlPrefix = QStringLiteral("http://router.project-osrm.org/route/v1/driving/");
        // for v4 it was "http://router.project-osrm.org/viaroute"

    if (parameters.contains(QStringLiteral("osm.routing.apiversion"))
            && (parameters.value(QStringLiteral("osm.routing.apiversion")).toString().toLatin1() == QByteArray("v4")))
        m_routeParser = new QGeoRouteParserOsrmV4(this);
    else
        m_routeParser = new QGeoRouteParserOsrmV5(this);
    if (parameters.contains(QStringLiteral("osm.routing.traffic_side"))) {
        QString trafficSide = parameters.value(QStringLiteral("mapbox.routing.traffic_side")).toString();
        if (trafficSide == QStringLiteral("right"))
            m_routeParser->setTrafficSide(QGeoRouteParser::RightHandTraffic);
        else if (trafficSide == QStringLiteral("left"))
            m_routeParser->setTrafficSide(QGeoRouteParser::LeftHandTraffic);
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoRoutingManagerEngineOsm::~QGeoRoutingManagerEngineOsm()
{
}

QGeoRouteReply* QGeoRoutingManagerEngineOsm::calculateRoute(const QGeoRouteRequest &request)
{
    QNetworkRequest networkRequest;
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    networkRequest.setUrl(routeParser()->requestUrl(request, m_urlPrefix));

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoRouteReplyOsm *routeReply = new QGeoRouteReplyOsm(reply, request, this);

    connect(routeReply, &QGeoRouteReplyOsm::finished,
            this, &QGeoRoutingManagerEngineOsm::replyFinished);
    connect(routeReply, &QGeoRouteReplyOsm::errorOccurred,
            this, &QGeoRoutingManagerEngineOsm::replyError);

    return routeReply;
}

const QGeoRouteParser *QGeoRoutingManagerEngineOsm::routeParser() const
{
    return m_routeParser;
}

void QGeoRoutingManagerEngineOsm::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoRoutingManagerEngineOsm::replyError(QGeoRouteReply::Error errorCode,
                                             const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}
