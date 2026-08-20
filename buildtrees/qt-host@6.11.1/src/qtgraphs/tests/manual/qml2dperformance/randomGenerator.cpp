// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "randomGenerator.h"
#include <iterator>

RandomGenerator::RandomGenerator(QObject *parent)
    : QObject(parent)
    , m_rowCount(0)
{}

int RandomGenerator::rowCount() const
{
    return m_rowCount;
}

void RandomGenerator::setRowCount(int rowCount)
{
    if (m_rowCount == rowCount)
        return;

    m_rowCount = rowCount;
    emit rowCountChanged(rowCount);
    generateCaches();
}

int RandomGenerator::activeSeriesIndex() const
{
    return m_activeSeriesIndex;
}

void RandomGenerator::setActiveSeriesIndex(int index)
{
    if (m_activeSeriesIndex == index)
        return;

    m_activeSeriesIndex = index;
    emit activeSeriesIndexChanged(index);
    nextCache();
}

void RandomGenerator::nextCache()
{
    m_currentCache = (m_currentCache + 1) % 60;

    if (m_activeSeriesIndex == 0)
        updateSurface3D();
    else if (m_activeSeriesIndex < 3)
        updateScatter3D(m_activeSeriesIndex);
    else
        update2DGraph(m_activeSeriesIndex);
}

void RandomGenerator::generateCaches()
{
    m_data.clear();
    m_data.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QList<qreal> &array = m_data[i];
        array.reserve(m_rowCount);
        for (int j = 0; j < m_rowCount; j++) {
            qreal y = float(QRandomGenerator::global()->bounded(100)) / (100.0);
            array.append(y);
        }
    }
}

void RandomGenerator::clearCaches()
{
    for (auto data : m_data) {
        data.clear();
    }
    m_data.clear();
}

void RandomGenerator::updateSurface3D()
{
    if (!m_surface3D)
        return;

    QSurfaceDataArray dataArray;
    dataArray.resize(m_rowCount);
    for (int i = 0; i < m_rowCount; i++) {
        //append two points
        QVector3D pos = QVector3D(i, m_data[m_currentCache][i], 0);
        dataArray[i].append(QSurfaceDataItem(pos));
    }

    m_surface3D->dataProxy()->resetArray(dataArray);
}

void RandomGenerator::updateScatter3D(int index)
{
    QScatter3DSeries *series = nullptr;
    if (index == 1)
        series = m_scatter3D;
    else
        series = m_spline3D;

    if (!series)
        return;

    QScatterDataArray dataArray;
    dataArray.reserve(m_rowCount);
    for (int i = 0; i < m_rowCount; i++) {
        QVector3D pos = QVector3D(i, m_data[m_currentCache][i], 0);
        dataArray.append(QScatterDataItem(pos));
    }

    series->dataProxy()->resetArray(dataArray);
}

void RandomGenerator::update2DGraph(int index)
{
    QXYSeries *series = nullptr;
    if (index == 3)
        series = m_line2D;
    else if (index == 4)
        series = m_spline2D;
    else
        series = m_scatter2D;

    if (!series)
        return;

    series->clear();
    for (int i = 0; i < m_rowCount; i++) {
        QPointF point = QPointF(i, m_data[m_currentCache][i]);
        series->append(point);
    }
}
