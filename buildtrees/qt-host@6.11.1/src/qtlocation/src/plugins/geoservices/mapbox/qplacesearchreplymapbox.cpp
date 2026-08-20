// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qplacesearchreplymapbox.h"
#include "qplacemanagerenginemapbox.h"
#include "qmapboxcommon.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceCategory>
#include <QtLocation/QPlaceIcon>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceSearchRequest>
#include <QtLocation/QPlaceContactDetail>

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace {

// https://www.mapbox.com/api-documentation/#response-object
QPlaceResult parsePlaceResult(const QJsonObject &response, const QString &attribution)
{
    QPlace place;

    place.setAttribution(attribution);
    place.setPlaceId(response.value(QStringLiteral("id")).toString());
    place.setVisibility(QLocation::PublicVisibility);

    QString placeName = response.value(QStringLiteral("text")).toString();
    if (placeName.isEmpty())
        placeName = response.value(QStringLiteral("place_name")).toString();

    place.setName(placeName);
    place.setDetailsFetched(true);

    // Unused data: type, place_type, relevance, properties.short_code,
    // properties.landmark, properties.wikidata

    // The property object is unstable and only Carmen GeoJSON properties are
    // guaranteed. This implementation should check for the presence of these
    // values in a response before it attempts to use them.
    if (response.value(QStringLiteral("properties")).isObject()) {
        const QJsonObject properties = response.value(QStringLiteral("properties")).toObject();

        const QString makiString = properties.value(QStringLiteral("maki")).toString();
        if (!makiString.isEmpty()) {
            QVariantMap iconParameters;
            iconParameters.insert(QPlaceIcon::SingleUrl,
                                  QUrl::fromLocalFile(QStringLiteral(":/mapbox/") + makiString + QStringLiteral(".svg")));

            QPlaceIcon icon;
            icon.setParameters(iconParameters);
            place.setIcon(icon);
        }

        const QString phoneString = properties.value(QStringLiteral("tel")).toString();
        if (!phoneString.isEmpty()) {
            QPlaceContactDetail phoneDetail;
            phoneDetail.setLabel(QPlaceContactDetail::Phone);
            phoneDetail.setValue(phoneString);
            place.setContactDetails(QPlaceContactDetail::Phone, QList<QPlaceContactDetail>() << phoneDetail);
        }

        const QString categoryString = properties.value(QStringLiteral("category")).toString();
        if (!categoryString.isEmpty()) {
            QList<QPlaceCategory> categories;
            for (const QString &categoryId : categoryString.split(QStringLiteral(", "), Qt::SkipEmptyParts)) {
                QPlaceCategory category;
                category.setName(QMapboxCommon::mapboxNameForCategory(categoryId));
                category.setCategoryId(categoryId);
                categories.append(category);
            }
            place.setCategories(categories);
        }
    }

    // XXX: matching_text, matching_place_name
    // XXX: text_{language}, place_name_{language}
    // XXX: language, language_{language}

    place.setLocation(QMapboxCommon::parseGeoLocation(response));

    // XXX: geometry, geometry.type, geometry.coordinates, geometry.interpolated

    QPlaceResult result;
    result.setPlace(place);
    result.setTitle(place.name());

    return result;
}

} // namespace

QPlaceSearchReplyMapbox::QPlaceSearchReplyMapbox(const QPlaceSearchRequest &request, QNetworkReply *reply, QPlaceManagerEngineMapbox *parent)
:   QPlaceSearchReply(parent)
{
    Q_ASSERT(parent);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    setRequest(request);

    connect(reply, &QNetworkReply::finished, this, &QPlaceSearchReplyMapbox::onReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &QPlaceSearchReplyMapbox::onNetworkError);

    connect(this, &QPlaceReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QPlaceSearchReplyMapbox::~QPlaceSearchReplyMapbox()
{
}

void QPlaceSearchReplyMapbox::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit errorOccurred(errorCode, errorString);

    setFinished(true);
    emit finished();
}

void QPlaceSearchReplyMapbox::onReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    const QJsonArray features = document.object().value(QStringLiteral("features")).toArray();
    const QString attribution = document.object().value(QStringLiteral("attribution")).toString();

    const QGeoCoordinate searchCenter = request().searchArea().center();
    const QList<QPlaceCategory> categories = request().categories();

    QList<QPlaceSearchResult> results;
    for (const QJsonValue feature : features) {
        QPlaceResult placeResult = parsePlaceResult(feature.toObject(), attribution);

        if (!categories.isEmpty()) {
            const QList<QPlaceCategory> placeCategories = placeResult.place().categories();
            bool categoryMatch = false;
            if (!placeCategories.isEmpty()) {
                for (const QPlaceCategory &placeCategory : placeCategories) {
                    if (categories.contains(placeCategory)) {
                        categoryMatch = true;
                        break;
                    }
                }
            }
            if (!categoryMatch)
                continue;
        }
        placeResult.setDistance(searchCenter.distanceTo(placeResult.place().location().coordinate()));
        results.append(placeResult);
    }

    if (request().relevanceHint() == QPlaceSearchRequest::DistanceHint) {
        std::sort(results.begin(), results.end(), [](const QPlaceResult &a, const QPlaceResult &b) -> bool {
                return a.distance() < b.distance();
        });
    } else if (request().relevanceHint() == QPlaceSearchRequest::LexicalPlaceNameHint) {
        std::sort(results.begin(), results.end(), [](const QPlaceResult &a, const QPlaceResult &b) -> bool {
                return a.place().name() < b.place().name();
        });
    }

    setResults(results);

    setFinished(true);
    emit finished();
}

void QPlaceSearchReplyMapbox::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
