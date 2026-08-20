// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "graphmodifier.h"
#include <QtGraphs/QValue3DAxis>
#include <QtGraphs/QSurfaceDataProxy>
#include <QtGraphs/QSurface3DSeries>
#include <QtGraphs/QGraphsTheme>

#include <limits>
#include <qmath.h>
#include <qrandom.h>
#include <QLinearGradient>
#include <QDebug>
#include <QComboBox>
#ifndef QT_NO_CURSOR
#include <QtGui/QCursor>
#endif

//#define JITTER_PLANE
//#define WONKY_PLANE

GraphModifier::GraphModifier(Q3DSurfaceWidgetItem *graph)
    : m_graph(graph),
      m_series1(new QSurface3DSeries),
      m_series2(new QSurface3DSeries),
      m_series3(new QSurface3DSeries),
      m_series4(new QSurface3DSeries),
      m_lineSeries(new QSurface3DSeries),
      m_gridSliderX(0),
      m_gridSliderZ(0),
      m_axisRangeSliderX(0),
      m_axisRangeSliderZ(0),
      m_axisMinSliderX(0),
      m_axisMinSliderZ(0),
      m_xCount(24),
      m_zCount(24),
      m_activeSample(0),
      m_fontSize(40),
      m_rangeX(34.0),
      m_rangeY(20.0),
      m_rangeZ(34.0),
      m_minX(-17.0),
      m_minY(-8.0),
      m_minZ(-17.0),
      m_addRowCounter(m_zCount),
      m_insertTestZPos(0),
      m_insertTestIndexPos(1),
      m_planeArray(0),
      m_theSeries(new QSurface3DSeries),
      m_drawMode(QSurface3DSeries::DrawSurfaceAndWireframe),
      m_drawMode2(QSurface3DSeries::DrawSurfaceAndWireframe),
      m_drawMode3(QSurface3DSeries::DrawSurfaceAndWireframe),
      m_drawMode4(QSurface3DSeries::DrawSurfaceAndWireframe),
      m_drawMode5(QSurface3DSeries::DrawSurface),
      m_offset(4.0f),
      m_ascendingX(true),
      m_ascendingZ(true)
{
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->axisX()->setTitle("X-Axis");
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->axisY()->setTitle("Value Axis");
    m_graph->setAxisZ(new QValue3DAxis);
    m_graph->axisZ()->setTitle("Z-Axis");
#ifdef MULTI_SERIES
    m_limitX = float(m_xCount) / 2.0f;
    m_limitZ = float(m_zCount) / 2.0f;
    // Series 1
    m_multiSampleOffsetX[0] = -m_offset;
    m_multiSampleOffsetZ[0] = -m_offset;
    // Series 2
    m_multiSampleOffsetX[1] = -m_offset;
    m_multiSampleOffsetZ[1] = m_offset;
    // Series 3
    m_multiSampleOffsetX[2] = m_offset;
    m_multiSampleOffsetZ[2] = -m_offset;
    // Series 4
    m_multiSampleOffsetX[3] = m_offset;
    m_multiSampleOffsetZ[3] = m_offset;

//    m_graph->axisX()->setRange(-m_limitX - m_offset, m_limitX + m_offset);
//    m_graph->axisY()->setRange(-1.0f, 4.5f);
//    m_graph->axisZ()->setRange(-m_limitZ - m_offset, m_limitZ + m_offset);
#else
    m_graph->addSeries(m_theSeries);
#endif
    m_graph->axisX()->setRange(m_minX, m_minX + m_rangeX);
    m_graph->axisY()->setRange(m_minY, m_minY + m_rangeY);
    m_graph->axisZ()->setRange(m_minZ, m_minZ + m_rangeZ);

    m_graph->setZoomAtTargetEnabled(true);

    for (int i = 0; i < 4; i++) {
        m_multiseries[i] = new QSurface3DSeries;
        m_multiseries[i]->setName(QStringLiteral("Series %1").arg(i+1));
        m_multiseries[i]->setItemLabelFormat(QStringLiteral("@seriesName: (@xLabel, @zLabel): @yLabel"));
    }

    fillSeries();
    changeStyle();

    m_lineSeries->setDrawMode(m_drawMode5);

    m_theSeries->setItemLabelFormat(QStringLiteral("@seriesName: (@xLabel, @zLabel): @yLabel"));

    connect(&m_timer, &QTimer::timeout, this, &GraphModifier::timeout);
    connect(&m_graphPositionQueryTimer, &QTimer::timeout, this, &GraphModifier::graphQueryTimeout);
    connect(m_theSeries, &QSurface3DSeries::selectedPointChanged, this, &GraphModifier::selectedPointChanged);

    QObject::connect(m_graph, &Q3DSurfaceWidgetItem::axisXChanged, this,
                     &GraphModifier::handleAxisXChanged);
    QObject::connect(m_graph, &Q3DSurfaceWidgetItem::axisYChanged, this,
                     &GraphModifier::handleAxisYChanged);
    QObject::connect(m_graph, &Q3DSurfaceWidgetItem::axisZChanged, this,
                     &GraphModifier::handleAxisZChanged);
    QObject::connect(m_graph, &Q3DGraphsWidgetItem::currentFpsChanged, this,
                     &GraphModifier::handleFpsChange);

    //m_graphPositionQueryTimer.start(100);
}

GraphModifier::~GraphModifier()
{
    delete m_graph;
}

void GraphModifier::fillSeries()
{
    float full = m_limitX * m_limitZ;

    QSurfaceDataArray dataArray1;
    dataArray1.reserve(m_zCount);
    QSurfaceDataArray dataArray2;
    dataArray2.reserve(m_zCount);
    QSurfaceDataArray dataArray3;
    dataArray3.reserve(m_zCount);
    QSurfaceDataArray dataArray4;
    dataArray4.reserve(m_zCount);


    for (int i = 0; i < m_zCount; i++) {
        QSurfaceDataRow newRow[4];
        float zAdjust = 0.0f;
        if (i == 2)
            zAdjust = 0.7f;

        for (int s = 0; s < 4; s++) {
            newRow[s].resize(m_xCount);
            float z = float(i) - m_limitZ + 0.5f + m_multiSampleOffsetZ[s] + zAdjust;
            for (int j = 0; j < m_xCount; j++) {
                float xAdjust = 0.0f;
                if (j == 4)
                    xAdjust = 0.7f;
                float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[s] + xAdjust;
                float angle = (z * x) / full * 1.57f;
                float y = (qSin(angle * float(qPow(1.3f, s))) + 1.1f * s) * 3.0f - 5.0f + xAdjust + zAdjust;
                newRow[s][j].setPosition(QVector3D(x, y, z));
            }
        }
        dataArray1 << newRow[0];
        dataArray2 << newRow[1];
        dataArray3 << newRow[2];
        dataArray4 << newRow[3];
    }

    m_multiseries[0]->dataProxy()->resetArray(dataArray1);
    m_multiseries[1]->dataProxy()->resetArray(dataArray2);
    m_multiseries[2]->dataProxy()->resetArray(dataArray3);
    m_multiseries[3]->dataProxy()->resetArray(dataArray4);

    QSurfaceDataArray lineDataArray;
    lineDataArray.reserve(m_zCount);
    for (int i = 0; i < m_zCount; i++) {
        QSurfaceDataRow row;
        float zAdjust = 0.0f;
        float z = float(i) - m_limitZ + 0.5f + m_multiSampleOffsetZ[3] + zAdjust;

        int j = m_xCount / 2;
        float xAdjust = 0.0f;
        if (j == 4)
            xAdjust = 0.7f;
        float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[3] + xAdjust;
        float angle = (z * x) / full * 1.57f;
        float y = (qSin(angle * float(qPow(1.3f, 4))) + 1.1f * 4) * 3.0f - 5.0f + xAdjust + zAdjust;
        row.append(QSurfaceDataItem(x,y,z));

        lineDataArray << row;
    }

    m_lineSeries->dataProxy()->resetArray(lineDataArray);
}

void GraphModifier::toggleSeries1(int enabled)
{
    qDebug() << __FUNCTION__ << " enabled = " << enabled;

    if (enabled) {
        m_graph->addSeries(m_multiseries[0]);
    } else {
        m_graph->removeSeries(m_multiseries[0]);
    }
}

