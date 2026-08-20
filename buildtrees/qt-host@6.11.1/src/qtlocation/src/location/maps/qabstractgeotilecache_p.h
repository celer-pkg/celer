// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QABSTRACTGEOTILECACHE_P_H
#define QABSTRACTGEOTILECACHE_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>

#include <QtCore/QSharedPointer>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QImage>

#include "qgeotilespec_p.h"


QT_BEGIN_NAMESPACE

class QGeoMappingManager;
class QGeoMappingManagerEngine;

class QGeoTile;
class QAbstractGeoTileCache;

class QThread;

/* This is also used in the mapgeometry */
struct QGeoTileTexture
{
    QGeoTileSpec spec;
    QImage image;
    bool textureBound = false;
};

class Q_LOCATION_EXPORT QAbstractGeoTileCache : public QObject
{
    Q_OBJECT
public:
    enum CostStrategy {
        Unitary,
        ByteSize
    };

    enum CacheArea {
        DiskCache = 0x01,
        MemoryCache = 0x02,
        AllCaches = 0xFF
    };
    Q_DECLARE_FLAGS(CacheAreas, CacheArea)

    virtual ~QAbstractGeoTileCache();

    virtual void setMaxDiskUsage(int diskUsage);
    virtual int maxDiskUsage() const;
    virtual int diskUsage() const;

    virtual void setMaxMemoryUsage(int memoryUsage);
    virtual int maxMemoryUsage() const;
    virtual int memoryUsage() const;

    virtual void setMinTextureUsage(int textureUsage) = 0;
    virtual void setExtraTextureUsage(int textureUsage) = 0;
    virtual int maxTextureUsage() const = 0;
    virtual int minTextureUsage() const = 0;
    virtual int textureUsage() const = 0;
    virtual void clearAll() = 0;
    virtual void setCostStrategyDisk(CostStrategy costStrategy) = 0;
    virtual CostStrategy costStrategyDisk() const = 0;
    virtual void setCostStrategyMemory(CostStrategy costStrategy) = 0;
    virtual CostStrategy costStrategyMemory() const = 0;
    virtual void setCostStrategyTexture(CostStrategy costStrategy) = 0;
    virtual CostStrategy costStrategyTexture() const = 0;

    virtual QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec) = 0;

    virtual void insert(const QGeoTileSpec &spec,
                const QByteArray &bytes,
                const QString &format,
                QAbstractGeoTileCache::CacheAreas areas = QAbstractGeoTileCache::AllCaches) = 0;
    virtual void handleError(const QGeoTileSpec &spec, const QString &errorString);
    virtual void init() = 0;

    static QString baseCacheDirectory();
    static QString baseLocationCacheDirectory();

protected:
    QAbstractGeoTileCache(QObject *parent = nullptr);
    virtual void printStats() = 0;

    friend class QGeoTiledMappingManagerEngine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractGeoTileCache::CacheAreas)

QT_END_NAMESPACE

#endif // QABSTRACTGEOTILECACHE_P_H
