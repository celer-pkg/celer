// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qconvexmeshshape_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype ConvexMeshShape
    \inherits CollisionShape
    \inqmlmodule QtQuick3D.Physics
    \since 6.4
    \brief A convex collision shape based on a 3D mesh.

    This type defines a convex shape based on the same 3D mesh file format used by
    \l [QtQuick3D]{Model::source}{QtQuick3D.Model}. If the mesh is not convex, the convex hull of the
    mesh will be used.

    \sa {Qt Quick 3D Physics Shapes and Bodies}{Shapes and Bodies overview documentation}, TriangleMeshShape
*/

/*!
    \qmlproperty url ConvexMeshShape::source
    This property defines the location of the mesh file used to define the shape. If the
    mesh is not convex, the convex hull of the mesh will be used. The maximum number of faces
    and vertices is 255: If the mesh is more detailed than that, it will be simplified.

    Internally, ConvexMeshShape converts the mesh to an optimized data structure. This conversion
    can be done in advance. See the \l{Qt Quick 3D Physics Cooking}{cooking overview documentation}
    for details.

    \note If both the \l{ConvexMeshShape::}{geometry} and \l{ConvexMeshShape::}{source} properties
    are set then only \l{ConvexMeshShape::}{geometry} will be used.
    \sa ConvexMeshShape::geometry
*/

/*!
    \qmlproperty Geometry ConvexMeshShape::geometry
    This property defines the geometry of a mesh used to define the shape. If the
    mesh is not convex, the convex hull of the mesh will be used. The maximum number of faces
    and vertices is 255: If the mesh is more detailed than that, it will be simplified.

    \note If both the \l{ConvexMeshShape::}{geometry} and \l{ConvexMeshShape::}{source} properties
    are set then only \l{ConvexMeshShape::}{geometry} will be used.
    \sa ConvexMeshShape::source
    \since 6.7
*/

QMeshShape::MeshType QConvexMeshShape::shapeType() const
{
    return QMeshShape::MeshType::CONVEX;
}

bool QConvexMeshShape::isStaticShape() const
{
    return false;
}

QT_END_NAMESPACE
