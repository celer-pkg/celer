// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOTILEDMAPESRI_H
#define GEOTILEDMAPESRI_H

#include <QtLocation/private/qgeotiledmap_p.h>
typedef QGeoTiledMap Map;

QT_BEGIN_NAMESPACE

class GeoTiledMappingManagerEngineEsri;

class GeoTiledMapEsri: public Map
{
    Q_OBJECT

public:
    explicit GeoTiledMapEsri(GeoTiledMappingManagerEngineEsri *engine, QObject *parent = nullptr);
    virtual ~GeoTiledMapEsri();

protected:
    void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles) override;

    inline GeoTiledMappingManagerEngineEsri *engine() const;

private:
    GeoTiledMappingManagerEngineEsri *m_engine;
    int m_mapId;
};

inline GeoTiledMappingManagerEngineEsri *GeoTiledMapEsri::engine() const
{
    return m_engine;
}

QT_END_NAMESPACE

#endif // GEOTILEDMAPESRI_H
