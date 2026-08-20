// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxstaticbody_p.h"

#include "PxPhysics.h"
#include "PxRigidActor.h"
#include "PxRigidStatic.h"

#include "qphysicsutils_p.h"
#include "qphysicsworld_p.h"
#include "qstaticrigidbody_p.h"
#include "qstaticphysxobjects_p.h"

QT_BEGIN_NAMESPACE

QPhysXStaticBody::QPhysXStaticBody(QStaticRigidBody *frontEnd) : QPhysXRigidBody(frontEnd) { }

DebugDrawBodyType QPhysXStaticBody::getDebugDrawBodyType()
{
    return DebugDrawBodyType::Static;
}

void QPhysXStaticBody::sync(float deltaTime, QHash<QQuick3DNode *, QMatrix4x4> &transformCache)
{
    auto *staticBody = static_cast<QStaticRigidBody *>(frontendNode);
    const physx::PxTransform poseNew = QPhysicsUtils::toPhysXTransform(staticBody->scenePosition(),
                                                                       staticBody->sceneRotation());
    const physx::PxTransform poseOld = actor->getGlobalPose();

    // For performance we only update static objects if they have been moved
    if (!QPhysicsUtils::fuzzyEquals(poseNew, poseOld))
        actor->setGlobalPose(poseNew);

    const bool disabledPrevious = actor->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION;
    const bool disabled = !staticBody->simulationEnabled();
    if (disabled != disabledPrevious) {
        actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, disabled);
    }

    QPhysXActorBody::sync(deltaTime, transformCache);
}

void QPhysXStaticBody::createActor(QPhysXWorld * /*physX*/)
{
    auto &s_physx = StaticPhysXObjects::getReference();
    const physx::PxTransform trf = QPhysicsUtils::toPhysXTransform(frontendNode->scenePosition(),
                                                                   frontendNode->sceneRotation());
    actor = s_physx.physics->createRigidStatic(trf);
}

QT_END_NAMESPACE
