// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geotiledmappingmanagerengine_esri.h"
#include "geotiledmap_esri.h"
#include "geotilefetcher_esri.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>
#include <QtLocation/private/qgeofiletilecache_p.h>

#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE

static const QString kPrefixEsri(QStringLiteral("esri."));
static const QString kParamUserAgent(kPrefixEsri + QStringLiteral("useragent"));
static const QString kParamToken(kPrefixEsri + QStringLiteral("token"));
static const QString kPrefixMapping(kPrefixEsri + QStringLiteral("mapping."));
static const QString kParamMinimumZoomLevel(kPrefixMapping + QStringLiteral("minimumZoomLevel"));
static const QString kParamMaximumZoomLevel(kPrefixMapping + QStringLiteral("maximumZoomLevel"));

static const QString kPropMapSources(QStringLiteral("mapSources"));
static const QString kPropStyle(QStringLiteral("style"));
static const QString kPropName(QStringLiteral("name"));
static const QString kPropDescription(QStringLiteral("description"));
static const QString kPropMobile(QStringLiteral("mobile"));
static const QString kPropNight(QStringLiteral("night"));
static const QString kPropUrl(QStringLiteral("url"));
static const QString kPropMapId(QStringLiteral("mapId"));
static const QString kPropCopyright(QStringLiteral("copyrightText"));

