// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QHeightMapSurfaceDataProxy>
#include <QtGraphs/QSurface3DSeries>

class tst_proxy: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void construct();

    void initialProperties();
    void initializeProperties();
    void invalidProperties();

private:
    QHeightMapSurfaceDataProxy *m_proxy;
    QSurface3DSeries *m_series;
};

void tst_proxy::initTestCase()
{
}

void tst_proxy::cleanupTestCase()
{
}

void tst_proxy::init()
{
    m_proxy = new QHeightMapSurfaceDataProxy();
    m_series = new QSurface3DSeries(m_proxy);
}

void tst_proxy::cleanup()
{
    delete m_series;
    m_proxy = 0;
}

void tst_proxy::construct()
{
    QHeightMapSurfaceDataProxy *proxy = new QHeightMapSurfaceDataProxy();
    QSurface3DSeries *series = new QSurface3DSeries(proxy);
    QVERIFY(proxy);
    QVERIFY(series);
    delete series;
    proxy = 0;

    QImage image(QSize(10, 10), QImage::Format_ARGB32);
    image.fill(0);
    proxy = new QHeightMapSurfaceDataProxy(image);
    series = new QSurface3DSeries(proxy);
    QCoreApplication::processEvents();
    QVERIFY(proxy);
    QVERIFY(series);
    QCoreApplication::processEvents();
    QCOMPARE(proxy->columnCount(), 10);
    QCOMPARE(proxy->rowCount(), 10);
    delete series;
    proxy = 0;

    proxy = new QHeightMapSurfaceDataProxy(":/customtexture.jpg");
    series = new QSurface3DSeries(proxy);
    QCoreApplication::processEvents();
    QVERIFY(proxy);
    QVERIFY(series);
    QCoreApplication::processEvents();
    QCOMPARE(proxy->columnCount(), 24);
    QCOMPARE(proxy->rowCount(), 24);
    delete series;
    proxy = 0;
}

void tst_proxy::initialProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QCOMPARE(m_proxy->heightMap(), QImage());
    QCOMPARE(m_proxy->heightMapFile(), QString(""));
    QCOMPARE(m_proxy->maxXValue(), 10.0f);
    QCOMPARE(m_proxy->maxZValue(), 10.0f);
    QCOMPARE(m_proxy->minXValue(), 0.0f);
    QCOMPARE(m_proxy->minZValue(), 0.0f);

    QCOMPARE(m_proxy->columnCount(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);

    QCOMPARE(m_proxy->type(), QAbstractDataProxy::DataType::Surface);
}

