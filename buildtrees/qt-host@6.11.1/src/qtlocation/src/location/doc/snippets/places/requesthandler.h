// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QDebug>
#include <QGeoCircle>
#include <QGeoServiceProvider>
#include <QPlaceDetailsReply>
#include <QPlaceIdReply>
#include <QPlaceManager>
#include <QPlaceSearchReply>
#include <QPlaceResult>
#include <QPlaceImage>
#include <QtCore/QMetaObject>

class RequestHandler : public QObject
{
public:
    void initializeManager() {
        //! [Initialize Manager]
        //The "provider name" is used to select a particular provider
        QGeoServiceProvider *provider = new QGeoServiceProvider("provider name");
        QPlaceManager *manager = provider->placeManager();
        //! [Initialize Manager]
        Q_UNUSED(provider);
        Q_UNUSED(manager);
    }

    void simpleSearch()
    {
        //! [Simple search]
        //1) Make an appropriate request
        QPlaceSearchRequest searchRequest;
        searchRequest.setSearchTerm("ice cream");
        searchRequest.setSearchArea(QGeoCircle(QGeoCoordinate(12.34, 56.78)));

        //2) Use the manager to initiate a request and retrieve a reply object
        QPlaceSearchReply * searchReply = manager->search(searchRequest);

        //3) Connect the reply object to a slot which is invoked upon operation completion
        connect(searchReply, &QPlaceSearchReply::finished,
                this, &RequestHandler::processSearchReply);
        //! [Simple search]
    }

    void search()
    {
        //! [Search for places cpp]

        //instantiate request and set parameters
        QPlaceSearchRequest searchRequest;
        searchRequest.setSearchTerm("ice cream");
        searchRequest.setSearchArea(QGeoCircle(QGeoCoordinate(12.34, 56.78)));

        //send off a search request
        /*QPlaceSearchReply * */ searchReply = manager->search(searchRequest);

        //connect a slot to handle the reply
        connect(searchReply, &QPlaceSearchReply::finished, this, &RequestHandler::handleSearchReply);

        //! [Search for places cpp]
    }

    void searchPaging()
    {
        //! [Search paging]
        QPlaceSearchRequest searchRequest;
        searchRequest.setLimit(15); //specify how many results are to be retrieved.
        //! [Search paging]
    }

    void details()
    {
        QPlace place;
        //! [Details check]
        if (!place.detailsFetched()) {
            /*QPlaceDetailsReply * */ detailsReply = manager->getPlaceDetails(place.placeId());
            connect(detailsReply, &QPlaceDetailsReply::finished, this, &RequestHandler::handleDetailsReply);
        }
        //! [Details check]
    }

    void images()
    {
        QPlace place;

        //! [Image request]
        QPlaceContentRequest request;
        request.setContentType(QPlaceContent::ImageType);
        request.setPlaceId(place.placeId());
        request.setLimit(5);
        /*QPlaceContentReply * */ contentReply = manager->getPlaceContent(request);
        connect(contentReply, &QPlaceContentReply::finished, this, &RequestHandler::handleImagesReply);
        //! [Image request]
    }


    void suggestion()
    {
        //! [Suggestion request]
        QPlaceSearchRequest request;
        request.setSearchTerm("piz");
        request.setSearchArea(QGeoCircle(QGeoCoordinate(12.34, 56.78)));
        /* QPlaceSearchSuggestion * */suggestionReply = manager->searchSuggestions(request);
        connect(suggestionReply, &QPlaceSearchSuggestion::finished, this, &RequestHandler::handleSuggestionReply);
        //! [Suggestion request]
    }

    void savePlace()
    {
        //! [Save place pt1]
        QPlace  place;
        place.setName( "Fred's Ice Cream Parlor" );

        QGeoLocation location;
        location.setCoordinate(QGeoCoordinate(12.34, 56.78));

        QGeoAddress address;
        address.setStreet("111 Nother Street");
        //! [Save place pt1]

        //! [Save place pt2]
        location.setAddress(address);
        place.setLocation(location);

        /* QPlaceIdReply * */savePlaceReply = manager->savePlace(place);
        connect(savePlaceReply, &QPlaceIdReply::finished, this, &RequestHandler::handleSavePlaceReply);
        //! [Save place pt2]
    }

