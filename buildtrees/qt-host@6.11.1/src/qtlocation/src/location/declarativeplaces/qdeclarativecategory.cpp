// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativecategory_p.h"
#include "qdeclarativegeoserviceprovider_p.h"
#include "error_messages_p.h"

#include <QtQml/QQmlInfo>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QPlaceIcon>
#include <QtLocation/QPlaceManager>
#include <QtLocation/QPlaceIdReply>
#include <QCoreApplication>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Category
    \nativetype QDeclarativeCategory
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-places
    \ingroup qml-QtLocation5-places-data

    \since QtLocation 5.5

    \brief The Category type represents a category that a \l Place can be associated with.

    Categories are used to search for places based on the categories they are associated with.  The
    list of available categories can be obtained from the \l CategoryModel.  The
    \l PlaceSearchModel has a \l {PlaceSearchModel::categories}{categories} property that is used
    to limit the search results to places with the specified categories.

    If the \l Plugin supports it, categories can be created or removed.  To create a new category
    construct a new Category object and set its properties, then invoke the \l save() method.

    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/places.qml Category
    \dots 0
    \snippet declarative/places.qml Category save

    To remove a category ensure that the \l plugin and categoryId properties are set and call the
    \l remove() method.

    \sa CategoryModel
*/

QDeclarativeCategory::QDeclarativeCategory(QObject *parent)
    : QObject(parent)
{
}

QDeclarativeCategory::QDeclarativeCategory(const QPlaceCategory &category,
                                           QDeclarativeGeoServiceProvider *plugin,
                                           QObject *parent)
    : QObject(parent), m_category(category), m_plugin(plugin)
{
    setCategory(category);
}

QDeclarativeCategory::~QDeclarativeCategory() {}

// From QQmlParserStatus
void QDeclarativeCategory::componentComplete()
{
    m_complete = true;
}

/*!
    \qmlproperty Plugin Category::plugin

    This property holds the location based service to which the category belongs.
*/
void QDeclarativeCategory::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (m_plugin == plugin)
        return;

    m_plugin = plugin;
    if (m_complete)
        emit pluginChanged();

    if (!m_plugin)
        return;

    if (m_plugin->isAttached()) {
        pluginReady();
    } else {
        connect(m_plugin, &QDeclarativeGeoServiceProvider::attached,
                this, &QDeclarativeCategory::pluginReady);
    }
}

QDeclarativeGeoServiceProvider *QDeclarativeCategory::plugin() const
{
    return m_plugin;
}

/*!
    \internal
*/
void QDeclarativeCategory::pluginReady()
{
    QGeoServiceProvider *serviceProvider = m_plugin->sharedGeoServiceProvider();
    QPlaceManager *placeManager = serviceProvider->placeManager();
    if (!placeManager || serviceProvider->error() != QGeoServiceProvider::NoError) {
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_ERROR)
                         .arg(m_plugin->name()).arg(serviceProvider->errorString()));
        return;
    }
}


/*!
    \internal
*/
void QDeclarativeCategory::setCategory(const QPlaceCategory &category)
{
    QPlaceCategory previous = m_category;
    m_category = category;

    if (category.name() != previous.name())
        emit nameChanged();

    if (category.categoryId() != previous.categoryId())
        emit categoryIdChanged();

    if (category.icon() != previous.icon())
        emit iconChanged();
}

QPlaceCategory QDeclarativeCategory::category()
{
    return m_category;
}

/*!
    \qmlproperty string Category::categoryId

    This property holds the identifier of the category.  The categoryId is a string which uniquely
    identifies this category within the categories \l plugin.
*/
void QDeclarativeCategory::setCategoryId(const QString &id)
{
    if (m_category.categoryId() != id) {
        m_category.setCategoryId(id);
        emit categoryIdChanged();
    }
}

QString QDeclarativeCategory::categoryId() const
{
    return m_category.categoryId();
}

/*!
    \qmlproperty string Category::name

    This property holds string based name of the category.
*/
void QDeclarativeCategory::setName(const QString &name)
{
    if (m_category.name() != name) {
        m_category.setName(name);
        emit nameChanged();
    }
}

QString QDeclarativeCategory::name() const
{
    return m_category.name();
}

/*!
    \qmlproperty enumeration Category::visibility

    This property holds the visibility of the category.  It can be one of:

    \table
        \row
            \li Category.UnspecifiedVisibility
            \li The visibility of the category is unspecified.  If saving a category, the
            plugin will automatically set a default visibility to the category saved in the backend.
            This default is dependent on the plugin implementation.
        \row
            \li Category.DeviceVisibility
            \li The category is limited to the current device.  The category will not be transferred
               off of the device.
        \row
            \li Category.PrivateVisibility
            \li The category is private to the current user.  The category may be transferred to an
               online service but is only ever visible to the current user.
        \row
            \li Category.PublicVisibility
            \li The category is public.
    \endtable

    Note that visibility does not affect how \l{Place}s associated with
    the category are displayed in the user-interface of an application
    on the device.  Instead, it defines the sharing semantics of the
    category.
*/
QDeclarativeCategory::Visibility QDeclarativeCategory::visibility() const
{
    return static_cast<QDeclarativeCategory::Visibility>(m_category.visibility());
}

