// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomappingmanagerengineitemsoverlay.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include "qgeomapitemsoverlay.h"

QT_BEGIN_NAMESPACE



QGeoMappingManagerEngineItemsOverlay::QGeoMappingManagerEngineItemsOverlay(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
:   QGeoMappingManagerEngine()
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);
    Q_UNUSED(errorString);
    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(30.0);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(89);
    cameraCaps.setMinimumFieldOfView(1.0);
    cameraCaps.setMaximumFieldOfView(179.0);
    setCameraCapabilities(cameraCaps);

    QList<QGeoMapType> mapTypes;
    mapTypes << QGeoMapType(QGeoMapType::NoMap, tr("Empty Map"), tr("Empty Map"), false, false, 1, "itemsoverlay", cameraCaps);
    setSupportedMapTypes(mapTypes);

    engineInitialized();
}

QGeoMappingManagerEngineItemsOverlay::~QGeoMappingManagerEngineItemsOverlay()
{
}

QGeoMap *QGeoMappingManagerEngineItemsOverlay::createMap()
{
    return new QGeoMapItemsOverlay(this, this);
}

QT_END_NAMESPACE