    void removePlace()
    {
        QPlace place;
        //! [Remove place]
        /* QPlaceIdReply * */removePlaceReply = manager->removePlace(place.placeId());
        connect(removePlaceReply, &QPlaceIdReply::finished, this, &RequestHandler::handleRemovePlaceReply);
        //! [Remove place]
    }

    void initializeCategories()
    {
        //! [Initialize categories]
        /* QPlaceReply * */initCatReply = manager->initializeCategories();
        connect(initCatReply, &QPlaceReply::finished, this, &RequestHandler::handleInitCatReply);
        //! [Initialize categories]
    }

    void saveCategory()
    {
        //! [Save category]
        QPlaceCategory fastFood;

        QPlaceCategory category;
        category.setName("pizza");
        /*QPlaceIdReply */ saveCategoryReply = manager->saveCategory(category);
        connect(saveCategoryReply, &QPlaceIdReply::finished, this, &RequestHandler::handleSaveCategoryReply);

        //we could have saved a category as a child by supplying a parent identifier.
        saveCategoryReply = manager->saveCategory(category, fastFood.categoryId());
        //! [Save category]
    }

    void removeCategory()
    {
        QPlaceCategory category;
        //! [Remove category]
        /* QPlaceIdReply * */removeCategoryReply = manager->removeCategory(place.placeId());
        connect(removeCategoryReply, &QPlaceIdReply::finished, this, &RequestHandler::handleRemoveCategoryReply);
        //! [Remove category]
    }

    void searchRequest() {
        QPlaceCategory diner;
        QPlaceCategory restaurant;

        //! [Search request]
        QPlaceSearchRequest searchRequest;
        searchRequest.setSearchTerm("Fast food"); //search term for what we are interested in

        //set a search center
        searchRequest.setSearchArea(QGeoCircle(QGeoCoordinate(2.3, 48.87)));

        //set a distance hint as a relevancy hint.
        //closer places have greater weighting in the ranking of results.
        searchRequest.setRelevanceHint(QPlaceSearchRequest::DistanceHint);

        //use limit to adjust pagination.
        //this limits the number of place results to 5 per page.
        searchRequest.setLimit(5);

        //provide some categories to narrow down search
        QList<QPlaceCategory> categories;
        categories << diner << restaurant;
        searchRequest.setCategories(categories);
        //! [Search request]
    }

    void content() {
        QPlace place;
        //! [Content request]
        QPlaceContentRequest request;
        request.setContentType(QPlaceContent::ImageType);
        request.setPlaceId(place.placeId());
        request.setLimit(5);

        QPlaceContentReply *contentReply = manager->getPlaceContent(request);
        //..connect signals..//

        //! [Content request]
        Q_UNUSED(contentReply);
    }

    void contentConversion()
    {
        //! [Content conversion]
        QPlaceImage image;
        image.setUrl(QUrl("www.example.com"));

        QPlaceContent content = image;

        QPlaceImage image2;
        image2 = content;
        qDebug() << image2.url(); //image2.url() == "www.example.com"
        //! [Content conversion]
    }

    void icon() {
        QPlace place;
        //! [icon]
        QUrl iconSourceUrl = place.icon().url(QSize(32,32));

        //A default icon may also be requested like so
        iconSourceUrl = place.icon().url();
        //! [icon]
    }

    void saveBetweenManagers()  {
        QPlaceResult result;
        QPlaceIdReply *saveReply;
        //! [ Save to different manager]
        //result retrieved from a different manager)
        QPlace place = manager->compatiblePlace(result.place());
        saveReply = manager->savePlace(place);
        //! [ Save to different manager]
        saveReply->abort();//needed to avoid warnings
    }

    void ratings() {
        //! [Ratings]
        qDebug() << QString("This place rated ") + place.ratings().average()
                    + "out of " + place.ratings().maximum() + "stars";
        //! [Ratings]
    }

    void matchPlaces() {
        QList<QPlaceSearchResult> results;
        //! [Match places]
        QPlaceMatchRequest request;
        request.setResults(results);
        QVariantMap parameters;
        parameters.insert(QPlaceMatchRequest::AlternativeId, "x_id_here");
        request.setParameters(parameters);
        matchReply = manager->matchingPlaces(request);
        //! [Match places]
    }

public slots:
    // ![Simple search handler]
    //4) Have the slot appropriately process the results of the operation
    void processSearchReply() {
        if (searchReply->error() == QPlaceReply::NoError) {
            for (const QPlaceSearchResult &result : searchReply->results()) {
                if (result.type() == QPlaceSearchResult::PlaceResult)
                    qDebug() << "Title:" << result.title();
            }
        }

    //5) Discard the rely object when done.
        searchReply->deleteLater();
        searchReply = nullptr;
    }
    // ![Simple search handler]