void GraphModifier::toggleSeries2(int enabled)
{
    qDebug() << __FUNCTION__ << " enabled = " << enabled;

    if (enabled) {
        m_graph->addSeries(m_multiseries[1]);
    } else {
        m_graph->removeSeries(m_multiseries[1]);
    }
}

void GraphModifier::toggleSeries3(int enabled)
{
    qDebug() << __FUNCTION__ << " enabled = " << enabled;

    if (enabled) {
        m_graph->addSeries(m_multiseries[2]);
    } else {
        m_graph->removeSeries(m_multiseries[2]);
    }
}

void GraphModifier::toggleSeries4(int enabled)
{
    qDebug() << __FUNCTION__ << " enabled = " << enabled;

    if (enabled) {
        m_graph->addSeries(m_multiseries[3]);
    } else {
        m_graph->removeSeries(m_multiseries[3]);
    }
}

void GraphModifier::toggleLineSeries(int enabled)
{
    qDebug() << __FUNCTION__ << " enabled = " << enabled;
    if (enabled)
        m_graph->addSeries(m_lineSeries);
    else
        m_graph->removeSeries(m_lineSeries);
}

void GraphModifier::toggleSmooth(int visible)
{
    qDebug() << "GraphModifier::toggleSmooth " << visible;
    QSurface3DSeries::Shading shading;
    if (visible)
        shading = QSurface3DSeries::Shading::Flat;
    else
        shading = QSurface3DSeries::Shading::Smooth;
    m_theSeries->setShading(shading);
#ifdef MULTI_SERIES
    m_multiseries[0]->setShading(shading);
#endif
}

void GraphModifier::toggleSurfaceGrid(int enable)
{
    qDebug() << "GraphModifier::toggleSurfaceGrid" << enable;
    if (enable)
        m_drawMode |= QSurface3DSeries::DrawWireframe;
    else
        m_drawMode &= ~QSurface3DSeries::DrawWireframe;

    m_theSeries->setDrawMode(m_drawMode);
#ifdef MULTI_SERIES
    m_multiseries[0]->setDrawMode(m_drawMode);
#endif
}

void GraphModifier::toggleSurface(int enable)
{
    qDebug() << "GraphModifier::toggleSurface" << enable;
    if (enable)
        m_drawMode |= QSurface3DSeries::DrawSurface;
    else
        m_drawMode &= ~QSurface3DSeries::DrawSurface;

    m_theSeries->setDrawMode(m_drawMode);
#ifdef MULTI_SERIES
    m_multiseries[0]->setDrawMode(m_drawMode);
#endif
}

void GraphModifier::toggleSeriesVisible(int enable)
{
    m_theSeries->setVisible(enable);
#ifdef MULTI_SERIES
    m_multiseries[0]->setVisible(enable);
#endif
}

void GraphModifier::toggleFill(int enable)
{
    qDebug() << "GraphModifier::toggleFill" << enable;
    if (enable)
        m_drawMode |= QSurface3DSeries::DrawFilledSurface;
    else
        m_drawMode &= ~QSurface3DSeries::DrawFilledSurface;

    m_theSeries->setDrawMode(m_drawMode);
#ifdef MULTI_SERIES
    m_multiseries[0]->setDrawMode(m_drawMode);
#endif
}

void GraphModifier::toggleSmoothS2(int visible)
{
    qDebug() << __FUNCTION__ << visible;
    QSurface3DSeries::Shading shading;
    if (visible)
        shading = QSurface3DSeries::Shading::Flat;
    else
        shading = QSurface3DSeries::Shading::Smooth;
    m_multiseries[1]->setShading(shading);
}

void GraphModifier::toggleSurfaceGridS2(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode2 |= QSurface3DSeries::DrawWireframe;
    else
        m_drawMode2 &= ~QSurface3DSeries::DrawWireframe;

    m_multiseries[1]->setDrawMode(m_drawMode2);
}

void GraphModifier::toggleSurfaceS2(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode2 |= QSurface3DSeries::DrawSurface;
    else
        m_drawMode2 &= ~QSurface3DSeries::DrawSurface;

    m_multiseries[1]->setDrawMode(m_drawMode2);
}

void GraphModifier::toggleSeries2Visible(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    m_multiseries[1]->setVisible(enable);
}

void GraphModifier::toggleFillS2(int enable)
{
    qDebug() << "GraphModifier::toggleFill" << enable;
    if (enable)
        m_drawMode2 |= QSurface3DSeries::DrawFilledSurface;
    else
        m_drawMode2 &= ~QSurface3DSeries::DrawFilledSurface;

    m_theSeries->setDrawMode(m_drawMode2);
#ifdef MULTI_SERIES
    m_multiseries[1]->setDrawMode(m_drawMode2);
#endif
}

void GraphModifier::toggleSmoothS3(int enabled)
{
    qDebug() << __FUNCTION__ << enabled;
    QSurface3DSeries::Shading shading;
    if (enabled)
        shading = QSurface3DSeries::Shading::Flat;
    else
        shading = QSurface3DSeries::Shading::Smooth;
    m_multiseries[2]->setShading(shading);
}

void GraphModifier::toggleSurfaceGridS3(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode3 |= QSurface3DSeries::DrawWireframe;
    else
        m_drawMode3 &= ~QSurface3DSeries::DrawWireframe;

    m_multiseries[2]->setDrawMode(m_drawMode3);
}

void GraphModifier::toggleSurfaceS3(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode3 |= QSurface3DSeries::DrawSurface;
    else
        m_drawMode3 &= ~QSurface3DSeries::DrawSurface;

    m_multiseries[2]->setDrawMode(m_drawMode3);
}

void GraphModifier::toggleSeries3Visible(int visible)
{
    qDebug() << __FUNCTION__ << visible;
    m_multiseries[2]->setVisible(visible);
}

void GraphModifier::toggleFillS3(int enable)
{
    qDebug() << "GraphModifier::toggleFill" << enable;
    if (enable)
        m_drawMode3 |= QSurface3DSeries::DrawFilledSurface;
    else
        m_drawMode3 &= ~QSurface3DSeries::DrawFilledSurface;

    m_theSeries->setDrawMode(m_drawMode3);
#ifdef MULTI_SERIES
    m_multiseries[2]->setDrawMode(m_drawMode3);
#endif
}

void GraphModifier::toggleSmoothS4(int visible)
{
    qDebug() << __FUNCTION__ << visible;
    QSurface3DSeries::Shading shading;
    if (visible)
        shading = QSurface3DSeries::Shading::Flat;
    else
        shading = QSurface3DSeries::Shading::Smooth;
    m_multiseries[3]->setShading(shading);
}

void GraphModifier::toggleSurfaceGridS4(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode4 |= QSurface3DSeries::DrawWireframe;
    else
        m_drawMode4 &= ~QSurface3DSeries::DrawWireframe;

    m_multiseries[3]->setDrawMode(m_drawMode4);
}

void GraphModifier::toggleSurfaceS4(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode4 |= QSurface3DSeries::DrawSurface;
    else
        m_drawMode4 &= ~QSurface3DSeries::DrawSurface;

    m_multiseries[3]->setDrawMode(m_drawMode4);
}

void GraphModifier::toggleLineSurfaceGrid(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    if (enable)
        m_drawMode5 |= QSurface3DSeries::DrawWireframe;
    else
        m_drawMode5 &= ~QSurface3DSeries::DrawWireframe;

    m_lineSeries->setDrawMode(m_drawMode5);

}

void GraphModifier::toggleSeries4Visible(int enable)
{
    qDebug() << __FUNCTION__ << enable;
    m_multiseries[3]->setVisible(enable);
}

void GraphModifier::toggleFillS4(int enable)
{
    qDebug() << "GraphModifier::toggleFill" << enable;
    if (enable)
        m_drawMode4 |= QSurface3DSeries::DrawFilledSurface;
    else
        m_drawMode4 &= ~QSurface3DSeries::DrawFilledSurface;

    m_theSeries->setDrawMode(m_drawMode);
#ifdef MULTI_SERIES
    m_multiseries[3]->setDrawMode(m_drawMode4);
#endif
}

void GraphModifier::toggleFillLine(int enable)
{
    qDebug() << "GraphModifier::toggleFill" << enable;
    if (enable)
        m_drawMode5 |= QSurface3DSeries::DrawFilledSurface;
    else
        m_drawMode5 &= ~QSurface3DSeries::DrawFilledSurface;

    m_lineSeries->setDrawMode(m_drawMode5);
}

