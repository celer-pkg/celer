// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qboxshape_p.h"

#include <QtQuick3D/QQuick3DGeometry>
#include <extensions/PxExtensionsAPI.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype BoxShape
    \inherits CollisionShape
    \inqmlmodule QtQuick3D.Physics
    \since 6.4
    \brief Defines a box collision shape.

    This type defines a box collision shape. The origin is at the center of the box.

    \note A non-uniform scaling transformation will scale the x, y and z directions individually.
    However, combining non-uniform scale and rotation may lead to shearing, which will not be applied
    to the BoxShape: it will always be a rectilinear box.

    \sa {Qt Quick 3D Physics Shapes and Bodies}{Shapes and Bodies overview documentation}
*/

/*!
    \qmlproperty vector3d BoxShape::extents
    This property defines the extents of the box in the x, y and z directions.

    Default value: \c{(100, 100, 100)}
*/

QBoxShape::QBoxShape() = default;
QBoxShape::~QBoxShape()
{
    delete m_physXGeometry;
}

QVector3D QBoxShape::extents() const
{
    return m_extents;
}

physx::PxGeometry *QBoxShape::getPhysXGeometry()
{
    if (!m_physXGeometry || m_scaleDirty) {
        updatePhysXGeometry();
    }
    return m_physXGeometry;
}

void QBoxShape::setExtents(QVector3D extents)
{
    if (m_extents == extents)
        return;

    m_extents = extents;
    updatePhysXGeometry();

    emit needsRebuild(this);
    emit extentsChanged(m_extents);
}

void QBoxShape::updatePhysXGeometry()
{
    delete m_physXGeometry;
    const QVector3D half = m_extents * sceneScale() * 0.5f;
    m_physXGeometry = new physx::PxBoxGeometry(half.x(), half.y(), half.z());
    m_scaleDirty = false;
}

QT_END_NAMESPACE
