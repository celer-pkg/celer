// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoserviceproviderpluginitemsoverlay.h"
#include "qgeomappingmanagerengineitemsoverlay.h"

QT_BEGIN_NAMESPACE

QGeoCodingManagerEngine *QGeoServiceProviderFactoryItemsOverlay::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryItemsOverlay::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoMappingManagerEngineItemsOverlay(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryItemsOverlay::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

QPlaceManagerEngine *QGeoServiceProviderFactoryItemsOverlay::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return nullptr;
}

QT_END_NAMESPACE
