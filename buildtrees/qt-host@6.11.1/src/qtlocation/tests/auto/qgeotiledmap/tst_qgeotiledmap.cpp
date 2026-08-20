// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qgeotiledmap_test.h"
#include "qgeotilefetcher_test.h"
#include "qgeotiledmappingmanagerengine_test.h"
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/private/qwebmercator_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/private/qgeotiledmap_p.h>
#include <QtLocation/private/qgeomappingmanager_p.h>
#include <QtLocation/private/qgeocameradata_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QGeoTiledMap::PrefetchStyle)

class FetchTileCounter: public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void tileFetched(const QGeoTileSpec& spec) {
        m_tiles << spec;
    }
public:
    QSet<QGeoTileSpec> m_tiles;
};

class tst_QGeoTiledMap : public QObject
{
    Q_OBJECT

public:
    tst_QGeoTiledMap();
    ~tst_QGeoTiledMap();

private:
    void waitForFetch(int count);

private Q_SLOTS:
    void initTestCase();
    void fetchTiles();
    void fetchTiles_data();

private:
    std::unique_ptr<QGeoServiceProvider> m_provider;
    std::unique_ptr<QGeoTiledMapTest> m_map;
    std::unique_ptr<FetchTileCounter> m_tilesCounter;
    QGeoTileFetcherTest *m_fetcher;

};

tst_QGeoTiledMap::tst_QGeoTiledMap():
    m_fetcher(0)
{
}

tst_QGeoTiledMap::~tst_QGeoTiledMap()
{
}

void tst_QGeoTiledMap::initTestCase()
{
#if QT_CONFIG(library)
    // Set custom path since CI doesn't install test plugins
#ifdef Q_OS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../../plugins"));
#else
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../plugins"));
#endif
#endif
      QVariantMap parameters;
      parameters["tileSize"] = 256;
      parameters["maxZoomLevel"] = 8;
      parameters["finishRequestImmediately"] = true;
      m_provider = std::make_unique<QGeoServiceProvider>("qmlgeo.test.plugin", parameters);
      m_provider->setAllowExperimental(true);
      QGeoMappingManager *mappingManager = m_provider->mappingManager();
      QVERIFY2(m_provider->error() == QGeoServiceProvider::NoError,
               "Could not load plugin: " + m_provider->errorString().toLatin1());
      m_map.reset(static_cast<QGeoTiledMapTest*>(mappingManager->createMap(this)));
      QVERIFY(m_map);
      m_map->setViewportSize(QSize(256, 256));
      m_map->setActiveMapType(m_map->m_engine->supportedMapTypes().first());
      m_fetcher = static_cast<QGeoTileFetcherTest*>(m_map->m_engine->tileFetcher());
      m_tilesCounter.reset(new FetchTileCounter());
      connect(m_fetcher, SIGNAL(tileFetched(QGeoTileSpec)),
              m_tilesCounter.get(), SLOT(tileFetched(QGeoTileSpec)));
}

