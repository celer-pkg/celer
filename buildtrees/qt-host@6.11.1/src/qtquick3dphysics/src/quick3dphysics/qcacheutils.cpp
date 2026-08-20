// Qt-Security score:critical reason:data-parser
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Security note: This file reads user provided cooked mesh files which are then
// fed to PhysX which creates meshes from these. This file also reads and writes
// mesh cache files but that is assumed to be safe.

#include "qcacheutils_p.h"

#include <QFile>
#include <QFileInfo>
#include <QtQml/QQmlFile>
#include <extensions/PxExtensionsAPI.h>
#include "qphysicsworld_p.h"

QT_BEGIN_NAMESPACE
namespace QCacheUtils {

enum class CacheGeometry { TriangleMesh, ConvexMesh, HeightField };

static QString MESH_CACHE_PATH = qEnvironmentVariable("QT_PHYSICS_CACHE_PATH");

static QString getCachedFilename(const QString &filePath, CacheGeometry geom)
{
    const char *extension = "unknown_physx";
    switch (geom) {
    case CacheGeometry::TriangleMesh:
        extension = "triangle_physx";
        break;
    case CacheGeometry::ConvexMesh:
        extension = "convex_physx";
        break;
    case CacheGeometry::HeightField:
        extension = "heightfield_physx";
        break;
    }

    return QString::fromUtf8("%1/%2.%3")
            .arg(MESH_CACHE_PATH, QFileInfo(filePath).fileName(), QLatin1StringView(extension));
}

static void readCachedMesh(const QString &meshFilename, physx::PxPhysics &physics,
                           physx::PxTriangleMesh *&triangleMesh, physx::PxConvexMesh *&convexMesh,
                           physx::PxHeightField *&heightField, CacheGeometry geom)
{
    if (MESH_CACHE_PATH.isEmpty())
        return;

    QString cacheFilename = getCachedFilename(meshFilename, geom);
    QFile cacheFile(cacheFilename);
    QFile meshFile(meshFilename);
    uchar *cacheData = nullptr;
    uchar *meshData = nullptr;

    auto cleanup = qScopeGuard([&] {
        if (cacheData)
            cacheFile.unmap(cacheData);
        if (meshData)
            meshFile.unmap(meshData);
        if (cacheFile.isOpen())
            cacheFile.close();
        if (meshFile.isOpen())
            meshFile.close();
    });

    if (!cacheFile.open(QIODevice::ReadOnly)) {
        return;
    }
    if (!meshFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << meshFilename;
        return;
    }

    // first uint64 (8 bytes) is hash of input file
    if (cacheFile.size() <= qint64(sizeof(uint64_t))) {
        qWarning() << "Invalid convex mesh from file" << cacheFilename;
        return;
    }

    cacheData = cacheFile.map(0, cacheFile.size());
    if (!cacheData) {
        qWarning() << "Could not map" << cacheFilename;
        return;
    }
    uint64_t cacheHash = *reinterpret_cast<uint64_t *>(cacheData);

    meshData = meshFile.map(0, meshFile.size());
    if (!meshData) {
        qWarning() << "Could not map" << meshFilename;
        return;
    }
    uint64_t meshHash = qHash(QByteArrayView(meshData, meshFile.size()));

    if (cacheHash != meshHash)
        return; // hash is different, need to re-cook

    physx::PxDefaultMemoryInputData input(cacheData + sizeof(uint64_t),
                                          physx::PxU32(cacheFile.size() - sizeof(uint64_t)));

    switch (geom) {
    case CacheGeometry::TriangleMesh: {
        triangleMesh = physics.createTriangleMesh(input);
        qCDebug(lcQuick3dPhysics) << "Read triangle mesh" << triangleMesh << "from file"
                                  << cacheFilename;
        break;
    }
    case CacheGeometry::ConvexMesh: {
        convexMesh = physics.createConvexMesh(input);
        qCDebug(lcQuick3dPhysics) << "Read convex mesh" << convexMesh << "from file"
                                  << cacheFilename;
        break;
    }
    case CacheGeometry::HeightField:
        heightField = physics.createHeightField(input);
        qCDebug(lcQuick3dPhysics) << "Read height field" << heightField << "from file"
                                  << cacheFilename;
        break;
    }
}

static void writeCachedMesh(const QString &meshFilename, physx::PxDefaultMemoryOutputStream &buf,
                            CacheGeometry geom)
{
    if (MESH_CACHE_PATH.isEmpty())
        return;

    QString cacheFilename = getCachedFilename(meshFilename, geom);
    QFile cacheFile(cacheFilename);
    QFile meshFile(meshFilename);
    uchar *cacheData = nullptr;
    uchar *meshData = nullptr;

    auto cleanup = qScopeGuard([&] {
        if (cacheData)
            cacheFile.unmap(cacheData);
        if (meshData)
            meshFile.unmap(meshData);
        if (cacheFile.isOpen())
            cacheFile.close();
        if (meshFile.isOpen())
            meshFile.close();
    });

    if (!cacheFile.open(QIODevice::WriteOnly)) {
        qCWarning(lcQuick3dPhysics) << "Could not open" << cacheFile.fileName() << "for writing.";
        return;
    }
    if (!meshFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << meshFilename;
        return;
    }

    meshData = meshFile.map(0, meshFile.size());
    if (!meshData) {
        qWarning() << "Could not map" << meshFilename;
        return;
    }
    uint64_t meshHash = qHash(QByteArrayView(meshData, meshFile.size()));

    cacheFile.write(reinterpret_cast<char *>(&meshHash), sizeof(uint64_t));
    cacheFile.write(reinterpret_cast<char *>(buf.getData()), buf.getSize());
    cacheFile.close();

    qCDebug(lcQuick3dPhysics) << "Wrote" << cacheFile.size() << "bytes to" << cacheFile.fileName();
}

void writeCachedTriangleMesh(const QString &filePath, physx::PxDefaultMemoryOutputStream &buf)
{
    writeCachedMesh(filePath, buf, CacheGeometry::TriangleMesh);
}

void writeCachedConvexMesh(const QString &filePath, physx::PxDefaultMemoryOutputStream &buf)
{
    writeCachedMesh(filePath, buf, CacheGeometry::ConvexMesh);
}

void writeCachedHeightField(const QString &filePath, physx::PxDefaultMemoryOutputStream &buf)
{
    writeCachedMesh(filePath, buf, CacheGeometry::HeightField);
}

static void readCookedMesh(const QString &meshFilename, physx::PxPhysics &physics,
                           physx::PxTriangleMesh *&triangleMesh, physx::PxConvexMesh *&convexMesh,
                           physx::PxHeightField *&heightField, CacheGeometry geom)
{
    QFile file(meshFilename);
    uchar *data = nullptr;

    auto cleanup = qScopeGuard([&] {
        if (data)
            file.unmap(data);
        if (file.isOpen())
            file.close();
    });

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << meshFilename;
        return;
    }

