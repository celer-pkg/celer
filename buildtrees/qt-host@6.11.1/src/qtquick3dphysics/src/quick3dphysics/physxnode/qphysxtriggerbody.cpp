// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysxtriggerbody_p.h"

#include "qphysicsutils_p.h"
#include "qtriggerbody_p.h"

#include "PxRigidActor.h"

QT_BEGIN_NAMESPACE

QPhysXTriggerBody::QPhysXTriggerBody(QTriggerBody *frontEnd) : QPhysXActorBody(frontEnd) { }

DebugDrawBodyType QPhysXTriggerBody::getDebugDrawBodyType()
{
    return DebugDrawBodyType::Trigger;
}

void QPhysXTriggerBody::sync(float /*deltaTime*/,
                             QHash<QQuick3DNode *, QMatrix4x4> & /*transformCache*/)
{
    auto *triggerBody = static_cast<QTriggerBody *>(frontendNode);
    const physx::PxTransform trf = QPhysicsUtils::toPhysXTransform(triggerBody->scenePosition(),
                                                                   triggerBody->sceneRotation());
    actor->setGlobalPose(trf);
}

QT_END_NAMESPACE
