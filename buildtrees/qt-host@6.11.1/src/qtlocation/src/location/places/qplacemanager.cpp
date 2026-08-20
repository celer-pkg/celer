// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qplacemanager.h"

#include "qplacemanagerengine.h"
#include "qplacemanagerengine_p.h"

#include <QtCore/QDebug>
#include <QtCore/QLocale>


#include <QtLocation/QPlace>
#include <QtLocation/QPlaceCategory>
#include <QtLocation/QPlaceContentReply>
#include <QtLocation/QPlaceContentRequest>
#include <QtLocation/QPlaceDetailsReply>
#include <QtLocation/QPlaceIcon>
#include <QtLocation/QPlaceIdReply>
#include <QtLocation/QPlaceMatchReply>
#include <QtLocation/QPlaceMatchRequest>
#include <QtLocation/QPlaceReply>
#include <QtLocation/QPlaceSearchSuggestionReply>
#include <QtLocation/QPlaceSearchRequest>
#include <QtLocation/QPlaceSearchResult>

QT_BEGIN_NAMESPACE

/*!
    \class QPlaceManager
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-manager
    \since 5.6

    \brief The QPlaceManager class provides the interface which allows clients to access
    places stored in a particular backend.

    The following table gives an overview of the functionality provided by the QPlaceManager
    \table
        \header
            \li Functionality
            \li Description
        \row
            \li Searching for places
            \li Using set of parameters such as a search term and search area, relevant places
               can be returned to the user.
        \row
            \li Categories
            \li Places can be classified as belonging to different categories.  The
               manager supports access to these categories.
        \row
            \li Search term suggestions
            \li Given a partially complete search term, a list of potential
               search terms can be given.
        \row
            \li Recommendations
            \li Given an existing place, a set of similar recommended places can
               be suggested to the user.
        \row
            \li Rich Content
            \li Rich content such as images, reviews etc can be retrieved in a paged
               fashion.
        \row
            \li Place or Category management
            \li Places and categories may be saved and removed.  It is possible
               for notifications to be given when this happens.
        \row
            \li Localization
            \li Different locales may be specified to return place
               data in different languages.
    \endtable

    \section1 Obtaining a QPlaceManager Instance
    Creation of a QPlaceManager is facilitated by the QGeoServiceProvider.
    See \l {Initializing a manager} for an example on how to create a manager.


    \section1 Asynchronous Interface
    The QPlaceManager class provides an abstraction of the datastore which contains place information.
    The functions provided by the QPlaceManager and primarily asynchronous and follow
    a request-reply model.   Typically a request is given to the manager, consisting
    of a various set of parameters and a reply object is created.  The reply object
    has a signal to notify when the request is done, and once completed, the reply
    contains the results of the request, along with any errors that occurred, if any.

    An asynchronous request is generally handled as follows:
    \snippet places/requesthandler.h Simple search
    \dots
    \dots
    \snippet places/requesthandler.h Simple search handler

    See \l {Common Operations} for a list of examples demonstrating how the QPlaceManger
    is used.

    \section1 Category Initialization
    Sometime during startup of an application, the initializeCategories() function should
    be called to setup the categories.  Initializing the categories enables the usage of the
    following functions:

    \list
        \li QPlaceManager::childCategories()
        \li QPlaceManager::category()
        \li QPlaceManager::parentCategoryId()
        \li QPlaceManager::childCategoryIds();
    \endlist

    If the categories need to be refreshed or reloaded, the initializeCategories() function
    may be called again.

*/

/*!
    Constructs a new manager with the specified \a parent and with the
    implementation provided by \a engine.

    This constructor is used internally by QGeoServiceProviderFactory. Regular
    users should acquire instances of QGeoRoutingManager with
    QGeoServiceProvider::routingManager();
*/
QPlaceManager::QPlaceManager(QPlaceManagerEngine *engine, QObject *parent)
    : QObject(parent), d(engine)
{
    if (d) {
        d->setParent(this);
        d->d_ptr->manager = this;

        qRegisterMetaType<QPlaceCategory>();

        connect(d, &QPlaceManagerEngine::finished,
                this, &QPlaceManager::finished);
        connect(d, &QPlaceManagerEngine::errorOccurred,
                this, &QPlaceManager::errorOccurred);

        connect(d, &QPlaceManagerEngine::placeAdded,
                this, &QPlaceManager::placeAdded, Qt::QueuedConnection);
        connect(d, &QPlaceManagerEngine::placeUpdated,
                this, &QPlaceManager::placeUpdated, Qt::QueuedConnection);
        connect(d, &QPlaceManagerEngine::placeRemoved,
                this, &QPlaceManager::placeRemoved, Qt::QueuedConnection);

        connect(d, &QPlaceManagerEngine::categoryAdded,
                this, &QPlaceManager::categoryAdded);
        connect(d, &QPlaceManagerEngine::categoryUpdated,
                this, &QPlaceManager::categoryUpdated);
        connect(d, &QPlaceManagerEngine::categoryRemoved,
                this, &QPlaceManager::categoryRemoved);
        connect(d, &QPlaceManagerEngine::dataChanged,
                this, &QPlaceManager::dataChanged, Qt::QueuedConnection);
    } else {
        qFatal("The place manager engine that was set for this place manager was NULL.");
    }
}

