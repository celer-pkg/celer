// Copyright (C) 2024 Stan Morris.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "IndexedSpiralItem.h"
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <algorithm>

namespace {
    std::array<int, 4> colorParts(const QColor &color) {
        return {color.red(), color.green(), color.blue(), color.alpha()};
    }
}

IndexedSpiralItem::IndexedSpiralItem()
{
    setFlag(QQuickItem::ItemHasContents);
    setWidth(720);
    setHeight(720);

    connect(this, &IndexedSpiralItem::indexCountChanged,
            this, &QQuickItem::update);
}

QSGNode *IndexedSpiralItem::updatePaintNode(QSGNode *oldNode,
                                            UpdatePaintNodeData *)
{
    auto *n = static_cast<QSGGeometryNode *>(oldNode);
    if (!n) {
        n = new QSGGeometryNode();
        configureGeometryNode(n);
    } else {
        if (applyIndexChange(n))
            n->markDirty(QSGNode::DirtyGeometry);
    }

    return n;
}

// Return true if index count is changed
bool IndexedSpiralItem::applyIndexChange(QSGGeometryNode *geometryNode)
{
    auto *geometry = geometryNode->geometry();
    if (!geometry)
        return false;

    const int indexCount = geometry->indexCount();

    /* When the exposed index count is less than or equal to the number
     * of allocated indexes, the spiral grows outward from the middle.
     *
     * However when the same index count is greater than the number of
     * allocated indexes, the spiral will receed from the center towards
     * the outer edge.
     */

    const bool willGrowFromCenter = m_indexCount <= m_maxIndices;

    const int nextCount = willGrowFromCenter ? m_indexCount
                                             : 2*m_maxIndices - m_indexCount;

    const bool isDirectionChanging = willGrowFromCenter != m_growFromCenter;

    if (isDirectionChanging || nextCount != indexCount) {
        if (isDirectionChanging) {
            auto * const indexData = geometry->indexDataAsUShort();

            /* Swap direction of growth by reversing all the indices.
             *
             * According to value of m_growFromCenter, if:
             *   true - one-less-than-capacity hides last-added vertex
             *       which is at outer edge of spiral, and as the index
             *       count decreases the spiral gets smaller
             *   false - one-less-than-capacity hides first-added vertex
             *       which is in the center, and as the index count decreases
             *       the spiral becomes increasingly hollow
             */
            std::reverse(indexData, indexData + m_maxIndices);
            m_growFromCenter = willGrowFromCenter;
        }

        const int newCount = qBound(1,
                                    nextCount,
                                    m_maxIndices);
        geometry->setIndexCount(newCount);

        return true;
    }

    return false;
}

void IndexedSpiralItem::configureGeometryNode(QSGGeometryNode *geometryNode)
{
    // Set the geometry on the node
    auto *geometry = createGeometry();
    geometryNode->setGeometry(geometry);
    geometryNode->setFlag(QSGNode::OwnsGeometry);

    // Set a ColoredPoint2D material
    geometryNode->setMaterial(new QSGVertexColorMaterial());
    geometryNode->setFlag(QSGNode::OwnsMaterial);
}

QSGGeometry *IndexedSpiralItem::createGeometry() const
{
    auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(),
                                     m_maxVertices,
                                     m_maxIndices);

    geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
    geometry->setLineWidth(3);

    // populate the geometry
    configureGeometry(geometry);
    configureColors(geometry);

    return geometry;
}

void IndexedSpiralItem::configureGeometry(QSGGeometry *geometry) const
{
    constexpr double pi = 3.14159265f;
    constexpr int linesPerRing = 360; // 360 fits 60000 verts into 720x720 area
    constexpr float angleIncrement = 2 * pi / linesPerRing;
    constexpr float lineSeparation = 8;
    constexpr float radiusIncrement = lineSeparation / linesPerRing;

    float radius = 1.5f;
    float xOffset = width()/2;
    float yOffset = height()/2;

    float angle = float(2 * pi / 1000.f);;

    // Iterate through all vertices
    auto *pt = geometry->vertexDataAsColoredPoint2D();
    for (int i = 0; i < m_maxVertices; ++i, ++pt) {
        pt->x = radius * cos(angle) + xOffset;
        pt->y = radius * sin(angle) + yOffset;

        radius += radiusIncrement;

        // Keep angle in 2*pi radians -- makes debugging easier
        angle += angleIncrement;
        if (angle > 2 * M_PI)
            angle = (2 * M_PI) - angle;
    }

    auto *indexData = geometry->indexDataAsUShort();
    for (ushort i = 0; i < m_maxIndices; ++i, ++indexData) {
        *indexData = i;
    }
}

void IndexedSpiralItem::configureColors(QSGGeometry *geometry) const
{
    const auto colors = std::array<QColor, 3> {
        QColor("lime"),
        QColor("magenta"),
        QColor("white")
    };
    constexpr int segmentLength = 20;

    auto *pt = geometry->vertexDataAsColoredPoint2D();
    for (int i = 0; i < m_maxVertices; ++i, ++pt) {
        const bool isEndOfSpiral = i <= 20*segmentLength
                                   || i > (m_maxVertices - segmentLength);
        auto index = isEndOfSpiral              ? 2 :
                     ((i/segmentLength)%2 == 0) ? 1
                                                : 0;
        const auto [r, g, b, a] = colorParts(colors[index]);

        pt->r = r;
        pt->g = g;
        pt->b = b;
        pt->a = a;
    }
}

int IndexedSpiralItem::indexCount() const
{
    return m_indexCount;
}

void IndexedSpiralItem::setIndexCount(int newIndexCount)
{
    if (m_indexCount == newIndexCount)
        return;
    m_indexCount = newIndexCount;
    emit indexCountChanged();
}

int IndexedSpiralItem::maxIndices() const
{
    return m_maxIndices;
}
