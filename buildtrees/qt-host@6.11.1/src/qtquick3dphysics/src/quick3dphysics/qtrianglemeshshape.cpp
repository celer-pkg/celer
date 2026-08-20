// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtrianglemeshshape_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype TriangleMeshShape
    \inqmlmodule QtQuick3D.Physics
    \inherits CollisionShape
    \since 6.4
    \brief A collision shape based on a 3D mesh.

    This type defines a shape based on the same 3D mesh file format used by
    \l [QtQuick3D]{Model::source}{QtQuick3D.Model}.

    Objects that are controlled by the physics simulation cannot use TriangleMeshShape: It can only
    be used with \l StaticRigidBody and \l {DynamicRigidBody::isKinematic}{kinematic bodies}. Use \l
    ConvexMeshShape for non-kinematic dynamic bodies.

    \sa {Qt Quick 3D Physics Shapes and Bodies}{Shapes and Bodies overview documentation}
*/

/*!
    \qmlproperty url TriangleMeshShape::source
    This property defines the location of the mesh file used to define the shape.

    Internally, TriangleMeshShape converts the mesh to an optimized data structure. This conversion
    can be done in advance. See the \l{Qt Quick 3D Physics Cooking}{cooking overview documentation}
    for details.

    \note If both the \l{TriangleMeshShape::}{geometry} and \l{TriangleMeshShape::}{source}
    properties are set then only \l{TriangleMeshShape::}{geometry} will be used.
    \sa TriangleMeshShape::geometry
*/

/*!
    \qmlproperty Geometry TriangleMeshShape::geometry
    This property defines the geometry of a mesh used to define the shape.
    \note If both the \l{TriangleMeshShape::}{geometry} and \l{TriangleMeshShape::}{source}
    properties are set then only \l{TriangleMeshShape::}{geometry} will be used.
    \sa TriangleMeshShape::source
    \since 6.7
*/

QMeshShape::MeshType QTriangleMeshShape::shapeType() const
{
    return QMeshShape::MeshType::TRIANGLE;
}

bool QTriangleMeshShape::isStaticShape() const
{
    return true;
}

QT_END_NAMESPACE