void GraphModifier::toggleSqrtSin(int enable)
{
    if (enable) {
        qDebug() << "Create Sqrt&Sin surface, (" << m_xCount << ", " << m_zCount << ")";

        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);

        QSurfaceDataArray dataArray;
        dataArray.reserve(m_zCount);
        for (float i = 0; i < m_zCount; i++) {
            QSurfaceDataRow newRow(m_xCount);
            // Keep values within range bounds, since just adding step can cause minor drift due
            // to the rounding errors.
            float z = qMin(maxZ, (i * stepZ + minZ));
            for (float j = 0; j < m_xCount; j++) {
                float x = qMin(maxX, (j * stepX + minX));
                float R = qSqrt(x * x + z * z) + 0.01f;
                float y = (qSin(R) / R + 0.24f) * 1.61f + 1.0f;
                newRow[j].setPosition(QVector3D(x, y, z));
            }
            dataArray << newRow;
        }

        m_graph->axisY()->setRange(1.0, 3.0);
        m_graph->axisX()->setLabelFormat("%.2f");
        m_graph->axisZ()->setLabelFormat("%.2f");

        m_theSeries->setName("Sqrt & Sin");

        resetArrayAndSliders(dataArray, minZ, maxZ, minX, maxX);

        m_activeSample = GraphModifier::SqrtSin;
    } else {
        qDebug() << "Remove surface";
    }
}

void GraphModifier::togglePlane(int enable)
{
    qDebug() << "GraphModifier::togglePlane " << enable;

    if (enable) {
        m_planeArray.clear();

#ifdef JITTER_PLANE
        m_timer.start(0);
#endif
        m_graph->axisY()->setRange(0.0, 1.0);
        m_graph->axisX()->setLabelFormat("%.2f");
        m_graph->axisZ()->setLabelFormat("%.2f");

        m_planeArray.reserve(m_zCount);
        float minX = -10.0f;
        float maxX = 20.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);
#ifdef WONKY_PLANE
        float halfZ = m_zCount / 2;
        float wonkyFactor = 0.01f;
        float maxStepX = 0.0f;
        float add = 0.0f;
        for (float i = 0; i < m_zCount; i++) {
            QSurfaceDataRow *newRow = new QSurfaceDataRow(m_xCount);
            if (i < halfZ) {
                stepX += wonkyFactor;
                maxStepX = stepX;
            } else {
                stepX -= wonkyFactor;
            }
            add = 0.0f;
            for (float j = 0; j < m_xCount; j++) {
                (*newRow)[j].setPosition(QVector3D(j * stepX + minX, -0.04f,
                                                   i * stepZ + minZ + add));
                add += 0.5f;

            }
            *m_planeArray << newRow;
        }

        m_theSeries->setName("Wonky Plane");

        resetArrayAndSliders(m_planeArray, minZ, maxZ + add, minX, m_xCount * maxStepX + minX);
#else
        for (float i = 0; i < m_zCount; i++) {
            QSurfaceDataRow newRow(m_xCount);
            // Keep values within range bounds, since just adding step can cause minor drift due
            // to the rounding errors.
            float zVal;
            if (i == (m_zCount - 1))
                zVal = maxZ;
            else
                zVal = i * stepZ + minZ;

            float j = 0;
            for (; j < m_xCount - 1; j++)
                newRow[j].setPosition(QVector3D(j * stepX + minX, -0.04f, zVal));
            newRow[j].setPosition(QVector3D(maxX, -0.04f, zVal));

            m_planeArray << newRow;
        }

        m_theSeries->setName("Plane");

        resetArrayAndSliders(m_planeArray, minZ, maxZ, minX, maxX);
#endif

        m_activeSample = GraphModifier::Plane;
    }
#ifdef JITTER_PLANE
    else {
        m_timer.stop();
    }
#endif
}

void GraphModifier::setHeightMapData(bool enable)
{
    if (enable) {
        // Do the height map the hard way.
        // Easier alternative would be to use the QHeightMapSurfaceDataProxy.
        QImage image(":/maps/map");

        QSurfaceDataArray dataArray;
        uchar *bits = image.bits();

        int p = image.width() * 4 * (image.height() - 1);
        dataArray.reserve(image.height());
        float minX = 34.0;
        float maxX = 40.0;
        float minZ = 18.0;
        float maxZ = 24.0;
        float xMul = (maxX - minX) / float(image.width() - 1);
        float zMul = (maxZ - minZ) / float(image.height() - 1);
        for (int i = 0; i < image.height(); i++, p -= image.width() * 4) {
            QSurfaceDataRow newRow(image.width());
            for (int j = 0; j < image.width(); j++) {
                newRow[j].setPosition(QVector3D((float(j) * xMul) + minX,
                                                   (float(bits[p + (j * 4)]) + 1.0f) / 1.0f,
                                                   (float(i) * zMul) + minZ));
            }
            dataArray << newRow;
        }

        m_graph->axisY()->setAutoAdjustRange(true);
        m_graph->axisX()->setLabelFormat("%.1f N");
        m_graph->axisZ()->setLabelFormat("%.1f E");

        m_theSeries->setName("Height Map");

        resetArrayAndSliders(dataArray, minZ, maxZ, minX, maxX);

        m_activeSample = GraphModifier::Map;
    }
}

void GraphModifier::toggleGridSliderLock(bool enable)
{
    m_gridSlidersLocked = enable;
    if (m_gridSlidersLocked) {
        m_gridSliderZ->setEnabled(false);
        m_gridSliderZ->setValue(m_gridSliderX->value());
    } else {
        m_gridSliderZ->setEnabled(true);
    }
}

void GraphModifier::adjustXCount(int count)
{
    m_xCount = count;
    if (m_gridSlidersLocked)
        m_gridSliderZ->setValue(count);

    updateSamples();

    qDebug() << "X count =" << count;
}

void GraphModifier::adjustZCount(int count)
{
    m_zCount = count;

    updateSamples();

    qDebug() << "Z count =" << count;
}

void GraphModifier::adjustXRange(int range)
{
    m_rangeX = range;
    m_graph->axisX()->setRange(m_minX, m_minX + m_rangeX);

    qDebug() << "X Range =" << range;
}

void GraphModifier::adjustYRange(int range)
{
    m_rangeY = range;
    m_graph->axisY()->setRange(m_minY, m_minY + m_rangeY);

    qDebug() << "Y Range =" << range;
}

void GraphModifier::adjustZRange(int range)
{
    m_rangeZ = range;
    m_graph->axisZ()->setRange(m_minZ, m_minZ + m_rangeZ);

    qDebug() << "Z Range =" << range;
}

void GraphModifier::adjustXMin(int min)
{
    m_minX = min;
    m_graph->axisX()->setRange(m_minX, m_minX + m_rangeX);

    qDebug() << "X Minimum =" << min;
}

void GraphModifier::adjustYMin(int min)
{
    m_minY = min;
    m_graph->axisY()->setRange(m_minY, m_minY + m_rangeY);

    qDebug() << "Y Minimum =" << min;
}

void GraphModifier::adjustZMin(int min)
{
    m_minZ = min;
    m_graph->axisZ()->setRange(m_minZ, m_minZ + m_rangeZ);

    qDebug() << "Z Minimum =" << min;
}

void GraphModifier::adjustCutoffMargin(int margin)
{
    m_graph->setCutoffMargin(float(margin) / 100.0f);

    qDebug() << "Cutogg margin" << float(margin) / 100.0f;
}

void GraphModifier::gradientPressed()
{
    static QGraphsTheme::ColorStyle colorStyle = QGraphsTheme::ColorStyle::Uniform;

    if (colorStyle == QGraphsTheme::ColorStyle::RangeGradient) {
        colorStyle = QGraphsTheme::ColorStyle::ObjectGradient;
        qDebug() << "Color style: ObjectGradient";
    } else if (colorStyle == QGraphsTheme::ColorStyle::ObjectGradient) {
        colorStyle = QGraphsTheme::ColorStyle::Uniform;
        qDebug() << "Color style: Uniform";
    } else {
        colorStyle = QGraphsTheme::ColorStyle::RangeGradient;
        qDebug() << "Color style: RangeGradient";
    }

    QLinearGradient gradient;
    gradient.setColorAt(0.0, QColor(0, 0, 0, 255));
    gradient.setColorAt(0.33, QColor(0, 0, 255, 100));
    gradient.setColorAt(0.67, QColor(255, 0, 0, 150));
    gradient.setColorAt(1.0, QColor(255, 255, 0, 200));

    QList<QLinearGradient> gradients;
    gradients << gradient;
    m_graph->activeTheme()->setSeriesGradients(gradients);
    m_graph->activeTheme()->setColorStyle(colorStyle);

}

