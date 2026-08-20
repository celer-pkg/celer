// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomappingmanager_p.h"
#include "qgeomappingmanager_p_p.h"
#include "qgeomappingmanagerengine_p.h"
#include "qgeotiledmapreply_p.h"
#include "qgeocameracapabilities_p.h"


#include "qgeomap_p.h"

#include <QTimer>
#include <QLocale>

QT_BEGIN_NAMESPACE

/*!
    \class QGeoMappingManager
    \inmodule QtLocation
    \ingroup QtLocation-maps
    \since 5.6
    \internal

    \brief The QGeoMappingManager class provides support for displaying
    and interacting with maps.
*/

/*!
    Constructs a new manager with the specified \a parent and with the
    implementation provided by \a engine.

    This constructor is used internally by QGeoServiceProviderFactory. Regular
    users should acquire instances of QGeoMappingManager with
    QGeoServiceProvider::mappingManager()
*/
QGeoMappingManager::QGeoMappingManager(QGeoMappingManagerEngine *engine, QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoMappingManagerPrivate)
{
    d_ptr->engine = engine;
    if (!d_ptr->engine) {
        qFatal("The mapping manager engine that was set for this mapping manager was NULL.");
    }

    connect(d_ptr->engine, &QGeoMappingManagerEngine::initialized,
            this, &QGeoMappingManager::initialized, Qt::QueuedConnection);

    connect(d_ptr->engine, &QGeoMappingManagerEngine::supportedMapTypesChanged,
            this, &QGeoMappingManager::supportedMapTypesChanged, Qt::QueuedConnection);
}

/*!
    Destroys this mapping manager.
*/
QGeoMappingManager::~QGeoMappingManager()
{
    delete d_ptr;
}

/*!
    \fn void QGeoMappingManager::initialized()

    This signal is emitted when the mapping manager has been initialized
    and is ready to be used.
*/

/*!
    Returns the name of the engine which implements the behaviour of this
    mapping manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
QString QGeoMappingManager::managerName() const
{
    return d_ptr->engine->managerName();
}

/*!
    Returns the version of the engine which implements the behaviour of this
    mapping manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
int QGeoMappingManager::managerVersion() const
{
    return d_ptr->engine->managerVersion();
}

/*!
    Returns a new QGeoMap instance which will be managed by this manager.
*/
QGeoMap *QGeoMappingManager::createMap(QObject *parent)
{
    Q_UNUSED(parent);
    return d_ptr->engine->createMap();
}

QList<QGeoMapType> QGeoMappingManager::supportedMapTypes() const
{
    return d_ptr->engine->supportedMapTypes();
}

/*!
    Return whether the manager has been initialized
    (will be done automatically but may take some time).

*/
bool QGeoMappingManager::isInitialized() const
{
    return d_ptr->engine->isInitialized();
}

/*!
    Sets the locale to be used by the this manager to \a locale.

    If this mapping manager supports returning map labels
    in different languages, they will be returned in the language of \a locale.

    The locale used defaults to the system locale if this is not set.
*/
void QGeoMappingManager::setLocale(const QLocale &locale)
{
    d_ptr->engine->setLocale(locale);
}

/*!
    Returns the locale used to hint to this mapping manager about what
    language to use for map labels.
*/
QLocale QGeoMappingManager::locale() const
{
    return d_ptr->engine->locale();
}

/*******************************************************************************
*******************************************************************************/

QGeoMappingManagerPrivate::QGeoMappingManagerPrivate() = default;

QGeoMappingManagerPrivate::~QGeoMappingManagerPrivate()
{
    delete engine;
}

QT_END_NAMESPACE
