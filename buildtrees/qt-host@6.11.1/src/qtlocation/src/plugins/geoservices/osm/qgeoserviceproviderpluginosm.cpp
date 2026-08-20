// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeoserviceproviderpluginosm.h"
#include "qgeotiledmappingmanagerengineosm.h"
#include "qgeocodingmanagerengineosm.h"
#include "qgeoroutingmanagerengineosm.h"
#include "qplacemanagerengineosm.h"

QT_BEGIN_NAMESPACE

QGeoCodingManagerEngine *QGeoServiceProviderFactoryOsm::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoCodingManagerEngineOsm(parameters, error, errorString);
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryOsm::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoTiledMappingManagerEngineOsm(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryOsm::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoRoutingManagerEngineOsm(parameters, error, errorString);
}

QPlaceManagerEngine *QGeoServiceProviderFactoryOsm::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QPlaceManagerEngineOsm(parameters, error, errorString);
}

QT_END_NAMESPACE