void GraphModifier::changeFont(const QFont &font)
{
    QFont newFont = font;
    newFont.setPointSizeF(m_fontSize);
    m_graph->activeTheme()->setLabelFont(newFont);
}

void GraphModifier::changeStyle()
{
    m_graph->activeTheme()->setLabelBackgroundVisible(
        !m_graph->activeTheme()->isLabelBackgroundVisible());
}

void GraphModifier::selectButtonClicked()
{
    QSurfaceDataProxy *proxy = m_theSeries->dataProxy();
    int row = QRandomGenerator::global()->bounded(proxy->rowCount());
    int col = QRandomGenerator::global()->bounded(proxy->columnCount());

    m_theSeries->setSelectedPoint(QPoint(row, col));
}

void GraphModifier::selectedPointChanged(const QPoint &point)
{
    QString labelText = QStringLiteral("Selected row: %1, column: %2").arg(point.x()).arg(point.y());
    m_selectionInfoLabel->setText(labelText);
}

void GraphModifier::changeColorScheme(int colorScheme)
{
    m_graph->activeTheme()->setColorScheme(QGraphsTheme::ColorScheme(colorScheme + 1));
}

void GraphModifier::changeTheme(int theme)
{
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme(theme));
}


void GraphModifier::flipViews()
{
    m_graph->scene()->setSecondarySubviewOnTop(!m_graph->scene()->isSecondarySubviewOnTop());
}

void GraphModifier::changeSubView()
{
    if (!m_customSubviews) {
        m_graph->scene()->setPrimarySubViewport(
            QRect(0, 0, m_graph->widget()->width() / 2, m_graph->widget()->height()));
        m_graph->scene()->setSecondarySubViewport(QRect(m_graph->widget()->width() / 2,
                                                        0,
                                                        m_graph->widget()->width() / 2,
                                                        m_graph->widget()->height()));
        m_customSubviews = true;
    } else {
        m_graph->scene()->setPrimarySubViewport(QRect());
        m_graph->scene()->setSecondarySubViewport(QRect());
        m_customSubviews = false;
    }
}

void GraphModifier::timeout()
{
    int rows = m_planeArray.size();
    int columns = m_planeArray.at(0).size();

    // Induce minor random jitter to the existing plane array
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            m_planeArray[i][j].setX(m_planeArray.at(i).at(j).x()
                                           * ((float((QRandomGenerator::global()->bounded(10)) + 5.0f) / 10000.0f) + 0.999f));
            m_planeArray[i][j].setY(m_planeArray.at(i).at(j).y()
                                           * ((float((QRandomGenerator::global()->bounded(10)) + 5.0f) / 1000.0f) + 0.99f) + 0.0001f);
            m_planeArray[i][j].setZ(m_planeArray.at(i).at(j).z()
                                           * ((float((QRandomGenerator::global()->bounded(10)) + 5.0f) / 10000.0f) + 0.999f));
        }
    }

    // Reset same array to make it redraw
    m_theSeries->dataProxy()->resetArray(m_planeArray);
}

void GraphModifier::graphQueryTimeout()
{
#ifndef QT_NO_CURSOR
    m_graph->scene()->setGraphPositionQuery(m_graph->widget()->mapFromGlobal(QCursor::pos()));
    qDebug() << "pos: " << (m_graph->widget()->mapFromGlobal(QCursor::pos()));
#else
    m_graph->scene()->setGraphPositionQuery(QPoint(100, 100));
#endif
}

void GraphModifier::handleAxisXChanged(QValue3DAxis *axis)
{
    qDebug() << __FUNCTION__ << axis << axis->orientation() << (axis == m_graph->axisX());
}

void GraphModifier::handleAxisYChanged(QValue3DAxis *axis)
{
    qDebug() << __FUNCTION__ << axis << axis->orientation() << (axis == m_graph->axisY());
}

void GraphModifier::handleAxisZChanged(QValue3DAxis *axis)
{
    qDebug() << __FUNCTION__ << axis << axis->orientation() << (axis == m_graph->axisZ());
}

void GraphModifier::handleFpsChange(int fps)
{
    qDebug() << "FPS:" << fps;
}

void GraphModifier::changeLabelRotation(int rotation)
{
    m_graph->axisX()->setLabelAutoAngle(float(rotation));
    m_graph->axisY()->setLabelAutoAngle(float(rotation));
    m_graph->axisZ()->setLabelAutoAngle(float(rotation));
}

void GraphModifier::toggleAxisTitleVisibility(int visible)
{
    m_graph->axisX()->setTitleVisible(visible);
    m_graph->axisY()->setTitleVisible(visible);
    m_graph->axisZ()->setTitleVisible(visible);
}

void GraphModifier::toggleXAxisLabelsVisibility(int visible)
{
    m_graph->axisX()->setLabelsVisible(visible);
}

void GraphModifier::toggleYAxisLabelsVisibility(int visible)
{
    m_graph->axisY()->setLabelsVisible(visible);
}

void GraphModifier::toggleZAxisLabelsVisibility(int visible)
{
    m_graph->axisZ()->setLabelsVisible(visible);
}

void GraphModifier::toggleAxisTitleFixed(int enabled)
{
    m_graph->axisX()->setTitleFixed(enabled);
    m_graph->axisY()->setTitleFixed(enabled);
    m_graph->axisZ()->setTitleFixed(enabled);
}

void GraphModifier::toggleXAscending(int enabled)
{
    m_ascendingX = enabled;

    // Flip data array contents if necessary

    const auto surfaceSeriesList = m_graph->seriesList();
    for (const auto &series : surfaceSeriesList) {
        QSurfaceDataArray &array = const_cast<QSurfaceDataArray &>(series->dataArray());
        const int rowCount = array.size();
        const int columnCount = array.at(0).size();
        const bool dataAscending = array.at(0).at(0).x() < array.at(0).at(columnCount - 1).x();
        if (dataAscending != m_ascendingX) {
            // Create new array of equal size
            QSurfaceDataArray newArray;
            newArray.reserve(rowCount);
            for (int i = 0; i < rowCount; i++)
                newArray.append(QSurfaceDataRow(columnCount));

            // Flip each row
            for (int i = 0; i < rowCount; i++) {
                QSurfaceDataRow oldRow = array.at(i);
                QSurfaceDataRow &newRow = newArray[i];
                for (int j = 0; j < columnCount; j++)
                    newRow[j] = oldRow.at(columnCount - 1 - j);
            }

            series->dataProxy()->resetArray(newArray);
        }
    }
}

void GraphModifier::toggleZAscending(int enabled)
{
    m_ascendingZ = enabled;

    // Flip data array contents if necessary
    const auto surfaceSeriesList = m_graph->seriesList();
    for (const auto &series : surfaceSeriesList) {
        QSurfaceDataArray &array = const_cast<QSurfaceDataArray &>(series->dataArray());
        const int rowCount = array.size();
        const int columnCount = array.at(0).size();
        const bool dataAscending = array.at(0).at(0).z() < array.at(rowCount - 1).at(0).z();
        if (dataAscending != m_ascendingZ) {
            // Create new array of equal size
            QSurfaceDataArray newArray;
            newArray.reserve(rowCount);
            for (int i = 0; i < rowCount; i++)
                newArray.append(QSurfaceDataRow(columnCount));

            // Flip each column
            for (int i = 0; i < rowCount; i++) {
                QSurfaceDataRow oldRow = array.at(rowCount - 1 - i);
                QSurfaceDataRow &newRow = newArray[i];
                for (int j = 0; j < columnCount; j++)
                    newRow[j] = oldRow.at(j);
            }

            series->dataProxy()->resetArray(newArray);
        }
    }
}

void GraphModifier::togglePolar(int enabled)
{
    m_graph->setPolar(enabled);
}

void GraphModifier::setCameraTargetX(int value)
{
    // Value is (-100, 100), normalize
    m_cameraTarget.setX(float(value) / 100.0f);
    m_graph->setCameraTargetPosition(m_cameraTarget);
    qDebug() << "m_cameraTarget:" << m_cameraTarget;
}

