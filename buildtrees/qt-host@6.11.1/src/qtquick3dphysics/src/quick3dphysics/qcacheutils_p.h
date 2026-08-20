// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CACHEUTILS_H
#define CACHEUTILS_H

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

#include <QtCore/qtconfigmacros.h>
#include <QtCore/QString>

namespace physx {
class PxDefaultMemoryOutputStream;
class PxTriangleMesh;
class PxConvexMesh;
class PxPhysics;
class PxHeightField;
}

QT_BEGIN_NAMESPACE
namespace QCacheUtils {
void writeCachedTriangleMesh(const QString &filePath, physx::PxDefaultMemoryOutputStream &buf);
void writeCachedConvexMesh(const QString &filePath, physx::PxDefaultMemoryOutputStream &buf);
void writeCachedHeightField(const QString &filePath, physx::PxDefaultMemoryOutputStream &buf);

physx::PxTriangleMesh *readCookedTriangleMesh(const QString &filePath, physx::PxPhysics &physics);
physx::PxConvexMesh *readCookedConvexMesh(const QString &filePath, physx::PxPhysics &physics);
physx::PxHeightField *readCookedHeightField(const QString &filePath, physx::PxPhysics &physics);

physx::PxTriangleMesh *readCachedTriangleMesh(const QString &filePath, physx::PxPhysics &physics);
physx::PxConvexMesh *readCachedConvexMesh(const QString &filePath, physx::PxPhysics &physics);
physx::PxHeightField *readCachedHeightField(const QString &filePath, physx::PxPhysics &physics);
}
QT_END_NAMESPACE

#endif
