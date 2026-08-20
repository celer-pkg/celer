// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractphysxnode_p.h"

#include "qabstractphysicsnode_p.h"
#include "qphysicsmaterial_p.h"
#include "qstaticphysxobjects_p.h"

#include "PxPhysics.h"
#include "PxMaterial.h"
#include "PxShape.h"

#define PHYSX_RELEASE(x)                                                                           \
    if (x != nullptr) {                                                                            \
        x->release();                                                                              \
        x = nullptr;                                                                               \
    }

QT_BEGIN_NAMESPACE

physx::PxMaterial *QAbstractPhysXNode::sDefaultMaterial = nullptr;

QAbstractPhysXNode::QAbstractPhysXNode(QAbstractPhysicsNode *node) : frontendNode(node)
{
    Q_ASSERT(node->m_backendObject == nullptr);
    node->m_backendObject = this;
}

QAbstractPhysXNode::~QAbstractPhysXNode() {
    if (!frontendNode) {
        Q_ASSERT(isRemoved);
        return;
    }

    Q_ASSERT(frontendNode->m_backendObject == this);
    frontendNode->m_backendObject = nullptr;
}

bool QAbstractPhysXNode::cleanupIfRemoved(QPhysXWorld *physX)
{
    if (isRemoved) {
        cleanup(physX);
        delete this;
        return true;
    }
    return false;
}

void QAbstractPhysXNode::updateDefaultDensity(float) { }

void QAbstractPhysXNode::createMaterial(QPhysXWorld *physX)
{
    createMaterialFromQtMaterial(physX, nullptr);
}

void QAbstractPhysXNode::createMaterialFromQtMaterial(QPhysXWorld *, QPhysicsMaterial *qtMaterial)
{
    auto &s_physx = StaticPhysXObjects::getReference();

    if (qtMaterial) {
        material = s_physx.physics->createMaterial(qtMaterial->staticFriction(),
                                                   qtMaterial->dynamicFriction(),
                                                   qtMaterial->restitution());
    } else {
        if (!sDefaultMaterial) {
            sDefaultMaterial = s_physx.physics->createMaterial(
                    QPhysicsMaterial::defaultStaticFriction,
                    QPhysicsMaterial::defaultDynamicFriction, QPhysicsMaterial::defaultRestitution);
        }
        material = sDefaultMaterial;
    }
}

void QAbstractPhysXNode::markDirtyShapes() { }

void QAbstractPhysXNode::rebuildDirtyShapes(QPhysicsWorld *, QPhysXWorld *) { }

void QAbstractPhysXNode::updateFilters() { }

void QAbstractPhysXNode::cleanup(QPhysXWorld *)
{
    for (auto *shape : shapes)
        PHYSX_RELEASE(shape);
    if (material != sDefaultMaterial)
        PHYSX_RELEASE(material);
}

bool QAbstractPhysXNode::debugGeometryCapability()
{
    return false;
}

physx::PxTransform QAbstractPhysXNode::getGlobalPose()
{
    return {};
}

bool QAbstractPhysXNode::useTriggerFlag()
{
    return false;
}

DebugDrawBodyType QAbstractPhysXNode::getDebugDrawBodyType()
{
    return DebugDrawBodyType::Unknown;
}

bool QAbstractPhysXNode::shapesDirty() const
{
    return frontendNode && frontendNode->m_shapesDirty;
}

void QAbstractPhysXNode::setShapesDirty(bool dirty)
{
    frontendNode->m_shapesDirty = dirty;
}

bool QAbstractPhysXNode::filtersDirty() const
{
    return frontendNode && frontendNode->m_filtersDirty;
}

void QAbstractPhysXNode::setFiltersDirty(bool dirty)
{
    Q_ASSERT(frontendNode);
    frontendNode->m_filtersDirty = dirty;
}

QT_END_NAMESPACE
