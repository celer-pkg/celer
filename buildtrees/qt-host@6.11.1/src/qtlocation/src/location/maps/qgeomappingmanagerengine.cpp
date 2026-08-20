// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomappingmanagerengine_p.h"
#include "qgeomappingmanagerengine_p_p.h"
#include "qgeotiledmapreply_p.h"
#include "qgeotilespec_p.h"
#include "qgeocameracapabilities_p.h"
#include "qgeomaptype_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QGeoMappingManagerEngine
    \inmodule QtLocation
    \ingroup QtLocation-impl
    \since 5.6
    \internal

    \brief Provides support functionality for map display with QGeoServiceProvider.

    \note There are no source or binary compatibility guarantees for the
    backend classes. The API is only guaranteed to work with the Qt version it
    was developed against. API changes will however only be made in minor
    releases. (6.6, 6.7, and so on.)

    The QGeoMappingManagerEngine class provides an interface and convenience
    methods to implementors of QGeoServiceProvider plugins who want to
    provide support for displaying and interacting with maps.
*/

/*!
    Constructs a new engine with the specified \a parent.
*/
QGeoMappingManagerEngine::QGeoMappingManagerEngine(QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoMappingManagerEnginePrivate()) {}

/*!
    Destroys this engine.
*/
QGeoMappingManagerEngine::~QGeoMappingManagerEngine()
{
    Q_D(QGeoMappingManagerEngine);
    delete d;
}

/*!
    Marks the engine as initialized. Subclasses of QGeoMappingManagerEngine are to
    call this method after performing implementation-specific initializatioin within
    the constructor.
*/
void QGeoMappingManagerEngine::engineInitialized()
{
    Q_D(QGeoMappingManagerEngine);
    d->initialized = true;
    emit initialized();
}

/*!
    Sets the name which this engine implementation uses to distinguish itself
    from the implementations provided by other plugins to \a managerName.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
void QGeoMappingManagerEngine::setManagerName(const QString &managerName)
{
    d_ptr->managerName = managerName;
}

/*!
    Returns the name which this engine implementation uses to distinguish
    itself from the implementations provided by other plugins.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
QString QGeoMappingManagerEngine::managerName() const
{
    return d_ptr->managerName;
}

/*!
    Sets the version of this engine implementation to \a managerVersion.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
void QGeoMappingManagerEngine::setManagerVersion(int managerVersion)
{
    d_ptr->managerVersion = managerVersion;
}

/*!
    Returns the version of this engine implementation.

    The combination of managerName() and managerVersion() should be unique
    amongst plugin implementations.
*/
int QGeoMappingManagerEngine::managerVersion() const
{
    return d_ptr->managerVersion;
}

QList<QGeoMapType> QGeoMappingManagerEngine::supportedMapTypes() const
{
    Q_D(const QGeoMappingManagerEngine);
    return d->supportedMapTypes;
}

/*!
    Sets the list of map types supported by this engine to \a mapTypes.

    Subclasses of QGeoMappingManagerEngine should use this function to ensure
    that supportedMapTypes() provides accurate information.
*/
void QGeoMappingManagerEngine::setSupportedMapTypes(const QList<QGeoMapType> &supportedMapTypes)
{
    Q_D(QGeoMappingManagerEngine);
    d->supportedMapTypes = supportedMapTypes;
    emit supportedMapTypesChanged();
}

QGeoCameraCapabilities QGeoMappingManagerEngine::cameraCapabilities(int mapId) const
{
    Q_UNUSED(mapId);
    Q_D(const QGeoMappingManagerEngine);

    if (mapId == 0)
        return d->capabilities_;
    int idx = mapId - 1;
    if (idx >= supportedMapTypes().size())
        return d->capabilities_;
    return supportedMapTypes().at(idx).cameraCapabilities();
}

void QGeoMappingManagerEngine::setCameraCapabilities(const QGeoCameraCapabilities &capabilities)
{
    Q_D(QGeoMappingManagerEngine);
    d->capabilities_ = capabilities;
}

/*!
    Return whether the engine has been initialized and is ready to be used.
*/

bool QGeoMappingManagerEngine::isInitialized() const
{
    Q_D(const QGeoMappingManagerEngine);
    return d->initialized;
}

/*!
    Sets the locale to be used by the this manager to \a locale.

    If this mapping manager supports returning map labels
    in different languages, they will be returned in the language of \a locale.

    The locale used defaults to the system locale if this is not set.
*/
void QGeoMappingManagerEngine::setLocale(const QLocale &locale)
{
    d_ptr->locale = locale;
}

/*!
    Returns the locale used to hint to this mapping manager about what
    language to use for map labels.
*/
QLocale QGeoMappingManagerEngine::locale() const
{
    return d_ptr->locale;
}

/*******************************************************************************
*******************************************************************************/

QGeoMappingManagerEnginePrivate::QGeoMappingManagerEnginePrivate() = default;
QGeoMappingManagerEnginePrivate::~QGeoMappingManagerEnginePrivate() = default;

QT_END_NAMESPACE
