// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderclippingfrustum_p.h"

#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

QSSGClippingFrustum::QSSGClippingFrustum(const QMatrix4x4 &modelviewprojection, const QSSGClipPlane &nearPlane)
{
    const float *modelviewProjection = modelviewprojection.data();

    // update planes (http://read.pudn.com/downloads128/doc/542641/Frustum.pdf)
    // Google for Gribb plane extraction if that link doesn't work.
    // http://www.google.com/search?q=ravensoft+plane+extraction
#define M(_x, _y) modelviewProjection[(4 * (_y)) + (_x)]
    // left plane
    mPlanes[0].normal = { M(3, 0) + M(0, 0), M(3, 1) + M(0, 1), M(3, 2) + M(0, 2) };
    mPlanes[0].d = (M(3, 3) + M(0, 3)) / QSSGUtils::vec3::normalize(mPlanes[0].normal);

    // right plane
    mPlanes[1].normal = { M(3, 0) - M(0, 0), M(3, 1) - M(0, 1), M(3, 2) - M(0, 2) };
    mPlanes[1].d = (M(3, 3) - M(0, 3)) / QSSGUtils::vec3::normalize(mPlanes[1].normal);

    // far plane
    mPlanes[2].normal = { (M(3, 0) - M(2, 0)), M(3, 1) - M(2, 1), M(3, 2) - M(2, 2) };
    mPlanes[2].d = (M(3, 3) - M(2, 3)) / QSSGUtils::vec3::normalize(mPlanes[2].normal);

    // bottom plane
    mPlanes[3].normal = { M(3, 0) + M(1, 0), M(3, 1) + M(1, 1), M(3, 2) + M(1, 2) };
    mPlanes[3].d = (M(3, 3) + M(1, 3)) / QSSGUtils::vec3::normalize(mPlanes[3].normal);

    // top plane
    mPlanes[4].normal = { M(3, 0) - M(1, 0), M(3, 1) - M(1, 1), M(3, 2) - M(1, 2) };
    mPlanes[4].d = (M(3, 3) - M(1, 3)) / QSSGUtils::vec3::normalize(mPlanes[4].normal);
#undef M
    mPlanes[5] = nearPlane;
    // http://www.openscenegraph.org/projects/osg/browser/OpenSceneGraph/trunk/include/osg/Plane?rev=5328
    // setup the edges of the plane that we will clip against an axis-aligned bounding box.
    for (quint32 idx = 0; idx < 6; ++idx)
        mPlanes[idx].calculateBBoxEdges();
}

QT_END_NAMESPACE
