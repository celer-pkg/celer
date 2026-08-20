// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractphysicsbody_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype PhysicsBody
    \inherits PhysicsNode
    \inqmlmodule QtQuick3D.Physics
    \since 6.4
    \brief Base type for all concrete physical bodies.

    PhysicsBody is the base type for all objects that have a physical presence. These objects
    interact with other bodies. Some types are not influenced by the simulation, such as
    StaticRigidBody: They only influence other bodies. Other bodies are fully governed by the
    simulation.

    \sa {Qt Quick 3D Physics Shapes and Bodies}{Shapes and Bodies overview documentation}
*/

/*!
    \qmlproperty PhysicsMaterial PhysicsBody::physicsMaterial
    This property defines how the body behaves when it collides with or slides against other bodies in the simulation.
*/

/*!
    \qmlproperty bool PhysicsBody::simulationEnabled
    This property defines if the body will partake in the physical simulation.

    Default value: \c{true}
*/

QAbstractPhysicsBody::QAbstractPhysicsBody()
{
    m_physicsMaterial = new QPhysicsMaterial(this);
}

QPhysicsMaterial *QAbstractPhysicsBody::physicsMaterial() const
{
    return m_physicsMaterial;
}

void QAbstractPhysicsBody::setPhysicsMaterial(QPhysicsMaterial *newPhysicsMaterial)
{
    if (m_physicsMaterial == newPhysicsMaterial)
        return;
    m_physicsMaterial = newPhysicsMaterial;
    emit physicsMaterialChanged();
}

bool QAbstractPhysicsBody::simulationEnabled() const
{
    return m_simulationEnabled;
}

void QAbstractPhysicsBody::setSimulationEnabled(bool newSimulationEnabled)
{
    if (m_simulationEnabled == newSimulationEnabled)
        return;
    m_simulationEnabled = newSimulationEnabled;
    emit simulationEnabledChanged();
}

QT_END_NAMESPACE
