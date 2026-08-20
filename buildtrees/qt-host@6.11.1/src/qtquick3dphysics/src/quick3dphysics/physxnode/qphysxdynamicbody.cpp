// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxdynamicbody_p.h"

#include "PxRigidDynamic.h"

#include "qphysicscommands_p.h"
#include "qphysicsutils_p.h"
#include "qphysicsworld_p.h"
#include "qabstractphysicsbody_p.h"
#include "qdynamicrigidbody_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

static void processCommandQueue(QQueue<QPhysicsCommand *> &commandQueue,
                                const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body)
{
    for (auto command : commandQueue) {
        command->execute(rigidBody, body);
        delete command;
    }

    commandQueue.clear();
}

static QMatrix4x4 calculateKinematicNodeTransform(QQuick3DNode *node,
                                                  QHash<QQuick3DNode *, QMatrix4x4> &transformCache)
{
    // already calculated transform
    if (transformCache.contains(node))
        return transformCache[node];

    QMatrix4x4 localTransform;

    // DynamicRigidBody vs StaticRigidBody use different values for calculating the local transform
    if (auto drb = qobject_cast<const QDynamicRigidBody *>(node); drb != nullptr) {
        if (!drb->isKinematic()) {
            qWarning() << "Non-kinematic body as a parent of a kinematic body is unsupported";
        }
        localTransform = QSSGRenderNode::calculateTransformMatrix(
                drb->kinematicPosition(), drb->scale(), drb->kinematicPivot(),
                drb->kinematicRotation());
    } else {
        localTransform = QSSGRenderNode::calculateTransformMatrix(node->position(), node->scale(),
                                                                  node->pivot(), node->rotation());
    }

    QQuick3DNode *parent = node->parentNode();
    if (!parent) // no parent, local transform is scene transform
        return localTransform;

    // calculate the parent scene transform and apply the nodes local transform
    QMatrix4x4 parentTransform = calculateKinematicNodeTransform(parent, transformCache);
    QMatrix4x4 sceneTransform = parentTransform * localTransform;

    transformCache[node] = sceneTransform;
    return sceneTransform;
}

static physx::PxRigidDynamicLockFlags getLockFlags(QDynamicRigidBody *body)
{
    const auto lockAngular = body->angularAxisLock();
    const auto lockLinear = body->linearAxisLock();
    const int flags = (lockAngular & QDynamicRigidBody::AxisLock::LockX
                               ? physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X
                               : 0)
            | (lockAngular & QDynamicRigidBody::AxisLock::LockY
                       ? physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y
                       : 0)
            | (lockAngular & QDynamicRigidBody::AxisLock::LockZ
                       ? physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z
                       : 0)
            | (lockLinear & QDynamicRigidBody::AxisLock::LockX
                       ? physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X
                       : 0)
            | (lockLinear & QDynamicRigidBody::AxisLock::LockY
                       ? physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y
                       : 0)
            | (lockLinear & QDynamicRigidBody::AxisLock::LockZ
                       ? physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z
                       : 0);
    return static_cast<physx::PxRigidDynamicLockFlags>(flags);
}

static physx::PxTransform getPhysXWorldTransform(const QMatrix4x4 transform)
{
    auto rotationMatrix = transform;
    QSSGUtils::mat44::normalize(rotationMatrix);
    auto rotation =
            QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(rotationMatrix)).normalized();
    const QVector3D worldPosition = QSSGUtils::mat44::getPosition(transform);
    return physx::PxTransform(QPhysicsUtils::toPhysXType(worldPosition),
                              QPhysicsUtils::toPhysXType(rotation));
}

QPhysXDynamicBody::QPhysXDynamicBody(QDynamicRigidBody *frontEnd) : QPhysXRigidBody(frontEnd) { }

DebugDrawBodyType QPhysXDynamicBody::getDebugDrawBodyType()
{
    auto *dynamicRigidBody = static_cast<QDynamicRigidBody *>(frontendNode);
    return dynamicRigidBody->isSleeping() ? DebugDrawBodyType::DynamicSleeping
                                          : DebugDrawBodyType::DynamicAwake;
}