    data = file.map(0, file.size());
    if (!data) {
        qWarning() << "Could not map" << meshFilename;
        return;
    }

    physx::PxDefaultMemoryInputData input(data, physx::PxU32(file.size()));

    switch (geom) {
    case CacheGeometry::TriangleMesh: {
        triangleMesh = physics.createTriangleMesh(input);
        break;
    }
    case CacheGeometry::ConvexMesh: {
        convexMesh = physics.createConvexMesh(input);
        break;
    }
    case CacheGeometry::HeightField:
        heightField = physics.createHeightField(input);
        break;
    }
}

physx::PxTriangleMesh *readCachedTriangleMesh(const QString &filePath, physx::PxPhysics &physics)
{
    physx::PxTriangleMesh *triangleMesh = nullptr;
    physx::PxConvexMesh *convexMesh = nullptr;
    physx::PxHeightField *heightField = nullptr;
    readCachedMesh(filePath, physics, triangleMesh, convexMesh, heightField,
                   CacheGeometry::TriangleMesh);
    return triangleMesh;
}

physx::PxConvexMesh *readCachedConvexMesh(const QString &filePath, physx::PxPhysics &physics)
{
    physx::PxTriangleMesh *triangleMesh = nullptr;
    physx::PxConvexMesh *convexMesh = nullptr;
    physx::PxHeightField *heightField = nullptr;
    readCachedMesh(filePath, physics, triangleMesh, convexMesh, heightField,
                   CacheGeometry::ConvexMesh);
    return convexMesh;
}

physx::PxHeightField *readCachedHeightField(const QString &filePath, physx::PxPhysics &physics)
{
    physx::PxTriangleMesh *triangleMesh = nullptr;
    physx::PxConvexMesh *convexMesh = nullptr;
    physx::PxHeightField *heightField = nullptr;
    readCachedMesh(filePath, physics, triangleMesh, convexMesh, heightField,
                   CacheGeometry::HeightField);
    return heightField;
}

physx::PxTriangleMesh *readCookedTriangleMesh(const QString &filePath, physx::PxPhysics &physics)
{
    physx::PxTriangleMesh *triangleMesh = nullptr;
    physx::PxConvexMesh *convexMesh = nullptr;
    physx::PxHeightField *heightField = nullptr;
    readCookedMesh(filePath, physics, triangleMesh, convexMesh, heightField,
                   CacheGeometry::TriangleMesh);
    return triangleMesh;
}

physx::PxConvexMesh *readCookedConvexMesh(const QString &filePath, physx::PxPhysics &physics)
{
    physx::PxTriangleMesh *triangleMesh = nullptr;
    physx::PxConvexMesh *convexMesh = nullptr;
    physx::PxHeightField *heightField = nullptr;
    readCookedMesh(filePath, physics, triangleMesh, convexMesh, heightField,
                   CacheGeometry::ConvexMesh);
    return convexMesh;
}

physx::PxHeightField *readCookedHeightField(const QString &filePath, physx::PxPhysics &physics)
{
    physx::PxTriangleMesh *triangleMesh = nullptr;
    physx::PxConvexMesh *convexMesh = nullptr;
    physx::PxHeightField *heightField = nullptr;
    readCookedMesh(filePath, physics, triangleMesh, convexMesh, heightField,
                   CacheGeometry::HeightField);
    return heightField;
}

}
QT_END_NAMESPACE
