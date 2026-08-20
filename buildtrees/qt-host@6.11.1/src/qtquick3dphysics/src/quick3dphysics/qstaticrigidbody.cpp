// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qstaticrigidbody_p.h"

#include "physxnode/qphysxstaticbody_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype StaticRigidBody
    \inqmlmodule QtQuick3D.Physics
    \inherits PhysicsBody
    \since 6.4
    \brief A physical body that does not move.

    The StaticRigidBody type defines an immovable and static rigid body. Any collision shape is allowed for this body.

    \note Do not move a StaticRigidBody. It is technically possible to do so, but it
    will incur a performance penalty, and colliding dynamic objects may not react correctly.
    Use a DynamicRigidBody with \l {DynamicRigidBody::isKinematic}{isKinematic} set to \c true instead.
*/

QStaticRigidBody::QStaticRigidBody() = default;

QAbstractPhysXNode *QStaticRigidBody::createPhysXBackend()
{
    return new QPhysXStaticBody(this);
}

QT_END_NAMESPACE
