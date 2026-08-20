// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geotiledmap_esri.h"
#include "geotiledmappingmanagerengine_esri.h"

#include <QtLocation/private/qgeotilespec_p.h>

QT_BEGIN_NAMESPACE

GeoTiledMapEsri::GeoTiledMapEsri(GeoTiledMappingManagerEngineEsri *engine, QObject *parent) :
    Map(engine, parent), m_engine(engine), m_mapId(-1)
{
}

GeoTiledMapEsri::~GeoTiledMapEsri()
{
}

void GeoTiledMapEsri::evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles)
{
    if (visibleTiles.isEmpty())
        return;

    QGeoTileSpec tile = *(visibleTiles.constBegin());
    if (tile.mapId() == m_mapId)
        return;

    m_mapId = tile.mapId();

    GeoMapSource *mapSource = engine()->mapSource(m_mapId);

    if (mapSource)
        emit copyrightsChanged(mapSource->copyright());
}

QT_END_NAMESPACE