void QPhysXDynamicBody::sync(float deltaTime, QHash<QQuick3DNode *, QMatrix4x4> &transformCache)
{
    auto *dynamicRigidBody = static_cast<QDynamicRigidBody *>(frontendNode);
    // first update front end node from physx simulation
    dynamicRigidBody->updateFromPhysicsTransform(actor->getGlobalPose());

    auto *dynamicActor = static_cast<physx::PxRigidDynamic *>(actor);
    processCommandQueue(dynamicRigidBody->commandQueue(), *dynamicRigidBody, *dynamicActor);
    if (dynamicRigidBody->isKinematic()) {
        // Since this is a kinematic body we need to calculate the transform by hand and since
        // bodies can occur in other bodies we need to calculate the tranform recursively for all
        // parents. To save some computation we cache these transforms in 'transformCache'.
        QMatrix4x4 transform = calculateKinematicNodeTransform(dynamicRigidBody, transformCache);
        dynamicActor->setKinematicTarget(getPhysXWorldTransform(transform));
    } else {
        dynamicActor->setRigidDynamicLockFlags(getLockFlags(dynamicRigidBody));
    }

    const bool disabledPrevious = actor->getActorFlags() & physx::PxActorFlag::eDISABLE_SIMULATION;
    const bool disabled = !dynamicRigidBody->simulationEnabled();
    if (disabled != disabledPrevious) {
        actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, disabled);
        if (!disabled && !dynamicRigidBody->isKinematic())
            dynamicActor->wakeUp();
    }

    dynamicRigidBody->setIsSleeping(dynamicActor->isSleeping());

    QPhysXActorBody::sync(deltaTime, transformCache);
}

void QPhysXDynamicBody::rebuildDirtyShapes(QPhysicsWorld *world, QPhysXWorld *physX)
{
    if (!shapesDirty())
        return;

    buildShapes(physX);

    QDynamicRigidBody *drb = static_cast<QDynamicRigidBody *>(frontendNode);

    // Density must be set after shapes so the inertia tensor is set
    if (!drb->hasStaticShapes()) {
        // Body with only dynamic shapes, set/calculate mass
        QPhysicsCommand *command = nullptr;
        switch (drb->massMode()) {
        case QDynamicRigidBody::MassMode::DefaultDensity: {
            command = new QPhysicsCommandSetDensity(world->defaultDensity());
            break;
        }
        case QDynamicRigidBody::MassMode::CustomDensity: {
            command = new QPhysicsCommandSetDensity(drb->density());
            break;
        }
        case QDynamicRigidBody::MassMode::Mass: {
            const float mass = qMax(drb->mass(), 0.f);
            command = new QPhysicsCommandSetMass(mass);
            break;
        }
        case QDynamicRigidBody::MassMode::MassAndInertiaTensor: {
            const float mass = qMax(drb->mass(), 0.f);
            command = new QPhysicsCommandSetMassAndInertiaTensor(mass, drb->inertiaTensor());
            break;
        }
        case QDynamicRigidBody::MassMode::MassAndInertiaMatrix: {
            const float mass = qMax(drb->mass(), 0.f);
            command = new QPhysicsCommandSetMassAndInertiaMatrix(mass, drb->inertiaMatrix());
            break;
        }
        }

        drb->commandQueue().enqueue(command);
    } else if (!drb->isKinematic()) {
        // Body with static shapes that is not kinematic, this is disallowed
        qWarning() << "Cannot make body containing trimesh/heightfield/plane non-kinematic, "
                      "forcing kinematic.";
        drb->setIsKinematic(true);
    }

    const bool isKinematic = drb->isKinematic();
    auto *dynamicBody = static_cast<physx::PxRigidDynamic *>(actor);
    dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);

    if (world->enableCCD()) {
        // Regular sweep-based CCD is only available for non-kinematic bodies but speculative CCD
        // is available for kinematic bodies so we use that.
        dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, !isKinematic);
        dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, isKinematic);
    }

    setShapesDirty(false);
}

void QPhysXDynamicBody::updateDefaultDensity(float density)
{
    QDynamicRigidBody *rigidBody = static_cast<QDynamicRigidBody *>(frontendNode);
    rigidBody->updateDefaultDensity(density);
}

QT_END_NAMESPACE