void GraphModifier::setCameraTargetY(int value)
{
    // Value is (-100, 100), normalize
    m_cameraTarget.setY(float(value) / 100.0f);
    m_graph->setCameraTargetPosition(m_cameraTarget);
    qDebug() << "m_cameraTarget:" << m_cameraTarget;
}

void GraphModifier::setCameraTargetZ(int value)
{
    // Value is (-100, 100), normalize
    m_cameraTarget.setZ(float(value) / 100.0f);
    m_graph->setCameraTargetPosition(m_cameraTarget);
    qDebug() << "m_cameraTarget:" << m_cameraTarget;
}

void GraphModifier::setGraphMargin(int value)
{
    m_graph->setMargin(qreal(value) / 100.0);
    qDebug() << "Setting margin:" << m_graph->margin() << value;
}

void GraphModifier::setLabelMargin(int offset)
{
    m_graph->setLabelMargin(qreal(offset) / 1000.0f);
    qDebug() << "Setting label offset:" << m_graph->labelMargin() << offset;
}

void GraphModifier::setXAxisSegemntCount(int count)
{
    m_graph->axisX()->setSegmentCount(count);
}

void GraphModifier::setYAxisSegemntCount(int count)
{
    m_graph->axisY()->setSegmentCount(count);
}

void GraphModifier::setZAxisSegemntCount(int count)
{
    m_graph->axisZ()->setSegmentCount(count);
}

void GraphModifier::setXAxisSubsegemntCount(int count)
{
    m_graph->axisX()->setSubSegmentCount(count);
}

void GraphModifier::setYAxisSubsegemntCount(int count)
{
    m_graph->axisY()->setSubSegmentCount(count);
}

void GraphModifier::setZAxisSubsegemntCount(int count)
{
    m_graph->axisZ()->setSubSegmentCount(count);
}

void GraphModifier::resetArrayAndSliders(QSurfaceDataArray array, float minZ, float maxZ, float minX, float maxX)
{
    m_axisMinSliderX->setValue(minX);
    m_axisMinSliderZ->setValue(minZ);
    m_axisRangeSliderX->setValue(maxX - minX);
    m_axisRangeSliderZ->setValue(maxZ - minZ);

    m_theSeries->dataProxy()->resetArray(array);
}

void GraphModifier::changeShadowQuality(int quality)
{
    QtGraphs3D::ShadowQuality sq = QtGraphs3D::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}

void GraphModifier::changeTransparencyTechnique(int technique)
{
    QtGraphs3D::TransparencyTechnique sq = QtGraphs3D::TransparencyTechnique(technique);
    m_graph->setTransparencyTechnique(sq);
}

void GraphModifier::changeSelectionMode(int mode)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        int flags = comboBox->itemData(mode).toInt();
        m_graph->setSelectionMode(QtGraphs3D::SelectionFlags(flags));
    }
}

void GraphModifier::changeRow()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Generating new values to a row at random pos";
        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);
        float i = float(QRandomGenerator::global()->bounded(m_zCount));

        QSurfaceDataRow newRow(m_xCount);
        float z = qMin(maxZ, (i * stepZ + minZ));
        for (float j = 0; j < m_xCount; j++) {
            float x = qMin(maxX, (j * stepX + minX));
            float R = qSqrt(x * x + z * z) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f + 1.2f;
            newRow[j].setPosition(QVector3D(x, y, z));
        }

        m_theSeries->dataProxy()->setRow(int(i), newRow);
    } else {
#ifdef MULTI_SERIES
        static int changeRowSeries = 0;
        qDebug() << "Generating new values to a row at random pos for series " << changeRowSeries;

        int row = QRandomGenerator::global()->bounded(m_zCount);
        QSurfaceDataRow newRow = createMultiRow(row, changeRowSeries, true);
        if (m_ascendingZ)
            m_multiseries[changeRowSeries]->dataProxy()->setRow(row, newRow);
        else
            m_multiseries[changeRowSeries]->dataProxy()->setRow((m_zCount - 1) - row, newRow);

        changeRowSeries++;
        if (changeRowSeries > 3)
            changeRowSeries = 0;
#else
        qDebug() << "Change row function active only for SqrtSin";
#endif
    }
}

QSurfaceDataRow GraphModifier::createMultiRow(int row, int series, bool change)
{
    int full = m_limitX * m_limitZ;
    float i = float(row);
    QSurfaceDataRow newRow(m_xCount);
    float z = float(i) - m_limitZ + 0.5f + m_multiSampleOffsetZ[series];
    if (m_ascendingX) {
        for (int j = 0; j < m_xCount; j++) {
            float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[series];
            float angle = (z * x) / float(full) * 1.57f;
            float y = qSin(angle * float(qPow(1.3f, series))) + 0.2f * float(change) + 1.1f *series;
            newRow[j].setPosition(QVector3D(x, y, z));
        }
    } else {
        for (int j = m_xCount - 1; j >= 0 ; j--) {
            float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[series];
            float angle = (z * x) / float(full) * 1.57f;
            float y = qSin(angle * float(qPow(1.3f, series))) + 0.2f * float(change) + 1.1f *series;
            newRow[(m_xCount - 1) - j].setPosition(QVector3D(x, y, z));
        }
    }

    return newRow;
}

void GraphModifier::populateRisingSeries(QSurface3DSeries *series, int rows, int columns,
                                         float minValue, float maxValue, bool ascendingX,
                                         bool ascendingZ)
{
    QSurfaceDataArray dataArray;
    dataArray.reserve(rows);
    float range = maxValue - minValue;
    int arraySize = rows * columns;
    for (int i = 0; i < rows; i++) {
        QSurfaceDataRow dataRow(columns);
        for (int j = 0; j < columns; j++) {
            float xValue = ascendingX ? float(j) : float(columns - j - 1);
            float yValue = minValue + (range * i * j / arraySize);
            float zValue = ascendingZ ? float(i) : float(rows - i - 1);
            dataRow[j].setPosition(QVector3D(xValue, yValue, zValue));
        }
        dataArray.append(dataRow);
    }
    series->dataProxy()->resetArray(dataArray);

}

void GraphModifier::populateNanSeries(QSurface3DSeries *series, int rows, int columns)
{
    QSurfaceDataArray dataArray;
    dataArray.reserve(rows);
    float minValue = 0.0f;
    float maxValue = 50.0f;
    float range = maxValue - minValue;
    int arraySize = rows * columns;
    for (int i = 0; i < rows; i++) {
        QSurfaceDataRow dataRow(columns);
        for (int j = 0; j < columns; j++) {
            float xValue = float(j);
            float yValue = i % 2 == 0 ? minValue + (range * i * j / arraySize) : std::numeric_limits<float>::quiet_NaN();
            float zValue = float(i);
            dataRow[j].setPosition(QVector3D(xValue, yValue, zValue));
        }
        dataArray.append(dataRow);
    }
    series->dataProxy()->resetArray(dataArray);
}

void GraphModifier::changeRows()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Generating new values to 3 rows from random pos";

        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);
        float start = float(QRandomGenerator::global()->bounded(m_zCount - 3));

        QSurfaceDataArray dataArray;

        for (float i = start; i < (start + 3.0f); i++) {
            QSurfaceDataRow newRow(m_xCount);
            float z = qMin(maxZ, (i * stepZ + minZ));
            for (float j = 0; j < m_xCount; j++) {
                float x = qMin(maxX, (j * stepX + minX));
                float R = qSqrt(x * x + z * z) + 0.01f;
                float y = (qSin(R) / R + 0.24f) * 1.61f + 1.2f;
                newRow[j].setPosition(QVector3D(x, y, z));
            }
            dataArray.append(newRow);
        }

        m_theSeries->dataProxy()->setRows(int(start), dataArray);
    } else {
#ifdef MULTI_SERIES
        static int changeRowSeries = 0;
        qDebug() << "Generating new values for 3 rows at random pos for series " << changeRowSeries;

        int row = QRandomGenerator::global()->bounded(m_zCount - 3);
        QSurfaceDataArray dataArray;
        for (int i = 0; i < 3; i++) {
            QSurfaceDataRow newRow = createMultiRow(row + i, changeRowSeries, true);
            dataArray.append(newRow);
        }
        m_multiseries[changeRowSeries]->dataProxy()->setRows(row, dataArray);

        changeRowSeries++;
        if (changeRowSeries > 3)
            changeRowSeries = 0;
#else
        qDebug() << "Change row function active only for SqrtSin";
#endif
    }
}

