// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "custominstancing.h"
#include <math.h>
#include <QMatrix4x4>

struct InstanceTableEntry {
    QVector4D row0;
    QVector4D row1;
    QVector4D row2;
    QVector4D color;
    QVector4D instanceData;
};

void CustomInstancing::setInstanceCount(int instanceCount)
{
    if (m_instanceCount == instanceCount)
        return;

    m_instanceCount = instanceCount;
    emit instanceCountChanged(m_instanceCount);
}

QByteArray CustomInstancing::getInstanceBuffer(int *instanceCount)
{
    QByteArray instanceData;
    if (instanceCount)
        *instanceCount = m_instanceCount;
    if (m_instanceCount == 0)
        return instanceData;

    instanceData.resize(m_instanceCount * 20 * sizeof(float));
    auto array = reinterpret_cast<InstanceTableEntry*>(instanceData.data());

    const int rows = floor(sqrt(m_instanceCount));
    const int columns = m_instanceCount/rows;

    for (int i = 0; i < m_instanceCount; ++i) {
        QMatrix4x4 xform;

        int ix = i % columns;
        int iy = i / columns;
        float x = 100.0 * (ix - columns/2);
        float y = 100.0 * (iy - rows/2);
        float z = (x*x + y*y)/1000;

        xform(0, 3) += x;
        xform(1, 3) += y;
        xform(2, 3) += z;

        float scale = 0.75;
        xform(0, 0) = scale;
        xform(1, 1) = scale;
        xform(2, 2) = scale;

        QVector4D color(float(ix)/columns, float(iy)/columns, 1.0, float(i) / m_instanceCount);

        array[i] = {
            xform.row(0),
            xform.row(1),
            xform.row(2),
            color,
            {}
        };
    }

    return instanceData;
}
