// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QSpline3DSeries>

#include <QtGui/qquaternion.h>

class tst_series : public QObject
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

private:
    QSpline3DSeries *m_series;
};

void tst_series::initTestCase() {}

void tst_series::cleanupTestCase() {}

void tst_series::init()
{
    m_series = new QSpline3DSeries();
}

void tst_series::cleanup()
{
    delete m_series;
}

void tst_series::construct()
{
    QSpline3DSeries *series = new QSpline3DSeries();
    QVERIFY(series);
    delete series;

    QScatterDataProxy *proxy = new QScatterDataProxy();

    series = new QSpline3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(series->dataProxy(), proxy);
    delete series;
}

void tst_series::initialProperties()
{
    QVERIFY(m_series);

    //Scatter properties
    QVERIFY(m_series->dataProxy());
    QCOMPARE(m_series->itemSize(), 0.0f);
    QCOMPARE(m_series->selectedItem(), m_series->invalidSelectionIndex());

    // Common properties. The ones identical between different series are tested in QBar3DSeries tests
    QCOMPARE(m_series->itemLabelFormat(), QString("@xLabel, @yLabel, @zLabel"));
    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::Sphere);
    QCOMPARE(m_series->type(), QAbstract3DSeries::SeriesType::Scatter);

    //Spline properties
    QCOMPARE(m_series->isSplineVisible(), true);
    QCOMPARE(m_series->isSplineLooping(), false);
    QCOMPARE(m_series->splineTension(), 0.0f);
    QCOMPARE(m_series->splineKnotting(), 0.5f);
    QCOMPARE(m_series->splineColor(), QColor(255, 0, 0));
    QCOMPARE(m_series->splineResolution(), 10);
}

void tst_series::initializeProperties()
{
    QVERIFY(m_series);

    //Scatter properties
    m_series->setDataProxy(new QScatterDataProxy());
    m_series->setItemSize(0.5f);
    m_series->setSelectedItem(0);

    QCOMPARE(m_series->itemSize(), 0.5f);
    QCOMPARE(m_series->selectedItem(), 0);

    // Common properties. The ones identical between different series are tested in QBar3DSeries tests
    m_series->setMesh(QAbstract3DSeries::Mesh::Point);
    m_series->setMeshRotation(QQuaternion(1, 1, 10, 20));

    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::Point);
    QCOMPARE(m_series->meshRotation(), QQuaternion(1, 1, 10, 20));

    //spline properties
    m_series->setSplineVisible(false);
    m_series->setSplineLooping(true);
    m_series->setSplineTension(1.0f);
    m_series->setSplineKnotting(1.0f);
    m_series->setSplineColor(QColor(0, 255, 0));
    m_series->setSplineResolution(5);

    QCOMPARE(m_series->isSplineVisible(), false);
    QCOMPARE(m_series->isSplineLooping(), true);
    QCOMPARE(m_series->splineTension(), 1.0f);
    QCOMPARE(m_series->splineKnotting(), 1.0f);
    QCOMPARE(m_series->splineColor(), QColor(0, 255, 0));
    QCOMPARE(m_series->splineResolution(), 5);
}

QTEST_MAIN(tst_series)
#include "tst_series.moc"
