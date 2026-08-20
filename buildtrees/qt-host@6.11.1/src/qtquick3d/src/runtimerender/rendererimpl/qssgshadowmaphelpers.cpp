// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgshadowmaphelpers_p.h"

#include "qssgdebugdrawsystem_p.h"
#include "qssgrenderray_p.h"

#include <QVector3D>
#include <QMatrix4x4>
#include <QVector4D>
#include <cmath>

QT_BEGIN_NAMESPACE

static QMatrix4x4 buildOrthonormalBasis(const QVector3D &normal)
{
    // Assume normal is already normalized
    const QVector3D forward = normal;

    // Pick the axis least aligned with forward to use as a reference
    QVector3D ref;
    float absX = std::fabs(forward.x());
    float absY = std::fabs(forward.y());
    float absZ = std::fabs(forward.z());

    if (absX <= absY && absX <= absZ)
        ref = QVector3D(1, 0, 0);
    else if (absY <= absZ)
        ref = QVector3D(0, 1, 0);
    else
        ref = QVector3D(0, 0, 1);

    const QVector3D right = QVector3D::crossProduct(ref, forward).normalized();
    const QVector3D up = QVector3D::crossProduct(forward, right);

    QMatrix4x4 transform(Qt::Uninitialized);
    transform.setRow(0, QVector4D(right, 0.0f));
    transform.setRow(1, QVector4D(up, 0.0f));
    transform.setRow(2, QVector4D(forward, 0.0f));
    transform.setRow(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
    return transform;
}

// These indices need to match the order in QSSGBounds3::toQSSGBoxPointsNoEmptyCheck()
static constexpr std::array<std::array<int, 2>, 12> BOX_LINE_INDICES = {
    std::array<int, 2> { 0, 1 }, std::array<int, 2> { 1, 2 }, std::array<int, 2> { 2, 3 }, std::array<int, 2> { 3, 0 },
    std::array<int, 2> { 4, 5 }, std::array<int, 2> { 5, 6 }, std::array<int, 2> { 6, 7 }, std::array<int, 2> { 7, 4 },
    std::array<int, 2> { 0, 4 }, std::array<int, 2> { 1, 5 }, std::array<int, 2> { 2, 6 }, std::array<int, 2> { 3, 7 },
};

void ShadowmapHelpers::addDebugBox(const QSSGBoxPoints &box, const QColor &color, QSSGDebugDrawSystem *debugDrawSystem)
{
    if (!debugDrawSystem)
        return;

    for (const auto line : BOX_LINE_INDICES)
        debugDrawSystem->drawLine(box[line[0]], box[line[1]], color);

    debugDrawSystem->setEnabled(true);
}

void ShadowmapHelpers::addDebugFrustum(const QSSGBoxPoints &frustumPoints, const QColor &color, QSSGDebugDrawSystem *debugDrawSystem)
{
    ShadowmapHelpers::addDebugBox(frustumPoints, color, debugDrawSystem);
}

void ShadowmapHelpers::addDirectionalLightDebugBox(const QSSGBoxPoints &box, QSSGDebugDrawSystem *debugDrawSystem)
{
    if (!debugDrawSystem)
        return;

    constexpr QColor colorBox = QColorConstants::Yellow;

    //    .7------6
    //  .' |    .'|
    // 3---+--2'  |
    // |   |  |   |
    // |  .4--+---5
    // |.'    | .'
    // 0------1'

    // Find shortest axis line
    float shortestAxisSq = (box[0] - box[1]).lengthSquared();
    shortestAxisSq = std::min(shortestAxisSq, (box[0] - box[4]).lengthSquared());
    shortestAxisSq = std::min(shortestAxisSq, (box[0] - box[3]).lengthSquared());
    const float axisLength = qSqrt(shortestAxisSq) * 0.5f;

    auto drawAxisLine = [&](const QVector3D &from, const QVector3D &to, const QColor &axisColor) {
        const QVector3D dir = (to - from).normalized();
        const QVector3D mid = from + dir * axisLength;
        debugDrawSystem->drawLine(from, mid, axisColor);
        debugDrawSystem->drawLine(mid, to, colorBox);
    };

    // Draw x,y,z axis in r,g,b color and rest in box color

    // Closest to light (front)
    drawAxisLine(box[0], box[1], QColorConstants::Red);
    debugDrawSystem->drawLine(box[1], box[2], colorBox);
    drawAxisLine(box[0], box[3], QColorConstants::Green);
    debugDrawSystem->drawLine(box[2], box[3], colorBox);

    // Lines parallel to light direction
    drawAxisLine(box[0], box[4], QColorConstants::Blue);
    debugDrawSystem->drawLine(box[1], box[5], colorBox);
    debugDrawSystem->drawLine(box[2], box[6], colorBox);
    debugDrawSystem->drawLine(box[3], box[7], colorBox);

    // Furthest from light (back)
    debugDrawSystem->drawLine(box[4], box[5], colorBox);
    debugDrawSystem->drawLine(box[5], box[6], colorBox);
    debugDrawSystem->drawLine(box[6], box[7], colorBox);
    debugDrawSystem->drawLine(box[7], box[4], colorBox);

    debugDrawSystem->setEnabled(true);
}

static bool lineLineIntersection(QVector2D a, QVector2D b, QVector2D c, QVector2D d)
{
    // Line AB represented as a1x + b1y = c1
    double a1 = b.y() - a.y();
    double b1 = a.x() - b.x();
    double c1 = a1 * (a.x()) + b1 * (a.y());

    // Line CD represented as a2x + b2y = c2
    double a2 = d.y() - c.y();
    double b2 = c.x() - d.x();
    double c2 = a2 * (c.x()) + b2 * (c.y());

    double determinant = a1 * b2 - a2 * b1;

    if (qFuzzyCompare(determinant, 0)) {
        // The lines are parallel.
        return false;
    }

    double x = (b2 * c1 - b1 * c2) / determinant;
    double y = (a1 * c2 - a2 * c1) / determinant;
    const QVector2D min = QVector2D(qMin(a.x(), b.x()), qMin(a.y(), b.y()));
    const QVector2D max = QVector2D(qMax(a.x(), b.x()), qMax(a.y(), b.y()));
    return x > min.x() && x < max.x() && y > min.y() && y < max.y();
}

struct Vertex
{
    QVector3D position;
    std::array<int, 3> neighbors = { -1, -1, -1 };
    bool active = true;
    bool borked = false;

    void addNeighbor(int neighbor)
    {
        if (neighbor == neighbors[0] || neighbor == neighbors[1] || neighbor == neighbors[2]) {
            borked = true;
            return;
        }

        if (neighbors[0] == -1) {
            neighbors[0] = neighbor;
        } else if (neighbors[1] == -1) {
            neighbors[1] = neighbor;
        } else if (neighbors[2] == -1) {
            neighbors[2] = neighbor;
        } else {
            borked = true;
        }
    }

    void removeNeighbor(int neighbor)
    {
        if (neighbors[0] == neighbor) {
            neighbors[0] = -1;
        } else if (neighbors[1] == neighbor) {
            neighbors[1] = -1;
        } else if (neighbors[2] == neighbor) {
            neighbors[2] = -1;
        } else {
            borked = true;
        }
    }

    bool allNeighbors() const { return neighbors[0] != -1 && neighbors[1] != -1 && neighbors[2] != -1; }
};

static QList<QVector3D> sliceBoxByPlanes(const QList<std::array<QVector3D, 2>> &planes,
                                         const QSSGBoxPoints &castingBox,
                                         QSSGDebugDrawSystem *debugDrawSystem,
                                         const QColor &color)
{

    QList<Vertex> vertices;
    vertices.reserve(castingBox.size());
    for (const auto &p : castingBox) {
        Vertex point;
        point.position = p;
        vertices.push_back(point);
    }

    for (auto line : BOX_LINE_INDICES) {
        vertices[line[0]].addNeighbor(line[1]);
        vertices[line[1]].addNeighbor(line[0]);
    }

    QList<QVector2D> planePoints;
    QList<QPair<quint8, quint8>> lines;
    QList<bool> intersecting;
    QList<quint8> newVertexIndices;
    QList<Vertex> verticesPrev;

    for (const auto &plane : planes) {
        const QVector3D center = plane[0];
        const QVector3D normal = plane[1];
        newVertexIndices.clear();
        verticesPrev = vertices;
        for (int vertIndexI = 0, vertEnd = vertices.length(); vertIndexI < vertEnd; ++vertIndexI) {
            Vertex vertex = vertices[vertIndexI];
            if (!vertex.active)
                continue;
            // Check if 'p' is lying above or below plane
            // above = inside frustum, below = outside frustum
            if (QVector3D::dotProduct(vertex.position - center, normal) >= 0) {
                continue;
                // 'p' lies outside frustum so project 'p' onto the plane so it lies on the edge of the frustum
            }

            // Disable and remove vertex for neighbors
            vertices[vertIndexI].active = false; // disable
            for (int neighborIndex : vertex.neighbors) {
                if (neighborIndex == -1)
                    continue;
                vertices[neighborIndex].removeNeighbor(vertIndexI);
            }

            for (int neighborIndex : vertex.neighbors) {
                if (neighborIndex == -1)
                    continue;

                const quint32 idx0 = vertIndexI;
                const quint32 idx1 = neighborIndex;

                // non-intersecting line, skip
                if (QVector3D::dotProduct(vertices[idx1].position - center, normal) < 0)
                    continue;

                QVector3D planePoint = center;
                QVector3D planeNormal = normal;
                QVector3D linePoint = vertices[idx0].position;
                QVector3D lineDirection = vertices[idx0].position - vertices[idx1].position;
                QSSGPlane plane(planePoint, planeNormal);
                QSSGRenderRay ray(linePoint, lineDirection);

                if (auto intersect = QSSGRenderRay::intersect(plane, ray); intersect.has_value()) {
                    int addedIdx = vertices.length();
                    Q_ASSERT(addedIdx <= 255);
                    Vertex p;
                    p.position = intersect.value();
                    p.addNeighbor(idx1);
                    vertices[idx1].addNeighbor(addedIdx);
                    vertices.push_back(p);
                    newVertexIndices.push_back(addedIdx);
                }
            }
        }

        if (newVertexIndices.isEmpty())
            continue;

        // Create rotation matrix from plane
        const QMatrix4x4 transform = buildOrthonormalBasis(normal);
        planePoints.clear();
        planePoints.reserve(newVertexIndices.length());
        for (auto &p0 : newVertexIndices) {
            QVector3D p = transform.map(vertices[p0].position);
            planePoints.push_back(QVector2D(p.x(), p.y()));
        }

        // Create all possible lines from point
        // num lines = (num_points/2)^2
        lines.clear();
        lines.reserve((planePoints.length() * planePoints.length()) / 4);
        for (int i = 0, length = planePoints.length(); i < length; ++i) {
            for (int j = i + 1; j < length; ++j) {
                lines.push_back({ i, j });
            }
        }

        // O((num_lines/2)^2)
        intersecting.clear();
        intersecting.resize(lines.length(), false);
        for (int i = 0, length = lines.length(); i < length; ++i) {
            QPair<quint8, quint8> lineI = lines[i];
            QVector2D a = planePoints[lineI.first];
            QVector2D b = planePoints[lineI.second];
            for (int j = i + 1; j < length; ++j) {
                QPair<quint8, quint8> lineJ = lines[j];

                // Skip connected lines
                if (lineJ.first == lineI.first || lineJ.first == lineI.second || lineJ.second == lineI.first
                    || lineJ.second == lineI.second)
                    continue;

                QVector2D c = planePoints[lineJ.first];
                QVector2D d = planePoints[lineJ.second];
                if (lineLineIntersection(a, b, c, d)) {
                    intersecting[i] = true;
                    intersecting[j] = true;
                }
            }
        }

        for (int i = 0, length = lines.length(); i < length; ++i) {
            if (intersecting[i]) {
                continue;
            }

            QPair<quint8, quint8> lineI = lines[i];
            int a = newVertexIndices[lineI.first];
            int b = newVertexIndices[lineI.second];
            vertices[a].addNeighbor(b);
            vertices[b].addNeighbor(a);
        }

        // Sanity check and revert if any point is borked
        for (const Vertex &point : std::as_const(vertices)) {
            if (point.active && (point.borked || !point.allNeighbors())) {
                vertices = verticesPrev;
                break;
            }
        }
    }

    QList<QVector3D> result;
    result.reserve(vertices.length());
    for (const Vertex &vertex : std::as_const(vertices)) {
        if (vertex.active)
            result.push_back(vertex.position);
    }

    if (debugDrawSystem) {
        debugDrawSystem->setEnabled(true);
        for (int i = 0; i < vertices.length(); i++) {
            Vertex point = vertices[i];
            if (!point.active)
                continue;

            QVector3D position = vertices[i].position;
            debugDrawSystem->drawLine(position, vertices[point.neighbors[0]].position, color);
            debugDrawSystem->drawLine(position, vertices[point.neighbors[1]].position, color);
            debugDrawSystem->drawLine(position, vertices[point.neighbors[2]].position, color);
        }
    }

    return result;
}

QList<QVector3D> ShadowmapHelpers::intersectBoxByFrustum(const std::array<QVector3D, 8> &frustumPoints,
                                                         const QSSGBoxPoints &box,
                                                         QSSGDebugDrawSystem *debugDrawSystem,
                                                         const QColor &color)
{
    static std::array<std::array<int, 4>, 6> faceIndices = {
        std::array<int, 4> { 0, 1, 2, 3 }, std::array<int, 4> { 0, 3, 7, 4 }, std::array<int, 4> { 3, 2, 6, 7 },
        std::array<int, 4> { 7, 6, 5, 4 }, std::array<int, 4> { 4, 5, 1, 0 }, std::array<int, 4> { 2, 1, 5, 6 },
    };

    QList<std::array<QVector3D, 2>> faces;
    faces.resize(faceIndices.size());
    for (int i = 0; i < int(faceIndices.size()); ++i) {
        std::array<int, 4> face = faceIndices[i];
        QVector3D center = frustumPoints[face[0]]; // Since the plane is infinite we can take any point
        QVector3D b = frustumPoints[face[1]] - frustumPoints[face[0]];
        QVector3D a = frustumPoints[face[2]] - frustumPoints[face[0]];
        QVector3D n = QVector3D::crossProduct(a, b).normalized();
        faces[i] = { center, n };
    }

    return sliceBoxByPlanes(faces, box, debugDrawSystem, color);
}

QList<QVector3D> ShadowmapHelpers::intersectBoxByBox(const QSSGBounds3 &boxBounds, const QSSGBoxPoints &box)
{
    const float minX = boxBounds.minimum.x();
    const float minY = boxBounds.minimum.y();
    const float minZ = boxBounds.minimum.z();
    const float maxX = boxBounds.maximum.x();
    const float maxY = boxBounds.maximum.y();
    const float maxZ = boxBounds.maximum.z();

    QSSGBoxPoints points;
    points[0] = QVector3D(minX, minY, minZ);
    points[1] = QVector3D(maxX, minY, minZ);
    points[2] = QVector3D(maxX, maxY, minZ);
    points[3] = QVector3D(minX, maxY, minZ);
    points[4] = QVector3D(minX, minY, maxZ);
    points[5] = QVector3D(maxX, minY, maxZ);
    points[6] = QVector3D(maxX, maxY, maxZ);
    points[7] = QVector3D(minX, maxY, maxZ);

    // constexpr std::array<int, 4> NEAR = { 0, 1, 2, 3 };
    // constexpr std::array<int, 4> FAR = { 7, 6, 5, 4 };
    static constexpr std::array<int, 4> LEFT = { 0, 3, 7, 4 };
    static constexpr std::array<int, 4> TOP = { 3, 2, 6, 7 };
    static constexpr std::array<int, 4> BOTTOM = { 4, 5, 1, 0 };
    static constexpr std::array<int, 4> RIGHT = { 2, 1, 5, 6 };
    static constexpr std::array<std::array<int, 4>, 4> faceIndices = { LEFT, TOP, BOTTOM, RIGHT };

    QList<std::array<QVector3D, 2>> faces;
    faces.resize(faceIndices.size());
    for (int i = 0; i < int(faceIndices.size()); ++i) {
        std::array<int, 4> face = faceIndices[i];
        QVector3D center = points[face[0]]; // Since the plane is infinite we can take any point
        QVector3D b = points[face[1]] - points[face[0]];
        QVector3D a = points[face[2]] - points[face[0]];
        QVector3D n = -QVector3D::crossProduct(a, b).normalized();
        faces[i] = { center, n };
    }

    return sliceBoxByPlanes(faces, box, nullptr, QColorConstants::Black);
}

QT_END_NAMESPACE
