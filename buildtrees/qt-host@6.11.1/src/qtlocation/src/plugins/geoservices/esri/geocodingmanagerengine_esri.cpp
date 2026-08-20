// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geocodingmanagerengine_esri.h"
#include "geocodereply_esri.h"

#include <QVariantMap>
#include <QUrl>
#include <QUrlQuery>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QGeoCoordinate>
#include <QGeoAddress>
#include <QGeoShape>
#include <QGeoRectangle>

QT_BEGIN_NAMESPACE

// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-find-address-candidates.htm
// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-reverse-geocode.htm

static const QString kPrefixEsri(QStringLiteral("esri."));
static const QString kParamUserAgent(kPrefixEsri + QStringLiteral("useragent"));

static const QString kUrlGeocode(QStringLiteral("https://geocode.arcgis.com/arcgis/rest/services/"
                                                "World/GeocodeServer/findAddressCandidates"));
static const QString kUrlReverseGeocode(QStringLiteral(
        "https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/reverseGeocode"));

static QString addressToQuery(const QGeoAddress &address)
{
    return address.street() + QStringLiteral(", ")
           + address.district() + QStringLiteral(", ")
           + address.city() + QStringLiteral(", ")
           + address.state() + QStringLiteral(", ")
           + address.country();
}

static QString boundingBoxToLtrb(const QGeoRectangle &rect)
{
    return QString::number(rect.topLeft().longitude()) + QLatin1Char(',')
           + QString::number(rect.topLeft().latitude()) + QLatin1Char(',')
           + QString::number(rect.bottomRight().longitude()) + QLatin1Char(',')
           + QString::number(rect.bottomRight().latitude());
}

GeoCodingManagerEngineEsri::GeoCodingManagerEngineEsri(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(kParamUserAgent))
        m_userAgent = parameters.value(kParamUserAgent).toString().toLatin1();
    else
        m_userAgent = QByteArrayLiteral("Qt Location based application");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

GeoCodingManagerEngineEsri::~GeoCodingManagerEngineEsri()
{
}

QGeoCodeReply *GeoCodingManagerEngineEsri::geocode(const QGeoAddress &address,
                                                   const QGeoShape &bounds)
{
    return geocode(addressToQuery(address), 1, -1, bounds);
}

QGeoCodeReply *GeoCodingManagerEngineEsri::geocode(const QString &address, int limit, int offset,
                                                   const QGeoShape &bounds)
{
    Q_UNUSED(offset);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QUrl url(kUrlGeocode);

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("singleLine"), address);
    query.addQueryItem(QStringLiteral("f"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("outFields"), "*");

    if (bounds.type() != QGeoShape::UnknownType)
        query.addQueryItem(QStringLiteral("searchExtent"), boundingBoxToLtrb(bounds.boundingGeoRectangle()));

    if (limit != -1)
        query.addQueryItem(QStringLiteral("maxLocations"), QString::number(limit));

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);
    GeoCodeReplyEsri *geocodeReply = new GeoCodeReplyEsri(reply, GeoCodeReplyEsri::Geocode, this);

    connect(geocodeReply, &GeoCodeReplyEsri::finished,
            this, &GeoCodingManagerEngineEsri::replyFinished);
    connect(geocodeReply, &GeoCodeReplyEsri::errorOccurred,
            this, &GeoCodingManagerEngineEsri::replyError);

    return geocodeReply;
}

QGeoCodeReply *GeoCodingManagerEngineEsri::reverseGeocode(const QGeoCoordinate &coordinate,
                                                          const QGeoShape &bounds)
{
    Q_UNUSED(bounds);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QUrl url(kUrlReverseGeocode);

    QUrlQuery query;

    query.addQueryItem(QStringLiteral("f"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("langCode"), locale().name().left(2));
    query.addQueryItem(QStringLiteral("location"), QString::number(coordinate.longitude()) + QLatin1Char(',')
                       + QString::number(coordinate.latitude()));

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);
    GeoCodeReplyEsri *geocodeReply = new GeoCodeReplyEsri(reply, GeoCodeReplyEsri::ReverseGeocode,
                                                          this);

    connect(geocodeReply, &GeoCodeReplyEsri::finished,
            this, &GeoCodingManagerEngineEsri::replyFinished);
    connect(geocodeReply, &GeoCodeReplyEsri::errorOccurred,
            this, &GeoCodingManagerEngineEsri::replyError);

    return geocodeReply;
}

void GeoCodingManagerEngineEsri::replyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void GeoCodingManagerEngineEsri::replyError(QGeoCodeReply::Error errorCode,
                                            const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}

QT_END_NAMESPACE
