// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qplacemanagerengineosm.h"
#include "qplacesearchreplyosm.h"
#include "qplacecategoriesreplyosm.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QLocale>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrlQuery>
#include <QtCore/QXmlStreamReader>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtPositioning/QGeoCircle>

#include <QtLocation/QPlaceCategory>
#include <QtLocation/QPlaceSearchRequest>
#include <QtLocation/private/unsupportedreplies_p.h>

namespace
{
QString SpecialPhrasesBaseUrl = QStringLiteral("http://wiki.openstreetmap.org/wiki/Special:Export/Nominatim/Special_Phrases/");

QString nameForTagKey(const QString &tagKey)
{
    if (tagKey == QLatin1String("aeroway"))
        return QPlaceManagerEngineOsm::tr("Aeroway");
    else if (tagKey == QLatin1String("amenity"))
        return QPlaceManagerEngineOsm::tr("Amenity");
    else if (tagKey == QLatin1String("building"))
        return QPlaceManagerEngineOsm::tr("Building");
    else if (tagKey == QLatin1String("highway"))
        return QPlaceManagerEngineOsm::tr("Highway");
    else if (tagKey == QLatin1String("historic"))
        return QPlaceManagerEngineOsm::tr("Historic");
    else if (tagKey == QLatin1String("landuse"))
        return QPlaceManagerEngineOsm::tr("Land use");
    else if (tagKey == QLatin1String("leisure"))
        return QPlaceManagerEngineOsm::tr("Leisure");
    else if (tagKey == QLatin1String("man_made"))
        return QPlaceManagerEngineOsm::tr("Man made");
    else if (tagKey == QLatin1String("natural"))
        return QPlaceManagerEngineOsm::tr("Natural");
    else if (tagKey == QLatin1String("place"))
        return QPlaceManagerEngineOsm::tr("Place");
    else if (tagKey == QLatin1String("railway"))
        return QPlaceManagerEngineOsm::tr("Railway");
    else if (tagKey == QLatin1String("shop"))
        return QPlaceManagerEngineOsm::tr("Shop");
    else if (tagKey == QLatin1String("tourism"))
        return QPlaceManagerEngineOsm::tr("Tourism");
    else if (tagKey == QLatin1String("waterway"))
        return QPlaceManagerEngineOsm::tr("Waterway");
    else
        return tagKey;
}

}

QPlaceManagerEngineOsm::QPlaceManagerEngineOsm(const QVariantMap &parameters,
                                               QGeoServiceProvider::Error *error,
                                               QString *errorString)
