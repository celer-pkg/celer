// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "heightfieldgeometry_p.h"

/*!
    \qmltype HeightFieldGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.4
    \brief A height field geometry.

    This helper implements a height-field geometry. It defines a surface built from a grayscale image.
    The y-coordinate of the surface at a given point in the horizontal plane is determined by the
    pixel value at the corresponding point in the image. The image's x-axis and y-axis will go along
    the geometry's x-axis and z-axis respectively.
*/

/*!
    \qmlproperty vector3d HeightFieldGeometry::extents
    This property defines the extents of the height-field, that is
    the dimensions of a box large enough to always contain the geometry.
    The default value is (100, 100, 100) when the image is square.
*/

/*!
    \qmlproperty url HeightFieldGeometry::heightMap
    \obsolete

    This property defines the URL of the height map image.

    Use \l HeightFieldGeometry::source instead.
*/

/*!
    \qmlproperty url HeightFieldGeometry::source
    This property defines the URL of the height map image.
*/

/*!
    \qmlproperty bool HeightFieldGeometry::smoothShading
    This property defines whether the height map is shown with smooth shading
    or with hard angles between the squares of the map.

    The default value is \c true, meaning smooth shading is turned on.
*/


HeightFieldGeometry::HeightFieldGeometry()
{
    updateData();
}

const QUrl &HeightFieldGeometry::source() const
{
    return m_heightMapSource;
}

void HeightFieldGeometry::setSource(const QUrl &newSource)
{
    if (m_heightMapSource == newSource)
        return;
    m_heightMapSource = newSource;

    updateData();
    update();
    emit geometryChanged();

    emit sourceChanged();
}

bool HeightFieldGeometry::smoothShading() const
{
    return m_smoothShading;
}

void HeightFieldGeometry::setSmoothShading(bool smooth)
{
    if (m_smoothShading == smooth)
        return;
    m_smoothShading = smooth;

    updateData();
    update();
    emit geometryChanged();

    emit smoothShadingChanged();
}

const QVector3D &HeightFieldGeometry::extents() const
{
    return m_extents;
}

void HeightFieldGeometry::setExtents(const QVector3D &newExtents)
{
    m_extentsSetExplicitly = true;
    if (m_extents == newExtents)
        return;
    m_extents = newExtents;

    updateData();
    update();
    emit geometryChanged();

    emit extentsChanged();
}

namespace {
struct HeightFieldVertex
{
    QVector3D position;
    QVector3D normal;
    QVector2D uv;
};
}

void HeightFieldGeometry::updateData()
{
    const QQmlContext *context = qmlContext(this);

    const auto resolvedUrl = context ? context->resolvedUrl(m_heightMapSource) : m_heightMapSource;
    if (!resolvedUrl.isValid())
        return;

    clear();
    const auto qmlSource = QQmlFile::urlToLocalFileOrQrc(resolvedUrl);

    QImage heightMap(qmlSource);
    int numRows = heightMap.height();
    int numCols = heightMap.width();

    if (numRows < 2 || numCols < 2)
        return;

    const int numVertices = numRows * numCols;

    if (!m_extentsSetExplicitly) {
        auto prevExt = m_extents;
        if (numRows == numCols) {
            m_extents = {100, 100, 100};
        } else if (numRows < numCols) {
            float f = float(numRows) / float(numCols);
            m_extents = {100.f, 100.f, 100.f * f};
        } else {
            float f = float(numCols) / float(numRows);
            m_extents = {100.f * f, 100.f, 100.f};
        }
        if (m_extents != prevExt) {
            emit extentsChanged();
        }
    }

    QVector<HeightFieldVertex> vertices;
    vertices.reserve(numVertices);

    const float rowF = m_extents.z() / (numRows - 1);
    const float rowOffs = -m_extents.z() / 2;
    const float colF = m_extents.x() / (numCols - 1);
    const float colOffs = -m_extents.x() / 2;
    for (int x = 0; x < numCols; x++) {
        for (int y = 0; y < numRows; y++) {
            float f = heightMap.pixelColor(x, y).valueF() - 0.5;
            HeightFieldVertex vertex;
            vertex.position = QVector3D(x * colF + colOffs, f * m_extents.y(), y * rowF + rowOffs);
            vertex.normal = QVector3D(0, 0, 0);
            vertex.uv = QVector2D(float(x) / (numCols - 1), 1.f - float(y) / (numRows - 1));
            vertices.push_back(vertex);
        }
    }

    QVector<quint32> indices;
    for (int ix = 0; ix < numCols - 1; ++ix) {
        for (int iy = 0; iy < numRows - 1; ++iy) {
            const int idx = iy + ix * numRows;

            const auto tri0 = std::array<int, 3> { idx + numRows + 1, idx + numRows, idx };
            const auto tri1 = std::array<int, 3> { idx + 1, idx + numRows + 1, idx };

            for (const auto [i0, i1, i2] : { tri0, tri1 }) {
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                if (m_smoothShading) {
                    // Calculate face normal
                    const QVector3D e0 = vertices[i1].position - vertices[i0].position;
                    const QVector3D e1 = vertices[i2].position - vertices[i0].position;
                    QVector3D normal = QVector3D::crossProduct(e0, e1).normalized();

                    // Add normal to vertex, will normalize later
                    vertices[i0].normal += normal;
                    vertices[i1].normal += normal;
                    vertices[i2].normal += normal;
                }
            }
        }
    }

    if (m_smoothShading) {
        // Normalize
        for (auto &vertex : vertices)
            vertex.normal.normalize();
    }

    // Calculate bounds
    QVector3D boundsMin = vertices[0].position;
    QVector3D boundsMax = vertices[0].position;

    for (const auto &vertex : vertices) {
        const auto &p = vertex.position;
        boundsMin = QVector3D(qMin(boundsMin.x(), p.x()), qMin(boundsMin.y(), p.y()), qMin(boundsMin.z(), p.z()));
        boundsMax = QVector3D(qMax(boundsMax.x(), p.x()), qMax(boundsMax.y(), p.y()), qMax(boundsMax.z(), p.z()));
    }

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic, sizeof(QVector3D) * 2, QQuick3DGeometry::Attribute::F32Type);

    if (m_smoothShading)
        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, sizeof(QVector3D), QQuick3DGeometry::Attribute::F32Type);

    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0, QQuick3DGeometry::Attribute::ComponentType::U32Type);

    setStride(sizeof(HeightFieldVertex));
    QByteArray vertexBuffer(reinterpret_cast<char *>(vertices.data()), vertices.size() * sizeof(HeightFieldVertex));
    setVertexData(vertexBuffer);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    setBounds(boundsMin, boundsMax);

    QByteArray indexBuffer(reinterpret_cast<char *>(indices.data()), indices.size() * sizeof(quint32));
    setIndexData(indexBuffer);
}
