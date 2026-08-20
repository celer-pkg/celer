// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmapboxcommon.h"

#include <QtCore/QJsonArray>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoRectangle>

QString QMapboxCommon::mapboxNameForCategory(const QString &category)
{
    if (category.isEmpty())
        return category;

    QString categoryName = category;
    categoryName[0] = categoryName[0].toUpper();
    return categoryName;
}

// https://www.mapbox.com/api-documentation/#response-object
QGeoLocation QMapboxCommon::parseGeoLocation(const QJsonObject &response)
{
    QGeoLocation location;

    QGeoAddress address;

    QString streetAddress = response.value(QStringLiteral("address")).toString();

    // If place type is 'address', the street address is a combo of 'text' and
    // 'address'. The former provides the street name, and the latter provides
    // the street number in that case.
    if (response.value(QStringLiteral("place_type")).isArray()) {
        for (const QJsonValueConstRef value : response.value(QStringLiteral("place_type")).toArray()) {
            if (!value.isString())
                continue;
            if (value.toString() == QStringLiteral("address")) {
                streetAddress.prepend(response.value(QStringLiteral("text")).toString() + QLatin1Char(' '));
                break;
            }
        }
    }

    if (response.value(QStringLiteral("properties")).isObject()) {
        const QJsonObject properties = response.value(QStringLiteral("properties")).toObject();

        // Prefer properties.address over address.
        const QString addressString = properties.value(QStringLiteral("address")).toString();
        if (!addressString.isEmpty())
            streetAddress = addressString;
    }

    address.setStreet(streetAddress);

    if (response.value(QStringLiteral("context")).isArray()) {
        for (const QJsonValueConstRef value : response.value(QStringLiteral("context")).toArray()) {
            if (!value.isObject())
                continue;

            const QJsonObject object = value.toObject();
            const QString valueId = object.value(QStringLiteral("id")).toString();
            const QString valueText = object.value(QStringLiteral("text")).toString();

            if (valueId.isEmpty() || valueText.isEmpty())
                continue;

            // XXX: locality, neighborhood, address, poi, poi.landmark
            if (valueId.startsWith(QStringLiteral("country"))) {
                address.setCountry(valueText);
                const QString countryCode = object.value(QStringLiteral("short_code")).toString();
                if (!countryCode.isEmpty())
                    address.setCountryCode(countryCode);
            } else if (valueId.startsWith(QStringLiteral("region"))) {
                address.setState(valueText);
            } else if (valueId.startsWith(QStringLiteral("postcode"))) {
                address.setPostalCode(valueText);
            } else if (valueId.startsWith(QStringLiteral("district"))) {
                address.setDistrict(valueText);
            } else if (valueId.startsWith(QStringLiteral("place"))) {
                address.setCity(valueText);
            }
        }
    } else {
        // Fallback to using information from place_name.
        const QString placeName = response.value(QStringLiteral("place_name")).toString();

        // Remove actual place name.
        address.setText(placeName.mid(placeName.indexOf(QLatin1Char(',')) + 1));
    }

    location.setAddress(address);

    QJsonArray bbox = response.value(QStringLiteral("bbox")).toArray();
    double top = bbox.at(3).toDouble();
    double left = bbox.at(0).toDouble();
    double bottom = bbox.at(1).toDouble();
    double right = bbox.at(2).toDouble();
    location.setBoundingShape(QGeoRectangle(QGeoCoordinate(top, left), QGeoCoordinate(bottom, right)));

    QJsonArray center = response.value(QStringLiteral("center")).toArray();
    location.setCoordinate(QGeoCoordinate(center.at(1).toDouble(), center.at(0).toDouble()));

    return location;
}
