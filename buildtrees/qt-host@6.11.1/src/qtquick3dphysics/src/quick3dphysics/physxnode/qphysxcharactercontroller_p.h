// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PHYSXCHARACTERCONTROLLER_H
#define PHYSXCHARACTERCONTROLLER_H

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

#include "qabstractphysxnode_p.h"
#include "qtconfigmacros.h"

namespace physx {
class PxCapsuleController;
}

QT_BEGIN_NAMESPACE

class QCharacterController;
class ControllerCallback;

class QPhysXCharacterController : public QAbstractPhysXNode
{
public:
    QPhysXCharacterController(QCharacterController *frontEnd);
    void cleanup(QPhysXWorld *physX) override;
    void init(QPhysicsWorld *world, QPhysXWorld *physX) override;
    void sync(float deltaTime, QHash<QQuick3DNode *, QMatrix4x4> &transformCache) override;
    void createMaterial(QPhysXWorld *physX) override;
    bool debugGeometryCapability() override;
    DebugDrawBodyType getDebugDrawBodyType() override;

private:
    physx::PxCapsuleController *controller = nullptr;
    ControllerCallback *reportCallback = nullptr;
};

QT_END_NAMESPACE

#endif
