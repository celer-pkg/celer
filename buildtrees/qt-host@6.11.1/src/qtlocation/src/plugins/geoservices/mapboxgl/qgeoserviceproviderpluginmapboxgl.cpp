// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoserviceproviderpluginmapboxgl.h"
#include "qgeomappingmanagerenginemapboxgl.h"

#include <QtGui/QOpenGLContext>

QT_BEGIN_NAMESPACE

QGeoServiceProviderFactoryMapboxGL::QGeoServiceProviderFactoryMapboxGL()
{
}

QGeoCodingManagerEngine *QGeoServiceProviderFactoryMapboxGL::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryMapboxGL::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoMappingManagerEngineMapboxGL(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryMapboxGL::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

QPlaceManagerEngine *QGeoServiceProviderFactoryMapboxGL::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

QT_END_NAMESPACE
