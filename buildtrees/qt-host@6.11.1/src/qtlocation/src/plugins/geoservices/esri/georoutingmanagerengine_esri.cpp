// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "georoutingmanagerengine_esri.h"
#include "georoutereply_esri.h"

#include <QUrlQuery>

QT_BEGIN_NAMESPACE

static const QString kPrefixEsri(QStringLiteral("esri."));
static const QString kParamUserAgent(kPrefixEsri + QStringLiteral("useragent"));
static const QString kParamToken(kPrefixEsri + QStringLiteral("token"));

static const QString kUrlRouting(QStringLiteral(
        "https://route.arcgis.com/arcgis/rest/services/World/Route/NAServer/Route_World/solve"));

GeoRoutingManagerEngineEsri::GeoRoutingManagerEngineEsri(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString) :
    QGeoRoutingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(kParamUserAgent))
        m_userAgent = parameters.value(kParamUserAgent).toString().toLatin1();
    else
        m_userAgent = QByteArrayLiteral("Qt Location based application");

    m_token = parameters.value(kParamToken).toString();

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

GeoRoutingManagerEngineEsri::~GeoRoutingManagerEngineEsri()
{
}

// REST reference:
// https://resources.arcgis.com/en/help/arcgis-rest-api/index.html#//02r300000036000000

QGeoRouteReply *GeoRoutingManagerEngineEsri::calculateRoute(const QGeoRouteRequest &request)
{
    QNetworkRequest networkRequest;
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QUrl url(kUrlRouting);
    QUrlQuery query;
    QString stops;

    for (const QGeoCoordinate &coordinate : request.waypoints()) {
        if (!stops.isEmpty())
            stops += "; ";

        stops += QString::number(coordinate.longitude()) + QLatin1Char(',') +
                 QString::number(coordinate.latitude());
    }

    query.addQueryItem(QStringLiteral("stops"), stops);
    query.addQueryItem(QStringLiteral("f"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("directionsLanguage"), preferedDirectionLangage());
    query.addQueryItem(QStringLiteral("directionsLengthUnits"), preferedDirectionsLengthUnits());
    query.addQueryItem(QStringLiteral("token"), m_token);

    url.setQuery(query);
    networkRequest.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(networkRequest);
    GeoRouteReplyEsri *routeReply = new GeoRouteReplyEsri(reply, request, this);

    connect(routeReply, &GeoRouteReplyEsri::finished,
            this, &GeoRoutingManagerEngineEsri::replyFinished);
    connect(routeReply, &GeoRouteReplyEsri::errorOccurred,
            this, &GeoRoutingManagerEngineEsri::replyError);

    return routeReply;
}

void GeoRoutingManagerEngineEsri::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit finished(reply);
}

void GeoRoutingManagerEngineEsri::replyError(QGeoRouteReply::Error errorCode, const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}

QString GeoRoutingManagerEngineEsri::preferedDirectionLangage()
{
    // list of supported langages is defined in:
    // https://resources.arcgis.com/en/help/arcgis-rest-api/index.html#//02r300000036000000
    const QStringList supportedLanguages = {
        "ar", // Generate directions in Arabic
        "cs", // Generate directions in Czech
        "de", // Generate directions in German
        "el", // Generate directions in Greek
        "en", // Generate directions in English (default)
        "es", // Generate directions in Spanish
        "et", // Generate directions in Estonian
        "fr", // Generate directions in French
        "he", // Generate directions in Hebrew
        "it", // Generate directions in Italian
        "ja", // Generate directions in Japanese
        "ko", // Generate directions in Korean
        "lt", // Generate directions in Lithuanian
        "lv", // Generate directions in Latvian
        "nl", // Generate directions in Dutch
        "pl", // Generate directions in Polish
        "pt-BR", // Generate directions in Brazilian Portuguese
        "pt-PT", // Generate directions in Portuguese (Portugal)
        "ru", // Generate directions in Russian
        "sv", // Generate directions in Swedish
        "tr", // Generate directions in Turkish
        "zh-CN" // Simplified Chinese
    };

    for (const QString &language: locale().uiLanguages())
    {
        if (language.startsWith("pt_BR")) // Portuguese (Brazilian)
            return QStringLiteral("pt-BR");
        if (language.startsWith("pt"))    // Portuguese (Portugal)
            return QStringLiteral("pt-PT");
        if (language.startsWith("zh"))    // Portuguese (Portugal)
            return QStringLiteral("zh-CN");

        const QString country = language.left(2);
        if (supportedLanguages.contains(country))
            return country;
    }
    return QStringLiteral("en");  // default value
}

QString GeoRoutingManagerEngineEsri::preferedDirectionsLengthUnits()
{
    switch (measurementSystem())
    {
    case QLocale::MetricSystem:
        return QStringLiteral("esriNAUMeters");
        break;
    case QLocale::ImperialUSSystem:
        return QStringLiteral( "esriNAUFeet");
        break;
    case QLocale::ImperialUKSystem:
        return QStringLiteral("esriNAUFeet");
        break;
    default:
        return QStringLiteral("esriNAUMeters");
        break;
    }
    return QStringLiteral("esriNAUMeters");
}

QT_END_NAMESPACE
