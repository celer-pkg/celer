// Copyright (C) 2014 Canonical Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoserviceproviderpluginmapbox.h"
#include "qgeocodingmanagerenginemapbox.h"
#include "qgeotiledmappingmanagerenginemapbox.h"
#include "qgeoroutingmanagerenginemapbox.h"
#include "qplacemanagerenginemapbox.h"

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

QT_BEGIN_NAMESPACE

static inline QString msgAccessTokenParameter()
{
    return QGeoServiceProviderFactoryMapbox::tr("Mapbox plugin requires a 'mapbox.access_token' parameter.\n"
                                                "Please visit https://www.mapbox.com");
}

QGeoCodingManagerEngine *QGeoServiceProviderFactoryMapbox::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    const QString accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    if (!accessToken.isEmpty()) {
        return new QGeoCodingManagerEngineMapbox(parameters, error, errorString);
    } else {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = msgAccessTokenParameter();
        return nullptr;
    }
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryMapbox::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    const QString accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    if (!accessToken.isEmpty()) {
        return new QGeoTiledMappingManagerEngineMapbox(parameters, error, errorString);
    } else {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = msgAccessTokenParameter();
        return nullptr;
    }
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryMapbox::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    const QString accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    if (!accessToken.isEmpty()) {
        return new QGeoRoutingManagerEngineMapbox(parameters, error, errorString);
    } else {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = msgAccessTokenParameter();
        return nullptr;
    }
}

QPlaceManagerEngine *QGeoServiceProviderFactoryMapbox::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    const QString accessToken = parameters.value(QStringLiteral("mapbox.access_token")).toString();

    if (!accessToken.isEmpty()) {
        return new QPlaceManagerEngineMapbox(parameters, error, errorString);
    } else {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = msgAccessTokenParameter();
        return nullptr;
    }
}

QT_END_NAMESPACE
