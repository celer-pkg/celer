// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qplacesearchreplyhere.h"
#include "jsonparserhelpers.h"
#include "../qplacemanagerengine_nokiav2.h"
#include "../qgeoerror_messages.h"

#include <QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceAttribute>
#include <QtLocation/QPlaceIcon>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceRatings>
#include <QtLocation/QPlaceProposedSearchResult>
#include <QtLocation/private/qplacesearchrequest_p.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

QPlaceSearchReplyHere::QPlaceSearchReplyHere(const QPlaceSearchRequest &request,
                                             QNetworkReply *reply,
                                             QPlaceManagerEngineNokiaV2 *parent)
    :   QPlaceSearchReply(parent), m_engine(parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    setRequest(request);

    connect(reply, &QNetworkReply::finished,
            this, &QPlaceSearchReplyHere::replyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QPlaceSearchReplyHere::replyError);
    connect(this, &QPlaceReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QPlaceSearchReplyHere::~QPlaceSearchReplyHere()
{
}

void QPlaceSearchReplyHere::setError(QPlaceReply::Error error_, const QString &errorString)
{
    QPlaceReply::setError(error_, errorString);
    emit errorOccurred(error_, errorString);
    setFinished(true);
    emit finished();
}

void QPlaceSearchReplyHere::replyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, PARSE_ERROR));
        return;
    }

    QJsonObject resultsObject = document.object();

    if (resultsObject.contains(QStringLiteral("results")))
        resultsObject = resultsObject.value(QStringLiteral("results")).toObject();

    QJsonArray items = resultsObject.value(QStringLiteral("items")).toArray();

    QList<QPlaceSearchResult> results;
    for (int i = 0; i < items.count(); ++i) {
        QJsonObject item = items.at(i).toObject();

        const QString type = item.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("urn:nlp-types:place"))
            results.append(parsePlaceResult(item));
        else if (type == QStringLiteral("urn:nlp-types:search"))
            results.append(parseSearchResult(item));
    }

    QPlaceSearchRequest r_orig = request();
    QPlaceSearchRequestPrivate *rpimpl_orig = QPlaceSearchRequestPrivate::get(r_orig);

    if (resultsObject.contains(QStringLiteral("next"))) {
        QPlaceSearchRequest request;
        request.setSearchContext(QUrl(resultsObject.value(QStringLiteral("next")).toString()));
        QPlaceSearchRequestPrivate *rpimpl = QPlaceSearchRequestPrivate::get(request);
        rpimpl->related = true;
        rpimpl->page = rpimpl_orig->page + 1;
        setNextPageRequest(request);
    }

    if (resultsObject.contains(QStringLiteral("previous"))) {
        QPlaceSearchRequest request;
        request.setSearchContext(QUrl(resultsObject.value(QStringLiteral("previous")).toString()));
        QPlaceSearchRequestPrivate *rpimpl = QPlaceSearchRequestPrivate::get(request);
        rpimpl->related = true;
        rpimpl->page = rpimpl_orig->page - 1;
        setPreviousPageRequest(request);
    }

    setResults(results);

    setFinished(true);
    emit finished();
}

QPlaceResult QPlaceSearchReplyHere::parsePlaceResult(const QJsonObject &item) const
{
    QPlaceResult result;

    if (item.contains(QStringLiteral("distance")))
        result.setDistance(item.value(QStringLiteral("distance")).toDouble());

    QPlace place;

    QGeoLocation location;

    location.setCoordinate(parseCoordinate(item.value(QStringLiteral("position")).toArray()));

    const QString vicinity = item.value(QStringLiteral("vicinity")).toString();
    QGeoAddress address;
    address.setText(vicinity);
    location.setAddress(address);

    if (item.contains(QStringLiteral("bbox"))) {
        QJsonArray bbox = item.value(QStringLiteral("bbox")).toArray();
        QGeoRectangle box(QGeoCoordinate(bbox.at(3).toDouble(), bbox.at(0).toDouble()),
                            QGeoCoordinate(bbox.at(1).toDouble(), bbox.at(2).toDouble()));
        location.setBoundingShape(box);
    }

    place.setLocation(location);

    QPlaceRatings ratings;
    ratings.setAverage(item.value(QStringLiteral("averageRating")).toDouble());
    ratings.setMaximum(5.0);
    place.setRatings(ratings);

    const QString title = item.value(QStringLiteral("title")).toString();
    place.setName(title);
    result.setTitle(title);

    QPlaceIcon icon = m_engine->icon(item.value(QStringLiteral("icon")).toString());
    place.setIcon(icon);
    result.setIcon(icon);

    place.setCategory(parseCategory(item.value(QStringLiteral("category")).toObject(),
                                    m_engine));

    //QJsonArray having = item.value(QStringLiteral("having")).toArray();

    result.setSponsored(item.value(QStringLiteral("sponsored")).toBool());

    QUrl href = item.value(QStringLiteral("href")).toString();
    //QUrl type = item.value(QStringLiteral("type")).toString();

    place.setPlaceId(href.path().mid(18, 41));

    QPlaceAttribute provider;
    provider.setText(QStringLiteral("here"));
    place.setExtendedAttribute(QPlaceAttribute::Provider, provider);
    place.setVisibility(QLocation::PublicVisibility);

    result.setPlace(place);

    return result;
}

QPlaceProposedSearchResult QPlaceSearchReplyHere::parseSearchResult(const QJsonObject &item) const
{
    QPlaceProposedSearchResult result;

    result.setTitle(item.value(QStringLiteral("title")).toString());

    QPlaceIcon icon = m_engine->icon(item.value(QStringLiteral("icon")).toString());
    result.setIcon(icon);

    QPlaceSearchRequest request;
    request.setSearchContext(QUrl(item.value("href").toString()));

    result.setSearchRequest(request);

    return result;
}

void QPlaceSearchReplyHere::replyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (error == QNetworkReply::OperationCanceledError) {
        setError(QPlaceReply::CancelError, QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, CANCEL_ERROR));
    } else if (error == QNetworkReply::ContentNotFoundError) {
        setError(QPlaceReply::PlaceDoesNotExistError,
                 QString::fromLatin1("The id, %1, does not reference an existing place")
                 .arg(request().recommendationId()));
    } else {
        setError(QPlaceReply::CommunicationError, QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, NETWORK_ERROR));
    }
}

QT_END_NAMESPACE
