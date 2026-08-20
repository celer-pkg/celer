// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxcharactercontroller_p.h"

#include "PxRigidDynamic.h"
#include "characterkinematic/PxController.h"
#include "characterkinematic/PxControllerManager.h"
#include "characterkinematic/PxCapsuleController.h"

#include "qcapsuleshape_p.h"
#include "qphysicsutils_p.h"
#include "qphysicsworld_p.h"
#include "qcharactercontroller_p.h"

#define PHYSX_RELEASE(x)                                                                           \
    if (x != nullptr) {                                                                            \
        x->release();                                                                              \
        x = nullptr;                                                                               \
    }

QT_BEGIN_NAMESPACE

class ControllerCallback : public physx::PxUserControllerHitReport
{
public:
    ControllerCallback(QPhysicsWorld *worldIn) : world(worldIn) { }

    void onShapeHit(const physx::PxControllerShapeHit &hit) override
    {
        QMutexLocker locker(&world->m_removedPhysicsNodesMutex);

        QAbstractPhysicsNode *other = static_cast<QAbstractPhysicsNode *>(hit.actor->userData);
        QCharacterController *trigger =
                static_cast<QCharacterController *>(hit.controller->getUserData());

        if (!trigger || !other || !trigger->enableShapeHitCallback())
            return;

        QVector3D position = QPhysicsUtils::toQtType(physx::toVec3(hit.worldPos));
        QVector3D impulse = QPhysicsUtils::toQtType(hit.dir * hit.length);
        QVector3D normal = QPhysicsUtils::toQtType(hit.worldNormal);

        emit trigger->shapeHit(other, position, impulse, normal);
    }
    void onControllerHit(const physx::PxControllersHit & /*hit*/) override { }
    void onObstacleHit(const physx::PxControllerObstacleHit & /*hit*/) override { }

private:
    QPhysicsWorld *world = nullptr;
};

QPhysXCharacterController::QPhysXCharacterController(QCharacterController *frontEnd)
    : QAbstractPhysXNode(frontEnd)
{
}

void QPhysXCharacterController::cleanup(QPhysXWorld *physX)
{
    PHYSX_RELEASE(controller);
    delete reportCallback;
    reportCallback = nullptr;
    QAbstractPhysXNode::cleanup(physX);
}

void QPhysXCharacterController::init(QPhysicsWorld *world, QPhysXWorld *physX)
{
    Q_ASSERT(!controller);

    auto *characterController = static_cast<QCharacterController *>(frontendNode);

    auto shapes = characterController->getCollisionShapesList();
    if (shapes.length() != 1) {
        qWarning() << "CharacterController: invalid collision shapes list.";
        return;
    }
    auto *capsule = qobject_cast<QCapsuleShape *>(shapes.first());
    if (!capsule) {
        qWarning() << "CharacterController: collision shape is not a capsule.";
        return;
    }
    auto *mgr = world->controllerManager();
    if (!mgr) {
        qWarning() << "QtQuick3DPhysics internal error: missing controller manager.";
        return;
    }

    createMaterial(physX);

    const QVector3D scale = characterController->sceneScale();
    const qreal heightScale = scale.y();
    const qreal radiusScale = scale.x();
    physx::PxCapsuleControllerDesc desc;
    reportCallback = new ControllerCallback(world);
    desc.reportCallback = reportCallback;
    desc.radius = 0.5f * radiusScale * capsule->diameter();
    desc.height = heightScale * capsule->height();
    desc.stepOffset = 0.25f * desc.height; // TODO: API

    desc.material = material;
    const QVector3D pos = characterController->scenePosition();
    desc.position = { pos.x(), pos.y(), pos.z() };
    // Safe to static_cast since createController will always return a PxCapsuleController
    // if desc is of type PxCapsuleControllerDesc
    controller = static_cast<physx::PxCapsuleController *>(mgr->createController(desc));

    if (!controller) {
        qWarning() << "QtQuick3DPhysics internal error: could not create controller.";
        return;
    }

    controller->setUserData(static_cast<void *>(frontendNode));

    auto *actor = controller->getActor();
    if (actor)
        actor->userData = characterController;
    else
        qWarning() << "QtQuick3DPhysics internal error: CharacterController created without actor.";
}

void QPhysXCharacterController::sync(float deltaTime,
                                     QHash<QQuick3DNode *, QMatrix4x4> & /*transformCache*/)
{
    if (controller == nullptr)
        return;

    auto *characterController = static_cast<QCharacterController *>(frontendNode);

    // Update capsule height, radius, stepOffset
    const auto &shapes = characterController->getCollisionShapesList();
    auto capsule = shapes.length() == 1 ? qobject_cast<QCapsuleShape *>(shapes.front()) : nullptr;

    if (shapes.length() != 1) {
        qWarning() << "CharacterController: invalid collision shapes list.";
    } else if (!capsule) {
        qWarning() << "CharacterController: collision shape is not a capsule.";
    } else {
        const QVector3D sceneScale = characterController->sceneScale();
        const qreal heightScale = sceneScale.y();
        const qreal radiusScale = sceneScale.x();

        // Update height
        const float heightNew = heightScale * capsule->height();
        if (!qFuzzyCompare(controller->getHeight(), heightNew))
            controller->resize(heightNew);
        // Update radius
        const float radiusNew = 0.5f * radiusScale * capsule->diameter();
        if (!qFuzzyCompare(controller->getRadius(), radiusNew))
            controller->setRadius(radiusNew);
        // Update stepOffset
        const float stepOffsetNew = 0.25f * heightNew;
        if (!qFuzzyCompare(controller->getStepOffset(), stepOffsetNew))
            controller->setStepOffset(stepOffsetNew);
    }

    // update node from physX
    QVector3D position = QPhysicsUtils::toQtType(physx::toVec3(controller->getPosition()));
    const QQuick3DNode *parentNode = static_cast<QQuick3DNode *>(characterController->parentItem());
    if (!parentNode) {
        // then it is the same space
        characterController->setPosition(position);
    } else {
        characterController->setPosition(parentNode->mapPositionFromScene(position));
    }

    QVector3D teleportPos;
    bool teleport = characterController->getTeleport(teleportPos);
    if (teleport) {
        controller->setPosition({ teleportPos.x(), teleportPos.y(), teleportPos.z() });
    } else if (deltaTime > 0) {
        const auto displacement =
                QPhysicsUtils::toPhysXType(characterController->getDisplacement(deltaTime));
        auto collisions =
                controller->move(displacement, displacement.magnitude() / 100, deltaTime, {});
        characterController->setCollisions(QCharacterController::Collisions(uint(collisions)));
    }
    // QCharacterController has a material property, but we don't inherit from
    // QPhysXMaterialBody, so we create the material manually in init()
    // TODO: handle material changes
}

void QPhysXCharacterController::createMaterial(QPhysXWorld *physX)
{
    createMaterialFromQtMaterial(
            physX, static_cast<QCharacterController *>(frontendNode)->physicsMaterial());
}

bool QPhysXCharacterController::debugGeometryCapability()
{
    return true;
}

DebugDrawBodyType QPhysXCharacterController::getDebugDrawBodyType()
{
    return DebugDrawBodyType::Character;
}

QT_END_NAMESPACE
