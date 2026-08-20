// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qgeotiledmappingmanagerengineosm.h"
#include "qgeotilefetcherosm.h"
#include "qgeotiledmaposm.h"
#include "qgeofiletilecacheosm.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>
#include <QtLocation/private/qgeofiletilecache_p.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkDiskCache>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QGeoTiledMappingManagerEngineOsm::QGeoTiledMappingManagerEngineOsm(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
:   QGeoTiledMappingManagerEngine()
{
    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(19.0);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(80);
    cameraCaps.setMinimumFieldOfView(20.0);
    cameraCaps.setMaximumFieldOfView(120.0);
    cameraCaps.setOverzoomEnabled(true);
    setCameraCapabilities(cameraCaps);

    setTileSize(QSize(256, 256));

    const QByteArray pluginName = "osm";
    if (parameters.contains(QStringLiteral("osm.mapping.cache.directory"))) {
        m_cacheDirectory = parameters.value(QStringLiteral("osm.mapping.cache.directory")).toString();
    } else {
        // managerName() is not yet set, we have to hardcode the plugin name below
        m_cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String(pluginName);
    }
    QNetworkAccessManager *nmCached = new QNetworkAccessManager(this);
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(m_cacheDirectory + QLatin1String("/providers"));
    diskCache->setMaximumCacheSize(100000000000); // enough to prevent diskCache to fiddle with tile cache. it's anyway used only for providers.
    nmCached->setCache(diskCache);

    QNetworkAccessManager *nm = new QNetworkAccessManager(); // Gets owned by QGeoTileFetcherOsm
    QString domain = QStringLiteral("http://maps-redirect.qt.io/osm/5.8/");
    if (parameters.contains(QStringLiteral("osm.mapping.providersrepository.address"))) {
        QString customAddress = parameters.value(QStringLiteral("osm.mapping.providersrepository.address")).toString();
        // Allowing some malformed addresses
        if (customAddress.indexOf(QStringLiteral(":")) < 0) // defaulting to http:// if no prefix is found
            customAddress = QStringLiteral("http://") + customAddress;
        if (customAddress[customAddress.length()-1] != QLatin1Char('/'))
            customAddress += QLatin1Char('/');
        if (QUrl(customAddress).isValid())
            domain = customAddress;
        else
            qWarning() << "Invalid custom providers repository address: " << customAddress;
    }

    bool highdpi = false;
    if (parameters.contains(QStringLiteral("osm.mapping.highdpi_tiles"))) {
        const QString param = parameters.value(QStringLiteral("osm.mapping.highdpi_tiles")).toString().toLower();
        if (param == "true")
            highdpi = true;
    }

    /* TileProviders setup */
    QList<TileProvider *> providers_street;
    QList<TileProvider *> providers_satellite;
    QList<TileProvider *> providers_cycle;
    QList<TileProvider *> providers_transit;
    QList<TileProvider *> providers_nighttransit;
    QList<TileProvider *> providers_terrain;
    QList<TileProvider *> providers_hiking;
    if (highdpi) {
        providers_street.push_back(new TileProvider(QUrl(domain + "street-hires"_L1), true));
        providers_satellite.push_back(
                new TileProvider(QUrl(domain + "satellite-hires"_L1), true));
        providers_cycle.push_back(new TileProvider(QUrl(domain + "cycle-hires"_L1), true));
        providers_transit.push_back(new TileProvider(QUrl(domain + "transit-hires"_L1), true));
        providers_nighttransit.push_back(
                new TileProvider(QUrl(domain + "night-transit-hires"_L1), true));
        providers_terrain.push_back(new TileProvider(QUrl(domain + "terrain-hires"_L1), true));
        providers_hiking.push_back(new TileProvider(QUrl(domain + "hiking-hires"_L1), true));
    }
    providers_street.push_back(new TileProvider(QUrl(domain + "street"_L1)));
    providers_satellite.push_back(new TileProvider(QUrl(domain + "satellite"_L1)));
    providers_cycle.push_back(new TileProvider(QUrl(domain + "cycle"_L1)));
    providers_transit.push_back(new TileProvider(QUrl(domain + "transit"_L1)));
    providers_nighttransit.push_back(new TileProvider(QUrl(domain + "night-transit"_L1)));
    providers_terrain.push_back(new TileProvider(QUrl(domain + "terrain"_L1)));
    providers_hiking.push_back(new TileProvider(QUrl(domain + "hiking"_L1)));
    // Backups
    const QDateTime defaultTs = QDateTime::fromString(QStringLiteral("2016-06-01T00:00:00"), Qt::ISODate);
    providers_street.push_back(
        new TileProvider(QStringLiteral("http://c.tile.openstreetmap.org/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap.org</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")));
    providers_street.back()->setTimestamp(defaultTs);

    // No available open access satellite backup with satisfactory level of details at the present.

    providers_cycle.push_back(
        new TileProvider(QStringLiteral("http://c.tile.opencyclemap.org/cycle/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.thunderforest.com/'>Thunderforest</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")));
    providers_cycle.back()->setTimestamp(defaultTs);

    providers_transit.push_back(
        new TileProvider(QStringLiteral("http://c.tile2.opencyclemap.org/transport/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.thunderforest.com/'>Thunderforest</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")));
    providers_transit.back()->setTimestamp(defaultTs);

    providers_nighttransit.push_back(
        new TileProvider(QStringLiteral("http://a.tile.thunderforest.com/transport-dark/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.thunderforest.com/'>Thunderforest</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")) );
    providers_nighttransit.back()->setTimestamp(defaultTs);

    providers_terrain.push_back(
        new TileProvider(QStringLiteral("http://a.tile.thunderforest.com/landscape/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.thunderforest.com/'>Thunderforest</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")));
    providers_terrain.back()->setTimestamp(defaultTs);

    providers_hiking.push_back(
        new TileProvider(QStringLiteral("http://a.tile.thunderforest.com/outdoors/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.thunderforest.com/'>Thunderforest</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")));
    providers_hiking.back()->setTimestamp(defaultTs);


    /* QGeoTileProviderOsms setup */
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 1, pluginName, cameraCaps),
            providers_street, cameraCaps ));
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite Map"), tr("Satellite map view in daylight mode"), false, false, 2, pluginName, cameraCaps),
            providers_satellite, cameraCaps ));
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::CycleMap, tr("Cycle Map"), tr("Cycle map view in daylight mode"), false, false, 3, pluginName, cameraCaps),
            providers_cycle, cameraCaps ));
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::TransitMap, tr("Transit Map"), tr("Public transit map view in daylight mode"), false, false, 4, pluginName, cameraCaps),
            providers_transit, cameraCaps ));
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::TransitMap, tr("Night Transit Map"), tr("Public transit map view in night mode"), false, true, 5, pluginName, cameraCaps),
            providers_nighttransit, cameraCaps ));
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain Map"), tr("Terrain map view"), false, false, 6, pluginName, cameraCaps),
            providers_terrain, cameraCaps ));
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::PedestrianMap, tr("Hiking Map"), tr("Hiking map view"), false, false, 7, pluginName, cameraCaps),
            providers_hiking, cameraCaps ));

    if (parameters.contains(QStringLiteral("osm.mapping.custom.host"))
            || parameters.contains(QStringLiteral("osm.mapping.host"))) {
        // Adding a custom provider
        QString tmsServer;
        if (parameters.contains(QStringLiteral("osm.mapping.host")))
            tmsServer = parameters.value(QStringLiteral("osm.mapping.host")).toString();
        if (parameters.contains(QStringLiteral("osm.mapping.custom.host"))) // priority to the new one
            tmsServer = parameters.value(QStringLiteral("osm.mapping.custom.host")).toString();

        QString mapCopyright;
        QString dataCopyright;
        if (parameters.contains(QStringLiteral("osm.mapping.custom.mapcopyright")))
            mapCopyright = parameters.value(QStringLiteral("osm.mapping.custom.mapcopyright")).toString();
        if (parameters.contains(QStringLiteral("osm.mapping.custom.datacopyright")))
            dataCopyright = parameters.value(QStringLiteral("osm.mapping.custom.datacopyright")).toString();

        if (parameters.contains(QStringLiteral("osm.mapping.copyright")))
            m_customCopyright = parameters.value(QStringLiteral("osm.mapping.copyright")).toString();

        if (!tmsServer.contains("%x"))
             tmsServer += QStringLiteral("%z/%x/%y.png");
        m_providers.push_back(
            new QGeoTileProviderOsm( nmCached,
                QGeoMapType(QGeoMapType::CustomMap, tr("Custom URL Map"), tr("Custom url map view set via urlprefix parameter"), false, false, 8, pluginName, cameraCaps),
                { new TileProvider(tmsServer,
                    QStringLiteral("png"),
                    mapCopyright,
                    dataCopyright) }, cameraCaps
                ));

        m_providers.last()->disableRedirection();
   }

    bool disableRedirection = false;
    if (parameters.contains(QStringLiteral("osm.mapping.providersrepository.disabled")))
        disableRedirection = parameters.value(QStringLiteral("osm.mapping.providersrepository.disabled")).toBool();

    for (QGeoTileProviderOsm * provider: std::as_const(m_providers)) {
        // Providers are parented inside QGeoFileTileCacheOsm, as they are used in its destructor.
        if (disableRedirection) {
            provider->disableRedirection();
        } else {
            connect(provider, &QGeoTileProviderOsm::resolutionFinished,
                    this, &QGeoTiledMappingManagerEngineOsm::onProviderResolutionFinished);
            connect(provider, &QGeoTileProviderOsm::resolutionError,
                    this, &QGeoTiledMappingManagerEngineOsm::onProviderResolutionError);
        }
    }
    updateMapTypes();


    /* TILE CACHE */
    if (parameters.contains(QStringLiteral("osm.mapping.offline.directory")))
        m_offlineDirectory = parameters.value(QStringLiteral("osm.mapping.offline.directory")).toString();
    QGeoFileTileCacheOsm *tileCache = new QGeoFileTileCacheOsm(m_providers, m_offlineDirectory, m_cacheDirectory);

    /*
     * Disk cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("osm.mapping.cache.disk.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("osm.mapping.cache.disk.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyDisk(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("osm.mapping.cache.disk.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("osm.mapping.cache.disk.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxDiskUsage(cacheSize);
    }

    /*
     * Memory cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("osm.mapping.cache.memory.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("osm.mapping.cache.memory.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyMemory(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("osm.mapping.cache.memory.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("osm.mapping.cache.memory.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setMaxMemoryUsage(cacheSize);
    }

    /*
     * Texture cache setup -- defaults to ByteSize (old behavior)
     */
    if (parameters.contains(QStringLiteral("osm.mapping.cache.texture.cost_strategy"))) {
        QString cacheStrategy = parameters.value(QStringLiteral("osm.mapping.cache.texture.cost_strategy")).toString().toLower();
        if (cacheStrategy == QLatin1String("bytesize"))
            tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
        else
            tileCache->setCostStrategyTexture(QGeoFileTileCache::Unitary);
    } else {
        tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);
    }
    if (parameters.contains(QStringLiteral("osm.mapping.cache.texture.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("osm.mapping.cache.texture.size")).toString().toInt(&ok);
        if (ok)
            tileCache->setExtraTextureUsage(cacheSize);
    }


    setTileCache(tileCache);


    /* TILE FETCHER */
    QGeoTileFetcherOsm *tileFetcher = new QGeoTileFetcherOsm(m_providers, nm, this);
    if (parameters.contains(QStringLiteral("osm.useragent"))) {
        const QByteArray ua = parameters.value(QStringLiteral("osm.useragent")).toString().toLatin1();
        tileFetcher->setUserAgent(ua);
    }
    setTileFetcher(tileFetcher);

    /* PREFETCHING */
    if (parameters.contains(QStringLiteral("osm.mapping.prefetching_style"))) {
        const QString prefetchingMode = parameters.value(QStringLiteral("osm.mapping.prefetching_style")).toString();
        if (prefetchingMode == QStringLiteral("TwoNeighbourLayers"))
            m_prefetchStyle = QGeoTiledMap::PrefetchTwoNeighbourLayers;
        else if (prefetchingMode == QStringLiteral("OneNeighbourLayer"))
            m_prefetchStyle = QGeoTiledMap::PrefetchNeighbourLayer;
        else if (prefetchingMode == QStringLiteral("NoPrefetching"))
            m_prefetchStyle = QGeoTiledMap::NoPrefetching;
    }

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoTiledMappingManagerEngineOsm::~QGeoTiledMappingManagerEngineOsm()
{
}

QGeoMap *QGeoTiledMappingManagerEngineOsm::createMap()
{
    QGeoTiledMap *map = new QGeoTiledMapOsm(this);
    connect(qobject_cast<QGeoFileTileCacheOsm *>(tileCache()), &QGeoFileTileCacheOsm::mapDataUpdated
            , map, &QGeoTiledMap::clearScene);
    map->setPrefetchStyle(m_prefetchStyle);
    return map;
}

const QList<QGeoTileProviderOsm *> &QGeoTiledMappingManagerEngineOsm::providers()
{
    return m_providers;
}

QString QGeoTiledMappingManagerEngineOsm::customCopyright() const
{
    return m_customCopyright;
}

void QGeoTiledMappingManagerEngineOsm::onProviderResolutionFinished(const QGeoTileProviderOsm *provider)
{
    if (!provider->isResolved())
        return;
    updateMapTypes();
}

void QGeoTiledMappingManagerEngineOsm::onProviderResolutionError(const QGeoTileProviderOsm *provider)
{
    if (!provider->isResolved())
        return;
    updateMapTypes();
}

void QGeoTiledMappingManagerEngineOsm::updateMapTypes()
{
    QList<QGeoMapType> mapTypes;
    for (QGeoTileProviderOsm * provider : m_providers) {
        // assume provider are ok until they have been resolved invalid
        if (!provider->isResolved() || provider->isValid())
            mapTypes << provider->mapType();
    }
    const QList<QGeoMapType> currentlySupportedMapTypes = supportedMapTypes();
    if (currentlySupportedMapTypes != mapTypes)
        // See map type implementations in QGeoTiledMapOsm and QGeoTileFetcherOsm.
        setSupportedMapTypes(mapTypes);
}

QT_END_NAMESPACE
