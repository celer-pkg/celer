// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeocodereplyosm.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/private/qgeojson_p.h>

QT_BEGIN_NAMESPACE

QGeoCodeReplyOsm::QGeoCodeReplyOsm(QNetworkReply *reply, bool includeExtraData, QObject *parent)
:   QGeoCodeReply(parent), m_includeExtraData(includeExtraData)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &QGeoCodeReplyOsm::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoCodeReplyOsm::networkReplyError);
    connect(this, &QGeoCodeReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    setLimit(1);
    setOffset(0);
}

QGeoCodeReplyOsm::~QGeoCodeReplyOsm()
{
}

static QGeoAddress parseAddressObject(const QJsonObject &object)
{
    QGeoAddress address;
    address.setText(object.value(QStringLiteral("display_name")).toString());
    QJsonObject ao = object.value(QStringLiteral("address")).toObject();
    // setCountry
    address.setCountry(ao.value(QStringLiteral("country")).toString());
    // setCountryCode
    address.setCountryCode(ao.value(QStringLiteral("country_code")).toString());
    // setState
    address.setState(ao.value(QStringLiteral("state")).toString());
    // setCity
    if (ao.contains(QLatin1String("city")))
        address.setCity(ao.value(QStringLiteral("city")).toString());
    else if (ao.contains(QLatin1String("town")))
        address.setCity(ao.value(QLatin1String("town")).toString());
    else if (ao.contains(QLatin1String("village")))
        address.setCity(ao.value(QLatin1String("village")).toString());
    else
        address.setCity(ao.value(QLatin1String("hamlet")).toString());
    // setDistrict
    address.setDistrict(ao.value(QStringLiteral("suburb")).toString());
    // setPostalCode
    address.setPostalCode(ao.value(QStringLiteral("postcode")).toString());
    // setStreet
    address.setStreet(ao.value(QStringLiteral("road")).toString());
    address.setStreetNumber(ao.value(QStringLiteral("house_number")).toString());
    return address;
}

static void injectExtra(QGeoLocation &location, const QJsonObject &object)
{
    QVariantMap extra;
    static const QList<QString> extraKeys = {  QStringLiteral("geojson"),
                                               QStringLiteral("icon"),
                                               QStringLiteral("importance"),
                                               QStringLiteral("type"),
                                               QStringLiteral("osm_id"),
                                               QStringLiteral("osm_type"),
                                               QStringLiteral("licence"),
                                               QStringLiteral("place_id"),
                                               QStringLiteral("class") };

    for (const auto &k: extraKeys) {
        if (object.contains(k)) {
            extra[k] = object.value(k).toVariant();
            if (k == QStringLiteral("geojson"))
                extra[QStringLiteral("geojson_model")] =
                        QGeoJson::importGeoJson(QJsonDocument::fromVariant(extra[k]));
        }
    }

    location.setExtendedAttributes(extra);
}

void QGeoCodeReplyOsm::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QList<QGeoLocation> locations;
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    if (document.isObject()) {
        QJsonObject object = document.object();

        QGeoCoordinate coordinate;

        coordinate.setLatitude(object.value(QStringLiteral("lat")).toString().toDouble());
        coordinate.setLongitude(object.value(QStringLiteral("lon")).toString().toDouble());

        QGeoLocation location;
        location.setCoordinate(coordinate);
        location.setAddress(parseAddressObject(object));

        if (m_includeExtraData)
            injectExtra(location, object);
        locations.append(location);

        setLocations(locations);
    } else if (document.isArray()) {
        QJsonArray results = document.array();

        for (int i = 0; i < results.count(); ++i) {
            if (!results.at(i).isObject())
                continue;

            QJsonObject object = results.at(i).toObject();

            QGeoCoordinate coordinate;

            coordinate.setLatitude(object.value(QStringLiteral("lat")).toString().toDouble());
            coordinate.setLongitude(object.value(QStringLiteral("lon")).toString().toDouble());

            QGeoRectangle rectangle;

            if (object.contains(QStringLiteral("boundingbox"))) {
                QJsonArray a = object.value(QStringLiteral("boundingbox")).toArray();
                if (a.count() == 4) {
                    rectangle.setTopLeft(QGeoCoordinate(a.at(1).toString().toDouble(),
                                                        a.at(2).toString().toDouble()));
                    rectangle.setBottomRight(QGeoCoordinate(a.at(0).toString().toDouble(),
                                                            a.at(3).toString().toDouble()));
                }
            }

            QGeoLocation location;
            location.setCoordinate(coordinate);
            location.setBoundingShape(rectangle);
            location.setAddress(parseAddressObject(object));
            if (m_includeExtraData)
                injectExtra(location, object);
            locations.append(location);
        }

    }

    setLocations(locations);
    setFinished(true);
}

void QGeoCodeReplyOsm::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
