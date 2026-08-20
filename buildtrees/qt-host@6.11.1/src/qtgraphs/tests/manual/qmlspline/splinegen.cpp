// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "splinegen.h"

SplineGen::SplineGen(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QScatter3DSeries *>();
}

SplineGen::~SplineGen()
{
    m_series->deleteLater();
    m_splineCache.clear();
}

void SplineGen::generateSpline(QScatter3DSeries *series, SplineType type, int points)
{
    if (m_series != series)
        m_series = series;

    switch (type) {
    case (SplineType::Circle):
        genCircle(points);
        break;
    case (SplineType::Helix):
        genHelix(points);
        break;
    case (SplineType::Stitch):
        genStitch(points);
        break;
    }
    series->dataProxy()->resetArray(m_splineCache.at(0));
}

void SplineGen::tickSpline(QScatter3DSeries *series)
{
    if (!series || series->dataProxy()->itemCount() == 0)
        return;

    static int index = 0;
    int points = series->dataProxy()->itemCount();

    QScatterDataArray newArray;
    newArray.reserve(points);

    const QScatterDataArray &cache = m_splineCache.at(index);
    for (int i = 0; i < points; i++) {
        newArray.append(cache.at(i));
    }

    series->dataProxy()->resetArray(newArray);
    index++;
    if (index >= m_cacheCount)
        index = 0;
}

void SplineGen::genCircle(int points)
{
    for (int i = 0; i < m_splineCache.size(); i++) {
        QScatterDataArray &array = m_splineCache[i];
        array.clear();
    }

    m_splineCache.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QScatterDataArray &array = m_splineCache[i];
        array.reserve(points);
        float offset = 2 * M_PI * i / m_cacheCount;
        for (int j = 0; j < points; j++) {
            float t = 2 * M_PI * j / points;
            float x = qCos(t + offset);
            float y = 0.5;
            float z = qSin(t + offset);
            array.append(QScatterDataItem(x, y, z));
        }
    }
}

void SplineGen::genHelix(int points)
{
    for (int i = 0; i < m_splineCache.size(); i++) {
        QScatterDataArray &array = m_splineCache[i];
        array.clear();
    }

    // create cache array;

    m_splineCache.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QScatterDataArray &array = m_splineCache[i];
        array.reserve(points);
        float offset = 2 * M_PI * i / m_cacheCount;
        for (int j = 0; j < points; j++) {
            //8 points per revolution

            float t = 4 * M_PI * j / points;
            float x = qCos(t + offset);
            float y = t;
            float z = qSin(t + offset);
            array.append(QScatterDataItem(x, y, z));
        }
    }
}

void SplineGen::genStitch(int points)
{
    for (int i = 0; i < m_splineCache.size(); i++) {
        QScatterDataArray &array = m_splineCache[i];
        array.clear();
    }
    m_splineCache.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QScatterDataArray &array = m_splineCache[i];

        float t = qSin(2 * M_PI * float(i) / float(m_cacheCount));
        // round to closest fitting amount
        int pointsPerAxis = int(qFloor(float(points) / 3.0));
        int rounds = int(qCeil(float(pointsPerAxis) / 2.0));
        array.reserve(pointsPerAxis * 3 + rounds);
        const QVector3D masks[3] = {QVector3D(t, 1, 0), QVector3D(1, 0, t), QVector3D(0, t, 1)};
        for (int j = 1; j <= rounds; j++) {
            bool top = true;
            for (int k = 0; k < 7; k++) {
                float value = top ? pointsPerAxis + 1 - j : j;
                value /= pointsPerAxis;
                top = !top;
                QVector3D p = masks[k % 3] * value;
                array.append(QScatterDataItem(p));
            }
        }
    }
}