/*!
    Destroys the manager.  This destructor is used internally by QGeoServiceProvider
    and should never need to be called in application code.
*/
QPlaceManager::~QPlaceManager()
{
    delete d;
}

/*!
    Returns the name of the manager
*/
QString QPlaceManager::managerName() const
{
    return d->managerName();
}

/*!
    Returns the manager version.
*/
int QPlaceManager::managerVersion() const
{
    return d->managerVersion();
}

/*!
    Retrieves a details of place corresponding to the given \a placeId.

    See \l {QML Places API#Fetching Place Details}{Fetching Place Details} for an example of usage.
*/
QPlaceDetailsReply *QPlaceManager::getPlaceDetails(const QString &placeId) const
{
    return d->getPlaceDetails(placeId);
}

/*!
    Retrieves content for a place according to the parameters specified in \a request.

    See \l {Fetching Rich Content} for an example of usage.
*/
QPlaceContentReply *QPlaceManager::getPlaceContent(const QPlaceContentRequest &request) const
{
    return d->getPlaceContent(request);
}

/*!
    Searches for places according to the parameters specified in \a request.

    See \l {Discovery/Search} for an example of usage.
*/
QPlaceSearchReply *QPlaceManager::search(const QPlaceSearchRequest &request) const
{
    return d->search(request);
}

/*!
    Requests a set of search term suggestions  according to the parameters specified in \a request.
    The \a request can hold the incomplete search term, along with other data such
    as a search area to narrow down relevant results.

    See \l {Search Suggestions} for an example of usage.
*/
QPlaceSearchSuggestionReply *QPlaceManager::searchSuggestions(const QPlaceSearchRequest &request) const
{
    return d->searchSuggestions(request);
}

/*!
    Saves a specified \a place.

    See \l {Saving a place cpp} for an example of usage.
*/
QPlaceIdReply *QPlaceManager::savePlace(const QPlace &place)
{
    return d->savePlace(place);
}

/*!
    Removes the place corresponding to \a placeId from the manager.

    See \l {Removing a place cpp} for an example of usage.
*/
QPlaceIdReply *QPlaceManager::removePlace(const QString &placeId)
{
    return d->removePlace(placeId);
}

/*!
    Saves a \a category that is a child of the category specified by \a parentId.
    An empty \a parentId means \a category is saved as a top level category.

    See \l {Saving a category} for an example of usage.
*/
QPlaceIdReply *QPlaceManager::saveCategory(const QPlaceCategory &category, const QString &parentId)
{
    return d->saveCategory(category, parentId);
}

/*!
    Removes the category corresponding to \a categoryId from the manager.

    See \l {Removing a category} for an example of usage.
*/
QPlaceIdReply *QPlaceManager::removeCategory(const QString &categoryId)
{
    return d->removeCategory(categoryId);
}

/*!
    Initializes the categories of the manager.

    See \l {Using Categories} for an example of usage.
*/
QPlaceReply *QPlaceManager::initializeCategories()
{
    return d->initializeCategories();
}

/*!
    Returns the parent category identifier of the category corresponding to \a categoryId.
*/
QString QPlaceManager::parentCategoryId(const QString &categoryId) const
{
    return d->parentCategoryId(categoryId);
}

/*!
    Returns the child category identifiers of the category corresponding to \a parentId.
    If \a parentId is empty then all top level category identifiers are returned.
*/
QStringList QPlaceManager::childCategoryIds(const QString &parentId) const
{
    return d->childCategoryIds(parentId);
}

/*!
    Returns the category corresponding to the given \a categoryId.
*/
QPlaceCategory QPlaceManager::category(const QString &categoryId) const
{
    return d->category(categoryId);
}

/*!
    Returns a list of categories that are children of the category corresponding to \a parentId.
    If \a parentId is empty, all the top level categories are returned.
*/
QList<QPlaceCategory> QPlaceManager::childCategories(const QString &parentId) const
{
    return d->childCategories(parentId);
}

