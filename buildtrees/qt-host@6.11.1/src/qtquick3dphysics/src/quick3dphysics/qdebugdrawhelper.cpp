// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdebugdrawhelper_p.h"

#include "qcollisiondebugmeshbuilder_p.h"
#include "qphysicsutils_p.h"

#include <foundation/PxBounds3.h>
#include <foundation/PxVec3.h>
#include <geometry/PxConvexMesh.h>
#include <geometry/PxTriangleMesh.h>
#include <geometry/PxHeightField.h>

#include <QQuick3DGeometry>

QQuick3DGeometry *QDebugDrawHelper::generateBoxGeometry(const QVector3D &halfExtents)
{
    auto boxGeometry = new QQuick3DGeometry();
    boxGeometry->clear();
    boxGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                              QQuick3DGeometry::Attribute::ComponentType::F32Type);
    boxGeometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                              QQuick3DGeometry::Attribute::ComponentType::F32Type);
    boxGeometry->setStride(32);
    boxGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    boxGeometry->setBounds(-halfExtents, halfExtents);

    const float x = halfExtents.x();
    const float y = halfExtents.y();
    const float z = halfExtents.z();

    QCollisionDebugMeshBuilder builder;
    // top
    builder.addLine(QVector3D(-x, -y, z), QVector3D(-x, y, z));
    builder.addLine(QVector3D(-x, y, z), QVector3D(x, y, z));
    builder.addLine(QVector3D(x, y, z), QVector3D(x, -y, z));
    builder.addLine(QVector3D(x, -y, z), QVector3D(-x, -y, z));

    // bottom
    builder.addLine(QVector3D(-x, -y, -z), QVector3D(-x, y, -z));
    builder.addLine(QVector3D(-x, y, -z), QVector3D(x, y, -z));
    builder.addLine(QVector3D(x, y, -z), QVector3D(x, -y, -z));
    builder.addLine(QVector3D(x, -y, -z), QVector3D(-x, -y, -z));

    // front
    builder.addLine(QVector3D(x, -y, z), QVector3D(x, -y, -z));
    builder.addLine(QVector3D(-x, -y, -z), QVector3D(-x, -y, z));

    // back
    builder.addLine(QVector3D(x, y, z), QVector3D(x, y, -z));
    builder.addLine(QVector3D(-x, y, -z), QVector3D(-x, y, z));

    boxGeometry->setVertexData(builder.generateVertexArray());

    return boxGeometry;
}

QQuick3DGeometry *QDebugDrawHelper::generateSphereGeometry(const float radius)
{
    auto sphereGeometry = new QQuick3DGeometry();
    sphereGeometry->clear();
    sphereGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    sphereGeometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    sphereGeometry->setStride(32);
    sphereGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    sphereGeometry->setBounds(QVector3D(-radius, -radius, -radius),
                              QVector3D(radius, radius, radius));

    // One circle around each axis
    // So create a 2D circle first from points
    const int circleSegments = 24;
    constexpr double TAU = 2 * M_PI;
    const float step = float(TAU / circleSegments);
    QVector<QVector2D> circlePoints;
    for (float theta = 0; theta < TAU; theta += step) {
        const float x = radius * qCos(theta);
        const float y = radius * qSin(theta);
        circlePoints.append(QVector2D(x, y));
    }

    QCollisionDebugMeshBuilder builder;
    // X
    for (int i = 0; i < circlePoints.count(); ++i) {
        const auto refPoint1 = circlePoints[i];
        int index2 = i + 1;
        if (index2 == circlePoints.count())
            index2 = 0;
        const auto refPoint2 = circlePoints[index2];
        const auto vertex1 = QVector3D(0.0f, refPoint1.x(), refPoint1.y());
        const auto vertex2 = QVector3D(0.0f, refPoint2.x(), refPoint2.y());
        builder.addLine(vertex1, vertex2, QVector3D(1, 0, 0));
    }

    // Y
    for (int i = 0; i < circlePoints.count(); ++i) {
        const auto refPoint1 = circlePoints[i];
        int index2 = i + 1;
        if (index2 == circlePoints.count())
            index2 = 0;
        const auto refPoint2 = circlePoints[index2];
        const auto vertex1 = QVector3D(refPoint1.x(), 0.0f, refPoint1.y());
        const auto vertex2 = QVector3D(refPoint2.x(), 0.0f, refPoint2.y());
        builder.addLine(vertex1, vertex2, QVector3D(0, 1, 0));
    }

    // Z
    for (int i = 0; i < circlePoints.count(); ++i) {
        const auto refPoint1 = circlePoints[i];
        int index2 = i + 1;
        if (index2 == circlePoints.count())
            index2 = 0;
        const auto refPoint2 = circlePoints[index2];
        const auto vertex1 = QVector3D(refPoint1.x(), refPoint1.y(), 0.0f);
        const auto vertex2 = QVector3D(refPoint2.x(), refPoint2.y(), 0.0f);
        builder.addLine(vertex1, vertex2, QVector3D(0, 0, 1));
    }
    sphereGeometry->setVertexData(builder.generateVertexArray());

    return sphereGeometry;
}

