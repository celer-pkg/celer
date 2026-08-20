// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "datasource.h"
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>

#include <algorithm>

DataSource::DataSource(QObject *parent) :
    QObject(parent)
{
    //! [3]
    qRegisterMetaType<QSurface3DSeries *>();
    //! [3]
}

DataSource::~DataSource()
{
    clearData();
}

void DataSource::generateData(int cacheCount, int rowCount, int columnCount,
                              float xMin, float xMax,
                              float yMin, float yMax,
                              float zMin, float zMax)
{
    if (!cacheCount || !rowCount || !columnCount)
        return;

    clearData();

    // Re-create the cache array
    m_data.resize(cacheCount);
    for (int i = 0; i < cacheCount; ++i) {
        QSurfaceDataArray &array = m_data[i];
        array.reserve(rowCount);
        for (int j = 0; j < rowCount; ++j)
            array.append(new QSurfaceDataRow(columnCount));
    }

    float xRange = xMax - xMin;
    float yRange = yMax - yMin;
    float zRange = zMax - zMin;
    int cacheIndexStep = columnCount / cacheCount;
    float cacheStep = float(cacheIndexStep) * xRange / float(columnCount);

    //! [0]
    // Populate caches
    auto *generator = QRandomGenerator::global();
    for (int i = 0; i < cacheCount; ++i) {
        QSurfaceDataArray &cache = m_data[i];
        float cacheXAdjustment = cacheStep * i;
        float cacheIndexAdjustment = cacheIndexStep * i;
        for (int j = 0; j < rowCount; ++j) {
            QSurfaceDataRow &row = *(cache[j]);
            float rowMod = (float(j)) / float(rowCount);
            float yRangeMod = yRange * rowMod;
            float zRangeMod = zRange * rowMod;
            float z = zRangeMod + zMin;
            qreal rowColWaveAngleMul = M_PI * M_PI * rowMod;
            float rowColWaveMul = yRangeMod * 0.2f;
            for (int k = 0; k < columnCount; k++) {
                float colMod = (float(k)) / float(columnCount);
                float xRangeMod = xRange * colMod;
                float x = xRangeMod + xMin + cacheXAdjustment;
                float colWave = float(qSin((2.0 * M_PI * colMod) - (1.0 / 2.0 * M_PI)) + 1.0);
                float y = (colWave * ((float(qSin(rowColWaveAngleMul * colMod) + 1.0))))
                        * rowColWaveMul
                        + generator->bounded(0.15f) * yRangeMod;

                int index = k + cacheIndexAdjustment;
                if (index >= columnCount) {
                    // Wrap over
                    index -= columnCount;
                    x -= xRange;
                }
                row[index] = QVector3D(x, y, z);
            }
        }
    }
    //! [0]
}

void DataSource::update(QSurface3DSeries *series)
{
    if (series && !m_data.isEmpty()) {
        //! [1]
        // Each iteration uses data from a different cached array
        if (++m_index >= m_data.size())
            m_index = 0;

        const QSurfaceDataArray &array = m_data.at(m_index);
        int newRowCount = array.size();
        int newColumnCount = array.at(0)->size();

        // If the first time or the dimensions of the cache array have changed,
        // reconstruct the reset array
        if (!m_resetArray || series->dataProxy()->rowCount() != newRowCount
                || series->dataProxy()->columnCount() != newColumnCount) {
            m_resetArray = new QSurfaceDataArray();
            m_resetArray->reserve(newRowCount);
            for (int i = 0; i < newRowCount; ++i)
                m_resetArray->append(new QSurfaceDataRow(newColumnCount));
        }

        // Copy items from our cache to the reset array
        for (int i = 0; i < newRowCount; ++i) {
            const QSurfaceDataRow &sourceRow = *(array.at(i));
            QSurfaceDataRow &row = *(*m_resetArray)[i];
            std::copy(sourceRow.cbegin(), sourceRow.cend(), row.begin());
        }

        // Notify the proxy that data has changed
        series->dataProxy()->resetArray(m_resetArray);
        //! [1]
    }
}

void DataSource::clearData()
{
    for (auto &array : std::as_const(m_data))
        qDeleteAll(array);
    m_data.clear();
}
