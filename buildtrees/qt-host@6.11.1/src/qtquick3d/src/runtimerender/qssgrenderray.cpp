// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderray_p.h"

#include <QtQuick3DUtils/private/qssgplane_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DUtils/private/qssgmeshbvh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>

#include <optional>

QT_BEGIN_NAMESPACE

// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rayplane_intersection.htm

std::optional<QVector3D> QSSGRenderRay::intersect(const QSSGPlane &inPlane, const QSSGRenderRay &ray)
{
    float Vd = QVector3D::dotProduct(inPlane.n, ray.direction);
    if (std::abs(Vd) < .0001f)
        return std::nullopt;
    float V0 = -1.0f * (QVector3D::dotProduct(inPlane.n, ray.origin) + inPlane.d);
    float t = V0 / Vd;
    return ray.origin + (ray.direction * t);
}

QSSGRenderRay::RayData QSSGRenderRay::createRayData(const QMatrix4x4 &globalTransform,
                                                    const QSSGRenderRay &ray)
{
    using DirectionOp = RayData::DirectionOp;
    QMatrix4x4 originTransform = globalTransform.inverted();
    QVector3D transformedOrigin = QSSGUtils::mat44::transform(originTransform, ray.origin);
    float *outOriginTransformPtr(originTransform.data());
    outOriginTransformPtr[12] = outOriginTransformPtr[13] = outOriginTransformPtr[14] = 0.0f;
    const QVector3D &transformedDirection = QSSGUtils::mat44::rotate(originTransform, ray.direction).normalized();
    static auto getInverseAndDirOp = [](const QVector3D &dir, QVector3D &invDir, DirectionOp (&dirOp)[3]) {
        for (int i = 0; i != 3; ++i) {
            const float axisDir = dir[i];
            dirOp[i] = qFuzzyIsNull(axisDir) ? DirectionOp::Zero : ((axisDir < -std::numeric_limits<float>::epsilon())
                                                                    ? DirectionOp::Swap
                                                                    : DirectionOp::Normal);
            invDir[i] = qFuzzyIsNull(axisDir) ? 0.0f : (1.0f / axisDir);
        }
    };
    DirectionOp dirOp[3];
    QVector3D transformedDirectionInvers;
    getInverseAndDirOp(transformedDirection, transformedDirectionInvers, dirOp);
    return RayData{ globalTransform, ray, transformedOrigin, transformedDirectionInvers,
                    transformedDirection, { dirOp[0], dirOp[1], dirOp[2] } };
}

QSSGRenderRay::IntersectionResult QSSGRenderRay::createIntersectionResult(const QSSGRenderRay::RayData &data,
                                                                          const HitResult &hit)
{
    Q_ASSERT(hit.intersects());
    Q_ASSERT(hit.bounds != nullptr);
    const QSSGBounds3 &bounds = *hit.bounds;
    // Local postion
    const QVector3D &scaledDir = data.direction * hit.min;
    const QVector3D &localPosition = scaledDir + data.origin;
    // ray length squared
    const QVector3D &globalPosition = QSSGUtils::mat44::transform(data.globalTransform, localPosition);
    const QVector3D &cameraToLocal = data.ray.origin - globalPosition;
    const float rayLenSquared = QSSGUtils::vec3::magnitudeSquared(cameraToLocal);
    // UV coordinates
    const auto &boundsMin = bounds.minimum;
    const auto &boundsMax = bounds.maximum;
    const float xRange = boundsMax.x() - boundsMin.x();
    const float yRange = boundsMax.y() - boundsMin.y();
    const QVector2D uvCoords{((localPosition[0] - boundsMin.x()) / xRange), ((localPosition[1] - boundsMin.y()) / yRange)};

    // Since we just intersected with a bounding box, there is no face normal
    return IntersectionResult(rayLenSquared, uvCoords, globalPosition, localPosition, QVector3D(), QVector3D());
}

QSSGRenderRay::HitResult QSSGRenderRay::intersectWithAABBv2(const QSSGRenderRay::RayData &data,
                                                            const QSSGBounds3 &bounds)
{
    // Intersect the origin with the AABB described by bounds.

    // Scan each axis separately.  This code basically finds the distance
    // from the origin to the near and far bbox planes for a given
    // axis.  It then divides this distance by the direction for that axis to
    // get a range of t [near,far] that the ray intersects assuming the ray is
    // described via origin + t*(direction).  Running through all three axis means
    // that you need to min/max those ranges together to find a global min/max
    // that the pick could possibly be in.
    float tmax = std::numeric_limits<float>::max();
    float tmin = std::numeric_limits<float>::min();
    float origin;
    const QVector3D *const barray[] { &bounds.minimum, &bounds.maximum };

    for (int axis = 0; axis != 3; ++axis) {
        origin = data.origin[axis];
        const bool zeroDir = (data.dirOp[axis] == RayData::DirectionOp::Zero);
        if (zeroDir && (origin < bounds.minimum[axis] || origin > bounds.maximum[axis])) {
            // Pickray is roughly parallel to the plane of the slab
            // so, if the origin is not in the range, we have no intersection
            return { -1.0f, -1.0f, nullptr };
        }
        if (!zeroDir) {
            // Shrink the intersections to find the closest hit
            tmax = std::min(((*barray[1-quint8(data.dirOp[axis])])[axis] - origin) * data.directionInvers[axis], tmax);
            tmin = std::max(((*barray[quint8(data.dirOp[axis])])[axis] - origin) * data.directionInvers[axis], tmin);
        }
    }

    return { tmin, tmax, &bounds };
}

