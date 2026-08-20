// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEDMAPOSM_H
#define QGEOTILEDMAPOSM_H

#include "qgeotileproviderosm.h"

#include <QtLocation/private/qgeotiledmap_p.h>
typedef QGeoTiledMap Map;

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineOsm;
class QGeoTiledMapOsm: public Map
{
    Q_OBJECT

public:
    QGeoTiledMapOsm(QGeoTiledMappingManagerEngineOsm *engine, QObject *parent = nullptr);
    ~QGeoTiledMapOsm();

protected:
    void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles) override;

protected Q_SLOTS:
    void onProviderDataUpdated(const QGeoTileProviderOsm *provider);

private:
    int m_mapId;
    QGeoTiledMappingManagerEngineOsm *m_engine;
};

QT_END_NAMESPACE

#endif
