// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOFILETILECACHE_P_H
#define QGEOFILETILECACHE_P_H

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

#include <QObject>
#include "qcache3q_p.h"

#include "qabstractgeotilecache_p.h"

QT_BEGIN_NAMESPACE

class QGeoMappingManager;

class QGeoTile;
class QGeoCachedTileMemory;
class QGeoFileTileCache;

class QImage;

/* This would be internal to qgeofiletilecache.cpp except that the eviction
 * policy can't be defined without it being concrete here */
class QGeoCachedTileDisk
{
public:
    ~QGeoCachedTileDisk();

    QGeoTileSpec spec;
    QString filename;
    QString format;
    QGeoFileTileCache *cache = nullptr;
};

/* Custom eviction policy for the disk cache, to avoid deleting all the files
 * when the application closes */
class Q_LOCATION_EXPORT QCache3QTileEvictionPolicy : public QCache3QDefaultEvictionPolicy<QGeoTileSpec,QGeoCachedTileDisk>
{
protected:
    void aboutToBeRemoved(const QGeoTileSpec &key, QSharedPointer<QGeoCachedTileDisk> obj);
    void aboutToBeEvicted(const QGeoTileSpec &key, QSharedPointer<QGeoCachedTileDisk> obj);
};

class Q_LOCATION_EXPORT QGeoFileTileCache : public QAbstractGeoTileCache
{
    Q_OBJECT
public:
    QGeoFileTileCache(const QString &directory = QString(), QObject *parent = nullptr);
    ~QGeoFileTileCache();

    void setMaxDiskUsage(int diskUsage) override;
    int maxDiskUsage() const override;
    int diskUsage() const override;

    void setMaxMemoryUsage(int memoryUsage) override;
    int maxMemoryUsage() const override;
    int memoryUsage() const override;

    void setMinTextureUsage(int textureUsage) override;
    void setExtraTextureUsage(int textureUsage) override;
    int maxTextureUsage() const override;
    int minTextureUsage() const override;
    int textureUsage() const override;
    void clearAll() override;
    void clearMapId(int mapId);
    void setCostStrategyDisk(CostStrategy costStrategy) override;
    CostStrategy costStrategyDisk() const override;
    void setCostStrategyMemory(CostStrategy costStrategy) override;
    CostStrategy costStrategyMemory() const override;
    void setCostStrategyTexture(CostStrategy costStrategy) override;
    CostStrategy costStrategyTexture() const override;


    QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec) override;

    // can be called without a specific tileCache pointer
    static void evictFromDiskCache(QGeoCachedTileDisk *td);
    static void evictFromMemoryCache(QGeoCachedTileMemory *tm);

    void insert(const QGeoTileSpec &spec,
                const QByteArray &bytes,
                const QString &format,
                QAbstractGeoTileCache::CacheAreas areas = QAbstractGeoTileCache::AllCaches) override;

    static QString tileSpecToFilenameDefault(const QGeoTileSpec &spec, const QString &format, const QString &directory);
    static QGeoTileSpec filenameToTileSpecDefault(const QString &filename);

protected:
    void init() override;
    void printStats() override;
    void loadTiles();

    QString directory() const;

    QSharedPointer<QGeoCachedTileDisk> addToDiskCache(const QGeoTileSpec &spec, const QString &filename);
    bool addToDiskCache(const QGeoTileSpec &spec, const QString &filename, const QByteArray &bytes);
    void addToMemoryCache(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format);
    QSharedPointer<QGeoTileTexture> addToTextureCache(const QGeoTileSpec &spec, const QImage &image);
    QSharedPointer<QGeoTileTexture> getFromMemory(const QGeoTileSpec &spec);
    QSharedPointer<QGeoTileTexture> getFromDisk(const QGeoTileSpec &spec);

    virtual bool isTileBogus(const QByteArray &bytes) const;
    virtual QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const;
    virtual QGeoTileSpec filenameToTileSpec(const QString &filename) const;

    QCache3Q<QGeoTileSpec, QGeoCachedTileDisk, QCache3QTileEvictionPolicy> diskCache_;
    QCache3Q<QGeoTileSpec, QGeoCachedTileMemory> memoryCache_;
    QCache3Q<QGeoTileSpec, QGeoTileTexture> textureCache_;

    QString directory_;

    int minTextureUsage_ = 0;
    int extraTextureUsage_ = 0;
    CostStrategy costStrategyDisk_ = ByteSize;
    CostStrategy costStrategyMemory_ = ByteSize;
    CostStrategy costStrategyTexture_ = ByteSize;
    bool isDiskCostSet_ = false;
    bool isMemoryCostSet_ = false;
    bool isTextureCostSet_ = false;
};

QT_END_NAMESPACE

#endif // QGEOFILETILECACHE_P_H
