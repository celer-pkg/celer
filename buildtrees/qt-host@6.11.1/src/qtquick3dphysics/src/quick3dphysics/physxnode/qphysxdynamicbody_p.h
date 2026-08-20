// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PHYSXDYNAMICBODY_H
#define PHYSXDYNAMICBODY_H

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

#include "physxnode/qphysxrigidbody_p.h"

#include "qtconfigmacros.h"

QT_BEGIN_NAMESPACE

class QDynamicRigidBody;

class QPhysXDynamicBody : public QPhysXRigidBody
{
public:
    QPhysXDynamicBody(QDynamicRigidBody *frontEnd);

    DebugDrawBodyType getDebugDrawBodyType() override;
    void sync(float deltaTime, QHash<QQuick3DNode *, QMatrix4x4> &transformCache) override;
    void rebuildDirtyShapes(QPhysicsWorld *world, QPhysXWorld *physX) override;
    void updateDefaultDensity(float density) override;
};

QT_END_NAMESPACE

#endif