void tst_proxy::initializeProperties()
{
    QVERIFY(m_proxy);
    QVERIFY(m_series);

    QSignalSpy heightMapSpy(m_proxy, &QHeightMapSurfaceDataProxy::heightMapChanged);
    QSignalSpy heightMapFileSpy(m_proxy, &QHeightMapSurfaceDataProxy::heightMapFileChanged);
    QSignalSpy minXValueSpy(m_proxy, &QHeightMapSurfaceDataProxy::minXValueChanged);
    QSignalSpy maxXValueSpy(m_proxy, &QHeightMapSurfaceDataProxy::maxXValueChanged);
    QSignalSpy minYValueSpy(m_proxy, &QHeightMapSurfaceDataProxy::minYValueChanged);
    QSignalSpy maxYValueSpy(m_proxy, &QHeightMapSurfaceDataProxy::maxYValueChanged);
    QSignalSpy minZValueSpy(m_proxy, &QHeightMapSurfaceDataProxy::minZValueChanged);
    QSignalSpy maxZValueSpy(m_proxy, &QHeightMapSurfaceDataProxy::maxZValueChanged);

    m_proxy->setHeightMapFile(":/customtexture.jpg");
    m_proxy->setMaxXValue(11.0f);
    m_proxy->setMaxZValue(11.0f);
    m_proxy->setMinXValue(-10.0f);
    m_proxy->setMinZValue(-10.0f);
    m_proxy->setMinYValue(-10.0f);
    m_proxy->setMaxYValue(11.0f);

    QCoreApplication::processEvents();

    QCOMPARE(m_proxy->heightMapFile(), QString(":/customtexture.jpg"));
    QCOMPARE(m_proxy->maxXValue(), 11.0f);
    QCOMPARE(m_proxy->maxZValue(), 11.0f);
    QCOMPARE(m_proxy->minXValue(), -10.0f);
    QCOMPARE(m_proxy->minZValue(), -10.0f);

    QCOMPARE(m_proxy->columnCount(), 24);
    QCOMPARE(m_proxy->rowCount(), 24);

    QCOMPARE(heightMapFileSpy.size(), 1);
    QCOMPARE(minXValueSpy.size(), 1);
    QCOMPARE(minZValueSpy.size(), 1);
    QCOMPARE(maxXValueSpy.size(), 1);
    QCOMPARE(maxZValueSpy.size(), 1);
    QCOMPARE(minYValueSpy.size(), 1);
    QCOMPARE(maxYValueSpy.size(), 1);

    m_proxy->setHeightMapFile("");

    QCoreApplication::processEvents();

    QCOMPARE(m_proxy->columnCount(), 0);
    QCOMPARE(m_proxy->rowCount(), 0);

    m_proxy->setHeightMap(QImage(":/customtexture.jpg"));

    QCoreApplication::processEvents();

    QCOMPARE(m_proxy->columnCount(), 24);
    QCOMPARE(m_proxy->rowCount(), 24);

    QCOMPARE(heightMapSpy.size(), 3);
    QCOMPARE(heightMapFileSpy.size(), 2);
}

void tst_proxy::invalidProperties()
{
    QTest::ignoreMessage(QtWarningMsg, "setHeightMapFile height map file"
                        " :/nonexistenttexture.jpg does not exist.");
    QTest::ignoreMessage(QtWarningMsg, "setMaxXValue tried to set"
                        " maximum X to equal or smaller than minimum X for value range."
                        " Minimum automatically adjusted to a valid one: 0.000000 --> -11.000000");
    QTest::ignoreMessage(QtWarningMsg, "setMaxZValue tried to set"
                        " maximum Z to equal or smaller than minimum Z for value range."
                        " Minimum automatically adjusted to a valid one: 0.000000 --> -11.000000");
    QTest::ignoreMessage(QtWarningMsg, "setMinXValue tried to set"
                        " minimum X to equal or larger than maximum X for value range."
                        " Maximum automatically adjusted to a valid one: -10.000000 --> 11.000000");
    QTest::ignoreMessage(QtWarningMsg, "setMinZValue tried to set"
                        " minimum Z to equal or larger than maximum Z for value range."
                        " Maximum automatically adjusted to a valid one: -10.000000 --> 11.000000");

    m_proxy->setHeightMapFile(":/nonexistenttexture.jpg");
    QEXPECT_FAIL("", "Nonexistent file given", Continue);
    QCOMPARE(m_proxy->heightMapFile(), QString(":/nonexistenttexture.jpg"));
    m_proxy->setMaxXValue(-10.0f);
    m_proxy->setMaxZValue(-10.0f);
    QCOMPARE(m_proxy->maxXValue(), -10.0f);
    QCOMPARE(m_proxy->maxZValue(), -10.0f);
    QCOMPARE(m_proxy->minXValue(), -11.0f);
    QCOMPARE(m_proxy->minZValue(), -11.0f);
    m_proxy->setMinXValue(10.0f);
    m_proxy->setMinZValue(10.0f);
    QCOMPARE(m_proxy->maxXValue(), 11.0f);
    QCOMPARE(m_proxy->maxZValue(), 11.0f);
    QCOMPARE(m_proxy->minXValue(), 10.0f);
    QCOMPARE(m_proxy->minZValue(), 10.0f);
}

QTEST_MAIN(tst_proxy)
#include "tst_proxy.moc"