// Möller-Trumbore ray-triangle intersection
// https://www.graphics.cornell.edu/pubs/1997/MT97.pdf
// https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/raytri/
bool QSSGRenderRay::triangleIntersect(const QSSGRenderRay &ray,
                                      const QVector3D &v0,
                                      const QVector3D &v1,
                                      const QVector3D &v2,
                                      float &u,
                                      float &v,
                                      QVector3D &normal)
{
    const float epsilon = std::numeric_limits<float>::epsilon();

    // Compute the Triangle's Edges
    const QVector3D edge1 = v1 - v0;
    const QVector3D edge2 = v2 - v0;

    // Compute the vector P as the cross product of the ray direction and edge2
    const QVector3D P = QVector3D::crossProduct(ray.direction, edge2);

    // Compute the determinant
    const float determinant = QVector3D::dotProduct(edge1, P);

    QVector3D Q;

    // If determinant is near zero, the ray lies in the plane of the triangle
    if (determinant > epsilon) {
        // Compute the vector T from the ray origin to the first vertex of the triangle
        const QVector3D T = ray.origin - v0;

        // Calculate coordinate u and test bounds
        u = QVector3D::dotProduct(T, P);
        if (u < 0.0f || u > determinant)
            return false;

        // Compute the vector Q as the cross product of vector T and edge1
        Q = QVector3D::crossProduct(T, edge1);

        // Calculate coordinate v and test bounds
        v = QVector3D::dotProduct(ray.direction, Q);
        if (v < 0.0f || ((u + v) > determinant))
            return false;
    } /*else if (determinant < -epsilon) { // This would be if we cared about backfaces
        // Compute the vector T from the ray origin to the first vertex of the triangle
        const QVector3D T = ray.origin - v0;

        // Calculate coordinate u and test bounds
        u = QVector3D::dotProduct(T, P);
        if (u > 0.0f || u < determinant)
            return false;

        // Compute the vector Q as the cross product of vector T and edge1
        Q = QVector3D::crossProduct(T, edge1);

        // Calculate coordinate v and test bounds
        v = QVector3D::dotProduct(ray.direction, Q);
        if (v > 0.0f || ((u + v) < determinant))
            return false;
    } */else {
        // Ray is parallel to the plane of the triangle
        return false;
    }

    const float invDeterminant = 1.0f / determinant;

    // Calculate the value of t, the parameter of the intersection point along the ray
    const float t = QVector3D::dotProduct(edge2, Q) * invDeterminant;

    if (t > epsilon) {
        normal = QVector3D::crossProduct(edge1, edge2).normalized();
        u *= invDeterminant;
        v *= invDeterminant;
        return true;
    }

    return false;
}


void QSSGRenderRay::intersectWithBVH(const RayData &data,
                                     const QSSGMeshBVHNode *bvh,
                                     const QSSGRenderMesh *mesh,
                                     QVector<IntersectionResult> &intersections,
                                     int depth)
{
    if (!bvh || !mesh || !mesh->bvh)
        return;

    // If this is a leaf node, process it's triangles
    if (bvh->count != 0) {
        // If there is an intersection on a leaf node, then test against geometry
        auto results = intersectWithBVHTriangles(data, mesh->bvh->triangles(), bvh->offset, bvh->count);
        if (!results.isEmpty())
            intersections.append(results);
        return;
    }

    auto hit = QSSGRenderRay::intersectWithAABBv2(data, bvh->left->boundingData);
    if (hit.intersects())
        intersectWithBVH(data, static_cast<const QSSGMeshBVHNode *>(bvh->left), mesh, intersections, depth + 1);

    hit = QSSGRenderRay::intersectWithAABBv2(data, bvh->right->boundingData);
    if (hit.intersects())
        intersectWithBVH(data, static_cast<const QSSGMeshBVHNode *>(bvh->right), mesh, intersections, depth + 1);
}



