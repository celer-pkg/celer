// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "surfacegraphmodifier.h"

#include <QtCore/qmath.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGraphs/qsurface3dseries.h>
#include <QtGraphs/qsurfacedataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>
#include <QtGui/qimage.h>

SurfaceGraphModifier::SurfaceGraphModifier(Q3DSurfaceWidgetItem *surface, QObject *parent)
    : QObject(parent)
    , m_graph(surface)
{
    m_graph->setCameraZoomLevel(85.f);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::IsometricRight);
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme::MixSeries);
    m_graph->activeTheme()->setLabelBackgroundVisible(false);
    m_graph->activeTheme()->setLabelBorderVisible(false);

    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    m_sqrtSinProxy = new QSurfaceDataProxy();
    m_sqrtSinSeries = new QSurface3DSeries(m_sqrtSinProxy);
    fillSqrtSinProxy();

    m_sqrtSinSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    m_sqrtSinSeries->setShading(QSurface3DSeries::Shading::Flat);

    m_graph->axisX()->setLabelFormat("%.2f");
    m_graph->axisZ()->setLabelFormat("%.2f");
    m_graph->axisX()->setRange(-8.f, 8.f);
    m_graph->axisY()->setRange(0.f, 2.f);
    m_graph->axisZ()->setRange(-4.f, 4.f);
    m_graph->axisX()->setLabelAutoAngle(30.f);
    m_graph->axisY()->setLabelAutoAngle(90.f);
    m_graph->axisZ()->setLabelAutoAngle(30.f);

    m_graph->addSeries(m_sqrtSinSeries);

    m_graph->setDefaultInputHandler();
    m_graph->setZoomEnabled(true);
    m_graph->setSelectionMode(QtGraphs3D::SelectionFlag::None);

    connect(m_graph,
            &Q3DSurfaceWidgetItem::sliceImageChanged,
            this,
            &SurfaceGraphModifier::updateSliceImage);
}

SurfaceGraphModifier::~SurfaceGraphModifier() {}

void SurfaceGraphModifier::renderSliceToImage(QtGraphs3D::SliceCaptureType sliceType,
                                              int requestedIndex)
{
    m_graph->renderSliceToImage(-1, requestedIndex, sliceType);
}

void SurfaceGraphModifier::changeSelectionMode(bool checked, bool nopick)
{
    auto mode = nopick
                    ? QtGraphs3D::SelectionFlag::None
                    : (checked ? (QtGraphs3D::SelectionFlag::Row | QtGraphs3D::SelectionFlag::Slice)
                               : (QtGraphs3D::SelectionFlag::Column
                                  | QtGraphs3D::SelectionFlag::Slice));
    m_graph->setSelectionMode(mode);
}

void SurfaceGraphModifier::fillSqrtSinProxy()
{
    float stepX = (8.f - -8.f) / float(150 - 1);
    float stepZ = (4.f - -4.f) / float(150 - 1);

    QSurfaceDataArray dataArray;
    dataArray.reserve(150);
    for (int i = 0; i < 150; ++i) {
        QSurfaceDataRow newRow;
        newRow.reserve(150);
        float z = qMin(4.f, (i * stepZ + -4.f));
        for (int j = 0; j < 150; ++j) {
            float x = qMin(8.f, (j * stepX + -8.f));
            float R = qSqrt(0.25 * z * z + x * x) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f;
            newRow.append(QSurfaceDataItem(x, y, z));
        }
        dataArray.append(newRow);
    }

    m_sqrtSinProxy->resetArray(dataArray);
}