void GraphModifier::changeItem()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Generating new values for an item at random pos";
        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);
        float i = float(QRandomGenerator::global()->bounded(m_zCount));
        float j = float(QRandomGenerator::global()->bounded(m_xCount));

        float x = qMin(maxX, (j * stepX + minX));
        float z = qMin(maxZ, (i * stepZ + minZ));
        float R = qSqrt(x * x + z * z) + 0.01f;
        float y = (qSin(R) / R + 0.24f) * 1.61f + 1.2f;
        QSurfaceDataItem newItem(QVector3D(x, y, z));

        m_theSeries->dataProxy()->setItem(int(i), int(j), newItem);
    } else {
#ifdef MULTI_SERIES
        static int changeItemSeries = 0;
        int full = m_limitX * m_limitZ;
        float i = float(QRandomGenerator::global()->bounded(m_zCount));
        float j = float(QRandomGenerator::global()->bounded(m_xCount));
        float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[changeItemSeries];
        float z = float(i) - m_limitZ + 0.5f + m_multiSampleOffsetZ[changeItemSeries];
        float angle = (z * x) / float(full) * 1.57f;
        float y = qSin(angle * float(qPow(1.3f, changeItemSeries))) + 0.2f + 1.1f *changeItemSeries;
        QSurfaceDataItem newItem(QVector3D(x, y, z));

        if (m_ascendingZ && m_ascendingX)
            m_multiseries[changeItemSeries]->dataProxy()->setItem(int(i), int(j), newItem);
        else if (!m_ascendingZ && m_ascendingX)
            m_multiseries[changeItemSeries]->dataProxy()->setItem(m_zCount - 1 - int(i), int(j), newItem);
        else if (m_ascendingZ && !m_ascendingX)
            m_multiseries[changeItemSeries]->dataProxy()->setItem(int(i), m_xCount - 1 - int(j), newItem);
        else
            m_multiseries[changeItemSeries]->dataProxy()->setItem(m_zCount - 1 - int(i), m_xCount - 1 - int(j), newItem);
        //m_multiseries[changeItemSeries]->setSelectedPoint(QPoint(i, j));
        changeItemSeries++;
        if (changeItemSeries > 3)
            changeItemSeries = 0;
#else
        qDebug() << "Change item function active only for SqrtSin";
#endif
    }
}

void GraphModifier::changeMultipleRows()
{
    for (int i = 0; i < 30; i++)
        changeRow();
}

void GraphModifier::changeMultipleItem()
{
    for (int i = 0; i < 30; i++)
        changeItem();
}

void GraphModifier::addRow()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Adding a new row";

        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);

        QSurfaceDataRow newRow(m_xCount);
        float z = float(m_addRowCounter) * stepZ + minZ;
        for (float j = 0; j < m_xCount; j++) {
            float x = qMin(maxX, (j * stepX + minX));
            float R = qSqrt(x * x + z * z) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f + 1.0f;
            newRow[j].setPosition(QVector3D(x, y, z));
        }
        m_addRowCounter++;

        m_theSeries->dataProxy()->addRow(newRow);
    } else {
#ifdef MULTI_SERIES
        qDebug() << "Adding a row into series 3";
        int full = m_limitX * m_limitZ;
        int series = 2;

        QSurfaceDataRow newRow(m_xCount);
        float z = float(m_addRowCounter) - m_limitZ + 0.5f + m_multiSampleOffsetZ[series];
        for (int j = 0; j < m_xCount; j++) {
            float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[series];
            float angle = float(z * x) / float(full) * 1.57f;
            newRow[j].setPosition(QVector3D(x, qSin(angle *float(qPow(1.3f, series))) + 1.1f * series, z));
        }
        m_addRowCounter++;

        m_multiseries[series]->dataProxy()->addRow(newRow);
#else
            qDebug() << "Add row function active only for SqrtSin";
#endif
    }
}

void GraphModifier::addRows()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Adding few new row";

        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);

        QSurfaceDataArray dataArray;

        for (int i = 0; i < 3; i++) {
            QSurfaceDataRow newRow(m_xCount);
            float z = m_addRowCounter * stepZ + minZ;
            for (float j = 0; j < m_xCount; j++) {
                float x = qMin(maxX, (j * stepX + minX));
                float R = qSqrt(x * x + z * z) + 0.01f;
                float y = (qSin(R) / R + 0.24f) * 1.61f + 1.0f;
                newRow[j].setPosition(QVector3D(x, y, z));
            }
            dataArray.append(newRow);
            m_addRowCounter++;
        }

        m_theSeries->dataProxy()->addRows(dataArray);
    } else {
#ifdef MULTI_SERIES
    qDebug() << "Adding 3 rows into series 3";
    int changedSeries = 2;

    QSurfaceDataArray dataArray;
    for (int i = 0; i < 3; i++) {
        QSurfaceDataRow newRow = createMultiRow(m_addRowCounter, changedSeries, false);
        dataArray.append(newRow);
        m_addRowCounter++;
    }

    m_multiseries[changedSeries]->dataProxy()->addRows(dataArray);
#else
        qDebug() << "Add rows function active only for SqrtSin";
#endif
    }
}

void GraphModifier::insertRow()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Inserting a row";
        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);

        QSurfaceDataRow newRow(m_xCount);
        float z = qMin(maxZ, (float(m_insertTestZPos) * stepZ + minZ + (stepZ / 2.0f)));
        for (float j = 0; j < m_xCount; j++) {
            float x = qMin(maxX, (j * stepX + minX));
            float R = qSqrt(x * x + z * z) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f + 1.3f;
            newRow[j].setPosition(QVector3D(x, y, z));
        }
        m_insertTestZPos++;

        m_theSeries->dataProxy()->insertRow(m_insertTestIndexPos, newRow);
        m_insertTestIndexPos += 2;
    } else {
#ifdef MULTI_SERIES
    qDebug() << "Inserting a row into series 3";
    int full = m_limitX * m_limitZ;
    int changedSeries = 2;

    QSurfaceDataRow newRow(m_xCount);
    float z = float(m_insertTestZPos) - m_limitZ + m_multiSampleOffsetZ[changedSeries];
    for (int j = 0; j < m_xCount; j++) {
        float x = float(j) - m_limitX + m_multiSampleOffsetX[changedSeries];
        float angle = (z * x) / float(full) * 1.57f;
        newRow[j].setPosition(QVector3D(x + 0.5f,
                                           qSin(angle * float(qPow(1.3f, changedSeries))) + 1.2f * changedSeries,
                                           z + 1.0f));
    }

    m_insertTestZPos++;

    m_multiseries[2]->dataProxy()->insertRow(m_insertTestIndexPos, newRow);
    m_insertTestIndexPos += 2;
#else
        qDebug() << "Insert row function active only for SqrtSin";
#endif
    }
}

void GraphModifier::insertRows()
{
    if (m_activeSample == GraphModifier::SqrtSin) {
        qDebug() << "Inserting 3 rows";
        float minX = -10.0f;
        float maxX = 10.0f;
        float minZ = -10.0f;
        float maxZ = 10.0f;
        float stepX = (maxX - minX) / float(m_xCount - 1);
        float stepZ = (maxZ - minZ) / float(m_zCount - 1);

        QSurfaceDataArray dataArray;
        for (int i = 0; i < 3; i++) {
            QSurfaceDataRow newRow(m_xCount);
            float z = qMin(maxZ, (float(m_insertTestZPos) * stepZ + minZ + i * (stepZ / 4.0f)));
            for (float j = 0; j < m_xCount; j++) {
                float x = qMin(maxX, (j * stepX + minX));
                float R = qSqrt(x * x + z * z) + 0.01f;
                float y = (qSin(R) / R + 0.24f) * 1.61f + 1.3f;
                newRow[j].setPosition(QVector3D(x, y, z));
            }
            dataArray.append(newRow);
        }
        m_insertTestZPos++;

        m_theSeries->dataProxy()->insertRows(m_insertTestIndexPos, dataArray);
        m_insertTestIndexPos += 4;
    } else {
#ifdef MULTI_SERIES
    qDebug() << "Inserting 3 rows into series 3";
    int full = m_limitX * m_limitZ;
    int changedSeries = 2;

    QSurfaceDataArray dataArray;
    float zAdd = 0.25f;
    for (int i = 0; i < 3; i++) {
        QSurfaceDataRow newRow(m_xCount);
        float z = float(m_insertTestZPos) - m_limitZ  + 0.5f + zAdd + m_multiSampleOffsetZ[changedSeries];
        for (int j = 0; j < m_xCount; j++) {
            float x = float(j) - m_limitX + 0.5f + m_multiSampleOffsetX[changedSeries];
            float angle = (z * x) / float(full) * 1.57f;
            float y = qSin(angle * float(qPow(1.3f, changedSeries))) + + 1.2f * changedSeries;
            newRow[j].setPosition(QVector3D(x, y, z));
        }
        zAdd += 0.25f;
        dataArray.append(newRow);
    }

    m_insertTestZPos++;

    m_multiseries[2]->dataProxy()->insertRows(m_insertTestIndexPos, dataArray);
    m_insertTestIndexPos += 4;
#else
        qDebug() << "Insert rows function active only for SqrtSin";
#endif
    }
}

