// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "cppinstancetable.h"
#include <math.h>
#include <QMatrix4x4>
#include <QRandomGenerator>
#include <QColor>

// Quick-and-dirty smoothed out noise generation. Probably not suitable for general use.
static QVector<float> generateNoiseTable(int dimension, int randomSeed)
{
    const int tableSize = dimension * dimension;
    QVector<float> table(tableSize);
    QRandomGenerator rgen(randomSeed);

    for (float &f: table)
        f = rgen.bounded(1.0) * rgen.bounded(1.0);

    // We select some initial points that will not be modified. This is the distance between them: (power of two)
    constexpr int delta = 16;

    // Then we average out those points to the points half way between them,
    // and continue with the points half way between those, and so on.
    // Pattern:
    // STS
    // TTT
    // STS
    // where S = source and T = target
    auto smooth = [dimension, &table](int x, int y, int d) {
        auto lookup = [&table,dimension](int x, int y) -> float {
            return table[x + y*dimension];
        };
        auto assign = [&table,dimension,d](int x, int y, float v) {
            if (x < dimension && y < dimension) {
                float e = d*1.0/dimension;
                float &z = table[x + y*dimension];
                z = (e*z + v)/(e+1);
            }
        };

        int x1 = x + d/2;
        int y1 = y + d/2;
        int x2 = qMin(dimension-1, x + d);
        int y2 = qMin(dimension-1, y + d);
        float z1 = lookup(x,y);
        float z2 = lookup(x2, y);
        float z3 = lookup(x, y2);
        float z4 = lookup(x2, y2);
        assign(x1, y, (z1+z2)/2);
        assign(x, y1, (z1+z3)/2);
        assign(x1, y1, (z1+z2+z3+z4)/4);
        assign(x1, y2, (z3+z4)/2);
        assign(x2, y1, (z2+z4)/2);
    };

    int d = delta;
    while (d > 1) {
        for (int ix = 0; ix < dimension; ix += d) {
            for (int iy = 0; iy < dimension; iy += d) {
                smooth(ix, iy, d);
            }
        }
        d = d/2;
    }

    //low-pass filter
    for (int i = dimension + 1; i < tableSize; ++i)
        table[i] = (table[i] + table[i-1] + table[i-dimension])/3;

    //normalize
    float min = 1.0;
    float max = 0.0;
    for (auto z : table) {
        min = qMin(z, min);
        max = qMax(z, max);
    }
    for (auto &z : table)
        z = (z - min) / (max - min);

    return table;
}

CppInstanceTable::CppInstanceTable(QQuick3DObject *parent) : QQuick3DInstancing(parent)
{
    m_randomSeed = QRandomGenerator::global()->generate();
}

CppInstanceTable::~CppInstanceTable()
{
}

int CppInstanceTable::gridSize() const
{
    return m_gridSize;
}

float CppInstanceTable::gridSpacing() const
{
    return m_gridSpacing;
}

int CppInstanceTable::randomSeed() const
{
    return m_randomSeed;
}

void CppInstanceTable::setGridSize(int gridSize)
{
    if (m_gridSize == gridSize)
        return;

    m_gridSize = gridSize;
    emit gridSizeChanged();
    markDirty();
    m_dirty = true;
}

void CppInstanceTable::setGridSpacing(float gridSpacing)
{
    if (qFuzzyCompare(m_gridSpacing, gridSpacing))
        return;

    m_gridSpacing = gridSpacing;
    emit gridSpacingChanged();
    markDirty();
    m_dirty = true;
}

void CppInstanceTable::setRandomSeed(int randomSeed)
{
    if (m_randomSeed == randomSeed)
        return;

    m_randomSeed = randomSeed;
    emit randomSeedChanged();
    markDirty();
    m_dirty = true;
}

class BlockTable
{
public:
    BlockTable(int dimension, int randomSeed) : gridSize(dimension), seaLevel(gridSize / 8)
    {
        noiseTable = generateNoiseTable(gridSize, randomSeed);
        lowestBlock.resize(gridSize * gridSize);

        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                // optimization: skip blocks that are obscured by neighbours
                int lowestVisible;
                if (i == 0 || j == 0 || i == gridSize - 1 || j == gridSize - 1) {
                    lowestVisible = 0;
                } else {
                    lowestVisible = terrainHeight(i, j);
                    lowestVisible = qMin(lowestVisible, terrainHeight(i - 1, j));
                    lowestVisible = qMin(lowestVisible, terrainHeight(i, j - 1));
                    lowestVisible = qMin(lowestVisible, terrainHeight(i + 1, j));
                    lowestVisible = qMin(lowestVisible, terrainHeight(i, j + 1));
                    lowestVisible = qMax(lowestVisible, seaLevel);
                }
                lowestBlock[idx(i, j)] = lowestVisible;
            }
        }
    }

    QColor getBlockColor(int i, int j, int k) const
    {
        const int maxHeight = gridSize / 2;
        int snowLine = maxHeight * 4 / 5 - QRandomGenerator::global()->bounded(maxHeight / 5);
        int treeLine = maxHeight * 3 / 5 - QRandomGenerator::global()->bounded(maxHeight / 5);
        if (k > terrainHeight(i, j)) {
            return Qt::blue;
        } else if (k > snowLine) {
            return Qt::white;
        } else if (k > treeLine) {
            return Qt::darkGray;
        } else {
            return QColor::fromHsvF(k * 0.7f / maxHeight, 0.7f, 0.5f, 1.0f);
        }
    }
    bool isWaterSurface(int i, int j, int k) const { return k == seaLevel && k > terrainHeight(i, j); }
    int lowestVisible(int i, int j) { return lowestBlock[idx(i, j)]; }
    int highestBlock(int i, int j) { return qMax(seaLevel, terrainHeight(i, j)); }

private:
    int idx(int i, int j) const { return i + j * gridSize; }
    int terrainHeight(int i, int j) const
    {
        const int maxHeight = gridSize / 2;
        return maxHeight * noiseTable[idx(i, j)];
    }

    QVector<float> noiseTable;
    QVector<int> lowestBlock;
    int gridSize;
    int seaLevel;
};

//! [getInstanceBuffer]
QByteArray CppInstanceTable::getInstanceBuffer(int *instanceCount)
{
    if (m_dirty) {
        BlockTable blocks(m_gridSize, m_randomSeed);
        m_instanceData.resize(0);

        auto idxToPos = [this](int i) -> float { return m_gridSpacing * (i - m_gridSize / 2); };

        int instanceNumber = 0;
        for (int i = 0; i < m_gridSize; ++i) {
            float xPos = idxToPos(i);
            for (int j = 0; j < m_gridSize; ++j) {
                float zPos = idxToPos(j);
                int lowest = blocks.lowestVisible(i, j);
                int highest = blocks.highestBlock(i, j);
                for (int k = lowest; k <= highest; ++k) {
                    float yPos = idxToPos(k);
                    QColor color = blocks.getBlockColor(i, j, k);
                    float waterAnimation = blocks.isWaterSurface(i, j, k) ? 1.0 : 0.0;
                    auto entry = calculateTableEntry({ xPos, yPos, zPos }, { 1.0, 1.0, 1.0 }, {}, color, { waterAnimation, 0, 0, 0 });
                    m_instanceData.append(reinterpret_cast<const char *>(&entry), sizeof(entry));
                    instanceNumber++;
                }
            }
        }
        m_instanceCount = instanceNumber;
        m_dirty = false;
    }
    if (instanceCount)
        *instanceCount = m_instanceCount;

    return m_instanceData;
}
//! [getInstanceBuffer]