void QDeclarativeCategory::setVisibility(Visibility visibility)
{
    if (static_cast<QDeclarativeCategory::Visibility>(m_category.visibility()) == visibility)
        return;

    m_category.setVisibility(static_cast<QLocation::Visibility>(visibility));
    emit visibilityChanged();
}

/*!
    \qmlproperty PlaceIcon Category::icon

    This property holds the image source associated with the category. To display the icon you can use 
    the \l Image type.
*/
QPlaceIcon QDeclarativeCategory::icon() const
{
    return m_category.icon();
}

void QDeclarativeCategory::setIcon(const QPlaceIcon &icon)
{
    if (m_category.icon() != icon) {
        m_category.setIcon(icon);
        emit iconChanged();
    }
}

/*!
    \qmlmethod string Category::errorString()

    Returns a string description of the error of the last operation.
    If the last operation completed successfully then the string is empty.
*/
QString QDeclarativeCategory::errorString() const
{
    return m_errorString;
}

void QDeclarativeCategory::setStatus(Status status, const QString &errorString)
{
    Status originalStatus = m_status;
    m_status = status;
    m_errorString = errorString;

    if (originalStatus != m_status)
        emit statusChanged();
}

/*!
    \qmlproperty enumeration Category::status

    This property holds the status of the category.  It can be one of:

    \table
        \row
            \li Category.Ready
            \li No error occurred during the last operation, further operations may be performed on
               the category.
        \row
            \li Category.Saving
            \li The category is currently being saved, no other operations may be performed until the
               current operation completes.
        \row
            \li Category.Removing
            \li The category is currently being removed, no other operations can be performed until
               the current operation completes.
        \row
            \li Category.Error
            \li An error occurred during the last operation, further operations can still be
               performed on the category.
    \endtable
*/
QDeclarativeCategory::Status QDeclarativeCategory::status() const
{
    return m_status;
}

/*!
    \qmlmethod void Category::save()

    This method saves the category to the backend service.
*/
void QDeclarativeCategory::save(const QString &parentId)
{
    QPlaceManager *placeManager = manager();
    if (!placeManager)
        return;

    m_reply = placeManager->saveCategory(category(), parentId);
    connect(m_reply, &QPlaceReply::finished,
            this, &QDeclarativeCategory::replyFinished);
    setStatus(QDeclarativeCategory::Saving);
}

/*!
    \qmlmethod void Category::remove()

    This method permanently removes the category from the backend service.
*/
void QDeclarativeCategory::remove()
{
    QPlaceManager *placeManager = manager();
    if (!placeManager)
        return;

    m_reply = placeManager->removeCategory(m_category.categoryId());
    connect(m_reply, &QPlaceReply::finished,
            this, &QDeclarativeCategory::replyFinished);
    setStatus(QDeclarativeCategory::Removing);
}

/*!
    \internal
*/
void QDeclarativeCategory::replyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() == QPlaceReply::NoError) {
        switch (m_reply->type()) {
        case (QPlaceReply::IdReply) : {
            QPlaceIdReply *idReply = qobject_cast<QPlaceIdReply *>(m_reply);

            switch (idReply->operationType()) {
            case QPlaceIdReply::SaveCategory:
                setCategoryId(idReply->id());
                break;
            case QPlaceIdReply::RemoveCategory:
                setCategoryId(QString());
                break;
            default:
                //Other operation types shouldn't ever be received.
                break;
            }
            break;
        }
        default:
            //other types of replies shouldn't ever be received.
            break;
        }

        m_errorString.clear();

        m_reply->deleteLater();
        m_reply = nullptr;

        setStatus(QDeclarativeCategory::Ready);
    } else {
        QString errorString = m_reply->errorString();

        m_reply->deleteLater();
        m_reply = nullptr;

        setStatus(QDeclarativeCategory::Error, errorString);
    }
}

/*!
    \internal
    Helper function to return the manager, this manager is intended to be used to perform the next
    operation.  Sets status to Error and an appropriate m_errorString if the manager cannot be
    obtained.
*/
QPlaceManager *QDeclarativeCategory::manager()
{
    if (m_status != QDeclarativeCategory::Ready && m_status != QDeclarativeCategory::Error)
        return nullptr;

    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (!m_plugin) {
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_PROPERTY_NOT_SET));
        return nullptr;
    }

    QGeoServiceProvider *serviceProvider = m_plugin->sharedGeoServiceProvider();
    if (!serviceProvider) {
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_NOT_VALID));
        return nullptr;
    }
    QPlaceManager *placeManager = serviceProvider->placeManager();
    if (!placeManager) {
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_ERROR)
                         .arg(m_plugin->name()).arg(serviceProvider->errorString()));
        return nullptr;
    }

    return placeManager;
}

QT_END_NAMESPACE