QQuick3DGeometry *QDebugDrawHelper::generateCapsuleGeometry(const float radius,
                                                            const float halfHeight)
{
    auto capsuleGeometry = new QQuick3DGeometry();
    capsuleGeometry->clear();
    capsuleGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                                  QQuick3DGeometry::Attribute::ComponentType::F32Type);
    capsuleGeometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                                  QQuick3DGeometry::Attribute::ComponentType::F32Type);
    capsuleGeometry->setStride(32);
    capsuleGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    capsuleGeometry->setBounds(QVector3D(-(halfHeight + radius), -radius, -radius),
                               QVector3D(halfHeight + radius, radius, radius));

    // The total height is height+2*radius, so the height is just the height
    // between the center of each 'sphere' of the capsule caps.

    // Create a 2D circle first for points
    const int circleSegments = 32;
    constexpr double TAU = 2 * M_PI;

    Q_ASSERT(circleSegments % 4 == 0);
    const float step = float(TAU / circleSegments);
    QVector<QVector2D> circlePoints;
    for (float theta = 0; theta < TAU; theta += step) {
        const float x = radius * qCos(theta);
        const float y = radius * qSin(theta);
        circlePoints.append(QVector2D(x, y));
    }

    QCollisionDebugMeshBuilder builder;

    // Top Y Cirlce (y = height * 0.5)
    for (int i = 0; i < circlePoints.count(); ++i) {
        const auto refPoint1 = circlePoints[i];
        int index2 = i + 1;
        if (index2 == circlePoints.count())
            index2 = 0;
        const auto refPoint2 = circlePoints[index2];
        const auto vertex1 = QVector3D(halfHeight, refPoint1.x(), refPoint1.y());
        const auto vertex2 = QVector3D(halfHeight, refPoint2.x(), refPoint2.y());
        const auto normal = QVector3D(1, 0, 0);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Bottom Y Circle (y = -height * 0.5)
    for (int i = 0; i < circlePoints.count(); ++i) {
        const auto refPoint1 = circlePoints[i];
        int index2 = i + 1;
        if (index2 == circlePoints.count())
            index2 = 0;
        const auto refPoint2 = circlePoints[index2];
        const auto vertex1 = QVector3D(-halfHeight, refPoint1.x(), refPoint1.y());
        const auto vertex2 = QVector3D(-halfHeight, refPoint2.x(), refPoint2.y());
        const auto normal = QVector3D(1, 0, 0);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Front Cylinder Line (z = radius, y = length , x = 0)
    {
        const auto vertex1 = QVector3D(halfHeight, 0, radius);
        const auto vertex2 = QVector3D(-halfHeight, 0, radius);
        const auto normal = QVector3D(0, 0, 1);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Back Cylinder Line (z = -radius, y = length, x = 0)
    {
        const auto vertex1 = QVector3D(halfHeight, 0, -radius);
        const auto vertex2 = QVector3D(-halfHeight, 0, -radius);
        const auto normal = QVector3D(0, 0, -1);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Left Cylinder Line (x = -radius, y = length, z = 0)
    {
        const auto vertex1 = QVector3D(halfHeight, -radius, 0);
        const auto vertex2 = QVector3D(-halfHeight, -radius, 0);
        const auto normal = QVector3D(0, -1, 0);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Right Cylinder Line (x = radius, y = length, z = 0)
    {
        const auto vertex1 = QVector3D(halfHeight, radius, 0);
        const auto vertex2 = QVector3D(-halfHeight, radius, 0);
        const auto normal = QVector3D(0, 1, 0);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Get half circle values
    QVector<int> topIndexes;
    QVector<int> bottomIndexes;
    {
        const int half = circlePoints.count() / 2;
        for (int i = 0; i < half + 1; ++i)
            topIndexes.append(i);

        for (int i = half; i <= circlePoints.count(); ++i) {
            int index = i;
            if (i >= circlePoints.count())
                index = index - circlePoints.count();
            bottomIndexes.append(index);
        }
    }

    // Z Top Half Circle
    for (int i = 0; i < topIndexes.count(); ++i) {
        const auto refPoint1 = circlePoints[topIndexes[i]];
        int index2 = i + 1;
        if (index2 == topIndexes.count())
            break;
        const auto refPoint2 = circlePoints[topIndexes[index2]];
        const auto vertex1 = QVector3D(refPoint1.y() + halfHeight, refPoint1.x(), 0.0f);
        const auto vertex2 = QVector3D(refPoint2.y() + halfHeight, refPoint2.x(), 0.0f);
        const auto normal = QVector3D(0, 0, 1);
        builder.addLine(vertex1, vertex2, normal);
    }

    // Z Bottom Half Circle
    for (int i = 0; i < bottomIndexes.count(); ++i) {
        const auto refPoint1 = circlePoints[bottomIndexes[i]];
        int index2 = i + 1;
        if (index2 == bottomIndexes.count())
            break;
        const auto refPoint2 = circlePoints[bottomIndexes[index2]];
        const auto vertex1 = QVector3D(refPoint1.y() - halfHeight, refPoint1.x(), 0.0f);
        const auto vertex2 = QVector3D(refPoint2.y() - halfHeight, refPoint2.x(), 0.0f);
        const auto normal = QVector3D(0, 0, 1);
        builder.addLine(vertex1, vertex2, normal);
    }

    // X Top Half Circle
    for (int i = 0; i < topIndexes.count(); ++i) {
        const auto refPoint1 = circlePoints[topIndexes[i]];
        int index2 = i + 1;
        if (index2 == topIndexes.count())
            break;
        const auto refPoint2 = circlePoints[topIndexes[index2]];
        const auto vertex1 = QVector3D(refPoint1.y() + halfHeight, 0.0f, refPoint1.x());
        const auto vertex2 = QVector3D(refPoint2.y() + halfHeight, 0.0f, refPoint2.x());
        const auto normal = QVector3D(0, 1, 0);
        builder.addLine(vertex1, vertex2, normal);
    }

    // X Bottom Half Circle
    for (int i = 0; i < bottomIndexes.count(); ++i) {
        const auto refPoint1 = circlePoints[bottomIndexes[i]];
        int index2 = i + 1;
        if (index2 == bottomIndexes.count())
            break;
        const auto refPoint2 = circlePoints[bottomIndexes[index2]];
        const auto vertex1 = QVector3D(refPoint1.y() - halfHeight, 0.0f, refPoint1.x());
        const auto vertex2 = QVector3D(refPoint2.y() - halfHeight, 0.0f, refPoint2.x());
        const auto normal = QVector3D(0, 1, 0);
        builder.addLine(vertex1, vertex2, normal);
    }

    capsuleGeometry->setVertexData(builder.generateVertexArray());
    return capsuleGeometry;
}

QQuick3DGeometry *QDebugDrawHelper::generatePlaneGeometry()
{
    auto planeGeometry = new QQuick3DGeometry();
    planeGeometry->clear();
    planeGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                                QQuick3DGeometry::Attribute::ComponentType::F32Type);
    planeGeometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                                QQuick3DGeometry::Attribute::ComponentType::F32Type);
    planeGeometry->setStride(32);
    planeGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);

    // TODO: Some sort of debug scale? Or a level-of-detail grid thing? => QtQuick3DHelpers
    float s = 50;
    float h = 5; // Set to avoid flat bounding box
    planeGeometry->setBounds({ -s, -s, -h }, { s, s, h });
    QCollisionDebugMeshBuilder builder;

    builder.addLine({ -s, -s, 0 }, { s, -s, 0 });
    builder.addLine({ -s, -s, 0 }, { 0, 0, 0 });

    builder.addLine({ s, -s, 0 }, { s, s, 0 });
    builder.addLine({ s, -s, 0 }, { 0, 0, 0 });

    builder.addLine({ s, s, 0 }, { -s, s, 0 });
    builder.addLine({ s, s, 0 }, { 0, 0, 0 });

    builder.addLine({ -s, s, 0 }, { -s, -s, 0 });
    builder.addLine({ -s, s, 0 }, { 0, 0, 0 });

    planeGeometry->setVertexData(builder.generateVertexArray());
    return planeGeometry;
}

QQuick3DGeometry *QDebugDrawHelper::generateHeightFieldGeometry(physx::PxHeightField *heightField,
                                                                float heightScale, float rowScale,
                                                                float columnScale)
{
    if (!heightField || heightField->getNbRows() < 2 || heightField->getNbColumns() < 2)
        return nullptr;

    auto geometry = new QQuick3DGeometry();
    geometry->clear();
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                           QQuick3DGeometry::Attribute::ComponentType::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                           QQuick3DGeometry::Attribute::ComponentType::F32Type);
    geometry->setStride(32);
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);

    QCollisionDebugMeshBuilder builder;

    const int numRows = heightField->getNbRows();
    const int numCols = heightField->getNbColumns();

    const float sizeX = rowScale * (numRows - 1);
    const float sizeZ = columnScale * (numCols - 1);

    const float heightF = heightScale;

    float minHeight = 0.f;
    float maxHeight = 0.f;

    auto sample = [&](int row, int col) -> QVector3D {
        const float height = heightField->getSample(row, col).height * heightF;
        maxHeight = qMax(maxHeight, height);
        minHeight = qMin(minHeight, height);
        return QVector3D(row * rowScale, height, col * columnScale);
    };

    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            if (row < numRows - 1)
                builder.addLine(sample(row, col), sample(row + 1, col));
            if (col < numCols - 1)
                builder.addLine(sample(row, col), sample(row, col + 1));
        }
    }

    geometry->setBounds(QVector3D(0, minHeight, 0), QVector3D(sizeX, maxHeight, sizeZ));
    geometry->setVertexData(builder.generateVertexArray());
    return geometry;
}