/*!
    Returns a list of preferred locales. The locales are used as a hint to the manager for what language
    place and category details should be returned in.

    If the first specified locale cannot be accommodated, the manager falls back to the next and so forth.
    Some manager backends may not support a set of locales which are rigidly defined.  An arbitrary
    example is that some places in France could have French and English localizations, while
    certain areas in America may only have the English localization available.  In this example,
    the set of supported locales is context dependent on the search location.

    If the manager cannot accommodate any of the preferred locales, the manager falls
    back to using a supported language that is backend specific.

    Support for locales may vary from provider to provider.  For those that do support it,
    by default, the global default locale is set as the manager's only locale.

    For managers that do not support locales, the locale list is always empty.
*/
QList<QLocale> QPlaceManager::locales() const
{
    return d->locales();
}

/*!
    Convenience function which sets the manager's list of preferred locales
    to a single \a locale.
*/
void QPlaceManager::setLocale(const QLocale &locale)
{
    QList<QLocale> locales;
    locales << locale;
    d->setLocales(locales);
}

/*!
    Set the list of preferred \a locales.
*/
void QPlaceManager::setLocales(const QList<QLocale> &locales)
{
    d->setLocales(locales);
}

/*!
    Returns a pruned or modified version of the \a original place
    which is suitable to be saved into this manager.

    Only place details that are supported by this manager is
    present in the modified version.  Manager specific data such
    as the place id, is not copied over from the \a original.
*/
QPlace QPlaceManager::compatiblePlace(const QPlace &original) const
{
    return d->compatiblePlace(original);
}

/*!
    Returns a reply which contains a list of places which correspond/match those
    specified in the \a request.  The places specified in the request come from a
    different manager.
*/
QPlaceMatchReply *QPlaceManager::matchingPlaces(const QPlaceMatchRequest &request) const
{
    return d->matchingPlaces(request);
}

/*!
    \fn void QPlaceManager::finished(QPlaceReply *reply)

    This signal is emitted when \a reply has finished processing.

    If reply->error() equals QPlaceReply::NoError then the processing
    finished successfully.

    This signal and QPlaceReply::finished() will be emitted at the same time.

    \note Do not delete the \a reply object in the slot connected to this signal.
    Use deleteLater() instead.
*/

/*!
    \fn void QPlaceManager::errorOccurred(QPlaceReply *reply, QPlaceReply::Error error, const QString &errorString)

    This signal is emitted when an error has been detected in the processing of
    \a reply.  The QPlaceManager::finished() signal will probably follow.

    The error will be described by the error code \a error.  If \a errorString is
    not empty it will contain a textual description of the error meant for developers
    and not end users.

    This signal and QPlaceReply::errorOccurred() will be emitted at the same time.

    \note Do not delete the \a reply object in the slot connected to this signal.
    Use deleteLater() instead.
*/

/*!
    \fn void QPlaceManager::placeAdded(const QString &placeId)

    This signal is emitted if a place has been added to the manager engine's datastore.
    The particular added place is specified by \a placeId.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
    \sa dataChanged()
*/

/*!
    \fn void QPlaceManager::placeUpdated(const QString &placeId)

    This signal is emitted if a place has been modified in the manager's datastore.
    The particular modified place is specified by \a placeId.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
    \sa dataChanged()
*/

/*!
    \fn void QPlaceManager::placeRemoved(const QString &placeId)

    This signal is emitted if a place has been removed from the manager's datastore.
    The particular place that has been removed is specified by \a placeId.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
    \sa dataChanged()
*/

/*!
    \fn void QPlaceManager::categoryAdded(const QPlaceCategory &category, const QString &parentId)

    This signal is emitted if a \a category has been added to the manager's datastore.
    The parent of the \a category is specified by \a parentId.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
    \sa dataChanged()
*/

/*!
    \fn void QPlaceManager::categoryUpdated(const QPlaceCategory &category, const QString &parentId)

    This signal is emitted if a \a category has been modified in the manager's datastore.
    The parent of the modified category is specified by \a parentId.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
    \sa dataChanged()
*/

/*!
    \fn void QPlaceManager::categoryRemoved(const QString &categoryId, const QString &parentId)

    This signal is emitted when the category corresponding to \a categoryId has
    been removed from the manager's datastore.  The parent of the removed category
    is specified by \a parentId.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
    \sa dataChanged()
*/

/*!
    \fn QPlaceManager::dataChanged()
    This signal is emitted by the manager if there are large scale changes to its
    underlying datastore and the manager considers these changes radical enough
    to require clients to reload all data.

    If the signal is emitted, no other signals will be emitted for the associated changes.

    This signal is only emitted by managers that support the QPlaceManager::NotificationsFeature.
*/

QT_END_NAMESPACE
