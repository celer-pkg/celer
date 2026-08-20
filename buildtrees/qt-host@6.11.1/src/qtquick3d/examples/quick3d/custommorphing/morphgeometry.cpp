// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "morphgeometry.h"

#include <qmath.h>
//! [vertex struct]
struct Vertex {
    QVector3D position;
    QVector3D normal;
    QVector4D color;
};
//! [vertex struct]

//! [constructor]
MorphGeometry::MorphGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
    updateData();
}
//! [constructor]

//! [property]
void MorphGeometry::setGridSize(int gridSize)
{
    if (m_gridSize == gridSize)
        return;

    m_gridSize = gridSize;
    emit gridSizeChanged();
    updateData();
    update();
}
//! [property]

//! [updateData]
void MorphGeometry::updateData()
{
    clear();
    calculateGeometry();

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 3 * sizeof(float),
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::ColorSemantic, 6 * sizeof(float),
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);

    addTargetAttribute(0, QQuick3DGeometry::Attribute::PositionSemantic, 0);
    addTargetAttribute(0, QQuick3DGeometry::Attribute::NormalSemantic, m_targetPositions.size() * sizeof(float) * 3);
    addTargetAttribute(0, QQuick3DGeometry::Attribute::ColorSemantic,
                       m_targetPositions.size() * sizeof(float) * 3 + m_targetNormals.size() * sizeof(float) * 3);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::U32Type);

    const int numVertexes = m_positions.size();
    m_vertexBuffer.resize(numVertexes * sizeof(Vertex));
    Vertex *vert = reinterpret_cast<Vertex *>(m_vertexBuffer.data());

    for (int i = 0; i < numVertexes; ++i) {
        Vertex &v = vert[i];
        v.position = m_positions[i];
        v.normal = m_normals[i];
        v.color = m_colors[i];
    }
    m_targetBuffer.append(QByteArray(reinterpret_cast<char *>(m_targetPositions.data()), m_targetPositions.size() * sizeof(QVector3D)));
    m_targetBuffer.append(QByteArray(reinterpret_cast<char *>(m_targetNormals.data()), m_targetNormals.size() * sizeof(QVector3D)));
    m_targetBuffer.append(QByteArray(reinterpret_cast<char *>(m_targetColors.data()), m_targetColors.size() * sizeof(QVector4D)));

    setStride(sizeof(Vertex));
    setVertexData(m_vertexBuffer);
    setTargetData(m_targetBuffer);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    setBounds(boundsMin, boundsMax);

    m_indexBuffer = QByteArray(reinterpret_cast<char *>(m_indexes.data()), m_indexes.size() * sizeof(quint32));
    setIndexData(m_indexBuffer);
}
//! [updateData]

void MorphGeometry::calculateGeometry()
{
    float dw = 10.0; // width/2
    float dh = 10.0;
    int iw = m_gridSize;
    int ih = m_gridSize;
    float wf = dw * 2 / iw; //factor grid coord => position
    float hf = dh * 2 / ih;

    m_positions.clear();
    m_indexes.clear();
    m_targetPositions.clear();
    m_targetNormals.clear();
    m_targetColors.clear();

    constexpr float maxFloat = std::numeric_limits<float>::max();
    boundsMin = QVector3D(maxFloat, maxFloat, maxFloat);
    boundsMax = QVector3D(-maxFloat, -maxFloat, -maxFloat);

    // We construct a rectangular grid of iw times ih vertices;
    // ix and iy are indices into the grid. x, y, and z are the spatial
    // coordinates we calculate for each vertex. tx, ty, and tz are the
    // coordinates for the morph target.

    for (int iy = 0; iy < ih; ++iy) {
        for (int ix = 0; ix < iw; ++ix) {
            float x = ix * wf - dw;
            float z = iy * hf - dh;

            // The base shape is a cosine wave, and the corresponding normal
            // vectors are calculated from the partial derivatives.
            float y = 2 * qCos(x) + 3.0;
            QVector3D normal{2 * qSin(x), 2, 0};

            float tx = x * 1.2;
            float tz = z * 1.2;

            constexpr float R = 16;
            QVector3D targetPosition;
            QVector3D targetNormal;

            // The morph target shape is a hemisphere. Therefore we don't have
            // to do complex math to calculate the normal vector, since all vectors
            // from the center are normal to the surface of a sphere.
            if (tx*tx + tz*tz < R*R) {
                float ty = 0.4f * qSqrt(R*R - tx*tx - tz*tz);
                targetPosition = {tx, ty, tz};
                targetNormal = targetPosition;
            } else {
                targetPosition = {tx, -3, tz};
                targetNormal = {0, 1, 0};
            }

            // Finally, we make the outside edges of the shapes vertical, so they look nicer.
            if (ix == 0 || iy == 0 || ix == iw-1 || iy == ih-1) {
                int iix = qMin(iw-2, qMax(1, ix));
                int iiy = qMin(ih-2, qMax(1, iy));
                x = iix * wf - dw;
                z = iiy * hf - dh;
                y = -3.0;
                targetPosition.setY(-3);
            }

            if (iy >= ih-2)
                normal = {0, 0, 1};
            else if (iy <= 1)
                normal = {0, 0, -1};

            m_positions.append({x, y, z});
            m_normals.append(normal.normalized());

            m_targetPositions.append(targetPosition);
            m_targetNormals.append(targetNormal.normalized());

            // Set custom colors for the vertices:
            const float tmp = (y + 5.0f) / 10.0f;
            m_colors.append({1.0, tmp, tmp, 1.0f});

            const float ttmp = (ix % (iw / 8)) < 3 ? 0.5 : 1.0;
            m_targetColors.append({ttmp, ttmp, ttmp, 1.0f});

            // Note: We only use the bounds of the target positions since they are strictly
            // bigger than the original
            boundsMin.setX(std::min(boundsMin.x(), targetPosition.x()));
            boundsMin.setY(std::min(boundsMin.y(), targetPosition.y()));
            boundsMin.setZ(std::min(boundsMin.z(), targetPosition.z()));

            boundsMax.setX(std::max(boundsMax.x(), targetPosition.x()));
            boundsMax.setY(std::max(boundsMax.y(), targetPosition.y()));
            boundsMax.setZ(std::max(boundsMax.z(), targetPosition.z()));
        }
    }

    for (int ix = 0; ix < iw - 1; ++ix) {
        for (int iy = 0; iy < ih - 1; ++iy) {
            int idx = ix + iy * ih;
            m_indexes << idx << idx + iw << idx + iw + 1
                      << idx << idx + iw + 1 << idx + 1;
        }
    }
}

