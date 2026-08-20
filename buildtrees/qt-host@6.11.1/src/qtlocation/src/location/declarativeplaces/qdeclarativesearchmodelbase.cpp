// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativesearchmodelbase_p.h"
#include "qdeclarativeplace_p.h"
#include "error_messages_p.h"

#include <QtCore/QCoreApplication>
#include <QtQml/QQmlInfo>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QPlaceIcon>
#include <QtLocation/QPlaceManager>
#include <QtLocation/QPlaceSearchRequest>
#include <QtLocation/QPlaceSearchReply>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoPolygon>
#include <QtLocation/private/qplacesearchrequest_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeSearchModelBase::QDeclarativeSearchModelBase(QObject *parent)
    : QAbstractListModel(parent)
{
}

QDeclarativeSearchModelBase::~QDeclarativeSearchModelBase()
{
}

/*!
    \internal
*/
QDeclarativeGeoServiceProvider *QDeclarativeSearchModelBase::plugin() const
{
    return m_plugin;
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (m_plugin == plugin)
        return;

    initializePlugin(plugin);

    if (m_complete)
        emit pluginChanged();
}

/*!
    \internal
*/
QVariant QDeclarativeSearchModelBase::searchArea() const
{
    QGeoShape s = m_request.searchArea();
    if (s.type() == QGeoShape::RectangleType)
        return QVariant::fromValue(QGeoRectangle(s));
    else if (s.type() == QGeoShape::CircleType)
        return QVariant::fromValue(QGeoCircle(s));
    else if (s.type() == QGeoShape::PolygonType)
        return QVariant::fromValue(QGeoPolygon(s));
    else
        return QVariant::fromValue(s);
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::setSearchArea(const QVariant &searchArea)
{
    QGeoShape s;
    QGeoRoute route;
    bool routeSearchArea = false;
    if (searchArea.userType() == qMetaTypeId<QGeoRectangle>()) {
        s = searchArea.value<QGeoRectangle>();
    } else if (searchArea.userType() == qMetaTypeId<QGeoCircle>()) {
        s = searchArea.value<QGeoCircle>();
    } else if (searchArea.userType() == qMetaTypeId<QGeoShape>()) {
        s = searchArea.value<QGeoShape>();
    } else if (searchArea.typeId() == qMetaTypeId<QGeoRoute>()) {
        route = searchArea.value<QGeoRoute>();
        if (route == QGeoRoute())
            return;
        routeSearchArea = true;
    }

    QPlaceSearchRequestPrivate *rp = QPlaceSearchRequestPrivate::get(m_request);
    // Invalidating the other thing
    if (routeSearchArea)
        m_request.setSearchArea(QGeoShape());
    else
        rp->routeSearchArea = QGeoRoute();

    if (m_request.searchArea() == s
        && (route == QGeoRoute() || rp->routeSearchArea == route)) {
        return;
    }

    if (routeSearchArea)
        rp->routeSearchArea = route;
    else
        m_request.setSearchArea(s);
    emit searchAreaChanged();
}

/*!
    \internal
*/
int QDeclarativeSearchModelBase::limit() const
{
    return m_request.limit();
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::setLimit(int limit)
{
    if (m_request.limit() == limit)
        return;

    m_request.setLimit(limit);
    emit limitChanged();
}

/*!
    \internal
*/
bool QDeclarativeSearchModelBase::previousPagesAvailable() const
{
    return m_previousPageRequest != QPlaceSearchRequest();
}

/*!
    \internal
*/
bool QDeclarativeSearchModelBase::nextPagesAvailable() const
{
    return m_nextPageRequest != QPlaceSearchRequest();
}

/*!
    \internal
*/
QDeclarativeSearchModelBase::Status QDeclarativeSearchModelBase::status() const
{
    return m_status;
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::setStatus(Status status, const QString &errorString)
{
    Status prevStatus = m_status;

    m_status = status;
    m_errorString = errorString;

    if (prevStatus != m_status)
        emit statusChanged();
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::update()
{
    if (m_reply)
        return;

    setStatus(Loading);

    if (!m_plugin) {
        clearData();
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_PROPERTY_NOT_SET));
        return;
    }

    QGeoServiceProvider *serviceProvider = m_plugin->sharedGeoServiceProvider();
    if (!serviceProvider) {
        clearData();
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_PROVIDER_ERROR)
                         .arg(m_plugin->name()));
        return;
    }

    QPlaceManager *placeManager = serviceProvider->placeManager();
    if (!placeManager) {
        clearData();
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, PLUGIN_ERROR)
                         .arg(m_plugin->name()).arg(serviceProvider->errorString()));
        return;
    }

    m_reply = sendQuery(placeManager, m_request);
    if (!m_reply) {
        clearData();
        setStatus(Error, QCoreApplication::translate(CONTEXT_NAME, UNABLE_TO_MAKE_REQUEST));
        return;
    }

    m_reply->setParent(this);
    connect(m_reply, &QPlaceReply::finished,
            this, &QDeclarativeSearchModelBase::queryFinished);
    connect(m_reply, &QPlaceReply::contentUpdated,
            this, &QDeclarativeSearchModelBase::onContentUpdated);
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::cancel()
{
    if (!m_reply)
        return;

    if (!m_reply->isFinished())
        m_reply->abort();

    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    setStatus(Ready);
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::reset()
{
    beginResetModel();
    clearData();
    setStatus(Null);
    endResetModel();
}

/*!
    \internal
*/
QString QDeclarativeSearchModelBase::errorString() const
{
    return m_errorString;
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::previousPage()
{
    if (m_previousPageRequest == QPlaceSearchRequest())
        return;

    m_request = m_previousPageRequest;
    update();
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::nextPage()
{
    if (m_nextPageRequest == QPlaceSearchRequest())
        return;

    m_request = m_nextPageRequest;
    update();
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::clearData(bool suppressSignal)
{
    Q_UNUSED(suppressSignal);
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::classBegin()
{
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::componentComplete()
{
    m_complete = true;
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::initializePlugin(QDeclarativeGeoServiceProvider *plugin)
{
    beginResetModel();
    if (plugin != m_plugin) {
        if (m_plugin) {
            disconnect(m_plugin, &QDeclarativeGeoServiceProvider::nameChanged,
                       this, &QDeclarativeSearchModelBase::pluginNameChanged);
        }
        if (plugin) {
            connect(plugin, &QDeclarativeGeoServiceProvider::nameChanged,
                    this, &QDeclarativeSearchModelBase::pluginNameChanged);
        }
        m_plugin = plugin;
    }

    if (m_plugin) {
        QGeoServiceProvider *serviceProvider = m_plugin->sharedGeoServiceProvider();
        if (serviceProvider) {
            QPlaceManager *placeManager = serviceProvider->placeManager();
            if (placeManager) {
                if (placeManager->childCategoryIds().isEmpty()) {
                    QPlaceReply *reply = placeManager->initializeCategories();
                    connect(reply, &QPlaceReply::finished, reply, &QObject::deleteLater);
                }
            }
        }
    }

    endResetModel();
}

void QDeclarativeSearchModelBase::onContentUpdated()
{

}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::pluginNameChanged()
{
    initializePlugin(m_plugin);
}

/*!
    \internal
*/
void QDeclarativeSearchModelBase::setPreviousPageRequest(const QPlaceSearchRequest &previous)
{
    if (m_previousPageRequest == previous)
        return;

    m_previousPageRequest = previous;
    emit previousPagesAvailableChanged();
}

void QDeclarativeSearchModelBase::setNextPageRequest(const QPlaceSearchRequest &next)
{
    if (m_nextPageRequest == next)
        return;

    m_nextPageRequest = next;
    emit nextPagesAvailableChanged();
}

QT_END_NAMESPACE