QQuick3DGeometry *QDebugDrawHelper::generateConvexMeshGeometry(physx::PxConvexMesh *convexMesh)
{
    if (!convexMesh)
        return nullptr;

    auto geometry = new QQuick3DGeometry();
    geometry->clear();
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                           QQuick3DGeometry::Attribute::ComponentType::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                           QQuick3DGeometry::Attribute::ComponentType::F32Type);
    geometry->setStride(32);
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);

    QCollisionDebugMeshBuilder builder;

    const physx::PxU32 nbPolys = convexMesh->getNbPolygons();
    const physx::PxU8 *polygons = convexMesh->getIndexBuffer();
    const physx::PxVec3 *verts = convexMesh->getVertices();
    const physx::PxU32 nbVerts = convexMesh->getNbVertices();

    physx::PxHullPolygon data;
    for (physx::PxU32 i = 0; i < nbPolys; i++) {
        convexMesh->getPolygonData(i, data);

        Q_ASSERT(data.mNbVerts > 2);
        const physx::PxU32 nbTris = physx::PxU32(data.mNbVerts - 2);
        const physx::PxU8 vref0 = polygons[data.mIndexBase + 0];
        Q_ASSERT(vref0 < nbVerts);

        for (physx::PxU32 j = 0; j < nbTris; j++) {
            const physx::PxU32 vref1 = polygons[data.mIndexBase + 0 + j + 1];
            const physx::PxU32 vref2 = polygons[data.mIndexBase + 0 + j + 2];
            Q_ASSERT(vref1 < nbVerts);
            Q_ASSERT(vref2 < nbVerts);

            const QVector3D p0 = QPhysicsUtils::toQtType(verts[vref0]);
            const QVector3D p1 = QPhysicsUtils::toQtType(verts[vref1]);
            const QVector3D p2 = QPhysicsUtils::toQtType(verts[vref2]);

            builder.addLine(p0, p1);
            builder.addLine(p1, p2);
            builder.addLine(p2, p0);
        }
    }

    auto bounds = convexMesh->getLocalBounds();

    geometry->setBounds(QPhysicsUtils::toQtType(bounds.minimum),
                        QPhysicsUtils::toQtType(bounds.maximum));
    geometry->setVertexData(builder.generateVertexArray());
    return geometry;
}

