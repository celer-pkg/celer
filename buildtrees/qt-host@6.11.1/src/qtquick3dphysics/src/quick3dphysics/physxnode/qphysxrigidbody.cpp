// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxrigidbody_p.h"

#include "qabstractphysicsbody_p.h"

QT_BEGIN_NAMESPACE

QPhysXRigidBody::QPhysXRigidBody(QAbstractPhysicsBody *frontEnd) : QPhysXActorBody(frontEnd) { }

void QPhysXRigidBody::createMaterial(QPhysXWorld *physX)
{
    createMaterialFromQtMaterial(
            physX, static_cast<QAbstractPhysicsBody *>(frontendNode)->physicsMaterial());
}

QT_END_NAMESPACE
