// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphsWidgets/q3dscatterwidgetitem.h>

#include "cpptestutil.h"

class tst_scatter: public QObject
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

    void addSeries();
    void addMultipleSeries();
    void selectSeries();
    void removeSeries();
    void removeMultipleSeries();
    void hasSeries();

private:
    Q3DScatterWidgetItem *m_graph;
    QQuickWidget *m_quickWidget;
};

QScatter3DSeries *newSeries()
{
    QScatter3DSeries *series = new QScatter3DSeries;
    QScatterDataArray data;
    data << QScatterDataItem(0.5f, 0.5f, 0.5f) << QScatterDataItem(-0.3f, -0.5f, -0.4f)
         << QScatterDataItem(0.0f, -0.3f, 0.2f);
    series->dataProxy()->addItems(data);
    return series;
}

void tst_scatter::initTestCase()
{
    if (!CpptestUtil::isOpenGLSupported())
        QSKIP("OpenGL not supported on this platform");
}

void tst_scatter::cleanupTestCase()
{
}

void tst_scatter::init()
{
    m_graph = new Q3DScatterWidgetItem();
    m_quickWidget = new QQuickWidget;
    m_graph->setWidget(m_quickWidget);
}

void tst_scatter::cleanup()
{
    delete m_graph;
    delete m_quickWidget;
}

void tst_scatter::construct()
{
    Q3DScatterWidgetItem *graph = new Q3DScatterWidgetItem();
    QVERIFY(graph);
    delete graph;
}

void tst_scatter::initialProperties()
{
    QVERIFY(m_graph);
    QCOMPARE(m_graph->seriesList().size(), 0);
    QVERIFY(!m_graph->selectedSeries());
    QCOMPARE(m_graph->axisX()->orientation(), QAbstract3DAxis::AxisOrientation::X);
    QCOMPARE(m_graph->axisY()->orientation(), QAbstract3DAxis::AxisOrientation::Y);
    QCOMPARE(m_graph->axisZ()->orientation(), QAbstract3DAxis::AxisOrientation::Z);

    // Common properties
    QCOMPARE(m_graph->activeTheme()->theme(), QGraphsTheme::Theme::QtGreen);
    QCOMPARE(m_graph->selectionMode(), QtGraphs3D::SelectionFlag::Item);
    QCOMPARE(m_graph->shadowQuality(), QtGraphs3D::ShadowQuality::Medium);
    QVERIFY(m_graph->scene());
    QCOMPARE(m_graph->measureFps(), false);
    QCOMPARE(m_graph->isOrthoProjection(), false);
    QCOMPARE(m_graph->selectedElement(), QtGraphs3D::ElementType::None);
    QCOMPARE(m_graph->aspectRatio(), 2.0);
    QCOMPARE(m_graph->optimizationHint(), QtGraphs3D::OptimizationHint::Default);
    QCOMPARE(m_graph->isPolar(), false);
    QCOMPARE(m_graph->radialLabelOffset(), 1.0);
    QCOMPARE(m_graph->horizontalAspectRatio(), 0.0);
    QCOMPARE(m_graph->locale(), QLocale("C"));
    QCOMPARE(m_graph->queriedGraphPosition(), QVector3D(0, 0, 0));
    QCOMPARE(m_graph->margin(), -1.0);
    QCOMPARE(m_graph->labelMargin(), 0.1f);
    QCOMPARE(m_graph->cameraTargetPosition(), QVector3D(.0f, .0f, .0f));
}

