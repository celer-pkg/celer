// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QDEBUGDRAWHELPER_H
#define QDEBUGDRAWHELPER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtconfigmacros.h"

namespace physx {
class PxHeightField;
class PxTriangleMesh;
class PxConvexMesh;
}

QT_BEGIN_NAMESPACE
class QVector3D;
class QQuick3DGeometry;

namespace QDebugDrawHelper {
QQuick3DGeometry *generateBoxGeometry(const QVector3D &halfExtents);
QQuick3DGeometry *generateSphereGeometry(const float radius);
QQuick3DGeometry *generateCapsuleGeometry(const float radius, const float halfHeight);
QQuick3DGeometry *generatePlaneGeometry();
QQuick3DGeometry *generateHeightFieldGeometry(physx::PxHeightField *heightField, float heightScale,
                                              float rowScale, float columnScale);
QQuick3DGeometry *generateConvexMeshGeometry(physx::PxConvexMesh *convexMesh);
QQuick3DGeometry *generateTriangleMeshGeometry(physx::PxTriangleMesh *triangleMesh);

};

QT_END_NAMESPACE

#endif // QDEBUGDRAWHELPER_H
