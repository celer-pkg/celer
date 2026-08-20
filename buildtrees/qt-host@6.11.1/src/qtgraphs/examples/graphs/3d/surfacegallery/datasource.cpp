// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "datasource.h"
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>

DataSource::DataSource(QObject *parent)
    : QObject(parent)
    , m_index(-1)
{
    //! [3]
    qRegisterMetaType<QSurface3DSeries *>();
    //! [3]
}

DataSource::~DataSource()
{
    clearData();
}

void DataSource::generateData(int cacheCount,
                              int rowCount,
                              int columnCount,
                              float xMin,
                              float xMax,
                              float yMin,
                              float yMax,
                              float zMin,
                              float zMax)
{
    if (!cacheCount || !rowCount || !columnCount)
        return;

    clearData();

    // Re-create the cache array
    m_data.resize(cacheCount);
    for (int i = 0; i < cacheCount; i++) {
        QSurfaceDataArray &array = m_data[i];
        array.reserve(rowCount);
        for (int j = 0; j < rowCount; j++)
            array.append(QSurfaceDataRow(columnCount));
    }

    float xRange = xMax - xMin;
    float yRange = yMax - yMin;
    float zRange = zMax - zMin;
    int cacheIndexStep = columnCount / cacheCount;
    float cacheStep = float(cacheIndexStep) * xRange / float(columnCount);

    //! [0]
    // Populate caches
    for (int i = 0; i < cacheCount; i++) {
        QSurfaceDataArray &cache = m_data[i];
        float cacheXAdjustment = cacheStep * i;
        float cacheIndexAdjustment = cacheIndexStep * i;
        for (int j = 0; j < rowCount; j++) {
            QSurfaceDataRow &row = cache[j];
            float rowMod = (float(j)) / float(rowCount);
            float yRangeMod = yRange * rowMod;
            float zRangeMod = zRange * rowMod;
            float z = zRangeMod + zMin;
            qreal waveAngleMul = M_PI * M_PI * rowMod;
            float waveMul = yRangeMod * 0.2f;
            for (int k = 0; k < columnCount; k++) {
                float colMod = (float(k)) / float(columnCount);
                float xRangeMod = xRange * colMod;
                float x = xRangeMod + xMin + cacheXAdjustment;
                float colWave = float(qSin((2.0 * M_PI * colMod) - (1.0 / 2.0 * M_PI)) + 1.0);
                float y = (colWave * ((float(qSin(waveAngleMul * colMod) + 1.0)))) * waveMul
                          + QRandomGenerator::global()->bounded(0.15f) * yRangeMod;

                int index = k + cacheIndexAdjustment;
                if (index >= columnCount) {
                    // Wrap over
                    index -= columnCount;
                    x -= xRange;
                }
                row[index] = QSurfaceDataItem(x, y, z);
            }
        }
    }
    //! [0]
}

void DataSource::update(QSurface3DSeries *series)
{
    if (series && m_data.size()) {
        //! [1]
        // Each iteration uses data from a different cached array
        m_index++;
        if (m_index > m_data.count() - 1)
            m_index = 0;

        QSurfaceDataArray array = m_data.at(m_index);
        int newRowCount = array.size();
        int newColumnCount = array.at(0).size();

        // If the first time or the dimensions of the cache array have changed,
        // reconstruct the reset array
        if (m_resetArray.isEmpty() || series->dataProxy()->rowCount() != newRowCount
            || series->dataProxy()->columnCount() != newColumnCount) {
            m_resetArray.clear();
            m_resetArray.reserve(newRowCount);
            for (int i = 0; i < newRowCount; i++)
                m_resetArray.append(QSurfaceDataRow(newColumnCount));
        }

        // Copy items from our cache to the reset array
        for (int i = 0; i < newRowCount; i++) {
            const QSurfaceDataRow &sourceRow = array.at(i);
            QSurfaceDataRow &row = m_resetArray[i];
            for (int j = 0; j < newColumnCount; j++)
                row[j].setPosition(sourceRow.at(j).position());
        }

        // Notify the proxy that data has changed
        series->dataProxy()->resetArray(m_resetArray);
        //! [1]
    }
}

void DataSource::clearData()
{
    for (int i = 0; i < m_data.size(); i++) {
        QSurfaceDataArray &array = m_data[i];
        array.clear();
    }
}