void GraphModifier::removeRow()
{
    qDebug() << "Remove an arbitrary row";
    if (m_zCount < 1)
        return;

    int row = QRandomGenerator::global()->bounded(m_zCount);

#ifdef MULTI_SERIES
    int series = QRandomGenerator::global()->bounded(4);
    m_multiseries[series]->dataProxy()->removeRows(row, 1);
#else
    m_theSeries->dataProxy()->removeRows(row, 1);
#endif
    m_zCount--;
}

void GraphModifier::resetArray()
{
    qDebug() << "Reset series data array";
    int rows = 10;
    int columns = 10;
    float randFactor = float(QRandomGenerator::global()->bounded(100)) / 100.0f;
    QSurfaceDataArray planeArray;
    planeArray.reserve(rows);

    for (int i = 0; i < rows; i++) {
        planeArray.append(QSurfaceDataRow());
        planeArray[i].resize(columns);
        for (int j = 0; j < columns; j++) {
            planeArray[i][j].setX(float(j) * randFactor);
            planeArray[i][j].setY(float(i - j) * randFactor);
            planeArray[i][j].setZ(float(i));
        }
    }

#ifdef MULTI_SERIES
    int series = QRandomGenerator::global()->bounded(4);
    m_multiseries[series]->dataProxy()->resetArray(planeArray);
#else
    m_theSeries->dataProxy()->resetArray(planeArray);
#endif
}

void GraphModifier::resetArrayEmpty()
{
#ifdef MULTI_SERIES
    int series = QRandomGenerator::global()->bounded(4);
    m_multiseries[series]->dataProxy()->resetArray();
#else
    m_theSeries->dataProxy()->resetArray();
#endif
}

void GraphModifier::massiveDataTest()
{
    static int testPhase = 0;
    static const int cacheSize = 1000;
    const int columns = 200;
    const int rows = 200000;
    const int visibleRows = 200;
    const float yRangeMin = 0.0f;
    const float yRangeMax = 1.0f;
    const float yRangeMargin = 0.05f;
    static QTimer *massiveTestTimer = 0;
    static QSurface3DSeries *series = new QSurface3DSeries;

    // To speed up massive array creation, we generate a smaller cache array
    // and copy rows from that to our main array
    if (!m_massiveTestCacheArray.size()) {
        m_massiveTestCacheArray.reserve(cacheSize);
        float minY = yRangeMin + yRangeMargin;
        float maxY = yRangeMax - yRangeMargin;
        float rowBase = minY;
        float direction = 1.0f;
        for (int i = 0; i < cacheSize; i++) {
            m_massiveTestCacheArray.append(QSurfaceDataRow());
            m_massiveTestCacheArray[i].resize(columns);
            rowBase += direction * (float(QRandomGenerator::global()->bounded(3)) / 100.0f);
            if (rowBase > maxY) {
                rowBase = maxY;
                direction = -1.0f;
            } else if (rowBase < minY) {
                rowBase = minY;
                direction = 1.0f;
            }
            for (int j = 0; j < columns; j++) {
                float randFactor = float(QRandomGenerator::global()->bounded(100)) / (100 / yRangeMargin);
                m_massiveTestCacheArray[i][j].setX(float(j));
                m_massiveTestCacheArray[i][j].setY(rowBase + randFactor);
                // Z value is irrelevant, we replace it anyway when we take row to use
            }
        }
        massiveTestTimer = new QTimer;
    }

    switch (testPhase) {
    case 0: {
        qDebug() << __FUNCTION__ << testPhase << ": Setting the graph up...";
        QValue3DAxis *xAxis = new QValue3DAxis();
        QValue3DAxis *yAxis = new QValue3DAxis();
        QValue3DAxis *zAxis = new QValue3DAxis();
        xAxis->setRange(0.0f, float(columns));
        yAxis->setRange(yRangeMin, yRangeMax);
        zAxis->setRange(0.0f, float(visibleRows));
        xAxis->setSegmentCount(1);
        yAxis->setSegmentCount(1);
        zAxis->setSegmentCount(1);
        m_graph->setMeasureFps(true);
        m_graph->setAxisX(xAxis);
        m_graph->setAxisY(yAxis);
        m_graph->setAxisZ(zAxis);
        m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Right);
        m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::None);
        const auto surfaceSeriesList = m_graph->seriesList();
        for (const auto &series : surfaceSeriesList)
            m_graph->removeSeries(series);

        qDebug() << __FUNCTION__ << testPhase << ": Creating massive array..."
                 << rows << "x" << columns;
        // Reset to zero first to avoid having memory allocated for two massive arrays at the same
        // time on the second and subsequent runs.
        series->dataProxy()->resetArray();
        QSurfaceDataArray massiveArray;
        massiveArray.reserve(rows);

        for (int i = 0; i < rows; i++) {
            QSurfaceDataRow newRow(m_massiveTestCacheArray.at(i % cacheSize));
            for (int j = 0; j < columns; j++)
                newRow[j].setZ(float(i));
            massiveArray.append(newRow);
        }
        qDebug() << __FUNCTION__ << testPhase << ": Massive array creation finished!";

        series->dataProxy()->resetArray(massiveArray);
        m_graph->addSeries(series);
        break;
    }
    case 1: {
        qDebug() << __FUNCTION__ << testPhase << ": Scroll";
        QObject::disconnect(massiveTestTimer, 0, this, 0);
        QObject::connect(massiveTestTimer, &QTimer::timeout, this,
                         &GraphModifier::massiveTestScroll);
        massiveTestTimer->start(16);
        break;
    }
    case 2: {
        qDebug() << __FUNCTION__ << testPhase << ": Append and scroll";
        massiveTestTimer->stop();
        QObject::disconnect(massiveTestTimer, 0, this, 0);
        QObject::connect(massiveTestTimer, &QTimer::timeout, this,
                         &GraphModifier::massiveTestAppendAndScroll);
        m_graph->axisZ()->setRange(rows - visibleRows, rows);
        massiveTestTimer->start(16);
        break;
    }
    default:
        QObject::disconnect(massiveTestTimer, 0, this, 0);
        massiveTestTimer->stop();
        qDebug() << __FUNCTION__ << testPhase << ": Resetting the test";
        testPhase = -1;
    }
    testPhase++;
}

void GraphModifier::massiveTestScroll()
{
    const int scrollAmount = 20;
    int maxRows = m_graph->seriesList().at(0)->dataProxy()->rowCount();
    int min = m_graph->axisZ()->min() + scrollAmount;
    int max = m_graph->axisZ()->max() + scrollAmount;
    if (max >= maxRows) {
        max = max - min;
        min = 0;
    }
    m_graph->axisZ()->setRange(min, max);
}

void GraphModifier::massiveTestAppendAndScroll()
{
    const int addedRows = 50;
    int maxRows = m_graph->seriesList().at(0)->dataProxy()->rowCount();
    int columns = m_graph->seriesList().at(0)->dataProxy()->columnCount();

    QSurfaceDataArray appendArray;
    appendArray.reserve(addedRows);
    for (int i = 0; i < addedRows; i++) {
        QSurfaceDataRow newRow(m_massiveTestCacheArray.at((i + maxRows) % 1000));
        for (int j = 0; j < columns; j++)
            newRow[j].setZ(float(maxRows + i));
        appendArray.append(newRow);
    }
    m_graph->seriesList().at(0)->dataProxy()->addRows(appendArray);
    int min = m_graph->axisZ()->min() + addedRows;
    int max = m_graph->axisZ()->max() + addedRows;
    m_graph->axisZ()->setRange(min, max);
}

