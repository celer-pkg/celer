// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/QSurface3DSeries>

class tst_series: public QObject
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
    QSurface3DSeries *m_series;
};

void tst_series::initTestCase()
{
}

void tst_series::cleanupTestCase()
{
}

void tst_series::init()
{
    m_series = new QSurface3DSeries();
}

void tst_series::cleanup()
{
    delete m_series;
}

void tst_series::construct()
{
    QSurface3DSeries *series = new QSurface3DSeries();
    QVERIFY(series);
    delete series;

    QSurfaceDataProxy *proxy = new QSurfaceDataProxy();

    series = new QSurface3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(series->dataProxy(), proxy);
    delete series;
}

void tst_series::initialProperties()
{
    QVERIFY(m_series);

    QVERIFY(m_series->dataProxy());
    QCOMPARE(m_series->drawMode(), QSurface3DSeries::DrawSurfaceAndWireframe);
    QCOMPARE(m_series->isFlatShadingEnabled(), true);
    QCOMPARE(m_series->isFlatShadingSupported(), true);
    QCOMPARE(m_series->selectedPoint(), m_series->invalidSelectionPosition());
    QCOMPARE(m_series->wireframeColor(), QColor(Qt::black));
    // Common properties. The ones identical between different series are tested in QBar3DSeries tests
    QCOMPARE(m_series->itemLabelFormat(), QString("@xLabel, @yLabel, @zLabel"));
    QCOMPARE(m_series->mesh(), QAbstract3DSeries::MeshSphere);
    QCOMPARE(m_series->type(), QAbstract3DSeries::SeriesTypeSurface);
}

void tst_series::initializeProperties()
{
    QVERIFY(m_series);

    m_series->setDataProxy(new QSurfaceDataProxy());
    m_series->setDrawMode(QSurface3DSeries::DrawWireframe);
    m_series->setFlatShadingEnabled(false);
    m_series->setSelectedPoint(QPoint(0, 0));
    m_series->setWireframeColor(QColor(Qt::red));

    QCOMPARE(m_series->drawMode(), QSurface3DSeries::DrawWireframe);
    QCOMPARE(m_series->isFlatShadingEnabled(), false);
    QCOMPARE(m_series->selectedPoint(), QPoint(0, 0));
    QCOMPARE(m_series->wireframeColor(), QColor(Qt::red));

    // Common properties. The ones identical between different series are tested in QBar3DSeries tests
    m_series->setMesh(QAbstract3DSeries::MeshPyramid);

    QCOMPARE(m_series->mesh(), QAbstract3DSeries::MeshPyramid);
}

void tst_series::invalidProperties()
{
    m_series->setMesh(QAbstract3DSeries::MeshPoint);

    QCOMPARE(m_series->mesh(), QAbstract3DSeries::MeshSphere);
}

QTEST_MAIN(tst_series)
#include "tst_series.moc"