    //! [Search for places handler cpp]
    void handleSearchReply() {
        if (searchReply->error() == QPlaceReply::NoError) {
            for (const QPlaceSearchResult &result : searchReply->results()) {
                if (result.type() == QPlaceSearchResult::PlaceResult) {
                    QPlaceResult placeResult = result;
                    qDebug() << "Name: " << placeResult.place().name();
                    qDebug() << "Coordinate " << placeResult.place().location().coordinate().toString();
                    qDebug() << "Street: " << placeResult.place().location().address().street();
                    qDebug() << "Distance: " << placeResult.distance();
                }
            }
        }
        searchReply->deleteLater();  //discard reply
        searchReply = nullptr;
    }
    //! [Search for places handler cpp]

    //! [Details handler cpp]
    void handleDetailsReply() {
        QPlace place;
        if (detailsReply->error() == QPlaceReply::NoError)
            place = detailsReply->place();

        detailsReply->deleteLater(); //discard reply
        detailsReply = nullptr;
    }
    //! [Details handler cpp]

    //! [Image handler]
    void handleImagesReply() {
        if (contentReply->error() == QPlaceReply::NoError) {
            const auto content = contentReply->content();
            for (auto iter = content.cbegin(), end = content.cend(); iter != end; ++iter) {
                qDebug() << "Index: " << iter.key();
                QPlaceImage image = iter.value();
                qDebug() << image.url();
                qDebug() << image.mimeType();
            }

            //alternatively if indexes are irrelevant
            for (const QPlaceImage &image : contentReply->content()) {
                qDebug() << image.url();
                qDebug() << image.mimeType();
            }

            //we can assign content to the place that it belongs to.
            //the place object serves as a container where we can retrieve
            //content that has already been fetched
            place.insertContent(contentReply->request().contentType(), contentReply->content());
            place.setTotalContentCount(contentReply->request().contentType(), contentReply->totalCount());
        }

        contentReply->deleteLater();
        contentReply = nullptr;
    }
    //! [Image handler]

    //! [Suggestion handler]
    void handleSuggestionReply() {
        if (suggestionReply->error() == QPlaceReply::NoError) {
            for (const QString &suggestion : suggestionReply->suggestions())
                qDebug() << suggestion;
        }

        suggestionReply->deleteLater(); //discard reply
        suggestionReply = nullptr;
    }

    //! [Suggestion handler]

    //! [Save place handler]
    void handleSavePlaceReply() {
        if (savePlaceReply->error() == QPlaceReply::NoError)
            qDebug() << savePlaceReply->id();

        savePlaceReply->deleteLater(); //discard reply
        savePlaceReply = nullptr;
    }
    //! [Save place handler]

    //! [Remove place handler]
    void handleRemovePlaceReply() {
        if (removePlaceReply->error() == QPlaceReply::NoError)
            qDebug() << "Removal of place identified by"
                     << removePlaceReply->id() << "was successful";

        removePlaceReply->deleteLater(); //discard reply
        removePlaceReply = nullptr;
    }
    //! [Remove place handler]

    //! [Initialize categories reply]
    void handleInitCatReply() {
        if (initCatReply->error() == QPlaceReply::NoError)
            qDebug() << "Categories initialized";
        else
            qDebug() << "Failed to initialize categories";

        initCatReply->deleteLater();
        initCatReply = nullptr;
    }
    //! [Initialize categories reply]

    void categories() {
        QPlaceCategory pizza;
        //! [Top level categories]
        const QList<QPlaceCategory> topLevelCategories = manager->childCategories();
        for (const QPlaceCategory &category : topLevelCategories)
            qDebug() << category.name();
        //! [Top level categories]

        //! [Child categories]
        QList<QPlaceCategory> childCategories = manager->childCategories(pizza.categoryId());
        //! [Child categories]
    }

