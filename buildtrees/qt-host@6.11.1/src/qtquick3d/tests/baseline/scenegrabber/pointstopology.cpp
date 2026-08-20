// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "pointstopology.h"

#include <QVector3D>

PointsTopology::PointsTopology()
{
    updateData();
}

void PointsTopology::updateData()
{
    QByteArray vertexData;

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Points);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    setStride(32);

    // Four Points with Vertex + Normals
    const int vertexBufferSize = sizeof(float) * 4 * 2 * 4; // 2 x vec4 x 4
    vertexData.resize(vertexBufferSize);

    float *data = reinterpret_cast<float *>(vertexData.data());
    // Vertex 0
    data[0] = 0.0f;
    data[1] = 1.0f;
    data[2] = 0.0f;
    data[3] = 1.0f;
    // Normal 0
    data[4] = 0.0f;
    data[5] = 0.0f;
    data[6] = 1.0f;
    data[7] = 0.0f;

    // Vertex 1
    data[8] = -1.0f;
    data[9] = 0.0f;
    data[10] = 0.0f;
    data[11] = 1.0f;
    // Normal 1
    data[12] = 0.0f;
    data[13] = 0.0f;
    data[14] = 1.0f;
    data[15] = 0.0f;

    // Vertex 2
    data[16] = 1.0f;
    data[17] = 0.0f;
    data[18] = 0.0f;
    data[19] = 1.0f;
    // Normal 2
    data[20] = 0.0f;
    data[21] = 0.0f;
    data[22] = 1.0f;
    data[23] = 0.0f;

    // Vertex 3
    data[24] = 0.0f;
    data[25] = -1.0f;
    data[26] = 0.0f;
    data[27] = 1.0f;
    // Normal 3
    data[28] = 0.0f;
    data[29] = 0.0f;
    data[30] = 1.0f;
    data[31] = 0.0f;

    setVertexData(vertexData);

    setBounds(QVector3D(-1.0f, -1.0f, 0.0f), QVector3D(1.0f, 1.0f, 0.0f));
}
