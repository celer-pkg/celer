// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "georoutereply_esri.h"
#include "georoutejsonparser_esri.h"

#include <QJsonDocument>

QT_BEGIN_NAMESPACE

// JSON reference:
// https://resources.arcgis.com/en/help/arcgis-rest-api/#/Route_service_with_synchronous_execution/02r300000036000000/

GeoRouteReplyEsri::GeoRouteReplyEsri(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent) :
    QGeoRouteReply(request, parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &GeoRouteReplyEsri::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &GeoRouteReplyEsri::networkReplyError);
    connect(this, &QGeoRouteReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

GeoRouteReplyEsri::~GeoRouteReplyEsri()
{
}

void GeoRouteReplyEsri::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    GeoRouteJsonParserEsri parser(document);

    if (parser.isValid())
    {
        setRoutes(parser.routes());
        setFinished(true);
    } else {
        setError(QGeoRouteReply::ParseError, parser.errorString());
    }
}

void GeoRouteReplyEsri::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoRouteReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