void tst_scatter::initializeProperties()
{
    QGraphsTheme *theme = new QGraphsTheme();
    theme->setColorScheme(QGraphsTheme::ColorScheme::Light);
    theme->setTheme(QGraphsTheme::Theme::QtGreenNeon);
    m_graph->setActiveTheme(theme);
    m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::None);
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::SoftHigh);
    QCOMPARE(m_graph->shadowQuality(), QtGraphs3D::ShadowQuality::SoftHigh);
    m_graph->setMeasureFps(true);
    m_graph->setOrthoProjection(true);
    m_graph->setAspectRatio(1.0);
    m_graph->setOptimizationHint(QtGraphs3D::OptimizationHint::Default);
    m_graph->setPolar(true);
    m_graph->setRadialLabelOffset(0.1f);
    m_graph->setHorizontalAspectRatio(1.0);
    m_graph->setLocale(QLocale("FI"));
    m_graph->setMargin(1.0);
    m_graph->setLabelMargin(1.0f);
    m_graph->setCameraTargetPosition(QVector3D(1.f, 1.f, 1.f));

    QCOMPARE(m_graph->activeTheme()->theme(), QGraphsTheme::Theme::QtGreenNeon);
    QCOMPARE(m_graph->selectionMode(), QtGraphs3D::SelectionFlag::None);
    QCOMPARE(m_graph->shadowQuality(),
             QtGraphs3D::ShadowQuality::None); // Ortho disables shadows
    QCOMPARE(m_graph->measureFps(), true);
    QCOMPARE(m_graph->isOrthoProjection(), true);
    QCOMPARE(m_graph->aspectRatio(), 1.0);
    QCOMPARE(m_graph->optimizationHint(), QtGraphs3D::OptimizationHint::Default);
    QCOMPARE(m_graph->isPolar(), true);
    QCOMPARE(m_graph->radialLabelOffset(), 0.1f);
    QCOMPARE(m_graph->horizontalAspectRatio(), 1.0);
    QCOMPARE(m_graph->locale(), QLocale("FI"));
    QCOMPARE(m_graph->margin(), 1.0);
    QCOMPARE(m_graph->labelMargin(), 1.0f);
    QCOMPARE(m_graph->cameraTargetPosition(), QVector3D(1.f, 1.f, 1.f));
}

void tst_scatter::invalidProperties()
{
    m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::Column | QtGraphs3D::SelectionFlag::Row
                              | QtGraphs3D::SelectionFlag::Slice);
    m_graph->setAspectRatio(-1.0);
    m_graph->setHorizontalAspectRatio(-1.0);
    m_graph->setLocale(QLocale("XX"));
    m_graph->setCameraTargetPosition(QVector3D(2.f, -2.f, -2.f));

    QCOMPARE(m_graph->selectionMode(), QtGraphs3D::SelectionFlag::Item);
    QCOMPARE(m_graph->aspectRatio(), 2.0);
    QCOMPARE(m_graph->horizontalAspectRatio(), 0.0);
    QCOMPARE(m_graph->locale(), QLocale("C"));
    QCOMPARE(m_graph->cameraTargetPosition(), QVector3D(1.f, -1.f, -1.f));
}

void tst_scatter::addSeries()
{
    m_graph->addSeries(newSeries());

    QCOMPARE(m_graph->seriesList().size(), 1);
    QVERIFY(!m_graph->selectedSeries());
}

void tst_scatter::addMultipleSeries()
{
    QScatter3DSeries *series = newSeries();
    QScatter3DSeries *series2 = newSeries();
    QScatter3DSeries *series3 = newSeries();

    m_graph->addSeries(series);
    m_graph->addSeries(series2);
    m_graph->addSeries(series3);

    QCOMPARE(m_graph->seriesList().size(), 3);
}

void tst_scatter::selectSeries()
{
    QScatter3DSeries *series = newSeries();

    m_graph->addSeries(series);
    m_graph->seriesList()[0]->setSelectedItem(1);

    QCOMPARE(m_graph->seriesList().size(), 1);
    QCOMPARE(m_graph->selectedSeries(), series);

    m_graph->clearSelection();
    QVERIFY(!m_graph->selectedSeries());
}

void tst_scatter::removeSeries()
{
    QScatter3DSeries *series = newSeries();

    m_graph->addSeries(series);
    m_graph->removeSeries(series);
    QCOMPARE(m_graph->seriesList().size(), 0);

    delete series;
}

void tst_scatter::removeMultipleSeries()
{
    QScatter3DSeries *series = newSeries();
    QScatter3DSeries *series2 = newSeries();
    QScatter3DSeries *series3 = newSeries();

    m_graph->addSeries(series);
    m_graph->addSeries(series2);
    m_graph->addSeries(series3);

    m_graph->seriesList()[0]->setSelectedItem(1);
    QCOMPARE(m_graph->selectedSeries(), series);

    m_graph->removeSeries(series);
    QCOMPARE(m_graph->seriesList().size(), 2);
    QVERIFY(!m_graph->selectedSeries());

    m_graph->removeSeries(series2);
    QCOMPARE(m_graph->seriesList().size(), 1);

    m_graph->removeSeries(series3);
    QCOMPARE(m_graph->seriesList().size(), 0);

    delete series;
    delete series2;
    delete series3;
}

void tst_scatter::hasSeries()
{
    QScatter3DSeries *series1 = newSeries();
    m_graph->addSeries(series1);
    QCOMPARE(m_graph->hasSeries(series1), true);
    QScatter3DSeries *series2 = newSeries();
    QCOMPARE(m_graph->hasSeries(series2), false);
}

QTEST_MAIN(tst_scatter)
#include "tst_scatter.moc"
