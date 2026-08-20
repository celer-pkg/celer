// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QGEOTILEDMAPPINGMANAGER_P_H
#define QGEOTILEDMAPPINGMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QSize>
#include <QHash>
#include <QSet>
#include "qgeotiledmappingmanagerengine_p.h"

QT_BEGIN_NAMESPACE

class QGeoTiledMap;
class QAbstractGeoTileCache;
class QGeoTileSpec;
class QGeoTileFetcher;

class QGeoTiledMappingManagerEnginePrivate
{
public:
    QSize tileSize_;
    int m_tileVersion = -1;
    QHash<QGeoTiledMap *, QSet<QGeoTileSpec>> mapHash_;
    QHash<QGeoTileSpec, QSet<QGeoTiledMap *>> tileHash_;
    QAbstractGeoTileCache::CacheAreas cacheHint_ = QAbstractGeoTileCache::AllCaches;
    std::unique_ptr<QAbstractGeoTileCache> tileCache_;
    QGeoTileFetcher *fetcher_ = nullptr;
};

QT_END_NAMESPACE

#endif
