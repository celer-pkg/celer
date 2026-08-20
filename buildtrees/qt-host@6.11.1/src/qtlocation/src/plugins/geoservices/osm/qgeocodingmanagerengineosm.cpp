// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeocodingmanagerengineosm.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoRectangle>
#include "qgeocodereplyosm.h"


QT_BEGIN_NAMESPACE

static QString addressToQuery(const QGeoAddress &address)
{
    return address.street() + QStringLiteral(", ") +
           address.district() + QStringLiteral(", ") +
           address.city() + QStringLiteral(", ") +
           address.state() + QStringLiteral(", ") +
           address.country();
}

static QString boundingBoxToLtrb(const QGeoRectangle &rect)
{
    return QString::number(rect.topLeft().longitude()) + QLatin1Char(',') +
           QString::number(rect.topLeft().latitude()) + QLatin1Char(',') +
           QString::number(rect.bottomRight().longitude()) + QLatin1Char(',') +
           QString::number(rect.bottomRight().latitude());
}

QGeoCodingManagerEngineOsm::QGeoCodingManagerEngineOsm(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("osm.useragent")))
        m_userAgent = parameters.value(QStringLiteral("osm.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("osm.geocoding.host")))
        m_urlPrefix = parameters.value(QStringLiteral("osm.geocoding.host")).toString().toLatin1();
    else
        m_urlPrefix = QStringLiteral("https://nominatim.openstreetmap.org");

    if (parameters.contains(QStringLiteral("osm.geocoding.debug_query")))
        m_debugQuery = parameters.value(QStringLiteral("osm.geocoding.debug_query")).toBool();
    if (parameters.contains(QStringLiteral("osm.geocoding.include_extended_data")))
        m_includeExtraData = parameters.value(QStringLiteral("osm.geocoding.include_extended_data")).toBool();

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineOsm::~QGeoCodingManagerEngineOsm()
{
}

QGeoCodeReply *QGeoCodingManagerEngineOsm::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(addressToQuery(address), -1, -1, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineOsm::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset);

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(QString("%1/search").arg(m_urlPrefix));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), address);
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("accept-language"), locale().name().left(2));
    //query.addQueryItem(QStringLiteral("countrycodes"), QStringLiteral("au,jp"));
    if (bounds.type() != QGeoShape::UnknownType) {
        query.addQueryItem(QStringLiteral("viewbox"), boundingBoxToLtrb(bounds.boundingGeoRectangle()));
        query.addQueryItem(QStringLiteral("bounded"), QStringLiteral("1"));
    }
    query.addQueryItem(QStringLiteral("polygon_geojson"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("addressdetails"), QStringLiteral("1"));
    if (limit != -1)
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyOsm *geocodeReply = new QGeoCodeReplyOsm(reply, m_includeExtraData, this);

    connect(geocodeReply, &QGeoCodeReplyOsm::finished,
            this, &QGeoCodingManagerEngineOsm::replyFinished);
    connect(geocodeReply, &QGeoCodeReplyOsm::errorOccurred,
            this, &QGeoCodingManagerEngineOsm::replyError);

    return geocodeReply;
}

QGeoCodeReply *QGeoCodingManagerEngineOsm::reverseGeocode(const QGeoCoordinate &coordinate,
                                                          const QGeoShape &bounds)
{
    Q_UNUSED(bounds);

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(QString("%1/reverse").arg(m_urlPrefix));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("accept-language"), locale().name().left(2));
    query.addQueryItem(QStringLiteral("lat"), QString::number(coordinate.latitude()));
    query.addQueryItem(QStringLiteral("lon"), QString::number(coordinate.longitude()));
    query.addQueryItem(QStringLiteral("zoom"), QStringLiteral("18"));
    query.addQueryItem(QStringLiteral("addressdetails"), QStringLiteral("1"));

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyOsm *geocodeReply = new QGeoCodeReplyOsm(reply, m_includeExtraData, this);

    connect(geocodeReply, &QGeoCodeReplyOsm::finished,
            this, &QGeoCodingManagerEngineOsm::replyFinished);
    connect(geocodeReply, &QGeoCodeReplyOsm::errorOccurred,
            this, &QGeoCodingManagerEngineOsm::replyError);

    return geocodeReply;
}

void QGeoCodingManagerEngineOsm::replyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoCodingManagerEngineOsm::replyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}

QT_END_NAMESPACE
