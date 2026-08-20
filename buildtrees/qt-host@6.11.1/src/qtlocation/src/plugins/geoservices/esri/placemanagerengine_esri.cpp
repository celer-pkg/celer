// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "placemanagerengine_esri.h"
#include "placesearchreply_esri.h"
#include "placecategoriesreply_esri.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtCore/QUrlQuery>

#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/QGeoShape>

#include <QtLocation/QPlaceCategory>
#include <QtLocation/QPlaceSearchRequest>

QT_BEGIN_NAMESPACE

// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-find-address-candidates.htm
// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-category-filtering.htm
// https://developers.arcgis.com/rest/geocode/api-reference/geocoding-service-output.htm

static const QString kCategoriesKey(QStringLiteral("categories"));
static const QString kSingleLineKey(QStringLiteral("singleLine"));
static const QString kLocationKey(QStringLiteral("location"));
static const QString kNameKey(QStringLiteral("name"));
static const QString kOutFieldsKey(QStringLiteral("outFields"));
static const QString kCandidateFieldsKey(QStringLiteral("candidateFields"));
static const QString kCountriesKey(QStringLiteral("detailedCountries"));
static const QString kLocalizedNamesKey(QStringLiteral("localizedNames"));
static const QString kMaxLocationsKey(QStringLiteral("maxLocations"));

static const QUrl kUrlGeocodeServer(
        "https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer?f=pjson");
static const QUrl kUrlFindAddressCandidates("https://geocode.arcgis.com/arcgis/rest/services/World/"
                                            "GeocodeServer/findAddressCandidates");

PlaceManagerEngineEsri::PlaceManagerEngineEsri(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                                               QString *errorString) :
    QPlaceManagerEngine(parameters),
    m_networkManager(new QNetworkAccessManager(this))
{
    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

PlaceManagerEngineEsri::~PlaceManagerEngineEsri()
{
}

QList<QLocale> PlaceManagerEngineEsri::locales() const
{
    return m_locales;
}

void PlaceManagerEngineEsri::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

/***** Search *****/

QPlaceSearchReply *PlaceManagerEngineEsri::search(const QPlaceSearchRequest &request)
{
    bool unsupported = false;

    // Only public visibility supported
    unsupported |= request.visibilityScope() != QLocation::UnspecifiedVisibility &&
            request.visibilityScope() != QLocation::PublicVisibility;
    unsupported |= request.searchTerm().isEmpty() && request.categories().isEmpty();

    if (unsupported)
        return QPlaceManagerEngine::search(request);

    QUrlQuery queryItems;
    queryItems.addQueryItem(QStringLiteral("f"), QStringLiteral("json"));

    const QGeoCoordinate center = request.searchArea().center();
    if (center.isValid())
    {
        const QString location = QString("%1,%2").arg(center.longitude()).arg(center.latitude());
        queryItems.addQueryItem(kLocationKey, location);
    }

    const QGeoRectangle boundingBox = request.searchArea().boundingGeoRectangle();
    if (!boundingBox.isEmpty())
    {
        const QString searchExtent = QString("%1,%2,%3,%4")
                .arg(boundingBox.topLeft().longitude())
                .arg(boundingBox.topLeft().latitude())
                .arg(boundingBox.bottomRight().longitude())
                .arg(boundingBox.bottomRight().latitude());
        queryItems.addQueryItem(QStringLiteral("searchExtent"), searchExtent);
    }

    if (!request.searchTerm().isEmpty())
        queryItems.addQueryItem(kSingleLineKey, request.searchTerm());

    QStringList categories;
    if (!request.categories().isEmpty())
    {
        for (const QPlaceCategory &placeCategory : request.categories())
            categories.append(placeCategory.categoryId());
        queryItems.addQueryItem("category", categories.join(","));
    }

    if (request.limit() > 0)
        queryItems.addQueryItem(kMaxLocationsKey, QString::number(request.limit()));

    queryItems.addQueryItem(kOutFieldsKey, QStringLiteral("*"));

    QUrl requestUrl(kUrlFindAddressCandidates);
    requestUrl.setQuery(queryItems);

    QNetworkRequest networkRequest(requestUrl);
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *networkReply = m_networkManager->get(networkRequest);

    PlaceSearchReplyEsri *reply = new PlaceSearchReplyEsri(request, networkReply, m_candidateFieldsLocale, m_countriesLocale, this);
    connect(reply, &PlaceSearchReplyEsri::finished,
            this, &PlaceManagerEngineEsri::replyFinished);
    connect(reply, &PlaceSearchReplyEsri::errorOccurred,
            this, &PlaceManagerEngineEsri::replyError);

    return reply;
}

void PlaceManagerEngineEsri::replyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void PlaceManagerEngineEsri::replyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit errorOccurred(reply, errorCode, errorString);
}

/***** Categories *****/

QPlaceReply *PlaceManagerEngineEsri::initializeCategories()
{
    initializeGeocodeServer();

    PlaceCategoriesReplyEsri *reply = new PlaceCategoriesReplyEsri(this);
    connect(reply, &PlaceCategoriesReplyEsri::finished,
            this, &PlaceManagerEngineEsri::replyFinished);
    connect(reply, &PlaceCategoriesReplyEsri::errorOccurred,
            this, &PlaceManagerEngineEsri::replyError);

    // TODO delayed finished() emission
    if (!m_categories.isEmpty())
        reply->emitFinished();

    m_pendingCategoriesReply.append(reply);
    return reply;
}

