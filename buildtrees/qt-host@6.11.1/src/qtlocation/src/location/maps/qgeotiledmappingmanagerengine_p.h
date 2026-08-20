// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEDMAPPINGMANAGERENGINE_H
#define QGEOTILEDMAPPINGMANAGERENGINE_H

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

#include <QObject>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qabstractgeotilecache_p.h>
#include <QtLocation/private/qgeomappingmanagerengine_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>


QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEnginePrivate;
class QGeoTileFetcher;
struct QGeoTileTexture;
class QGeoTileSpec;
class QSize;

class Q_LOCATION_EXPORT QGeoTiledMappingManagerEngine : public QGeoMappingManagerEngine
{
    Q_OBJECT

public:
    explicit QGeoTiledMappingManagerEngine(QObject *parent = nullptr);
    virtual ~QGeoTiledMappingManagerEngine();

    QGeoTileFetcher *tileFetcher();

    QGeoMap *createMap() override;
    void releaseMap(QGeoTiledMap *map);

    QSize tileSize() const;
    int tileVersion() const;

    virtual void updateTileRequests(QGeoTiledMap *map,
                            const QSet<QGeoTileSpec> &tilesAdded,
                            const QSet<QGeoTileSpec> &tilesRemoved);

    QAbstractGeoTileCache *tileCache();
    virtual QSharedPointer<QGeoTileTexture> getTileTexture(const QGeoTileSpec &spec);

    QAbstractGeoTileCache::CacheAreas cacheHint() const;

protected Q_SLOTS:
    virtual void engineTileFinished(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format);
    virtual void engineTileError(const QGeoTileSpec &spec, const QString &errorString);

Q_SIGNALS:
    void tileError(const QGeoTileSpec &spec, const QString &errorString);
    void tileVersionChanged();

protected:
    void setTileFetcher(QGeoTileFetcher *fetcher);
    void setTileSize(const QSize &tileSize);
    void setTileVersion(int version);
    void setCacheHint(QAbstractGeoTileCache::CacheAreas cacheHint);
    void setTileCache(QAbstractGeoTileCache *cache);

    QGeoTiledMap::PrefetchStyle m_prefetchStyle = QGeoTiledMap::PrefetchTwoNeighbourLayers;
    QGeoTiledMappingManagerEnginePrivate *d_ptr;

    Q_DECLARE_PRIVATE(QGeoTiledMappingManagerEngine)
    Q_DISABLE_COPY(QGeoTiledMappingManagerEngine)

    friend class QGeoTileFetcher;
};

QT_END_NAMESPACE

#endif