void GraphModifier::testAxisReverse()
{
    static int counter = 0;
    const int rowCount = 16;
    const int colCount = 16;
    static QSurface3DSeries *series0 = 0;
    static QSurface3DSeries *series1 = 0;

    switch (counter) {
    case 0: {
        qDebug() << __FUNCTION__ << counter << "Setup test";
        const auto surfaceSeriesList = m_graph->seriesList();
        for (const auto &series : surfaceSeriesList)
            m_graph->removeSeries(series);
        const auto axes = m_graph->axes();
        for (const auto &axis : axes)
            m_graph->releaseAxis(axis);
        delete series0;
        delete series1;
        series0 = new QSurface3DSeries;
        series1 = new QSurface3DSeries;
        populateRisingSeries(series0, rowCount, colCount, 0.0f, 50.0f, true, true);
        populateRisingSeries(series1, rowCount, colCount, -20.0f, 30.0f, true, true);
        m_graph->axisX()->setRange(0.0f, 10.0f);
        m_graph->axisY()->setRange(-20.0f, 50.0f);
        m_graph->axisZ()->setRange(5.0f, 15.0f);
        m_graph->addSeries(series0);
        m_graph->addSeries(series1);
    }
        break;
    case 1: {
        qDebug() << __FUNCTION__ << counter << "Reverse X axis";
        m_graph->axisX()->setReversed(true);
    }
        break;
    case 2: {
        qDebug() << __FUNCTION__ << counter << "Reverse Y axis";
        m_graph->axisY()->setReversed(true);
    }
        break;
    case 3: {
        qDebug() << __FUNCTION__ << counter << "Reverse Z axis";
        m_graph->axisZ()->setReversed(true);
    }
        break;
    case 4: {
        qDebug() << __FUNCTION__ << counter << "Return all axes to normal";
        m_graph->axisX()->setReversed(false);
        m_graph->axisY()->setReversed(false);
        m_graph->axisZ()->setReversed(false);
    }
        break;
    case 5: {
        qDebug() << __FUNCTION__ << counter << "Reverse all axes";
        m_graph->axisX()->setReversed(true);
        m_graph->axisY()->setReversed(true);
        m_graph->axisZ()->setReversed(true);
    }
        break;
    default:
        qDebug() << __FUNCTION__ << "Resetting test";
        counter = -1;
    }
    counter++;
}

void GraphModifier::testNanSeries()
{
    static bool initalized = false;
    static bool enabled = false;
    const int rowCount = 20;
    const int colCount = 20;

    if (!initalized) {
        m_nanSeries = new QSurface3DSeries;
        populateNanSeries(m_nanSeries, rowCount, colCount);
        initalized = true;
    }

    if (!enabled) {
        const auto axes = m_graph->axes();
        for (const auto &axis : axes)
            m_graph->releaseAxis(axis);
        m_graph->addSeries(m_nanSeries);
        enabled = true;
    } else {
        const auto axes = m_graph->axes();
        for (const auto &axis : axes)
            m_graph->releaseAxis(axis);
        m_graph->removeSeries(m_nanSeries);
        enabled = false;
    }
}

void GraphModifier::setRowSanitization(int enabled) {
    if (m_nanSeries)
        m_nanSeries->setRowsSanitized(enabled);
}

void GraphModifier::testDataOrdering()
{
    static int counter = 0;
    const int rowCount = 20;
    const int colCount = 20;
    static QSurface3DSeries *series0 = 0;
    static QSurface3DSeries *series1 = 0;
    const float series0min = 0.0f;
    const float series0max = 50.0f;
    const float series1min = -20.0f;
    const float series1max = 30.0f;

    switch (counter) {
    case 0: {
        qDebug() << __FUNCTION__ << counter << "Setup test - both ascending";
        const auto surfaceSeriesList = m_graph->seriesList();
        for (const auto &series : surfaceSeriesList)
            m_graph->removeSeries(series);
        const auto axes = m_graph->axes();
        for (const auto &axis : axes)
            m_graph->releaseAxis(axis);
        delete series0;
        delete series1;
        series0 = new QSurface3DSeries;
        series1 = new QSurface3DSeries;
        populateRisingSeries(series0, rowCount, colCount, series0min, series0max, true, true);
        populateRisingSeries(series1, rowCount, colCount, series1min, series1max, true, true);
        m_graph->axisX()->setRange(5.0f, 15.0f);
        m_graph->axisY()->setRange(-20.0f, 50.0f);
        m_graph->axisZ()->setRange(5.0f, 15.0f);
        m_graph->addSeries(series0);
        m_graph->addSeries(series1);
    }
        break;
    case 1: {
        qDebug() << __FUNCTION__ << counter << "Ascending X, descending Z";
        populateRisingSeries(series0, rowCount, colCount, series0min, series0max, true, false);
        populateRisingSeries(series1, rowCount, colCount, series1min, series1max, true, false);
    }
        break;
    case 2: {
        qDebug() << __FUNCTION__ << counter << "Descending X, ascending Z";
        populateRisingSeries(series0, rowCount, colCount, series0min, series0max, false, true);
        populateRisingSeries(series1, rowCount, colCount, series1min, series1max, false, true);
    }
        break;
    case 3: {
        qDebug() << __FUNCTION__ << counter << "Both descending";
        populateRisingSeries(series0, rowCount, colCount, series0min, series0max, false, false);
        populateRisingSeries(series1, rowCount, colCount, series1min, series1max, false, false);
    }
        break;
    default:
        qDebug() << __FUNCTION__ << "Resetting test";
        counter = -1;
    }
    counter++;
}

void GraphModifier::changeMesh()
{
    static int model = 0;
    switch (model) {
    case 0:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Cylinder);
        m_graph->selectedSeries()->setMeshSmooth(false);
        break;
    case 1:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Cylinder);
        m_graph->selectedSeries()->setMeshSmooth(true);
        break;
    case 2:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Cone);
        m_graph->selectedSeries()->setMeshSmooth(false);
        break;
    case 3:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Cone);
        m_graph->selectedSeries()->setMeshSmooth(true);
        break;
    case 4:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Bar);
        m_graph->selectedSeries()->setMeshSmooth(false);
        break;
    case 5:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Bar);
        m_graph->selectedSeries()->setMeshSmooth(true);
        break;
    case 6:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Pyramid);
        m_graph->selectedSeries()->setMeshSmooth(false);
        break;
    case 7:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::Pyramid);
        m_graph->selectedSeries()->setMeshSmooth(true);
        break;
    case 8:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::BevelBar);
        m_graph->selectedSeries()->setMeshSmooth(false);
        break;
    case 9:
        m_graph->selectedSeries()->setMesh(QAbstract3DSeries::Mesh::BevelBar);
        m_graph->selectedSeries()->setMeshSmooth(true);
        break;
    }
    model++;
    if (model > 9)
        model = 0;
}

void GraphModifier::updateSamples()
{
    switch (m_activeSample) {
    case SqrtSin:
        toggleSqrtSin(true);
        break;

    case Plane:
        togglePlane(true);
        break;

    default:
        break;
    }
}

void GraphModifier::setAspectRatio(int ratio)
{
    qreal aspectRatio = qreal(ratio) / 10.0;
    m_graph->setAspectRatio(aspectRatio);
}

void GraphModifier::setHorizontalAspectRatio(int ratio)
{
    qreal aspectRatio = qreal(ratio) / 100.0;
    m_graph->setHorizontalAspectRatio(aspectRatio);
}

void GraphModifier::setSurfaceTexture(int enabled)
{
    if (enabled)
        m_multiseries[3]->setTexture(QImage(":/maps/mapimage"));
    else
        m_multiseries[3]->setTexture(QImage());
}

void GraphModifier::setSurfaceAlphaTexture(int enabled)
{
    if (enabled)
        m_multiseries[2]->setTexture(QImage(":/maps/opacitymask"));
    else
        m_multiseries[2]->setTexture(QImage());
}

void GraphModifier::setSurfaceAlphaTextureFile(int enabled)
{
    if (enabled)
        m_multiseries[1]->setTextureFile(":/maps/opacitymask");
    else
        m_multiseries[1]->setTextureFile("");
}
