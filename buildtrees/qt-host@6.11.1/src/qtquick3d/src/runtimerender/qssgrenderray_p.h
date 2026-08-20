// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_RAY_H
#define QSSG_RENDER_RAY_H

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

#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DUtils/private/qssgplane_p.h>

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>

#include <optional>

QT_BEGIN_NAMESPACE
class QSSGMeshBVHNode;
struct QSSGRenderMesh;
struct QSSGMeshBVHTriangle;
enum class QSSGRenderBasisPlanes
{
    XY,
    YZ,
    XZ,
};

struct Q_AUTOTEST_EXPORT QSSGRenderRay
{
    QVector3D origin;
    QVector3D direction;
    QSSGRenderRay() = default;
    QSSGRenderRay(const QVector3D &inOrigin, const QVector3D &inDirection)
        : origin(inOrigin), direction(inDirection)
    {
    }
    // If we are parallel, then no intersection of course.
    static std::optional<QVector3D> intersect(const QSSGPlane &inPlane, const QSSGRenderRay &ray);

    // Perform an intersection aslo returning Barycentric Coordinates
    static bool triangleIntersect(const QSSGRenderRay &ray,
                                  const QVector3D &v0,
                                  const QVector3D &v1,
                                  const QVector3D &v2,
                                  float &u,
                                  float &v,
                                  QVector3D &normal);

    struct IntersectionResult
    {
        bool intersects = false;
        float rayLengthSquared = 0.; // Length of the ray in world coordinates for the hit.
        QVector2D relXY; // UV coords for further mouse picking against a offscreen-rendered object.
        QVector3D scenePosition;
        QVector3D localPosition;
        QVector3D faceNormal;
        QVector3D sceneFaceNormal;
        IntersectionResult() = default;
        inline constexpr IntersectionResult(float rl,
                                            const QVector2D &relxy,
                                            const QVector3D &scenePosition,
                                            const QVector3D &localPosition,
                                            const QVector3D &normal,
                                            const QVector3D &sceneNormal)
            : intersects(true)
            , rayLengthSquared(rl)
            , relXY(relxy)
            , scenePosition(scenePosition)
            , localPosition(localPosition)
            , faceNormal(normal)
            , sceneFaceNormal(sceneNormal)
        {}
    };

    struct HitResult
    {
        float min;
        float max;
        const QSSGBounds3 *bounds;
        inline bool intersects() const { return bounds && max >= std::max(min, 0.0f); }
    };

    struct RayData
    {
        enum class DirectionOp : quint8
        {
            Normal,
            Swap,
            Zero = 0x10
        };

        const QMatrix4x4 &globalTransform;
        const QSSGRenderRay &ray;
        // Cached data calculated from the global transform and the ray
        const QVector3D origin;
        const QVector3D directionInvers;
        const QVector3D direction;
        const DirectionOp dirOp[3];
    };

    static RayData createRayData(const QMatrix4x4 &globalTransform,
                                 const QSSGRenderRay &ray);
    static IntersectionResult createIntersectionResult(const RayData &data,
                                                       const HitResult &hit);

    static HitResult intersectWithAABBv2(const RayData &data,
                                         const QSSGBounds3 &bounds);

    static void intersectWithBVH(const RayData &data,
                                        const QSSGMeshBVHNode *bvh,
                                        const QSSGRenderMesh *mesh,
                                        QVector<IntersectionResult> &intersections,
                                        int depth = 0);

    static QVector<IntersectionResult> intersectWithBVHTriangles(const RayData &data,
                                                                 const std::vector<QSSGMeshBVHTriangle> &bvhTriangles,
                                                                 int triangleOffset,
                                                                 int triangleCount);

    std::optional<QVector2D> relative(const QMatrix4x4 &inGlobalTransform,
                                        const QSSGBounds3 &inBounds,
                                        QSSGRenderBasisPlanes inPlane) const;

    std::optional<QVector2D> relativeXY(const QMatrix4x4 &inGlobalTransform, const QSSGBounds3 &inBounds) const
    {
        return relative(inGlobalTransform, inBounds, QSSGRenderBasisPlanes::XY);
    }
};
QT_END_NAMESPACE
#endif
