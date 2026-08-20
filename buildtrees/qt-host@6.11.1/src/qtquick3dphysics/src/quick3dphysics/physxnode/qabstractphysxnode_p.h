// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ABSTRACTPHYSXNODE_H
#define ABSTRACTPHYSXNODE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "foundation/PxTransform.h"
#include "qtconfigmacros.h"

#include <QVector>

namespace physx {
class PxMaterial;
class PxShape;
}

QT_BEGIN_NAMESPACE

class QAbstractPhysicsNode;
class QMatrix4x4;
class QQuick3DNode;
class QPhysicsWorld;
class QPhysicsMaterial;
class QPhysXWorld;

// Used for debug drawing
enum class DebugDrawBodyType {
    Static = 0,
    DynamicAwake = 1,
    DynamicSleeping = 2,
    Trigger = 3,
    Character = 4,
    Unknown = 5
};

/*
   NOTE
   The inheritance hierarchy is not ideal, since both controller and rigid body have materials,
   but trigger doesn't. AND both trigger and rigid body have actors, but controller doesn't.

   TODO: defaultMaterial isn't used for rigid bodies, since they always create their own
   QPhysicsMaterial with default values. We should only have a qt material when set explicitly.
*/

class QAbstractPhysXNode
{
public:
    QAbstractPhysXNode(QAbstractPhysicsNode *node);
    virtual ~QAbstractPhysXNode();

    bool cleanupIfRemoved(QPhysXWorld *physX); // TODO rename??

    virtual void init(QPhysicsWorld *world, QPhysXWorld *physX) = 0;
    virtual void updateDefaultDensity(float density);
    virtual void createMaterial(QPhysXWorld *physX);
    void createMaterialFromQtMaterial(QPhysXWorld *physX, QPhysicsMaterial *qtMaterial);
    virtual void markDirtyShapes();
    virtual void rebuildDirtyShapes(QPhysicsWorld *, QPhysXWorld *);
    virtual void updateFilters();

    virtual void sync(float deltaTime, QHash<QQuick3DNode *, QMatrix4x4> &transformCache) = 0;
    virtual void cleanup(QPhysXWorld *);
    virtual bool debugGeometryCapability();
    virtual physx::PxTransform getGlobalPose();

    virtual bool useTriggerFlag();
    virtual DebugDrawBodyType getDebugDrawBodyType();

    bool shapesDirty() const;
    void setShapesDirty(bool dirty);

    bool filtersDirty() const;
    void setFiltersDirty(bool dirty);

    QVector<physx::PxShape *> shapes;
    physx::PxMaterial *material = nullptr;
    QAbstractPhysicsNode *frontendNode = nullptr;
    bool isRemoved = false;
    static physx::PxMaterial *sDefaultMaterial;
};

QT_END_NAMESPACE

#endif