:   QPlaceManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this)),
    m_categoriesReply(0)
{
    if (parameters.contains(QStringLiteral("osm.useragent")))
        m_userAgent = parameters.value(QStringLiteral("osm.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("osm.places.host")))
        m_urlPrefix = parameters.value(QStringLiteral("osm.places.host")).toString();
    else
        m_urlPrefix = QStringLiteral("http://nominatim.openstreetmap.org/search");


    if (parameters.contains(QStringLiteral("osm.places.debug_query")))
        m_debugQuery = parameters.value(QStringLiteral("osm.places.debug_query")).toBool();

    if (parameters.contains(QStringLiteral("osm.places.page_size"))
            && parameters.value(QStringLiteral("osm.places.page_size")).canConvert<int>())
        m_pageSize = parameters.value(QStringLiteral("osm.places.page_size")).toInt();

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QPlaceManagerEngineOsm::~QPlaceManagerEngineOsm()
{
}

QPlaceSearchReply *QPlaceManagerEngineOsm::search(const QPlaceSearchRequest &request)
{
    bool unsupported = false;

    // Only public visibility supported
    unsupported |= request.visibilityScope() != QLocation::UnspecifiedVisibility &&
                   request.visibilityScope() != QLocation::PublicVisibility;
    unsupported |= request.searchTerm().isEmpty() && request.categories().isEmpty();

    if (unsupported)
        return QPlaceManagerEngine::search(request);

    QUrlQuery queryItems;

    queryItems.addQueryItem(QStringLiteral("format"), QStringLiteral("jsonv2"));

    //queryItems.addQueryItem(QStringLiteral("accept-language"), QStringLiteral("en"));

    QGeoRectangle boundingBox = request.searchArea().boundingGeoRectangle();

    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bounded"), QStringLiteral("1"));
        QString coordinates;
        coordinates = QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.topLeft().latitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().latitude());
        queryItems.addQueryItem(QStringLiteral("viewbox"), coordinates);
    }

    QStringList queryParts;
    if (!request.searchTerm().isEmpty())
        queryParts.append(request.searchTerm());

    for (const QPlaceCategory &category : request.categories()) {
        QString id = category.categoryId();
        queryParts.append(QLatin1Char('[') + id + QLatin1Char(']'));
    }

    queryItems.addQueryItem(QStringLiteral("q"), queryParts.join(QLatin1Char('+')));

    QVariantMap parameters = request.searchContext().toMap();

    QStringList placeIds = parameters.value(QStringLiteral("ExcludePlaceIds")).toStringList();
    if (!placeIds.isEmpty())
        queryItems.addQueryItem(QStringLiteral("exclude_place_ids"), placeIds.join(QLatin1Char(',')));

    queryItems.addQueryItem(QStringLiteral("addressdetails"), QStringLiteral("1"));
    queryItems.addQueryItem(QStringLiteral("limit"), (request.limit() > 0) ? QString::number(request.limit())
                                                                           : QString::number(m_pageSize));

    QUrl requestUrl(m_urlPrefix);
    requestUrl.setQuery(queryItems);

    QNetworkRequest rq(requestUrl);
    rq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *networkReply = m_networkManager->get(rq);

    QPlaceSearchReplyOsm *reply = new QPlaceSearchReplyOsm(request, networkReply, this);
    connect(reply, &QPlaceSearchReplyOsm::finished,
            this, &QPlaceManagerEngineOsm::replyFinished);
    connect(reply, &QPlaceSearchReplyOsm::errorOccurred,
            this, &QPlaceManagerEngineOsm::replyError);

    if (m_debugQuery)
        reply->requestUrl = requestUrl.url(QUrl::None);

    return reply;
}

QPlaceReply *QPlaceManagerEngineOsm::initializeCategories()
{
    // Only fetch categories once
    if (m_categories.isEmpty() && !m_categoriesReply) {
        m_categoryLocales = m_locales;
        m_categoryLocales.append(QLocale(QLocale::English));
        fetchNextCategoryLocale();
    }

    QPlaceCategoriesReplyOsm *reply = new QPlaceCategoriesReplyOsm(this);
    connect(reply, &QPlaceCategoriesReplyOsm::finished,
            this, &QPlaceManagerEngineOsm::replyFinished);
    connect(reply, &QPlaceCategoriesReplyOsm::errorOccurred,
            this, &QPlaceManagerEngineOsm::replyError);

    // TODO delayed finished() emission
    if (!m_categories.isEmpty())
        reply->emitFinished();

    m_pendingCategoriesReply.append(reply);
    return reply;
}

QString QPlaceManagerEngineOsm::parentCategoryId(const QString &categoryId) const
{
    Q_UNUSED(categoryId);

    // Only a two category levels
    return QString();
}

QStringList QPlaceManagerEngineOsm::childCategoryIds(const QString &categoryId) const
{
    return m_subcategories.value(categoryId);
}

QPlaceCategory QPlaceManagerEngineOsm::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> QPlaceManagerEngineOsm::childCategories(const QString &parentId) const
{
    QList<QPlaceCategory> categories;
    for (const QString &id : m_subcategories.value(parentId))
        categories.append(m_categories.value(id));
    return categories;
}

QList<QLocale> QPlaceManagerEngineOsm::locales() const
{
    return m_locales;
}

void QPlaceManagerEngineOsm::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

void QPlaceManagerEngineOsm::categoryReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    QXmlStreamReader parser(reply);
    while (!parser.atEnd() && parser.readNextStartElement()) {
        if (parser.name() == QLatin1String("mediawiki"))
            continue;
        if (parser.name() == QLatin1String("page"))
            continue;
        if (parser.name() == QLatin1String("revision"))
            continue;
        if (parser.name() == QLatin1String("text")) {
            // parse
            QString page = parser.readElementText();
            QRegularExpression regex(QStringLiteral("\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([\\-YN])"));
            QRegularExpressionMatchIterator i = regex.globalMatch(page);
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString name = match.capturedView(1).toString();
                QString tagKey = match.capturedView(2).toString();
                QString tagValue = match.capturedView(3).toString();
                QString op = match.capturedView(4).toString();
                QString plural = match.capturedView(5).toString();

                // Only interested in any operator plural forms
                if (op != QLatin1String("-") || plural != QLatin1String("Y"))
                    continue;

                if (!m_categories.contains(tagKey)) {
                    QPlaceCategory category;
                    category.setCategoryId(tagKey);
                    category.setName(nameForTagKey(tagKey));
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[QString()].append(tagKey);
                    emit categoryAdded(category, QString());
                }

                QPlaceCategory category;
                category.setCategoryId(tagKey + QLatin1Char('=') + tagValue);
                category.setName(name);

                if (!m_categories.contains(category.categoryId())) {
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[tagKey].append(category.categoryId());
                    emit categoryAdded(category, tagKey);
                }
            }
        }

        parser.skipCurrentElement();
    }

    if (m_categories.isEmpty() && !m_categoryLocales.isEmpty()) {
        fetchNextCategoryLocale();
        return;
    } else {
        m_categoryLocales.clear();
    }

    for (QPlaceCategoriesReplyOsm *reply : m_pendingCategoriesReply)
        reply->emitFinished();
    m_pendingCategoriesReply.clear();
}

void QPlaceManagerEngineOsm::categoryReplyError()
{
    for (QPlaceCategoriesReplyOsm *reply : m_pendingCategoriesReply)
        reply->setError(QPlaceReply::CommunicationError, tr("Network request error"));
}

void QPlaceManagerEngineOsm::replyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QPlaceManagerEngineOsm::replyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}

void QPlaceManagerEngineOsm::fetchNextCategoryLocale()
{
    if (m_categoryLocales.isEmpty()) {
        qWarning("No locales specified to fetch categories for");
        return;
    }

    QLocale locale = m_categoryLocales.takeFirst();

    // FIXME: Categories should be cached.
    QUrl requestUrl = QUrl(SpecialPhrasesBaseUrl + locale.name().left(2).toUpper());

    m_categoriesReply = m_networkManager->get(QNetworkRequest(requestUrl));
    connect(m_categoriesReply, &QNetworkReply::finished,
            this, &QPlaceManagerEngineOsm::categoryReplyFinished);
    connect(m_categoriesReply, &QNetworkReply::errorOccurred,
            this, &QPlaceManagerEngineOsm::categoryReplyError);
}
