// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeocodingmanagerenginemapbox.h"
#include "qgeocodereplymapbox.h"
#include "qmapboxcommon.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>

QT_BEGIN_NAMESPACE

namespace {
    static const QString allAddressTypes = QStringLiteral("address,district,locality,neighborhood,place,postcode,region,country");
}

QGeoCodingManagerEngineMapbox::QGeoCodingManagerEngineMapbox(const QVariantMap &parameters,
                                                       QGeoServiceProvider::Error *error,
                                                       QString *errorString)
:   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("mapbox.useragent")))
        m_userAgent = parameters.value(QStringLiteral("mapbox.useragent")).toString().toLatin1();
    else
        m_userAgent = QByteArrayLiteral("Qt Location based application");

    m_accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    m_isEnterprise = parameters.value(QStringLiteral("mapbox.enterprise")).toBool();
    m_urlPrefix = m_isEnterprise ? mapboxGeocodingEnterpriseApiPath : mapboxGeocodingApiPath;

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineMapbox::~QGeoCodingManagerEngineMapbox()
{
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    QUrlQuery queryItems;

    // If address text() is not generated: a manual setText() has been made.
    if (!address.isTextGenerated()) {
        queryItems.addQueryItem(QStringLiteral("type"), allAddressTypes);
        return doSearch(address.text().simplified(), queryItems, bounds);
    }

    QStringList addressString;
    QStringList typeString;

    if (!address.street().isEmpty()) {
        addressString.append(address.street());
        typeString.append(QStringLiteral("address"));
    }

    if (!address.district().isEmpty()) {
        addressString.append(address.district());
        typeString.append(QStringLiteral("district"));
        typeString.append(QStringLiteral("locality"));
        typeString.append(QStringLiteral("neighborhood"));
    }

    if (!address.city().isEmpty()) {
        addressString.append(address.city());
        typeString.append(QStringLiteral("place"));
    }

    if (!address.postalCode().isEmpty()) {
        addressString.append(address.postalCode());
        typeString.append(QStringLiteral("postcode"));
    }

    if (!address.state().isEmpty()) {
        addressString.append(address.state());
        typeString.append(QStringLiteral("region"));
    }

    if (!address.country().isEmpty()) {
        addressString.append(address.country());
        typeString.append(QStringLiteral("country"));
    }

    queryItems.addQueryItem(QStringLiteral("type"), typeString.join(QLatin1Char(',')));
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(1));

    return doSearch(addressString.join(QStringLiteral(", ")), queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset);

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("type"), allAddressTypes);
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return doSearch(address, queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::reverseGeocode(const QGeoCoordinate &coordinate, const QGeoShape &bounds)
{
    const QString coordinateString = QString::number(coordinate.longitude()) + QLatin1Char(',') + QString::number(coordinate.latitude());

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("limit"), QString::number(1));

    return doSearch(coordinateString, queryItems, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineMapbox::doSearch(const QString &request, QUrlQuery &queryItems, const QGeoShape &bounds)
{
    queryItems.addQueryItem(QStringLiteral("access_token"), m_accessToken);

    const QString &languageCode = QLocale::system().name().section(QLatin1Char('_'), 0, 0);
    queryItems.addQueryItem(QStringLiteral("language"), languageCode);

    QGeoRectangle boundingBox = bounds.boundingGeoRectangle();
    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bbox"),
                QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().latitude()) + QLatin1Char(',') +
                QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                QString::number(boundingBox.topLeft().latitude()));
    }

    QUrl requestUrl(m_urlPrefix + request + QStringLiteral(".json"));
    requestUrl.setQuery(queryItems);

    QNetworkRequest networkRequest(requestUrl);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);

    QNetworkReply *networkReply = m_networkManager->get(networkRequest);
    QGeoCodeReplyMapbox *reply = new QGeoCodeReplyMapbox(networkReply, this);

    connect(reply, &QGeoCodeReplyMapbox::finished,
            this, &QGeoCodingManagerEngineMapbox::onReplyFinished);
    connect(reply, &QGeoCodeReply::errorOccurred,
            this, &QGeoCodingManagerEngineMapbox::onReplyError);

    return reply;
}

void QGeoCodingManagerEngineMapbox::onReplyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoCodingManagerEngineMapbox::onReplyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}

QT_END_NAMESPACE
