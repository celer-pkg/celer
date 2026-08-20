// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PHYSXACTORBODY_H
#define PHYSXACTORBODY_H

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

#include "qtconfigmacros.h"
#include "qabstractphysxnode_p.h"

namespace physx {
class PxRigidActor;
}

QT_BEGIN_NAMESPACE

class QPhysXActorBody : public QAbstractPhysXNode
{
public:
    QPhysXActorBody(QAbstractPhysicsNode *frontEnd);
    void cleanup(QPhysXWorld *physX) override;
    void init(QPhysicsWorld *world, QPhysXWorld *physX) override;
    void sync(float deltaTime, QHash<QQuick3DNode *, QMatrix4x4> &transformCache) override;
    void markDirtyShapes() override;
    void rebuildDirtyShapes(QPhysicsWorld *world, QPhysXWorld *physX) override;
    virtual void createActor(QPhysXWorld *physX);

    bool debugGeometryCapability() override;
    physx::PxTransform getGlobalPose() override;
    void buildShapes(QPhysXWorld *physX);
    void updateFilters() override;

    physx::PxRigidActor *actor = nullptr;
};

QT_END_NAMESPACE

#endif
