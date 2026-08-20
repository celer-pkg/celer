// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeorouteparser_p.h"
#include "qgeorouteparser_p_p.h"
#include "qgeoroutesegment.h"
#include "qgeomaneuver.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#include <QtCore/private/qobject_p.h>
#include <QtPositioning/private/qlocationutils_p.h>

QT_BEGIN_NAMESPACE

/*
    Private class implementations
*/

QGeoRouteParserPrivate::QGeoRouteParserPrivate()
    : QObjectPrivate()
{
}

QGeoRouteParserPrivate::~QGeoRouteParserPrivate()
{
}

/*
    Public class implementations
*/

QGeoRouteParser::~QGeoRouteParser()
{

}

QGeoRouteParser::QGeoRouteParser(QGeoRouteParserPrivate &dd, QObject *parent) : QObject(dd, parent)
{

}

QGeoRouteReply::Error QGeoRouteParser::parseReply(QList<QGeoRoute> &routes, QString &errorString, const QByteArray &reply) const
{
    Q_D(const QGeoRouteParser);
    return d->parseReply(routes, errorString, reply);
}

QUrl QGeoRouteParser::requestUrl(const QGeoRouteRequest &request, const QString &prefix) const
{
    Q_D(const QGeoRouteParser);
    return d->requestUrl(request, prefix);
}

QGeoRouteParser::TrafficSide QGeoRouteParser::trafficSide() const
{
    Q_D(const QGeoRouteParser);
    return d->trafficSide;
}

void QGeoRouteParser::setTrafficSide(QGeoRouteParser::TrafficSide trafficSide)
{
    Q_D(QGeoRouteParser);
    if (d->trafficSide == trafficSide) return;
    d->trafficSide = trafficSide;
    Q_EMIT trafficSideChanged(trafficSide);
}

QT_END_NAMESPACE


