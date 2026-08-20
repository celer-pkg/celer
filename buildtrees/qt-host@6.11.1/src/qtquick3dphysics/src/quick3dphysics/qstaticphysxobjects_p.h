// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef STATICPHYSXOBJECTS_H
#define STATICPHYSXOBJECTS_H

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

#include "extensions/PxDefaultAllocator.h"
#include "extensions/PxDefaultErrorCallback.h"

namespace physx {
class PxPvdTransport;
class PxPvd;
class PxFoundation;
class PxDefaultCpuDispatcher;
class PxCooking;
}

QT_BEGIN_NAMESPACE

struct StaticPhysXObjects
{
    physx::PxDefaultErrorCallback defaultErrorCallback;
    physx::PxDefaultAllocator defaultAllocatorCallback;
    physx::PxFoundation *foundation = nullptr;
    physx::PxPvd *pvd = nullptr;
    physx::PxPvdTransport *transport = nullptr;
    physx::PxPhysics *physics = nullptr;
    physx::PxDefaultCpuDispatcher *dispatcher = nullptr;
    physx::PxCooking *cooking = nullptr;

    unsigned int foundationRefCount = 0;
    bool foundationCreated = false;
    bool physicsCreated = false;

    static StaticPhysXObjects &getReference();
};

QT_END_NAMESPACE

#endif
