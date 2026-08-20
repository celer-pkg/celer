// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qsphereshape_p.h"
#include <QtQuick3D/QQuick3DGeometry>

#include <geometry/PxSphereGeometry.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype SphereShape
    \inqmlmodule QtQuick3D.Physics
    \inherits CollisionShape
    \since 6.4
    \brief Defines a spherical collision shape.

    This type defines a spherical shape. The origin is at the center of the sphere.
    \note When using a scaling transformation with this shape, the x component will be used to scale the diameter of
    the sphere. The resulting shape will be perfectly round, even if a non-uniform scaling transformation is used.
*/

/*!
    \qmlproperty real SphereShape::diameter
    This property defines the diameter of the sphere

    Default value: \c{100}
*/

QSphereShape::QSphereShape() = default;

QSphereShape::~QSphereShape()
{
    delete m_physXGeometry;
}

float QSphereShape::diameter() const
{
    return m_diameter;
}

physx::PxGeometry *QSphereShape::getPhysXGeometry()
{
    if (!m_physXGeometry || m_scaleDirty) {
        updatePhysXGeometry();
    }
    return m_physXGeometry;
}

void QSphereShape::setDiameter(float diameter)
{
    if (qFuzzyCompare(m_diameter, diameter))
        return;

    m_diameter = diameter;
    updatePhysXGeometry();

    emit needsRebuild(this);
    emit diameterChanged(m_diameter);
}

void QSphereShape::updatePhysXGeometry()
{
    delete m_physXGeometry;
    auto s = sceneScale();
    m_physXGeometry = new physx::PxSphereGeometry(m_diameter * 0.5f * s.x());
    m_scaleDirty = false;
}

QT_END_NAMESPACE
