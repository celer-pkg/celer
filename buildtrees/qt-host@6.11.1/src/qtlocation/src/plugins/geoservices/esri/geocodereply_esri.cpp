// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geocodereply_esri.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGeoCoordinate>
#include <QGeoAddress>
#include <QGeoLocation>
#include <QGeoRectangle>

QT_BEGIN_NAMESPACE

GeoCodeReplyEsri::GeoCodeReplyEsri(QNetworkReply *reply, OperationType operationType,
                                   QObject *parent) :
    QGeoCodeReply(parent), m_operationType(operationType)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished, this, &GeoCodeReplyEsri::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &GeoCodeReplyEsri::networkReplyError);
    connect(this, &QGeoCodeReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);

    setLimit(1);
    setOffset(0);
}

GeoCodeReplyEsri::~GeoCodeReplyEsri()
{
}

void GeoCodeReplyEsri::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

void GeoCodeReplyEsri::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    if (document.isObject()) {
        QJsonObject object = document.object();

        switch (operationType()) {
        case Geocode:
        {
            const QJsonArray candidates = object.value(QStringLiteral("candidates")).toArray();

            QList<QGeoLocation> locations;

            for (const auto candidate : candidates) {
                if (!candidate.isObject())
                    continue;
                locations.append(parseCandidate(candidate.toObject()));
            }

            setLocations(locations);
            setFinished(true);
        }
            break;

        case ReverseGeocode:
        {
            setLocations({parseAddress(object)});
            setFinished(true);
        }
            break;
        }

    } else {
        setError(QGeoCodeReply::CommunicationError, QStringLiteral("Unknown document"));
    }
}

QGeoLocation GeoCodeReplyEsri::parseAddress(const QJsonObject& object)
{
    QJsonObject addressObject = object.value(QStringLiteral("address")).toObject();

    QGeoAddress address;

    address.setCountryCode(addressObject.value(QStringLiteral("CountryCode")).toString());
    address.setState(addressObject.value(QStringLiteral("Region")).toString());
    address.setCity(addressObject.value(QStringLiteral("City")).toString());
    address.setDistrict(addressObject.value(QStringLiteral("Subregion")).toString());
    address.setPostalCode(addressObject.value(QStringLiteral("Postal")).toString());
    address.setStreet(addressObject.value(QStringLiteral("Address")).toString());

    QGeoCoordinate coordinate;

    QJsonObject locationObject = object.value(QStringLiteral("location")).toObject();

    coordinate.setLongitude(locationObject.value(QStringLiteral("x")).toDouble());
    coordinate.setLatitude(locationObject.value(QStringLiteral("y")).toDouble());

    QGeoLocation location;

    location.setCoordinate(coordinate);
    location.setAddress(address);

    return location;
}

QGeoLocation GeoCodeReplyEsri::parseCandidate(const QJsonObject& candidate)
{
    QGeoCoordinate coordinate;

    QJsonObject locationObject = candidate.value(QStringLiteral("location")).toObject();

    coordinate.setLongitude(locationObject.value(QStringLiteral("x")).toDouble());
    coordinate.setLatitude(locationObject.value(QStringLiteral("y")).toDouble());

    QGeoRectangle extent;

    if (candidate.contains(QStringLiteral("extent"))) {
        QJsonObject extentObject = candidate.value(QStringLiteral("extent")).toObject();

        extent.setTopLeft(QGeoCoordinate(extentObject.value(QStringLiteral("ymin")).toDouble(),
                                         extentObject.value(QStringLiteral("xmin")).toDouble()));

        extent.setBottomRight(QGeoCoordinate(extentObject.value(QStringLiteral("ymax")).toDouble(),
                                             extentObject.value(QStringLiteral("xmax")).toDouble()));
    }

    QJsonObject attributesObject = candidate.value(QStringLiteral("attributes")).toObject();

    QGeoAddress address;

    address.setText(candidate.value(QStringLiteral("address")).toString());

    address.setCountry(attributesObject.value(QStringLiteral("Country")).toString());
    address.setCountryCode(attributesObject.value(QStringLiteral("Country")).toString());
    address.setState(attributesObject.value(QStringLiteral("Region")).toString());
    address.setCity(attributesObject.value(QStringLiteral("City")).toString());
    address.setDistrict(attributesObject.value(QStringLiteral("Subregion")).toString());
    address.setPostalCode(attributesObject.value(QStringLiteral("Postal")).toString());

    QGeoLocation location;

    location.setCoordinate(coordinate);
    location.setBoundingShape(extent);
    location.setAddress(address);

    return location;
}

QT_END_NAMESPACE
