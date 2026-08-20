// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PHYSXWORLD_H
#define PHYSXWORLD_H

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

namespace physx {
class PxScene;
class PxControllerManager;
}

QT_BEGIN_NAMESPACE

class SimulationEventCallback;
class QPhysicsWorld;
class QVector3D;

class QPhysXWorld
{
public:
    void createWorld();
    void deleteWorld();
    void createScene(float typicalLength, float typicalSpeed, const QVector3D &gravity,
                     bool enableCCD, QPhysicsWorld *physicsWorld, unsigned int numThreads);

    // variables unique to each world/scene
    physx::PxControllerManager *controllerManager = nullptr;
    SimulationEventCallback *callback = nullptr;
    physx::PxScene *scene = nullptr;
    bool isRunning = false;
};

QT_END_NAMESPACE

#endif