QQuick3DGeometry *
QDebugDrawHelper::generateTriangleMeshGeometry(physx::PxTriangleMesh *triangleMesh)
{
    if (!triangleMesh)
        return nullptr;

    auto geometry = new QQuick3DGeometry();
    geometry->clear();
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                           QQuick3DGeometry::Attribute::ComponentType::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                           QQuick3DGeometry::Attribute::ComponentType::F32Type);
    geometry->setStride(32);
    geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);

    QCollisionDebugMeshBuilder builder;

    const physx::PxU32 triangleCount = triangleMesh->getNbTriangles();
    const physx::PxU32 has16BitIndices =
            triangleMesh->getTriangleMeshFlags() & physx::PxTriangleMeshFlag::e16_BIT_INDICES;
    const void *indexBuffer = triangleMesh->getTriangles();
    const physx::PxVec3 *vertexBuffer = triangleMesh->getVertices();
    const physx::PxU32 *intIndices = reinterpret_cast<const physx::PxU32 *>(indexBuffer);
    const physx::PxU16 *shortIndices = reinterpret_cast<const physx::PxU16 *>(indexBuffer);
    for (physx::PxU32 i = 0; i < triangleCount; ++i) {
        physx::PxVec3 triVert[3];

        if (has16BitIndices) {
            triVert[0] = vertexBuffer[*shortIndices++];
            triVert[1] = vertexBuffer[*shortIndices++];
            triVert[2] = vertexBuffer[*shortIndices++];
        } else {
            triVert[0] = vertexBuffer[*intIndices++];
            triVert[1] = vertexBuffer[*intIndices++];
            triVert[2] = vertexBuffer[*intIndices++];
        }

        const QVector3D p0 = QPhysicsUtils::toQtType(triVert[0]);
        const QVector3D p1 = QPhysicsUtils::toQtType(triVert[1]);
        const QVector3D p2 = QPhysicsUtils::toQtType(triVert[2]);

        builder.addLine(p0, p1);
        builder.addLine(p1, p2);
        builder.addLine(p2, p0);
    }

    auto bounds = triangleMesh->getLocalBounds();

    geometry->setBounds(QPhysicsUtils::toQtType(bounds.minimum),
                        QPhysicsUtils::toQtType(bounds.maximum));
    geometry->setVertexData(builder.generateVertexArray());
    return geometry;
}