GeoTiledMappingManagerEngineEsri::GeoTiledMappingManagerEngineEsri(const QVariantMap &parameters,
                                                                   QGeoServiceProvider::Error *error,
                                                                   QString *errorString) :
    QGeoTiledMappingManagerEngine()
{
    QGeoCameraCapabilities cameraCaps;

    double minimumZoomLevel = 0;
    double maximumZoomLevel = 19;

    if (parameters.contains(kParamMinimumZoomLevel))
        minimumZoomLevel = parameters[kParamMinimumZoomLevel].toDouble();

    if (parameters.contains(kParamMaximumZoomLevel))
        maximumZoomLevel = parameters[kParamMaximumZoomLevel].toDouble();

    cameraCaps.setMinimumZoomLevel(minimumZoomLevel);
    cameraCaps.setMaximumZoomLevel(maximumZoomLevel);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(80);
    cameraCaps.setMinimumFieldOfView(20.0);
    cameraCaps.setMaximumFieldOfView(120.0);
    cameraCaps.setOverzoomEnabled(true);
    setCameraCapabilities(cameraCaps);

    setTileSize(QSize(256, 256));

    if (!initializeMapSources(error, errorString, cameraCaps))
        return;

    QList<QGeoMapType> mapTypes;

    for (GeoMapSource *mapSource : m_mapSources) {
        mapTypes << QGeoMapType(
                        mapSource->style(),
                        mapSource->name(),
                        mapSource->description(),
                        mapSource->mobile(),
                        mapSource->night(),
                        mapSource->mapId(),
                        "esri",
                        cameraCaps);
    }

    setSupportedMapTypes(mapTypes);

    GeoTileFetcherEsri *tileFetcher = new GeoTileFetcherEsri(this);

    if (parameters.contains(kParamUserAgent))
        tileFetcher->setUserAgent(parameters.value(kParamUserAgent).toString().toLatin1());

    if (parameters.contains(kParamToken))
        tileFetcher->setToken(parameters.value(kParamToken).toString());

    setTileFetcher(tileFetcher);

    /* TILE CACHE */
    QString cacheDirectory;
    if (parameters.contains(QStringLiteral("esri.mapping.cache.directory"))) {
        cacheDirectory = parameters.value(QStringLiteral("esri.mapping.cache.directory")).toString();
    } else {
        // managerName() is not yet set, we have to hardcode the plugin name below
        cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String("esri");
    }
    QGeoFileTileCache *tileCache = new QGeoFileTileCache(cacheDirectory);

    /*
     * Disk cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("esri.mapping.cache.disk.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("esri.mapping.cache.disk.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyDisk(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("esri.mapping.cache.disk.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("esri.mapping.cache.disk.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxDiskUsage(cacheSize);
    }

    /*
     * Memory cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("esri.mapping.cache.memory.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("esri.mapping.cache.memory.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyMemory(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("esri.mapping.cache.memory.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("esri.mapping.cache.memory.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxMemoryUsage(cacheSize);
    }

    /*
     * Texture cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("esri.mapping.cache.texture.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("esri.mapping.cache.texture.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyTexture(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("esri.mapping.cache.texture.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("esri.mapping.cache.texture.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setExtraTextureUsage(cacheSize);
    }

    /* PREFETCHING */
    if (parameters.contains(QStringLiteral("esri.mapping.prefetching_style"))) {
        const QString prefetchingMode = parameters.value(QStringLiteral("esri.mapping.prefetching_style")).toString();
        if (prefetchingMode == QStringLiteral("TwoNeighbourLayers"))
            m_prefetchStyle = QGeoTiledMap::PrefetchTwoNeighbourLayers;
        else if (prefetchingMode == QStringLiteral("OneNeighbourLayer"))
            m_prefetchStyle = QGeoTiledMap::PrefetchNeighbourLayer;
        else if (prefetchingMode == QStringLiteral("NoPrefetching"))
            m_prefetchStyle = QGeoTiledMap::NoPrefetching;
    }

    setTileCache(tileCache);
    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

GeoTiledMappingManagerEngineEsri::~GeoTiledMappingManagerEngineEsri()
{
    qDeleteAll(m_mapSources);
}

QGeoMap *GeoTiledMappingManagerEngineEsri::createMap()
{
    QGeoTiledMap *map = new GeoTiledMapEsri(this);
    map->setPrefetchStyle(m_prefetchStyle);
    return map;
}

// ${z} = Zoom
// ${x} = X
// ${y} = Y
// ${token} = Token

// template =
// 'https://services.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{{z}}/{{y}}/{{x}}.png'

bool GeoTiledMappingManagerEngineEsri::initializeMapSources(QGeoServiceProvider::Error *error,
                                                            QString *errorString,
                                                            const QGeoCameraCapabilities &cameraCaps)
{
    QFile mapsFile(":/esri/maps.json");

    if (!mapsFile.open(QIODevice::ReadOnly)) {
        *error = QGeoServiceProvider::NotSupportedError;
        *errorString = Q_FUNC_INFO + QStringLiteral("Unable to open: ") + mapsFile.fileName();

        return false;
    }

    QByteArray mapsData = mapsFile.readAll();
    mapsFile.close();

    QJsonParseError parseError;

    QJsonDocument mapsDocument = QJsonDocument::fromJson(mapsData, &parseError);

    if (!mapsDocument.isObject()) {
        *error = QGeoServiceProvider::NotSupportedError;
        *errorString = QString("%1JSON error: %2, offset: %3, details: %4")
                            .arg(Q_FUNC_INFO)
                            .arg((int)parseError.error)
                            .arg(parseError.offset)
                            .arg(parseError.errorString());
        return false;
    }

    QVariantMap maps = mapsDocument.object().toVariantMap();

    QVariantList mapSources = maps["mapSources"].toList();

    for (const QVariant &mapSourceElement : mapSources) {
        QVariantMap mapSource = mapSourceElement.toMap();

        int mapId = m_mapSources.count() + 1;

        m_mapSources << new GeoMapSource(
                            GeoMapSource::mapStyle(mapSource[kPropStyle].toString()),
                            mapSource[kPropName].toString(),
                            mapSource[kPropDescription].toString(),
                            mapSource[kPropMobile].toBool(),
                            mapSource[kPropMapId].toBool(),
                            mapId,
                            GeoMapSource::toFormat(mapSource[kPropUrl].toString()),
                            mapSource[kPropCopyright].toString(),
                            cameraCaps
                            );
    }

    return true;
}

GeoMapSource *GeoTiledMappingManagerEngineEsri::mapSource(int mapId) const
{
    for (GeoMapSource *mapSource : mapSources()) {
        if (mapSource->mapId() == mapId)
            return mapSource;
    }

    return nullptr;
}

QT_END_NAMESPACE
