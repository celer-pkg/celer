// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "scatterdatamodifier.h"
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGraphs/qscatter3dseries.h>
#include <QtGraphs/qscatterdataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtWidgets/qcombobox.h>

using namespace Qt::StringLiterals;

// #define RANDOM_SCATTER // Uncomment this to switch to random scatter

const int numberOfItems = 10000;
const float curveDivider = 7.5f;
const int lowerNumberOfItems = 900;
const float lowerCurveDivider = 0.75f;

ScatterDataModifier::ScatterDataModifier(Q3DScatterWidgetItem *scatter, QObject *parent)
    : QObject(parent)
    , m_graph(scatter)
    , m_itemCount(lowerNumberOfItems)
    , m_curveDivider(lowerCurveDivider)
{
    //! [0]
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::SoftHigh);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Front);
    m_graph->setCameraZoomLevel(80.f);
    // These are set through active theme
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme::MixSeries);
    m_graph->activeTheme()->setColorScheme(QGraphsTheme::ColorScheme::Dark);
    //! [0]

    //! [1]
    auto *proxy = new QScatterDataProxy;
    auto *series = new QScatter3DSeries(proxy);
    series->setItemLabelFormat(u"@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"_s);
    series->setMeshSmooth(m_smooth);
    m_graph->addSeries(series);
    //! [1]


    // Give ownership of the handler to the graph and make it the active handler
    //! [8]
    connect(m_graph,
            &Q3DGraphsWidgetItem::selectedElementChanged,
            this,
            &ScatterDataModifier::handleElementSelected);
    connect(m_graph, &Q3DGraphsWidgetItem::dragged, this, &ScatterDataModifier::handleAxisDragging);
    m_graph->setDragButton(Qt::LeftButton);
    //! [8]

    //! [2]
    addData();
    //! [2]
}

ScatterDataModifier::~ScatterDataModifier() = default;

void ScatterDataModifier::addData()
{
    // Configure the axes according to the data
    //! [3]
    m_graph->axisX()->setTitle("X");
    m_graph->axisY()->setTitle("Y");
    m_graph->axisZ()->setTitle("Z");
    //! [3]

    //! [4]
    QScatterDataArray dataArray;
    dataArray.reserve(m_itemCount);
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
            dataArray.append(QScatterDataItem(x, y, z));
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
    const auto *comboBox = qobject_cast<QComboBox *>(sender());
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
    QGraphsTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setTheme(QGraphsTheme::Theme(theme));
    emit backgroundVisibleChanged(currentTheme->isPlotAreaBackgroundVisible());
    emit gridVisibleChanged(currentTheme->isGridVisible());
}

void ScatterDataModifier::changePresetCamera()
{
    static int preset = int(QtGraphs3D::CameraPreset::FrontLow);

    m_graph->setCameraPreset((QtGraphs3D::CameraPreset) preset);

    if (++preset > int(QtGraphs3D::CameraPreset::DirectlyBelow))
        preset = int(QtGraphs3D::CameraPreset::FrontLow);
}

void ScatterDataModifier::shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality); // connected to a checkbox in scattergraph.cpp
}

void ScatterDataModifier::handleElementSelected(QtGraphs3D::ElementType type)
{
    //! [9]
    switch (type) {
    case QtGraphs3D::ElementType::AxisXLabel:
        m_state = StateDraggingX;
        break;
    case QtGraphs3D::ElementType::AxisYLabel:
        m_state = StateDraggingY;
        break;
    case QtGraphs3D::ElementType::AxisZLabel:
        m_state = StateDraggingZ;
        break;
    default:
        m_state = StateNormal;
        break;
    }
    //! [9]
}

//! [10]
void ScatterDataModifier::handleAxisDragging(QVector2D delta)
//! [10]
{
    float distance = 0.0f;
    //! [11]
    // Get scene orientation from active camera
    float xRotation = m_graph->cameraXRotation();
    float yRotation = m_graph->cameraYRotation();
    //! [11]

    //! [12]
    // Calculate directional drag multipliers based on rotation
    float xMulX = qCos(qDegreesToRadians(xRotation));
    float xMulY = qSin(qDegreesToRadians(xRotation));
    float zMulX = qSin(qDegreesToRadians(xRotation));
    float zMulY = qCos(qDegreesToRadians(xRotation));
    //! [12]

    //! [13]
    // Get the drag amount
    QPoint move = delta.toPoint();

    // Flip the effect of y movement if we're viewing from below
    float yMove = (yRotation < 0) ? -move.y() : move.y();
    //! [13]

    //! [14]
    // Adjust axes
    QValue3DAxis *axis = nullptr;
    switch (m_state) {
    case StateDraggingX:
        axis = m_graph->axisX();
        distance = (move.x() * xMulX - yMove * xMulY) / m_dragSpeedModifier;
        axis->setRange(axis->min() - distance, axis->max() - distance);
        break;
    case StateDraggingZ:
        axis = m_graph->axisZ();
        distance = (move.x() * zMulX + yMove * zMulY) / m_dragSpeedModifier;
        axis->setRange(axis->min() + distance, axis->max() + distance);
        break;
    case StateDraggingY:
        axis = m_graph->axisY();
        distance = move.y() / m_dragSpeedModifier; // No need to use adjusted y move here
        axis->setRange(axis->min() + distance, axis->max() + distance);
        break;
    default:
        break;
    }
    //! [14]
}

void ScatterDataModifier::changeShadowQuality(int quality)
{
    const auto sq = QtGraphs3D::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}

void ScatterDataModifier::setBackgroundVisible(int visible)
{
    m_graph->activeTheme()->setPlotAreaBackgroundVisible((bool)visible);
}

void ScatterDataModifier::setGridVisible(int visible)
{
    m_graph->activeTheme()->setGridVisible((bool)visible);
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
    m_graph->seriesList().at(0)->dataProxy()->resetArray();
    addData();
}

void ScatterDataModifier::toggleRanges()
{
    if (!m_autoAdjust) {
        m_graph->axisX()->setAutoAdjustRange(true);
        m_graph->axisZ()->setAutoAdjustRange(true);
        m_dragSpeedModifier = 1.5f;
        m_autoAdjust = true;
    } else {
        m_graph->axisX()->setRange(-10.0f, 10.0f);
        m_graph->axisZ()->setRange(-10.0f, 10.0f);
        m_dragSpeedModifier = 15.0f;
        m_autoAdjust = false;
    }
}

void ScatterDataModifier::adjustMinimumRange(float range)
{
    if (m_itemCount == lowerNumberOfItems)
        range *= 1.45f;
    else
        range *= 4.95f;
    m_graph->axisX()->setMin(range);
    m_graph->axisZ()->setMin(range);
    m_autoAdjust = false;
}

void ScatterDataModifier::adjustMaximumRange(float range)
{
    if (m_itemCount == lowerNumberOfItems)
        range *= 1.45f;
    else
        range *= 4.95f;
    m_graph->axisX()->setMax(range);
    m_graph->axisZ()->setMax(range);
    m_autoAdjust = false;
}

QVector3D ScatterDataModifier::randVector()
{
    auto *generator = QRandomGenerator::global();
    const auto x = float(generator->bounded(100)) / 2.0f - float(generator->bounded(100)) / 2.0f;
    const auto y = float(generator->bounded(100)) / 100.0f
                   - float(generator->bounded(100)) / 100.0f;
    const auto z = float(generator->bounded(100)) / 2.0f - float(generator->bounded(100)) / 2.0f;
    return {x, y, z};
}
