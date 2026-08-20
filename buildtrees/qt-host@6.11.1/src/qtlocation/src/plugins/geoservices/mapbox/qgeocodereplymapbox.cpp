// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeocodereplymapbox.h"
#include "qmapboxcommon.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>

QT_BEGIN_NAMESPACE

QGeoCodeReplyMapbox::QGeoCodeReplyMapbox(QNetworkReply *reply, QObject *parent)
:   QGeoCodeReply(parent)
{
    Q_ASSERT(parent);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }

    connect(reply, &QNetworkReply::finished, this, &QGeoCodeReplyMapbox::onNetworkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &QGeoCodeReplyMapbox::onNetworkReplyError);

    connect(this, &QGeoCodeReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QGeoCodeReplyMapbox::~QGeoCodeReplyMapbox()
{
}

void QGeoCodeReplyMapbox::onNetworkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QList<QGeoLocation> locations;
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    const QJsonArray features = document.object().value(QStringLiteral("features")).toArray();
    for (const QJsonValueConstRef value : features)
        locations.append(QMapboxCommon::parseGeoLocation(value.toObject()));

    setLocations(locations);

    setFinished(true);
}

void QGeoCodeReplyMapbox::onNetworkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
