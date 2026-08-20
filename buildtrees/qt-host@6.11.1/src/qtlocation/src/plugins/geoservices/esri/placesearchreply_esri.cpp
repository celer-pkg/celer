// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "placesearchreply_esri.h"
#include "placemanagerengine_esri.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceAttribute>
#include <QtLocation/QPlaceContactDetail>
#include <QtLocation/QPlaceRatings>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceSearchRequest>
#include <QtLocation/private/qplacesearchrequest_p.h>

static const QString kCandidatesKey(QStringLiteral("candidates"));
static const QString kAttributesKey(QStringLiteral("attributes"));
static const QString kAddressKey(QStringLiteral("address"));
static const QString kLongLabelKey(QStringLiteral("LongLabel"));
static const QString kCityKey(QStringLiteral("City"));
static const QString kCountryKey(QStringLiteral("Country"));
static const QString kRegionKey(QStringLiteral("Region"));
static const QString kPostalKey(QStringLiteral("Postal"));
static const QString kStAddrKey(QStringLiteral("StAddr"));
static const QString kStateKey(QStringLiteral("State"));
static const QString kDistrictKey(QStringLiteral("District"));
static const QString kLocationKey(QStringLiteral("location"));
static const QString kXKey(QStringLiteral("x"));
static const QString kYKey(QStringLiteral("y"));
static const QString kDistanceKey(QStringLiteral("Distance"));
static const QString kPhoneKey(QStringLiteral("Phone"));
static const QString kExtentKey(QStringLiteral("extent"));
static const QString kXminKey(QStringLiteral("xmin"));
static const QString kYminKey(QStringLiteral("ymin"));
static const QString kXmaxKey(QStringLiteral("xmax"));
static const QString kYmaxKey(QStringLiteral("ymax"));

QT_BEGIN_NAMESPACE

PlaceSearchReplyEsri::PlaceSearchReplyEsri(const QPlaceSearchRequest &request, QNetworkReply *reply,
                                           const QHash<QString, QString> &candidateFields,
                                           const QHash<QString, QString> &countries, PlaceManagerEngineEsri *parent) :
    QPlaceSearchReply(parent), m_candidateFields(candidateFields), m_countries(countries)
{
    Q_ASSERT(parent);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    setRequest(request);

    connect(reply, &QNetworkReply::finished, this, &PlaceSearchReplyEsri::replyFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &PlaceSearchReplyEsri::networkError);
    connect(this, &QPlaceReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

PlaceSearchReplyEsri::~PlaceSearchReplyEsri()
{
}

void PlaceSearchReplyEsri::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit errorOccurred(errorCode, errorString);
    setFinished(true);
    emit finished();
}

void PlaceSearchReplyEsri::replyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    QJsonValue suggestions = document.object().value(kCandidatesKey);
    if (!suggestions.isArray()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    const QJsonArray resultsArray = suggestions.toArray();

    QList<QPlaceSearchResult> results;
    for (const auto result : resultsArray)
        results.append(parsePlaceResult(result.toObject()));

    setResults(results);
    setFinished(true);
    emit finished();
}

void PlaceSearchReplyEsri::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QPlaceReply::CommunicationError, reply->errorString());
}

QPlaceResult PlaceSearchReplyEsri::parsePlaceResult(const QJsonObject &item) const
{
    QPlace place;
    QHash<QString, QString> keys;

    // set attributes
    const QJsonObject attributes = item.value(kAttributesKey).toObject();
    for (const QString &key: attributes.keys())
    {
        const QString value = attributes.value(key).toVariant().toString();
        if (!value.isEmpty())
        {
            QPlaceAttribute attribute;
            attribute.setLabel(m_candidateFields.value(key, key)); // local name or key
            attribute.setText(value);
            place.setExtendedAttribute(key, attribute);
            keys.insert(key, value);
        }
    }

    if (keys.contains(kPhoneKey))
    {
        QPlaceContactDetail contactDetail;
        contactDetail.setLabel(m_candidateFields.value(kPhoneKey, kPhoneKey)); // local name or key
        contactDetail.setValue(keys.value(kPhoneKey));
        place.appendContactDetail(QPlaceContactDetail::Phone, contactDetail);
    }

    // set address
    QGeoAddress geoAddress;
    geoAddress.setCity(keys.value(kCityKey));
    geoAddress.setCountry(m_countries.value(keys.value(kCountryKey))); // mismatch code ISO2 vs ISO3
    geoAddress.setCounty(keys.value(kRegionKey));
    geoAddress.setPostalCode(keys.value(kPostalKey));
    geoAddress.setStreet(keys.value(kStAddrKey));
    geoAddress.setState(keys.value(kStateKey));
    geoAddress.setDistrict(keys.value(kDistrictKey));

    // set location
    const QJsonObject location = item.value(kLocationKey).toObject();
    const QGeoCoordinate coordinate = QGeoCoordinate(location.value(kYKey).toDouble(),
                                                     location.value(kXKey).toDouble());

    // set boundingBox
    const QJsonObject extent = item.value(kExtentKey).toObject();
    const QGeoCoordinate topLeft(extent.value(kYminKey).toDouble(),
                                 extent.value(kXminKey).toDouble());
    const QGeoCoordinate bottomRight(extent.value(kYmaxKey).toDouble(),
                                     extent.value(kXmaxKey).toDouble());
    const QGeoRectangle boundingBox(topLeft, bottomRight);

    // set geolocation
    QGeoLocation geoLocation;
    geoLocation.setCoordinate(coordinate);
    geoLocation.setAddress(geoAddress);
    geoLocation.setBoundingShape(boundingBox);

    // set place
    place.setName(keys.value(kLongLabelKey));
    place.setLocation(geoLocation);
    place.setPlaceId(attributes.value(kLongLabelKey).toString());

    // set place result
    QPlaceResult result;
    result.setPlace(place);
    result.setTitle(keys.value(kAddressKey));
    result.setDistance(keys.value(kDistanceKey).toDouble());

    return result;
}

QT_END_NAMESPACE