void PlaceManagerEngineEsri::parseCategories(const QJsonArray &jsonArray, const QString &parentCategoryId)
{
    for (const QJsonValueConstRef jsonValue : jsonArray)
    {
        if (!jsonValue.isObject())
            continue;

        const QJsonObject jsonCategory = jsonValue.toObject();
        const QString key = jsonCategory.value(kNameKey).toString();
        const QString localeName = localizedName(jsonCategory);

        if (key.isEmpty())
            continue;

        QPlaceCategory category;
        category.setCategoryId(key);
        category.setName(localeName.isEmpty() ? key : localeName); // localizedNames
        m_categories.insert(key, category);
        m_subcategories[parentCategoryId].append(key);
        m_parentCategory.insert(key, parentCategoryId);
        emit categoryAdded(category, parentCategoryId);

        if (jsonCategory.contains(kCategoriesKey))
        {
            const QJsonArray jsonArray = jsonCategory.value(kCategoriesKey).toArray();
            parseCategories(jsonArray, key);
        }
    }
}

QString PlaceManagerEngineEsri::parentCategoryId(const QString &categoryId) const
{
    return m_parentCategory.value(categoryId);
}

QStringList PlaceManagerEngineEsri::childCategoryIds(const QString &categoryId) const
{
    return m_subcategories.value(categoryId);
}

QPlaceCategory PlaceManagerEngineEsri::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> PlaceManagerEngineEsri::childCategories(const QString &parentId) const
{
    QList<QPlaceCategory> categories;
    for (const QString &id : m_subcategories.value(parentId))
        categories.append(m_categories.value(id));
    return categories;
}

void PlaceManagerEngineEsri::finishCategories()
{
    for (PlaceCategoriesReplyEsri *reply : m_pendingCategoriesReply)
        reply->emitFinished();
    m_pendingCategoriesReply.clear();
}

void PlaceManagerEngineEsri::errorCaterogies(const QString &error)
{
    for (PlaceCategoriesReplyEsri *reply : m_pendingCategoriesReply)
        reply->setError(QPlaceReply::CommunicationError, error);
}

/***** GeocodeServer *****/

void PlaceManagerEngineEsri::initializeGeocodeServer()
{
    // Only fetch categories once
    if (m_categories.isEmpty() && !m_geocodeServerReply)
    {
        m_geocodeServerReply = m_networkManager->get(QNetworkRequest(kUrlGeocodeServer));
        connect(m_geocodeServerReply, &QNetworkReply::finished,
                this, &PlaceManagerEngineEsri::geocodeServerReplyFinished);
        connect(m_geocodeServerReply, &QNetworkReply::errorOccurred,
                this, &PlaceManagerEngineEsri::geocodeServerReplyError);
    }
}

QString PlaceManagerEngineEsri::localizedName(const QJsonObject &jsonObject)
{
    const QJsonObject localizedNames = jsonObject.value(kLocalizedNamesKey).toObject();

    for (const QLocale &locale : std::as_const(m_locales)) {
        const QString localeStr = locale.name();
        if (localizedNames.contains(localeStr))
        {
            return localizedNames.value(localeStr).toString();
        }

        const QString shortLocale = localeStr.left(2);
        if (localizedNames.contains(shortLocale))
        {
            return localizedNames.value(shortLocale).toString();
        }
    }
    return QString();
}

void PlaceManagerEngineEsri::parseCandidateFields(const QJsonArray &jsonArray)
{
    for (const QJsonValueConstRef jsonValue : jsonArray)
    {
        if (!jsonValue.isObject())
            continue;

        const QJsonObject jsonCandidateField = jsonValue.toObject();
        if (!jsonCandidateField.contains(kLocalizedNamesKey))
            continue;

        const QString key = jsonCandidateField.value(kNameKey).toString();
        m_candidateFieldsLocale.insert(key, localizedName(jsonCandidateField));
    }
}

void PlaceManagerEngineEsri::parseCountries(const QJsonArray &jsonArray)
{
    for (const QJsonValueConstRef jsonValue : jsonArray)
    {
        if (!jsonValue.isObject())
            continue;

        const QJsonObject jsonCountry = jsonValue.toObject();
        if (!jsonCountry.contains(kLocalizedNamesKey))
            continue;

        const QString key = jsonCountry.value(kNameKey).toString();
        m_countriesLocale.insert(key, localizedName(jsonCountry));
    }
}

void PlaceManagerEngineEsri::geocodeServerReplyFinished()
{
    if (!m_geocodeServerReply)
        return;

    QJsonDocument document = QJsonDocument::fromJson(m_geocodeServerReply->readAll());
    if (!document.isObject())
    {
        errorCaterogies(m_geocodeServerReply->errorString());
        return;
    }

    QJsonObject jsonObject = document.object();

    // parse categories
    if (jsonObject.contains(kCategoriesKey))
    {
        const QJsonArray jsonArray = jsonObject.value(kCategoriesKey).toArray();
        parseCategories(jsonArray, QString());
    }

    // parse candidateFields
    if (jsonObject.contains(kCandidateFieldsKey))
    {
        const QJsonArray jsonArray = jsonObject.value(kCandidateFieldsKey).toArray();
        parseCandidateFields(jsonArray);
    }

    // parse countries
    if (jsonObject.contains(kCountriesKey))
    {
        const QJsonArray jsonArray = jsonObject.value(kCountriesKey).toArray();
        parseCountries(jsonArray);
    }

    finishCategories();

    m_geocodeServerReply->deleteLater();
}

void PlaceManagerEngineEsri::geocodeServerReplyError()
{
    if (m_categories.isEmpty() && !m_geocodeServerReply)
        return;

    errorCaterogies(m_geocodeServerReply->errorString());
}

QT_END_NAMESPACE
