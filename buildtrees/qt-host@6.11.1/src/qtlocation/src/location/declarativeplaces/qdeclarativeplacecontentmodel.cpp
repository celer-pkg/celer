// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativeplacecontentmodel_p.h"
#include "qdeclarativeplace_p.h"
#include "qdeclarativegeoserviceprovider_p.h"
#include "error_messages_p.h"

#include <QtQml/QQmlInfo>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QPlaceManager>
#include <QtLocation/QPlaceContentRequest>
#include <QtLocation/QPlaceUser>

QT_BEGIN_NAMESPACE

/*!
    \qmltype EditorialModel
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-models
    \since QtLocation 5.5

    \brief The EditorialModel type provides a model of place editorials.

    The EditorialModel is a read-only model used to fetch editorials related to a \l Place.
    Binding a \l Place via \l EditorialModel::place initiates an initial fetch of editorials.
    The model performs fetches incrementally and is intended to be used in conjunction
    with a View such as a \l ListView.  When the View reaches the last of the editorials
    currently in the model, a fetch is performed to retrieve more if they are available.
    The View is automatically updated as the editorials are received.  The number of
    editorials which are fetched at a time is specified by the
    \l {EditorialModel::}{batchSize} property. The total number of editorials available can be
    accessed via the \l {EditorialModel::}{totalCount} property.

    The model returns data for the following roles:

    \table
        \header
            \li Role
            \li Type
            \li Description
        \row
            \li supplier
            \li \l supplier
            \li The supplier of the content.
        \row
            \li user
            \li \l user
            \li The user who contributed the content.
        \row
            \li attribution
            \li string
            \li Attribution text which must be displayed when displaying the content.
        \row
            \li url
            \li url
            \li The URL of the image.
        \row
            \li imageId
            \li string
            \li The identifier of the image.
        \row
            \li mimeType
            \li string
            \li The MIME type of the image.
        \row
            \li text
            \li string
            \li The editorial's textual description of the place.  It can be either rich (HTML based) text or plain text
               depending upon the provider.
        \row
            \li title
            \li string
            \li The title of the editorial.
        \row
            \li language
            \li string
            \li The language that the editorial is written in.
        \row
            \li dateTime
            \li datetime
            \li The date and time that the review was posted.
        \row
            \li text
            \li string
            \li The review's textual description of the place.  It can be either rich (HTML based) text or plain text
               depending on the provider.
        \row
            \li language
            \li string
            \li The language that the review is written in.
        \row
            \li rating
            \li real
            \li The rating that the reviewer gave to the place.
        \row
            \li reviewId
            \li string
            \li The identifier of the review.
        \row
            \li title
            \li string
            \li The title of the review.
    \endtable

    \section1 Example

    The following example shows how to display editorials for a place:

    \snippet declarative/places.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/places.qml EditorialModel

*/

/*!
    \qmlproperty Place EditorialModel::place

    This property holds the Place that the editorials are for.
*/

/*!
    \qmlproperty int EditorialModel::batchSize

    This property holds the batch size to use when fetching more editorials items.
*/

/*!
    \qmlproperty int EditorialModel::totalCount

    This property holds the total number of editorial items for the place.
*/

/*!
    \qmltype ImageModel
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-models
    \since QtLocation 5.5

    \brief The ImageModel type provides a model of place images.

    \sa EditorialModel
*/

/*!
    \qmltype ReviewModel
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-models
    \since QtLocation 5.5

    \brief The ReviewModel type provides a model of place reviews.

    \sa EditorialModel
*/

QDeclarativePlaceContentModel::QDeclarativePlaceContentModel(QPlaceContent::Type type,
                                                             QObject *parent)
    : QAbstractListModel(parent), m_type(type)
{
}

QDeclarativePlaceContentModel::~QDeclarativePlaceContentModel()
{
}

