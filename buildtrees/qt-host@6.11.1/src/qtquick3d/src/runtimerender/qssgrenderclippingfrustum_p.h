// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_CLIPPING_PLANE_H
#define QSSG_RENDER_CLIPPING_PLANE_H

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

#include <QtQuick3DUtils/private/qssgplane_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DRuntimeRender/qtquick3druntimerenderexports.h>

QT_BEGIN_NAMESPACE

struct QSSGClipPlane
{
    enum BoxEdgeID : quint8
    {
        None = 0,
        xMax = 1,
        yMax = 1 << 1,
        zMax = 1 << 2,
    };
    using BoxEdgeFlag = std::underlying_type_t<BoxEdgeID>;

    // For an intesection test, we only need two points of the bounding box.
    // There will be a point nearest to the plane, and a point furthest from the plane.
    // We can derive these points from the plane normal equation.
    struct BoxEdge
    {
        BoxEdgeFlag lowerEdge;
        BoxEdgeFlag upperEdge;
    };

    QVector3D normal;
    float d;
    BoxEdge mEdges;

    // For intersection tests, we only need to know if the numerator is greater than, equal to,
    // or less than zero.
    [[nodiscard]] constexpr inline float distance(const QVector3D &pt) const { return QVector3D::dotProduct(normal, pt) + d; }

    // Only works if p0 is above the line and p1 is below the plane.
    inline QVector3D intersectWithLine(const QVector3D &p0, const QVector3D &p1) const
    {
        QVector3D dir = p1 - p0;
        QVector3D pointOnPlane = normal * (-d);
#ifdef _DEBUG
        float distanceOfPoint = distance(pointOnPlane);
        Q_ASSERT(qAbs(distanceOfPoint) < 0.0001f);
#endif
        float numerator = QVector3D::dotProduct(pointOnPlane - p0, normal);
        float denominator = QVector3D::dotProduct(dir, normal);

        Q_ASSERT(qAbs(denominator) > .0001f);
        float t = (numerator / denominator);
        QVector3D retval = p0 + dir * t;
#ifdef _DEBUG
        float retvalDistance = distance(retval);
        Q_ASSERT(qAbs(retvalDistance) < .0001f);
#endif
        return retval;
    }

    static inline constexpr QVector3D corner(const QSSGBounds3 &bounds, BoxEdgeFlag edge)
    {
        return QVector3D((edge & BoxEdgeID::xMax) ? bounds.maximum[0] : bounds.minimum[0],
                         (edge & BoxEdgeID::yMax) ? bounds.maximum[1] : bounds.minimum[1],
                         (edge & BoxEdgeID::zMax) ? bounds.maximum[2] : bounds.minimum[2]);
    }

    // dividing the distance numerator

    // I got this code from osg, but it is in graphics gems
    // as well.
    /** intersection test between plane and bounding sphere.
    return 1 if the bs is completely above plane,
    return 0 if the bs intersects the plane,
    return -1 if the bs is completely below the plane.*/
    inline int intersect(const QSSGBounds3 &bounds) const
    {
        // if lowest point above plane than all above.
        if (distance(corner(bounds, mEdges.lowerEdge)) > 0.0f)
            return 1;

        // if highest point is below plane then all below.
        if (distance(corner(bounds, mEdges.upperEdge)) < 0.0f)
            return -1;

        // d_lower<=0.0f && d_upper>=0.0f
        // therefore must be crossing plane.
        return 0;
    }

    [[nodiscard]] constexpr bool intersectSimple(const QSSGBounds3 &bounds) const
    {
        return (distance(corner(bounds, mEdges.lowerEdge)) > 0.0f) || !(distance(corner(bounds, mEdges.upperEdge)) < 0.0f);
    }

    inline void calculateBBoxEdges()
    {
        mEdges.upperEdge = (normal[0] >= 0.0f ? BoxEdgeID::xMax : BoxEdgeID::None)
                | (normal[1] >= 0.0f ? BoxEdgeID::yMax : BoxEdgeID::None)
                | (normal[2] >= 0.0f ? BoxEdgeID::zMax : BoxEdgeID::None);

        mEdges.lowerEdge = ((~(BoxEdgeFlag(mEdges.upperEdge))) & 7);
    }
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGClippingFrustum
{
    QSSGClipPlane mPlanes[6];

    QSSGClippingFrustum() = default;

    QSSGClippingFrustum(const QMatrix4x4 &modelviewprojection, const QSSGClipPlane &nearPlane);

    bool intersectsWith(const QSSGBounds3 &bounds) const
    {
        bool ret = true;

        for (quint32 idx = 0; idx < 6 && ret; ++idx)
            ret = mPlanes[idx].intersectSimple(bounds);
        return ret;
    }

    bool intersectsWith(const QVector3D &point, float radius = 0.0f) const
    {
        bool ret = true;
        for (quint32 idx = 0; idx < 6 && ret; ++idx)
            ret = !(mPlanes[idx].distance(point) < radius);
        return ret;
    }
};
QT_END_NAMESPACE

#endif