void tst_QGeoTiledMap::fetchTiles()
{
    QFETCH(double, zoomLevel);
    QFETCH(int, visibleCount);
    QFETCH(int, prefetchCount);
    QFETCH(QGeoTiledMap::PrefetchStyle, style);
    QFETCH(int, nearestNeighbourLayer);

    m_map->setPrefetchStyle(style);

    QGeoCameraData camera;
    camera.setCenter(QWebMercator::mercatorToCoord(QDoubleVector2D( 0.5 ,  0.5 )));

    //prev_visible
    camera.setZoomLevel(zoomLevel-1);
    // Delay needed on slow targets (e.g. Qemu)
    QTest::qWait(10);
    m_map->clearData();
    m_tilesCounter->m_tiles.clear();
    m_map->setCameraData(camera);
    waitForFetch(visibleCount);
    QSet<QGeoTileSpec> prev_visible = m_tilesCounter->m_tiles;

    //visible + prefetch
    camera.setZoomLevel(zoomLevel);
    // Delay needed on slow targets (e.g. Qemu)
    QTest::qWait(10);
    m_map->clearData();
    m_tilesCounter->m_tiles.clear();
    m_map->setCameraData(camera);
    waitForFetch(visibleCount);
    const QSet<QGeoTileSpec> visible = m_tilesCounter->m_tiles;
    m_map->clearData();
    m_tilesCounter->m_tiles.clear();
    m_map->prefetchData();
    waitForFetch(prefetchCount);
    QSet<QGeoTileSpec> prefetched = m_tilesCounter->m_tiles;

    //next visible
    camera.setZoomLevel(zoomLevel + 1);
    // Delay needed on slow targets (e.g. Qemu)
    QTest::qWait(10);
    m_map->clearData();
    m_tilesCounter->m_tiles.clear();
    m_map->setCameraData(camera);
    waitForFetch(visibleCount);
    QSet<QGeoTileSpec> next_visible = m_tilesCounter->m_tiles;

    QVERIFY2(visibleCount == visible.size(), "visible count incorrect");
    QVERIFY2(prefetchCount == prefetched.size(), "prefetch count incorrect");
    for (const QGeoTileSpec &tile : visible)
        QVERIFY2(prefetched.contains(tile),"visible tile missing from prefetched tiles");

    //for zoomLevels wihtout fractions more tiles are fetched for current zoomlevel due to ViewExpansion
    if (qCeil(zoomLevel) != zoomLevel && style == QGeoTiledMap::PrefetchNeighbourLayer && nearestNeighbourLayer < zoomLevel)
        QVERIFY2(prefetched == prev_visible + visible, "wrongly prefetched tiles");

    if (qCeil(zoomLevel) != zoomLevel && style == QGeoTiledMap::PrefetchNeighbourLayer && nearestNeighbourLayer > zoomLevel)
        QVERIFY2(prefetched == next_visible + visible, "wrongly prefetched tiles");

    if (qCeil(zoomLevel) != zoomLevel && style == QGeoTiledMap::PrefetchTwoNeighbourLayers)
        QVERIFY2(prefetched == prev_visible + visible + next_visible, "wrongly prefetched tiles");
}

void tst_QGeoTiledMap::fetchTiles_data()
{
    QTest::addColumn<double>("zoomLevel");
    QTest::addColumn<int>("visibleCount");
    QTest::addColumn<int>("prefetchCount");
    QTest::addColumn<QGeoTiledMap::PrefetchStyle>("style");
    QTest::addColumn<int>("nearestNeighbourLayer");
    QTest::newRow("zoomLevel: 4 , visible count: 4 : prefetch count: 16") << 4.0 << 4 << 4 + 16 << QGeoTiledMap::PrefetchNeighbourLayer << 3;
    QTest::newRow("zoomLevel: 4.06 , visible count: 4 : prefetch count: 4") << 4.06 << 4 << 4 + 4 << QGeoTiledMap::PrefetchNeighbourLayer << 3;
    QTest::newRow("zoomLevel: 4.1 , visible count: 4 : prefetch count: 4") << 4.1 << 4 << 4 + 4 << QGeoTiledMap::PrefetchNeighbourLayer << 3;
    QTest::newRow("zoomLevel: 4.5 , visible count: 4 : prefetch count: 4") << 4.5 << 4 << 4 + 4 << QGeoTiledMap::PrefetchNeighbourLayer << 3;
    QTest::newRow("zoomLevel: 4.6 , visible count: 4 : prefetch count: 4") << 4.6 << 4 << 4 + 4 << QGeoTiledMap::PrefetchNeighbourLayer << 5;
    QTest::newRow("zoomLevel: 4.9 , visible count: 4 : prefetch count: 4") << 4.9 << 4 <<4 + 4 << QGeoTiledMap::PrefetchNeighbourLayer << 5;
    QTest::newRow("zoomLevel: 4 , visible count: 4 : prefetch count: 4") << 4.0 << 4 << 16 + 4 + 4 << QGeoTiledMap::PrefetchTwoNeighbourLayers << 3;
    QTest::newRow("zoomLevel: 4.1 , visible count: 4 : prefetch count: 4") << 4.1 << 4 << 4 + 4 + 4 << QGeoTiledMap::PrefetchTwoNeighbourLayers << 3;
    QTest::newRow("zoomLevel: 4.6 ,visible count: 4 : prefetch count: 4") << 4.6 << 4 << 4 + 4  + 4 << QGeoTiledMap::PrefetchTwoNeighbourLayers << 5;
}

void tst_QGeoTiledMap::waitForFetch(int count)
{
    int timeout = 0;
    while (m_tilesCounter->m_tiles.count() < count && timeout < count) {
        //250ms for each tile fetch
        QTest::qWait(250);
        timeout++;
    }
}

QTEST_MAIN(tst_QGeoTiledMap)

#include "tst_qgeotiledmap.moc"
