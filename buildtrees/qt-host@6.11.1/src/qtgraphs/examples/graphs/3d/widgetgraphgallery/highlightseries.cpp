// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "highlightseries.h"

//! [2]
const float darkRedPos = 1.f;
const float redPos = 0.8f;
const float yellowPos = 0.6f;
const float greenPos = 0.4f;
const float darkGreenPos = 0.2f;
//! [2]

HighlightSeries::HighlightSeries()
{
    setDrawMode(QSurface3DSeries::DrawSurface);
    setShading(QSurface3DSeries::Shading::Flat);
    setVisible(false);
}

HighlightSeries::~HighlightSeries() = default;

//! [0]
void HighlightSeries::setTopographicSeries(TopographicSeries *series)
{
    m_topographicSeries = series;
    m_srcWidth = m_topographicSeries->dataArray().at(0).size();
    m_srcHeight = m_topographicSeries->dataArray().size();

    QObject::connect(m_topographicSeries,
                     &QSurface3DSeries::selectedPointChanged,
                     this,
                     &HighlightSeries::handlePositionChange);
}
//! [0]

//! [1]
void HighlightSeries::handlePositionChange(const QPoint &position)
{
    m_position = position;

    if (position == invalidSelectionPosition()) {
        setVisible(false);
        return;
    }

    int halfWidth = m_width / 2;
    int halfHeight = m_height / 2;

    int startX = position.x() - halfWidth;
    if (startX < 0)
        startX = 0;
    int endX = position.x() + halfWidth;
    if (endX > (m_srcWidth - 1))
        endX = m_srcWidth - 1;
    int startZ = position.y() - halfHeight;
    if (startZ < 0)
        startZ = 0;
    int endZ = position.y() + halfHeight;
    if (endZ > (m_srcHeight - 1))
        endZ = m_srcHeight - 1;

    const QSurfaceDataArray &srcArray = m_topographicSeries->dataArray();

    QSurfaceDataArray dataArray;
    dataArray.reserve(endZ - startZ);
    for (int i = startZ; i < endZ; ++i) {
        QSurfaceDataRow newRow;
        newRow.reserve(endX - startX);
        QSurfaceDataRow srcRow = srcArray.at(i);
        for (int j = startX; j < endX; ++j) {
            QVector3D pos = srcRow.at(j).position();
            pos.setY(pos.y() + m_heightAdjustment);
            newRow.append(QSurfaceDataItem(pos));
        }
        dataArray.append(newRow);
    }

    dataProxy()->resetArray(dataArray);
    setVisible(true);
}
//! [1]

//! [3]
void HighlightSeries::handleGradientChange(float value)
{
    float ratio = m_minHeight / value;

    QLinearGradient gr;
    gr.setColorAt(0.f, Qt::black);
    gr.setColorAt(darkGreenPos * ratio, Qt::darkGreen);
    gr.setColorAt(greenPos * ratio, Qt::green);
    gr.setColorAt(yellowPos * ratio, Qt::yellow);
    gr.setColorAt(redPos * ratio, Qt::red);
    gr.setColorAt(darkRedPos * ratio, Qt::darkRed);

    setBaseGradient(gr);
    setColorStyle(QGraphsTheme::ColorStyle::RangeGradient);

    handleZoomChange(ratio);
}
//! [3]

void HighlightSeries::handleZoomChange(float zoom)
{
    m_heightAdjustment = (1.2f - zoom) * 10.f;
}
