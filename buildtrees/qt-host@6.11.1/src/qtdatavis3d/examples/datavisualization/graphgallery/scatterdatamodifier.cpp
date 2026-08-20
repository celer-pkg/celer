// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "scatterdatamodifier.h"
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtWidgets/qcombobox.h>

using namespace Qt::StringLiterals;

//#define RANDOM_SCATTER // Uncomment this to switch to random scatter

const int numberOfItems = 10000;
const float curveDivider = 7.5f;
const int lowerNumberOfItems = 900;
const float lowerCurveDivider = 0.75f;

ScatterDataModifier::ScatterDataModifier(Q3DScatter *scatter, QObject *parent) :
      QObject(parent),
      m_graph(scatter),
      m_itemCount(lowerNumberOfItems),
      m_curveDivider(lowerCurveDivider),
      //! [7]
      m_inputHandler(new AxesInputHandler(scatter))
      //! [7]
{
    //! [0]
    m_graph->activeTheme()->setType(Q3DTheme::ThemeStoneMoss);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualitySoftHigh);
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
    m_graph->scene()->activeCamera()->setZoomLevel(80.f);
    //! [0]

    //! [1]
    auto *proxy = new QScatterDataProxy;
    auto *series = new QScatter3DSeries(proxy);
    series->setItemLabelFormat(u"@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"_s);
    series->setMeshSmooth(m_smooth);
    m_graph->addSeries(series);
    //! [1]

    //! [8]
    // Give ownership of the handler to the graph and make it the active handler
    m_graph->setActiveInputHandler(m_inputHandler);
    //! [8]

    //! [9]
    // Give our axes to the input handler
    m_inputHandler->setAxes(m_graph->axisX(), m_graph->axisZ(), m_graph->axisY());
    //! [9]

    //! [2]
    addData();
    //! [2]
}

void ScatterDataModifier::addData()
{
    // Configure the axes according to the data
    //! [3]
    m_graph->axisX()->setTitle("X");
    m_graph->axisY()->setTitle("Y");
    m_graph->axisZ()->setTitle("Z");
    //! [3]

    //! [4]
    auto *dataArray = new QScatterDataArray;
    dataArray->reserve(m_itemCount);
    //! [4]

#ifdef RANDOM_SCATTER
    for (int i = 0; i < m_itemCount; ++i)
        dataArray->append(QScatterDataItem(randVector()));
#else
    //! [5]
    const float limit = qSqrt(m_itemCount) / 2.0f;
    for (int i = -limit; i < limit; ++i) {
        for (int j = -limit; j < limit; ++j) {
            const float x = float(i) + 0.5f;
            const float y = qCos(qDegreesToRadians(float(i * j) / m_curveDivider));
            const float z = float(j) + 0.5f;
            dataArray->append(QScatterDataItem({x, y, z}));
        }
    }
    //! [5]
#endif

    //! [6]
    m_graph->seriesList().at(0)->dataProxy()->resetArray(dataArray);
    //! [6]
}

void ScatterDataModifier::changeStyle(int style)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_style = comboBox->itemData(style).value<QAbstract3DSeries::Mesh>();
        if (!m_graph->seriesList().isEmpty())
            m_graph->seriesList().at(0)->setMesh(m_style);
    }
}

void ScatterDataModifier::setSmoothDots(int smooth)
{
    m_smooth = bool(smooth);
    QScatter3DSeries *series = m_graph->seriesList().at(0);
    series->setMeshSmooth(m_smooth);
}

void ScatterDataModifier::changeTheme(int theme)
{
    Q3DTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setType(Q3DTheme::Theme(theme));
    emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
    emit gridEnabledChanged(currentTheme->isGridEnabled());
}

void ScatterDataModifier::changePresetCamera()
{
    static int preset = Q3DCamera::CameraPresetFrontLow;

    m_graph->scene()->activeCamera()->setCameraPreset((Q3DCamera::CameraPreset)preset);

    if (++preset > Q3DCamera::CameraPresetDirectlyBelow)
        preset = Q3DCamera::CameraPresetFrontLow;
}

void ScatterDataModifier::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality); // connected to a checkbox in scattergraph.cpp
}

void ScatterDataModifier::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}

void ScatterDataModifier::setBackgroundEnabled(int enabled)
{
    m_graph->activeTheme()->setBackgroundEnabled(enabled == Qt::Checked);
}

void ScatterDataModifier::setGridEnabled(int enabled)
{
    m_graph->activeTheme()->setGridEnabled(enabled == Qt::Checked);
}

void ScatterDataModifier::toggleItemCount()
{
    if (m_itemCount == numberOfItems) {
        m_itemCount = lowerNumberOfItems;
        m_curveDivider = lowerCurveDivider;
    } else {
        m_itemCount = numberOfItems;
        m_curveDivider = curveDivider;
    }
    m_graph->seriesList().at(0)->dataProxy()->resetArray(0);
    addData();
}

void ScatterDataModifier::toggleRanges()
{
    if (!m_autoAdjust) {
        m_graph->axisX()->setAutoAdjustRange(true);
        m_graph->axisZ()->setAutoAdjustRange(true);
        m_inputHandler->setDragSpeedModifier(1.5f);
        m_autoAdjust = true;
    } else {
        m_graph->axisX()->setRange(-10.0f, 10.0f);
        m_graph->axisZ()->setRange(-10.0f, 10.0f);
        m_inputHandler->setDragSpeedModifier(15.0f);
        m_autoAdjust = false;
    }
}

QVector3D ScatterDataModifier::randVector()
{
    auto *generator = QRandomGenerator::global();
    const auto x = float(generator->bounded(100)) / 2.0f - float(generator->bounded(100)) / 2.0f;
    const auto y = float(generator->bounded(100)) / 100.0f - float(generator->bounded(100)) / 100.0f;
    const auto z = float(generator->bounded(100)) / 2.0f - float(generator->bounded(100)) / 2.0f;
    return {x, y, z};
}