QVector<QSSGRenderRay::IntersectionResult> QSSGRenderRay::intersectWithBVHTriangles(const RayData &data,
                                                                                    const QSSGMeshBVHTriangles &bvhTriangles,
                                                                                    int triangleOffset,
                                                                                    int triangleCount)
{
    Q_ASSERT(bvhTriangles.size() >= size_t(triangleOffset + triangleCount));

    QVector<QSSGRenderRay::IntersectionResult> results;

    for (int i = triangleOffset; i < triangleCount + triangleOffset; ++i) {
        const auto &triangle = bvhTriangles[i];

        QSSGRenderRay relativeRay(data.origin, data.direction);

        // Use Barycentric Coordinates to get the intersection values
        float u = 0.f;
        float v = 0.f;
        QVector3D normal;
        const bool intersects = triangleIntersect(relativeRay,
                                                  triangle.vertex1,
                                                  triangle.vertex2,
                                                  triangle.vertex3,
                                                  u,
                                                  v,
                                                  normal);
        if (intersects) {
            const float w = 1.0f - u - v;
            const QVector3D localIntersectionPoint = w * triangle.vertex1 +
                                                     u * triangle.vertex2 +
                                                     v * triangle.vertex3;

            const QVector2D uvCoordinate = w * triangle.uvCoord1 +
                                           u * triangle.uvCoord2 +
                                           v * triangle.uvCoord3;
            // Get the intersection point in scene coordinates
            const QVector3D sceneIntersectionPos = QSSGUtils::mat44::transform(data.globalTransform,
                                                                    localIntersectionPoint);
            const QVector3D hitVector = data.ray.origin - sceneIntersectionPos;
            // Get the magnitude of the hit vector
            const float rayLengthSquared = QSSGUtils::vec3::magnitudeSquared(hitVector);

            // Get the normal vector in scene coordinates
            const QMatrix3x3 normalMatrix = data.globalTransform.normalMatrix();
            const QVector3D sceneNormal = QSSGUtils::mat33::transform(normalMatrix, normal);

            results.append(IntersectionResult(rayLengthSquared,
                                              uvCoordinate,
                                              sceneIntersectionPos,
                                              localIntersectionPoint,
                                              normal,
                                              sceneNormal));
        }
    }

    // Does not intersect with any of the triangles
    return results;
}

std::optional<QVector2D> QSSGRenderRay::relative(const QMatrix4x4 &inGlobalTransform,
                                                     const QSSGBounds3 &inBounds,
                                                     QSSGRenderBasisPlanes inPlane) const
{
    QMatrix4x4 theOriginTransform = inGlobalTransform.inverted();

    QVector3D theTransformedOrigin = QSSGUtils::mat44::transform(theOriginTransform, origin);
    float *outOriginTransformPtr(theOriginTransform.data());
    outOriginTransformPtr[12] = outOriginTransformPtr[13] = outOriginTransformPtr[14] = 0.0f;
    QVector3D theTransformedDirection = QSSGUtils::mat44::rotate(theOriginTransform, direction);

    // The XY plane is going to be a plane with either positive or negative Z direction that runs
    // through
    QVector3D theDirection(0, 0, 1);
    QVector3D theRight(1, 0, 0);
    QVector3D theUp(0, 1, 0);
    switch (inPlane) {
    case QSSGRenderBasisPlanes::XY:
        break;
    case QSSGRenderBasisPlanes::XZ:
        theDirection = QVector3D(0, 1, 0);
        theUp = QVector3D(0, 0, 1);
        break;
    case QSSGRenderBasisPlanes::YZ:
        theDirection = QVector3D(1, 0, 0);
        theRight = QVector3D(0, 0, 1);
        break;
    }
    QSSGPlane thePlane(theDirection,
                         QVector3D::dotProduct(theDirection, theTransformedDirection) > 0.0f
                                 ? QVector3D::dotProduct(theDirection, inBounds.maximum)
                                 : QVector3D::dotProduct(theDirection, inBounds.minimum));

    const QSSGRenderRay relativeRay(theTransformedOrigin, theTransformedDirection);
    std::optional<QVector3D> localIsect = QSSGRenderRay::intersect(thePlane, relativeRay);
    if (localIsect.has_value()) {
        float xRange = QVector3D::dotProduct(theRight, inBounds.maximum) - QVector3D::dotProduct(theRight, inBounds.minimum);
        float yRange = QVector3D::dotProduct(theUp, inBounds.maximum) - QVector3D::dotProduct(theUp, inBounds.minimum);
        float xOrigin = xRange / 2.0f + QVector3D::dotProduct(theRight, inBounds.minimum);
        float yOrigin = yRange / 2.0f + QVector3D::dotProduct(theUp, inBounds.minimum);
        return QVector2D((QVector3D::dotProduct(theRight, *localIsect) - xOrigin) / xRange,
                         (QVector3D::dotProduct(theUp, *localIsect) - yOrigin) / yRange);
    }
    return std::nullopt;
}

QT_END_NAMESPACE
