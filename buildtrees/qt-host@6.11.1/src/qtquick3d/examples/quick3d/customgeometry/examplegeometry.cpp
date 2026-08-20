// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "examplegeometry.h"
#include <QRandomGenerator>
#include <QVector3D>

ExampleTriangleGeometry::ExampleTriangleGeometry()
{
    updateData();
}

void ExampleTriangleGeometry::setNormals(bool enable)
{
    if (m_hasNormals == enable)
        return;

    m_hasNormals = enable;
    emit normalsChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setNormalXY(float xy)
{
    if (m_normalXY == xy)
        return;

    m_normalXY = xy;
    emit normalXYChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setUV(bool enable)
{
    if (m_hasUV == enable)
        return;

    m_hasUV = enable;
    emit uvChanged();
    updateData();
    update();
}

void ExampleTriangleGeometry::setUVAdjust(float f)
{
    if (m_uvAdjust == f)
        return;

    m_uvAdjust = f;
    emit uvAdjustChanged();
    updateData();
    update();
}

//! [update data]
void ExampleTriangleGeometry::updateData()
{
    clear();

    int stride = 3 * sizeof(float);
    if (m_hasNormals)
        stride += 3 * sizeof(float);
    if (m_hasUV)
        stride += 2 * sizeof(float);

    QByteArray vertexData(3 * stride, Qt::Initialization::Uninitialized);
    float *p = reinterpret_cast<float *>(vertexData.data());

    // a triangle, front face = counter-clockwise
    *p++ = -1.0f; *p++ = -1.0f; *p++ = 0.0f;
    if (m_hasNormals) {
        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
    }
    if (m_hasUV) {
        *p++ = 0.0f + m_uvAdjust; *p++ = 0.0f + m_uvAdjust;
    }
    *p++ = 1.0f; *p++ = -1.0f; *p++ = 0.0f;
    if (m_hasNormals) {
        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
    }
    if (m_hasUV) {
        *p++ = 1.0f - m_uvAdjust; *p++ = 0.0f + m_uvAdjust;
    }
    *p++ = 0.0f; *p++ = 1.0f; *p++ = 0.0f;
    if (m_hasNormals) {
        *p++ = m_normalXY; *p++ = m_normalXY; *p++ = 1.0f;
    }
    if (m_hasUV) {
        *p++ = 1.0f - m_uvAdjust; *p++ = 1.0f - m_uvAdjust;
    }

    setVertexData(vertexData);
    setStride(stride);
    setBounds(QVector3D(-1.0f, -1.0f, 0.0f), QVector3D(+1.0f, +1.0f, 0.0f));

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);

    if (m_hasNormals) {
        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                     3 * sizeof(float),
                     QQuick3DGeometry::Attribute::F32Type);
    }

    if (m_hasUV) {
        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic,
                     m_hasNormals ? 6 * sizeof(float) : 3 * sizeof(float),
                     QQuick3DGeometry::Attribute::F32Type);
    }
}
//! [update data]

ExamplePointGeometry::ExamplePointGeometry()
{
    updateData();
}

void ExamplePointGeometry::updateData()
{
    clear();

    constexpr auto randomFloat = [](const float lowest, const float highest) -> float {
        return lowest + QRandomGenerator::global()->generateDouble() * (highest - lowest);
    };
    constexpr int NUM_POINTS = 2000;
    constexpr int stride = 3 * sizeof(float);

    QByteArray vertexData;
    vertexData.resize(NUM_POINTS * stride);
    float *p = reinterpret_cast<float *>(vertexData.data());

    for (int i = 0; i < NUM_POINTS; ++i) {
        *p++ = randomFloat(-5.0f, +5.0f);
        *p++ = randomFloat(-5.0f, +5.0f);
        *p++ = 0.0f;
    }

    setVertexData(vertexData);
    setStride(stride);
    setBounds(QVector3D(-5.0f, -5.0f, 0.0f), QVector3D(+5.0f, +5.0f, 0.0f));

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Points);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);
}