/*!
    \internal
*/
QDeclarativePlace *QDeclarativePlaceContentModel::place() const
{
    return m_place;
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::setPlace(QDeclarativePlace *place)
{
    if (m_place != place) {
        beginResetModel();

        int initialCount = m_contentCount;
        clearData();
        m_place = place;
        endResetModel();

        emit placeChanged();
        if (initialCount != -1)
            emit totalCountChanged();

        fetchMore(QModelIndex());
    }
}

/*!
    \internal
*/
int QDeclarativePlaceContentModel::batchSize() const
{
    return m_batchSize;
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::setBatchSize(int batchSize)
{
    if (m_batchSize != batchSize) {
        m_batchSize = batchSize;
        emit batchSizeChanged();
    }
}

/*!
    \internal
*/
int QDeclarativePlaceContentModel::totalCount() const
{
    return m_contentCount;
}

/*!
    \internal
    Clears the model data but does not reset it.
*/
void QDeclarativePlaceContentModel::clearData()
{
    m_users.clear();
    m_suppliers.clear();
    m_content.clear();

    m_contentCount = -1;

    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    m_nextRequest.clear();
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::initializeCollection(int totalCount, const QPlaceContent::Collection &collection)
{
    beginResetModel();

    int initialCount = m_contentCount;
    clearData();

    for (auto i = collection.cbegin(), end = collection.cend(); i != end; ++i) {
        const QPlaceContent &content = i.value();
        if (content.type() != m_type)
            continue;

        m_content.insert(i.key(), content);
        const auto supplier = content.value(QPlaceContent::ContentSupplier)
                                     .value<QPlaceSupplier>();
        if (!m_suppliers.contains(supplier.supplierId()))
            m_suppliers.insert(supplier.supplierId(), supplier);
        const auto user = content.value(QPlaceContent::ContentUser)
                                 .value<QPlaceUser>();
        if (!m_users.contains(user.userId()))
            m_users.insert(user.userId(), user);
    }

    m_contentCount = totalCount;

    if (initialCount != totalCount)
        emit totalCountChanged();

    endResetModel();
}

/*!
    \internal
*/
int QDeclarativePlaceContentModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_content.count();
}

/*!
    \internal
*/
QVariant QDeclarativePlaceContentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount(index.parent()) || index.row() < 0)
        return QVariant();

    const QPlaceContent &content = m_content.value(index.row());
    if (content.type() != m_type)
        return QVariant();

    switch (role) {
    case ContentSupplierRole:
        return QVariant::fromValue(m_suppliers.value(content.value(QPlaceContent::ContentSupplier)
                                              .value<QPlaceSupplier>().supplierId()));
    case ContentUserRole:
        return QVariant::fromValue(m_users.value(content.value(QPlaceContent::ContentUser)
                                          .value<QPlaceUser>().userId()));
    case ContentAttributionRole:
        return content.value(QPlaceContent::ContentAttribution);
    case ImageUrlRole:
        return content.value(QPlaceContent::ImageUrl);
    case ImageIdRole:
        return content.value(QPlaceContent::ImageId);
    case ImageMimeTypeRole:
        return content.value(QPlaceContent::ImageMimeType);

    case EditorialTextRole:
        return content.value(QPlaceContent::EditorialText);
    case EditorialTitleRole:
        return content.value(QPlaceContent::EditorialTitle);
    case EditorialLanguageRole:
        return content.value(QPlaceContent::EditorialLanguage);

    case ReviewDateTimeRole:
        return content.value(QPlaceContent::ReviewDateTime);
    case ReviewTextRole:
        return content.value(QPlaceContent::ReviewText);
    case ReviewLanguageRole:
        return content.value(QPlaceContent::ReviewLanguage);
    case ReviewRatingRole:
        return content.value(QPlaceContent::ReviewRating);
    case ReviewIdRole:
        return content.value(QPlaceContent::ReviewId);
    case ReviewTitleRole:
        return content.value(QPlaceContent::ReviewTitle);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> QDeclarativePlaceContentModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(ContentSupplierRole, "supplier");
    roles.insert(ContentUserRole, "user");
    roles.insert(ContentAttributionRole, "attribution");

    switch (m_type) {
    case QPlaceContent::EditorialType:
        roles.insert(EditorialTextRole, "text");
        roles.insert(EditorialTitleRole, "title");
        roles.insert(EditorialLanguageRole, "language");
        break;
    case QPlaceContent::ImageType:
        roles.insert(ImageUrlRole, "url");
        roles.insert(ImageIdRole, "imageId");
        roles.insert(ImageMimeTypeRole, "mimeType");
        break;
    case QPlaceContent::ReviewType:
        roles.insert(ReviewDateTimeRole, "dateTime");
        roles.insert(ReviewTextRole, "text");
        roles.insert(ReviewLanguageRole, "language");
        roles.insert(ReviewRatingRole, "rating");
        roles.insert(ReviewIdRole, "reviewId");
        roles.insert(ReviewTitleRole, "title");
        break;
    default:
        break;
    }
    return roles;
}

/*!
    \internal
*/
bool QDeclarativePlaceContentModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    if (!m_place)
        return false;

    if (m_contentCount == -1)
        return true;

    return m_content.count() != m_contentCount;
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    if (!m_place)
        return;

    if (m_reply)
        return;

    if (!m_place->plugin())
        return;

    QDeclarativeGeoServiceProvider *plugin = m_place->plugin();

    QGeoServiceProvider *serviceProvider = plugin->sharedGeoServiceProvider();
    if (!serviceProvider)
        return;

    QPlaceManager *placeManager = serviceProvider->placeManager();
    if (!placeManager)
        return;

    if (m_nextRequest == QPlaceContentRequest()) {
        QPlaceContentRequest request;
        request.setContentType(m_type);
        request.setPlaceId(m_place->place().placeId());
        request.setLimit(m_batchSize);

        m_reply = placeManager->getPlaceContent(request);
    } else {
        m_reply = placeManager->getPlaceContent(m_nextRequest);
    }

    connect(m_reply, &QPlaceReply::finished,
            this, &QDeclarativePlaceContentModel::fetchFinished, Qt::QueuedConnection);
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::classBegin()
{
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::componentComplete()
{
    m_complete = true;
    fetchMore(QModelIndex());
}

/*!
    \internal
*/
void QDeclarativePlaceContentModel::fetchFinished()
{
    if (!m_reply)
        return;

    QPlaceContentReply *reply = m_reply;
    m_reply = nullptr;

    m_nextRequest = reply->nextPageRequest();

    if (m_contentCount != reply->totalCount()) {
        m_contentCount = reply->totalCount();
        emit totalCountChanged();
    }

    if (!reply->content().isEmpty()) {
        QPlaceContent::Collection contents = reply->content();

        //find out which indexes are new and which ones have changed.
        QList<int> changedIndexes;
        QList<int> newIndexes;
        for (auto it = contents.cbegin(), end = contents.cend(); it != end; ++it) {
            if (!m_content.contains(it.key()))
                newIndexes.append(it.key());
            else if (it.value() != m_content.value(it.key()))
                changedIndexes.append(it.key());
        }

        //insert new indexes in blocks where within each
        //block, the indexes are consecutive.
        int startIndex = -1;
        for (auto it = newIndexes.cbegin(), end = newIndexes.cend(); it != end; ++it) {
            int currentIndex = *it;
            if (startIndex == -1)
                startIndex = currentIndex;

            auto next = std::next(it);
            if (next == end || *next > (currentIndex + 1)) {
                beginInsertRows(QModelIndex(),startIndex,currentIndex);
                for (int i = startIndex; i <= currentIndex; ++i) {
                    const QPlaceContent &content = contents.value(i);

                    m_content.insert(i, content);
                    const auto supplier = content.value(QPlaceContent::ContentSupplier)
                                                 .value<QPlaceSupplier>();
                    if (!m_suppliers.contains(supplier.supplierId()))
                        m_suppliers.insert(supplier.supplierId(), supplier);
                    const auto user = content.value(QPlaceContent::ContentUser)
                                             .value<QPlaceUser>();
                    if (!m_users.contains(user.userId()))
                        m_users.insert(user.userId(), user);
                }
                endInsertRows();
                startIndex = -1;
            }
        }

        //modify changed indexes in blocks where within each
        //block, the indexes are consecutive.
        startIndex = -1;
        for (auto it = changedIndexes.cbegin(), end = changedIndexes.cend(); it != end; ++it) {
            int currentIndex = *it;
            if (startIndex == -1)
                startIndex = currentIndex;

            auto next = std::next(it);
            if (next == end || *next > (currentIndex + 1)) {
                for (int i = startIndex; i <= currentIndex; ++i) {
                    const QPlaceContent &content = contents.value(i);
                    m_content.insert(i, content);
                    const auto supplier = content.value(QPlaceContent::ContentSupplier)
                                                 .value<QPlaceSupplier>();
                    if (!m_suppliers.contains(supplier.supplierId()))
                        m_suppliers.insert(supplier.supplierId(), supplier);
                    const auto user = content.value(QPlaceContent::ContentUser)
                                             .value<QPlaceUser>();
                    if (!m_users.contains(user.userId()))
                        m_users.insert(user.userId(), user);
                }
                emit dataChanged(index(startIndex),index(currentIndex));
                startIndex = -1;
            }
        }

        // The fetch didn't add any new content and we haven't fetched all content yet. This is
        // likely due to the model being prepopulated by Place::getDetails(). Keep fetching more
        // data until new content is available.
        if (newIndexes.isEmpty() && m_content.count() != m_contentCount)
            fetchMore(QModelIndex());
    }

    reply->deleteLater();
}

QT_END_NAMESPACE
