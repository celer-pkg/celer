// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxactorbody_p.h"

#include "PxMaterial.h"
#include "PxPhysics.h"
#include "PxRigidDynamic.h"
#include "PxRigidActor.h"
#include "PxScene.h"

#include "physxnode/qphysxworld_p.h"
#include "qabstractphysicsbody_p.h"
#include "qheightfieldshape_p.h"
#include "qphysicsutils_p.h"
#include "qplaneshape_p.h"
#include "qstaticphysxobjects_p.h"

#include <QtGui/qquaternion.h>

#define PHYSX_RELEASE(x)                                                                           \
    if (x != nullptr) {                                                                            \
        x->release();                                                                              \
        x = nullptr;                                                                               \
    }

QT_BEGIN_NAMESPACE

static physx::PxTransform getPhysXLocalTransform(const QQuick3DNode *node)
{
    // Modify transforms to make the PhysX shapes match the QtQuick3D conventions
    if (qobject_cast<const QPlaneShape *>(node) != nullptr) {
        // Rotate the plane to make it match the built-in rectangle
        const QQuaternion rotation = QPhysicsUtils::kMinus90YawRotation * node->rotation();
        return physx::PxTransform(QPhysicsUtils::toPhysXType(node->position()),
                                  QPhysicsUtils::toPhysXType(rotation));
    } else if (auto *hf = qobject_cast<const QHeightFieldShape *>(node)) {
        // Shift the height field so it's centered at the origin
        return physx::PxTransform(QPhysicsUtils::toPhysXType(node->position() + hf->hfOffset()),
                                  QPhysicsUtils::toPhysXType(node->rotation()));
    }

    const QQuaternion &rotation = node->rotation();
    const QVector3D &localPosition = node->position();
    const QVector3D &scale = node->sceneScale();
    return physx::PxTransform(QPhysicsUtils::toPhysXType(localPosition * scale),
                              QPhysicsUtils::toPhysXType(rotation));
}

QPhysXActorBody::QPhysXActorBody(QAbstractPhysicsNode *frontEnd) : QAbstractPhysXNode(frontEnd) { }

void QPhysXActorBody::cleanup(QPhysXWorld *physX)
{
    if (actor) {
        physX->scene->removeActor(*actor);
        PHYSX_RELEASE(actor);
    }
    QAbstractPhysXNode::cleanup(physX);
}

void QPhysXActorBody::init(QPhysicsWorld * /*world*/, QPhysXWorld *physX)
{
    Q_ASSERT(!actor);

    createMaterial(physX);
    createActor(physX);

    actor->userData = reinterpret_cast<void *>(frontendNode);
    physX->scene->addActor(*actor);
    setShapesDirty(true);
}

void QPhysXActorBody::sync(float /*deltaTime*/,
                           QHash<QQuick3DNode *, QMatrix4x4> & /*transformCache*/)
{
    auto *body = static_cast<QAbstractPhysicsBody *>(frontendNode);
    if (QPhysicsMaterial *qtMaterial = body->physicsMaterial()) {
        const float staticFriction = qtMaterial->staticFriction();
        const float dynamicFriction = qtMaterial->dynamicFriction();
        const float restitution = qtMaterial->restitution();
        if (material->getStaticFriction() != staticFriction)
            material->setStaticFriction(staticFriction);
        if (material->getDynamicFriction() != dynamicFriction)
            material->setDynamicFriction(dynamicFriction);
        if (material->getRestitution() != restitution)
            material->setRestitution(restitution);
    }
}

void QPhysXActorBody::markDirtyShapes()
{
    if (!frontendNode || !actor)
        return;

    // Go through the shapes and look for a change in pose (rotation, position)
    // TODO: it is likely cheaper to connect a signal for changes on the position and rotation
    // property and mark the node dirty then.
    if (!shapesDirty()) {
        const auto &collisionShapes = frontendNode->getCollisionShapesList();
        const auto &physXShapes = shapes;

        const int len = collisionShapes.size();
        if (physXShapes.size() != len) {
            // This should not really happen but check it anyway
            setShapesDirty(true);
        } else {
            for (int i = 0; i < len; i++) {
                auto poseNew = getPhysXLocalTransform(collisionShapes[i]);
                auto poseOld = physXShapes[i]->getLocalPose();

                if (!QPhysicsUtils::fuzzyEquals(poseNew, poseOld)) {
                    setShapesDirty(true);
                    break;
                }
            }
        }
    }
}

void QPhysXActorBody::rebuildDirtyShapes(QPhysicsWorld * /*world*/, QPhysXWorld *physX)
{
    if (!shapesDirty())
        return;
    buildShapes(physX);
    setShapesDirty(false);
}

void QPhysXActorBody::createActor(QPhysXWorld * /*physX*/)
{
    auto &s_physx = StaticPhysXObjects::getReference();
    const physx::PxTransform trf = QPhysicsUtils::toPhysXTransform(frontendNode->scenePosition(),
                                                                   frontendNode->sceneRotation());
    actor = s_physx.physics->createRigidDynamic(trf);
}

bool QPhysXActorBody::debugGeometryCapability()
{
    return true;
}

physx::PxTransform QPhysXActorBody::getGlobalPose()
{
    return actor->getGlobalPose();
}

void QPhysXActorBody::buildShapes(QPhysXWorld * /*physX*/)
{
    auto body = actor;
    for (auto *shape : shapes) {
        body->detachShape(*shape);
        PHYSX_RELEASE(shape);
    }

    // TODO: Only remove changed shapes?
    shapes.clear();

    for (const auto &collisionShape : frontendNode->getCollisionShapesList()) {
        // TODO: shapes can be shared between multiple actors.
        // Do we need to create new ones for every body?
        auto *geom = collisionShape->getPhysXGeometry();
        if (!geom || !material)
            continue;

        auto &s_physx = StaticPhysXObjects::getReference();
        auto physXShape = s_physx.physics->createShape(*geom, *material);

        if (useTriggerFlag()) {
            physXShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
            physXShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
        }

        { // Setup filtering
            physx::PxFilterData filterData;
            filterData.word0 = frontendNode->filterGroup();
            filterData.word1 = frontendNode->filterIgnoreGroups();
            physXShape->setSimulationFilterData(filterData);
        }

        shapes.push_back(physXShape);
        physXShape->setLocalPose(getPhysXLocalTransform(collisionShape));
        body->attachShape(*physXShape);
    }

    // Filters are always clean after building shapes
    setFiltersDirty(false);
}

void QPhysXActorBody::updateFilters()
{
    if (!filtersDirty())
        return;

    // Go through all shapes and set the filter group and mask.
    // TODO: What about shared shapes on several actors?
    for (auto &physXShape : shapes) {
        physx::PxFilterData filterData;
        filterData.word0 = frontendNode->filterGroup();
        filterData.word1 = frontendNode->filterIgnoreGroups();
        physXShape->setSimulationFilterData(filterData);
    }

    setFiltersDirty(false);
}

QT_END_NAMESPACE
