// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geoserviceproviderfactory_esri.h"
#include "geotiledmappingmanagerengine_esri.h"
#include "geocodingmanagerengine_esri.h"
#include "georoutingmanagerengine_esri.h"
#include "placemanagerengine_esri.h"

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>

QT_BEGIN_NAMESPACE

QGeoCodingManagerEngine *GeoServiceProviderFactoryEsri::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new GeoCodingManagerEngineEsri(parameters, error, errorString);
}

QGeoMappingManagerEngine *GeoServiceProviderFactoryEsri::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new GeoTiledMappingManagerEngineEsri(parameters, error, errorString);
}

QGeoRoutingManagerEngine *GeoServiceProviderFactoryEsri::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    const QString token = parameters.value(QStringLiteral("esri.token")).toString();

    if (!token.isEmpty()) {
        return new GeoRoutingManagerEngineEsri(parameters, error, errorString);
    } else {
        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = tr("Esri plugin requires a 'esri.token' parameter.\n"
                          "Please visit https://developers.arcgis.com/authentication/accessing-arcgis-online-services/");
        return nullptr;
    }
}

QPlaceManagerEngine *GeoServiceProviderFactoryEsri::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new PlaceManagerEngineEsri(parameters, error, errorString);
}

QT_END_NAMESPACE
