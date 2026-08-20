// Copyright (C) 2024 Stan Morris.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>
#include <cmath>
#include <numbers>
#include "SpiralItem.h"

namespace {
    std::array<int, 4> getParts(const QColor &color) {
        return {color.red(), color.green(), color.blue(), color.alpha()};
    }

    using ColorPalette = std::array<QColor, 3>;
    auto allocPalette = ColorPalette { QColor("aqua"), QColor("deepskyblue"), QColor("yellow") };
    auto setVCPalette = ColorPalette { QColor("yellow"), QColor("orange"), QColor("white") };
}

SpiralItem::SpiralItem()
{
    setFlag(QQuickItem::ItemHasContents);
    setWidth(960);
    setHeight(960);

    connect(this, &SpiralItem::vertexCountChanged,
            this, &QQuickItem::update);
    connect(this, &SpiralItem::resizeByVertexCountChanged,
            this, &SpiralItem::handleResizeByVertexCountChanged);

    emit resizeByVertexCountChanged();
}

QSGNode *SpiralItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
    auto *n = static_cast<QSGGeometryNode *>(oldNode);
    if (!n) {
        n = new QSGGeometryNode();
        setupGeometryNode(n);
    }

    if (auto *geometry = n->geometry()) {
        if (m_vertexCount != geometry->vertexCount()) {
            updateGeometry(geometry);
        }
    }

    n->markDirty(QSGNode::DirtyGeometry);

    return n;
}

void SpiralItem::updateGeometry(QSGGeometry *&geometry) {
    if (m_resetVertices) {
        m_resetVertices = false;

        // must reallocate vertices if using setVertexCount to resize
        if (m_resizeByVertexCount)
            reallocateVertices(geometry, m_maxVertices);
    }

    const int newCount = qBound(0, m_vertexCount, m_maxVertices);
    if (m_resizeByVertexCount) {
        // change vertex count using new setVertexCount() function
        geometry->setVertexCount(newCount);
    } else {
        // change vertex count using allocate()
        reallocateVertices(geometry, newCount);
    }

    logStatistics();
}

void SpiralItem::setupGeometryNode(QSGGeometryNode *geometryNode)
{
    // Set the geometry on the node
    auto *geometry = createGeometry();
    geometryNode->setGeometry(geometry);
    geometryNode->setFlag(QSGNode::OwnsGeometry);

    // Set a ColoredPoint2D material
    geometryNode->setMaterial(new QSGVertexColorMaterial());
    geometryNode->setFlag(QSGNode::OwnsMaterial);
}

QSGGeometry *SpiralItem::createGeometry()
{
    auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(),
                                     m_maxVertices);

    geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
    geometry->setLineWidth(3);

    // populate the geometry
    populateVertices(geometry, m_maxVertices);
    populateColors(geometry, m_maxVertices);

    return geometry;
}

void SpiralItem::populateVertices(QSGGeometry *geometry, const int count)
{
    constexpr double pi = 3.14159265f;
    constexpr int linesPerRing = 720;
    constexpr float angleIncrement = 2 * pi / linesPerRing;
    constexpr float lineSeparation = 6;
    constexpr float radiusIncrement = lineSeparation / linesPerRing;

    float radius = 1.5f;
    float xOffset = width()/2;
    float yOffset = height()/2;

    float angle = float(2 * pi / 1000.f);;

    // Iterate through all vertices
    auto *pt = geometry->vertexDataAsColoredPoint2D();
    for (int i = 0; i < count; ++i, ++pt) {
        pt->x = radius * cos(angle) + xOffset;
        pt->y = radius * sin(angle) + yOffset;

        radius += radiusIncrement;

        // Keep angle in 2*pi radians -- makes debugging easier
        angle += angleIncrement;
        if (angle > 2 * M_PI)
            angle = (2 * M_PI) - angle;
    }
}

void SpiralItem::populateColors(QSGGeometry *geometry, const int count)
{
    constexpr int segmentLength = 20;

    const auto& colors = m_resizeByVertexCount ? setVCPalette : allocPalette;

    auto *pt = geometry->vertexDataAsColoredPoint2D();
    for (int i = 0; i < count; ++i, ++pt) {
        const bool isEndOfSpiral = i <= 20*segmentLength
                                   || i > (m_maxVertices - segmentLength);
        auto index = isEndOfSpiral              ? 2 :
                     ((i/segmentLength)%2 == 0) ? 1
                                                : 0;
        const auto [r, g, b, a] = getParts(colors[index]);

        pt->r = r;
        pt->g = g;
        pt->b = b;
        pt->a = a;
    }
}

void SpiralItem::reallocateVertices(QSGGeometry *geometry, const int count) {
    geometry->allocate(count);
    populateVertices(geometry, count);
    populateColors(geometry, count);
}

void SpiralItem::logStatistics()
{
    if (!m_animating)
        return;

    if (m_vertexCount > m_maxCount)
        m_maxCount = m_vertexCount;

    if (m_vertexCount < m_minCount)
        m_minCount = m_vertexCount;

    if (m_statisticsTimer.elapsed() >= 1000) {
        qDebug() << m_mode
                 << "\tfps:" << m_fps
                 << "\tvertices:" << m_minCount << "-" << m_maxCount;
        m_statisticsTimer.restart();
        m_maxCount = -1;
        m_minCount = m_maxVertices;
    }
}

int SpiralItem::vertexCount() const
{
    return m_vertexCount;
}

void SpiralItem::setVertexCount(int newVertexCount)
{
    if (m_vertexCount == newVertexCount)
        return;
    m_vertexCount = newVertexCount;
    emit vertexCountChanged();
}

int SpiralItem::maxVertices() const
{
    return m_maxVertices;
}

int SpiralItem::fps() const
{
    return m_fps;
}

void SpiralItem::setFps(int newFps)
{
    if (m_fps == newFps)
        return;
    m_fps = newFps;
    emit fpsChanged();
}

void SpiralItem::toggleMode()
{
    if (!m_animating) {
        // Set up to alternate between techniques for resizing geometry
        m_statisticsTimer.start();
        m_animating = true;
    }

    // Toggle between using setVertexCount() and allocate() to resize geometry
    setResizeByVertexCount(!m_resizeByVertexCount);

    m_resetVertices = true;
}

void SpiralItem::handleResizeByVertexCountChanged()
{
    setMode(m_resizeByVertexCount ? "setVertexCount()" : "allocate()");
}

QString SpiralItem::mode() const
{
    return m_mode;
}

void SpiralItem::setMode(const QString &newMode)
{
    if (m_mode == newMode)
        return;
    m_mode = newMode;
    emit modeChanged();
}

bool SpiralItem::resizeByVertexCount() const
{
    return m_resizeByVertexCount;
}

void SpiralItem::setResizeByVertexCount(bool newResizeByVertexCount)
{
    if (m_resizeByVertexCount == newResizeByVertexCount)
        return;
    m_resizeByVertexCount = newResizeByVertexCount;
    emit resizeByVertexCountChanged();
}