    //! [Save category handler]
    void handleSaveCategoryReply() {
        if (saveCategoryReply->error() == QPlaceReply::NoError) {
            qDebug() << "Saved category id =" << saveCategoryReply->id();
        }

        saveCategoryReply->deleteLater();
        saveCategoryReply = nullptr;
    }
    //! [Save category handler]

    //! [Remove category handler]
    void handleRemoveCategoryReply() {
        if (removeCategoryReply->error() == QPlaceReply::NoError)
            qDebug() << "Removal of category identified by"
                     << removeCategoryReply->id() << "was successful";

        removeCategoryReply->deleteLater(); //discard reply
        removeCategoryReply = nullptr;
    }
    //! [Remove category handler]

    //! [Content handler]
    void contentHandler() {
        if (contentReply->error() == QPlaceReply::NoError)  {
            place.insertContent(contentReply->request().contentType(),
                                contentReply->content());
        }
    }
    //! [Content handler]

    void phoneNumbers() {
        //! [Phone numbers]
        if (place.contactTypes().contains(QPlaceContactDetail::Phone)) {
            for (const QPlaceContactDetail &number : place.contactDetails(QPlaceContactDetail::Phone))
                qDebug() << number.label() << ":" << number.value();
        }
        //! [Phone numbers]
    }


    void openingHours() {
        //! [Opening hours]
        if (place.extendedAttributeTypes().contains(QPlaceAttribute::OpeningHours))
            qDebug() << place.extendedAttribute(QPlaceAttribute::OpeningHours).text();
        //! [Opening hours]
    }

    //! [Match places handler]
    void matchHandler() {
        if (matchReply->error() == QPlaceReply::NoError) {
            const auto places = matchReply->places();
            for (const QPlace &place : places) {
                if (place != QPlace())
                    qDebug() << "Place is a favorite with name" << place.name();
                else
                    qDebug() << "Place is not a favorite";
            }
        }

        matchReply->deleteLater();
        matchReply = nullptr;
    }
    //! [Match places handler]

    void convertSearchResult() {
        QPlaceSearchResult result;
        //! [Convert search result]
        if (result.type() == QPlaceSearchResult::PlaceResult) {
            QPlaceResult placeResult = result;
            qDebug() << placeResult.place().name();
            qDebug() << placeResult.place().location().coordinate();
            qDebug() << placeResult.distance();
        }
        //! [Convert search result]
    }

QPlaceSearchReply *searchReply;
QPlaceManager *manager;
QPlaceDetailsReply *detailsReply;
QPlaceContentReply *contentReply;
QPlaceSearchSuggestionReply *suggestionReply;
QPlaceIdReply *savePlaceReply;
QPlaceIdReply *removePlaceReply;
QPlaceIdReply *saveCategoryReply;
QPlaceIdReply *removeCategoryReply;
QPlaceReply *initCatReply;
QPlaceMatchReply *matchReply;
QPlace place;
};

class ManagerEngine : public QObject
{
};

//! [Implement reply pt1]
class SearchReply : public QPlaceSearchReply
{
public:
    explicit SearchReply(ManagerEngine *engine)
        : QPlaceSearchReply(engine), m_engine(engine){}

    ~SearchReply();
    void setResults(const QList<QPlaceSearchResult> &results);
    void setRequest(const QPlaceSearchRequest &request);
//! [Implement reply pt1]

//! [Implement reply pt2]
    void triggerDone(QPlaceReply::Error error = QPlaceReply::NoError,
                     const QString &errorString = QString());

    ManagerEngine *m_engine;
};
//! [Implement reply pt2]

class SearchSuggestionReply : public QPlaceSearchSuggestionReply
{
public:
    void triggerDone(QPlaceReply::Error error = QPlaceReply::NoError,
                     const QString &errorString = QString());

    ManagerEngine *m_engine;

};

//! [Trigger done]
void SearchSuggestionReply::triggerDone(QPlaceReply::Error error,
                         const QString &errorString)
{
    if (error != QPlaceReply::NoError) {
        this->setError(error,errorString);
        QMetaObject::invokeMethod(m_engine, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *,this),
                                  Q_ARG(QPlaceReply::Error, error),
                                  Q_ARG(QString, errorString));
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error),
                                  Q_ARG(QString, errorString));
    }

    this->setFinished(true);
    QMetaObject::invokeMethod(m_engine, "finished", Qt::QueuedConnection,
                              Q_ARG(QPlaceReply *,this));
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}
//! [Trigger done]
