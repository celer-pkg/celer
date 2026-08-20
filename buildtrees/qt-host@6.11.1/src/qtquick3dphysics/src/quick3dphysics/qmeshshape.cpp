// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcacheutils_p.h"
#include "qmeshshape_p.h"

#include <QtQuick3D/QQuick3DGeometry>
#include <extensions/PxExtensionsAPI.h>

#include "foundation/PxVec3.h"
#include "cooking/PxConvexMeshDesc.h"
#include "extensions/PxDefaultStreams.h"

#include <QtQml/qqml.h>
#include <QtQml/qqmlcontext.h>

#include <QtQuick3DUtils/private/qssgmesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3D/QQuick3DGeometry>

#include "qmeshshape_p.h"
#include "qphysicsworld_p.h"
#include "qphysicsmeshutils_p_p.h"

QT_BEGIN_NAMESPACE

static QQuick3DGeometry::Attribute
attributeBySemantic(const QQuick3DGeometry *geometry,
                    QQuick3DGeometry::Attribute::Semantic semantic)
{
    for (int i = 0; i < geometry->attributeCount(); i++) {
        const auto attr = geometry->attribute(i);
        if (attr.semantic == semantic)
            return attr;
    }

    Q_UNREACHABLE();
    return QQuick3DGeometry::Attribute();
};

physx::PxConvexMesh *QQuick3DPhysicsMesh::convexMesh()
{
    if (m_convexMesh != nullptr)
        return m_convexMesh;

    physx::PxPhysics *thePhysics = QPhysicsWorld::getPhysics();
    if (thePhysics == nullptr)
        return nullptr;

    if (m_meshGeometry)
        return convexMeshGeometrySource();
    if (!m_meshPath.isEmpty())
        return convexMeshQmlSource();
    return nullptr;
}

physx::PxTriangleMesh *QQuick3DPhysicsMesh::triangleMesh()
{
    if (m_triangleMesh != nullptr)
        return m_triangleMesh;

    physx::PxPhysics *thePhysics = QPhysicsWorld::getPhysics();
    if (thePhysics == nullptr)
        return nullptr;

    if (m_meshGeometry)
        return triangleMeshGeometrySource();
    if (!m_meshPath.isEmpty())
        return triangleMeshQmlSource();
    return nullptr;
}

physx::PxConvexMesh *QQuick3DPhysicsMesh::convexMeshQmlSource()
{
    physx::PxPhysics *thePhysics = QPhysicsWorld::getPhysics();

    m_convexMesh = QCacheUtils::readCachedConvexMesh(m_meshPath, *thePhysics);
    if (m_convexMesh != nullptr)
        return m_convexMesh;

    m_convexMesh = QCacheUtils::readCookedConvexMesh(m_meshPath, *thePhysics);
    if (m_convexMesh != nullptr)
        return m_convexMesh;

    loadSsgMesh();

    if (!m_ssgMesh.isValid())
        return nullptr;

    const int vStride = m_ssgMesh.vertexBuffer().stride;
    const int vCount = m_ssgMesh.vertexBuffer().data.size() / vStride;

    qCDebug(lcQuick3dPhysics) << "prepare cooking" << vCount << "verts";

    physx::PxConvexMeshDesc convexDesc;
    convexDesc.points.count = vCount;
    convexDesc.points.stride = vStride;
    convexDesc.points.data = m_ssgMesh.vertexBuffer().data.constData() + m_posOffset;
    convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

    // NOTE: Since we are making a mesh for the convex hull and are only
    // interested in the positions we can Skip the index array.

    physx::PxDefaultMemoryOutputStream buf;
    physx::PxConvexMeshCookingResult::Enum result;
    const auto cooking = QPhysicsWorld::getCooking();
    if (cooking && cooking->cookConvexMesh(convexDesc, buf, &result)) {
        auto size = buf.getSize();
        auto *data = buf.getData();
        physx::PxDefaultMemoryInputData input(data, size);
        m_convexMesh = thePhysics->createConvexMesh(input);
        qCDebug(lcQuick3dPhysics) << "Created convex mesh" << m_convexMesh << "for mesh" << this;
        QCacheUtils::writeCachedConvexMesh(m_meshPath, buf);
    } else {
        qCWarning(lcQuick3dPhysics) << "Could not create convex mesh from" << m_meshPath;
    }

    return m_convexMesh;
}

physx::PxConvexMesh *QQuick3DPhysicsMesh::convexMeshGeometrySource()
{
    auto vertexBuffer = m_meshGeometry->vertexData();

    if (m_meshGeometry->primitiveType() != QQuick3DGeometry::PrimitiveType::Triangles) {
        qWarning() << "QQuick3DPhysicsMesh: Invalid geometry primitive type, must be Triangles. ";
        return nullptr;
    }

    if (!vertexBuffer.size()) {
        qWarning() << "QQuick3DPhysicsMesh: Invalid geometry, vertexData is empty. ";
        return nullptr;
    }

    const auto vertexAttribute =
            attributeBySemantic(m_meshGeometry, QQuick3DGeometry::Attribute::PositionSemantic);
    Q_ASSERT(vertexAttribute.componentType == QQuick3DGeometry::Attribute::F32Type);

    const auto stride = m_meshGeometry->stride();
    const auto numVertices = vertexBuffer.size() / stride;

    physx::PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numVertices;
    convexDesc.points.stride = stride;
    convexDesc.points.data = vertexBuffer.constData() + vertexAttribute.offset;
    convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

    // NOTE: Since we are making a mesh for the convex hull and are only
    // interested in the positions we can Skip the index array.

    const auto cooking = QPhysicsWorld::getCooking();
    physx::PxDefaultMemoryOutputStream buf;
    physx::PxConvexMeshCookingResult::Enum result;
    if (cooking && cooking->cookConvexMesh(convexDesc, buf, &result)) {
        auto size = buf.getSize();
        auto *data = buf.getData();
        physx::PxDefaultMemoryInputData input(data, size);
        m_convexMesh = QPhysicsWorld::getPhysics()->createConvexMesh(input);
        qCDebug(lcQuick3dPhysics) << "Created convex mesh" << m_convexMesh << "for mesh" << this;
    } else {
        qCWarning(lcQuick3dPhysics) << "Could not create convex mesh for" << this;
    }

    return m_convexMesh;
}

physx::PxTriangleMesh *QQuick3DPhysicsMesh::triangleMeshQmlSource()
{
    physx::PxPhysics *thePhysics = QPhysicsWorld::getPhysics();

    m_triangleMesh = QCacheUtils::readCachedTriangleMesh(m_meshPath, *thePhysics);
    if (m_triangleMesh != nullptr)
        return m_triangleMesh;

    m_triangleMesh = QCacheUtils::readCookedTriangleMesh(m_meshPath, *thePhysics);
    if (m_triangleMesh != nullptr)
        return m_triangleMesh;

    loadSsgMesh();
    if (!m_ssgMesh.isValid())
        return nullptr;

    auto vertexBuffer = m_ssgMesh.vertexBuffer().data;

    const int posOffset = m_posOffset;
    const auto stride =  m_ssgMesh.vertexBuffer().stride;
    const auto numVertices = vertexBuffer.size() / stride;

    physx::PxTriangleMeshDesc triangleDesc;
    triangleDesc.points.count = numVertices;
    triangleDesc.points.stride = stride;
    triangleDesc.points.data = vertexBuffer.constData() + posOffset;

    auto indexBuffer = m_ssgMesh.indexBuffer().data;
    if (indexBuffer.size()) {
        const bool u16IndexType =
                m_ssgMesh.indexBuffer().componentType == QSSGMesh::Mesh::ComponentType::UnsignedInt16;

        Q_ASSERT(m_ssgMesh.indexBuffer().componentType
                         == QSSGMesh::Mesh::ComponentType::UnsignedInt16
                 || m_ssgMesh.indexBuffer().componentType
                         == QSSGMesh::Mesh::ComponentType::UnsignedInt32);

        triangleDesc.triangles.data = indexBuffer.constData();
        if (u16IndexType) {
            triangleDesc.flags.set(physx::PxMeshFlag::e16_BIT_INDICES);
            triangleDesc.triangles.stride = sizeof(quint16) * 3;
        } else {
            triangleDesc.triangles.stride = sizeof(quint32) * 3;
        }
        triangleDesc.triangles.count = indexBuffer.size() / triangleDesc.triangles.stride;
    }

    physx::PxDefaultMemoryOutputStream buf;
    physx::PxTriangleMeshCookingResult::Enum result;
    const auto cooking = QPhysicsWorld::getCooking();
    if (cooking && cooking->cookTriangleMesh(triangleDesc, buf, &result)) {
        auto size = buf.getSize();
        auto *data = buf.getData();
        physx::PxDefaultMemoryInputData input(data, size);
        m_triangleMesh = thePhysics->createTriangleMesh(input);
        qCDebug(lcQuick3dPhysics) << "Created triangle mesh" << m_triangleMesh << "for mesh"
                                  << this;
        QCacheUtils::writeCachedTriangleMesh(m_meshPath, buf);
    } else {
        qCWarning(lcQuick3dPhysics) << "Could not create triangle mesh from" << m_meshPath;
    }

    return m_triangleMesh;
}

physx::PxTriangleMesh *QQuick3DPhysicsMesh::triangleMeshGeometrySource()
{
    auto vertexBuffer = m_meshGeometry->vertexData();

    if (m_meshGeometry->primitiveType() != QQuick3DGeometry::PrimitiveType::Triangles) {
        qWarning() << "QQuick3DPhysicsMesh: Invalid geometry primitive type, must be Triangles. ";
        return nullptr;
    }

    if (!vertexBuffer.size()) {
        qWarning() << "QQuick3DPhysicsMesh: Invalid geometry, vertexData is empty. ";
        return nullptr;
    }

    const auto vertexAttribute =
            attributeBySemantic(m_meshGeometry, QQuick3DGeometry::Attribute::PositionSemantic);
    Q_ASSERT(vertexAttribute.componentType == QQuick3DGeometry::Attribute::F32Type);

    const int posOffset = vertexAttribute.offset;
    const auto stride = m_meshGeometry->stride();
    const auto numVertices = vertexBuffer.size() / stride;

    physx::PxTriangleMeshDesc triangleDesc;
    triangleDesc.points.count = numVertices;
    triangleDesc.points.stride = stride;
    triangleDesc.points.data = vertexBuffer.constData() + posOffset;

    auto indexBuffer = m_meshGeometry->indexData();
    if (indexBuffer.size()) {
        const auto indexAttribute =
                attributeBySemantic(m_meshGeometry, QQuick3DGeometry::Attribute::IndexSemantic);
        const bool u16IndexType =
                indexAttribute.componentType == QQuick3DGeometry::Attribute::U16Type;

        Q_ASSERT(indexAttribute.componentType == QQuick3DGeometry::Attribute::U16Type
                 || indexAttribute.componentType == QQuick3DGeometry::Attribute::U32Type);

        triangleDesc.triangles.data = indexBuffer.constData();
        if (u16IndexType) {
            triangleDesc.flags.set(physx::PxMeshFlag::e16_BIT_INDICES);
            triangleDesc.triangles.stride = sizeof(quint16) * 3;
        } else {
            triangleDesc.triangles.stride = sizeof(quint32) * 3;
        }
        triangleDesc.triangles.count = indexBuffer.size() / triangleDesc.triangles.stride;
    }

    physx::PxDefaultMemoryOutputStream buf;
    physx::PxTriangleMeshCookingResult::Enum result;
    const auto cooking = QPhysicsWorld::getCooking();
    if (cooking && cooking->cookTriangleMesh(triangleDesc, buf, &result)) {
        auto size = buf.getSize();
        auto *data = buf.getData();
        physx::PxDefaultMemoryInputData input(data, size);
        m_triangleMesh = QPhysicsWorld::getPhysics()->createTriangleMesh(input);
        qCDebug(lcQuick3dPhysics) << "Created triangle mesh" << m_triangleMesh << "for mesh"
                                  << this;
    } else {
        qCWarning(lcQuick3dPhysics) << "Could not create triangle mesh for" << this;
    }

    return m_triangleMesh;
}

void QQuick3DPhysicsMesh::loadSsgMesh()
{
    if (m_ssgMesh.isValid())
        return;

    // A bit ugly to use QSSGRenderPath here but it is just a wrapper for
    // a QString and its hash.
    // Security note: m_meshPath is a user provided file but QSSGBufferManager::loadMeshData is
    // assumed to handle invalid meshes
    m_ssgMesh = QSSGBufferManager::loadMeshData(QSSGRenderPath(m_meshPath));

    static const char *compTypes[] = { "Null",  "UnsignedInt8",  "Int8",    "UnsignedInt16",
                                       "Int16", "UnsignedInt32", "Int32",   "UnsignedInt64",
                                       "Int64", "Float16",       "Float32", "Float64" };

    qCDebug(lcQuick3dPhysics) << "Loaded SSG mesh from" << m_meshPath << m_ssgMesh.isValid()
                              << "draw" << int(m_ssgMesh.drawMode()) << "wind"
                              << int(m_ssgMesh.winding()) << "subs" << m_ssgMesh.subsets().count()
                              << "attrs" << m_ssgMesh.vertexBuffer().entries.count()
                              << m_ssgMesh.vertexBuffer().data.size() << "stride"
                              << m_ssgMesh.vertexBuffer().stride << "verts"
                              << m_ssgMesh.vertexBuffer().data.size()
                    / m_ssgMesh.vertexBuffer().stride;

    for (auto &v : m_ssgMesh.vertexBuffer().entries) {
        qCDebug(lcQuick3dPhysics) << "  attr" << v.name << compTypes[int(v.componentType)] << "cc"
                                  << v.componentCount << "offs" << v.offset;
        Q_ASSERT(v.componentType == QSSGMesh::Mesh::ComponentType::Float32);
        if (v.name == "attr_pos")
            m_posOffset = v.offset;
    }

    if (m_ssgMesh.isValid()) {
        auto sub = m_ssgMesh.subsets().constFirst();
        qCDebug(lcQuick3dPhysics) << "..." << sub.name << "count" << sub.count << "bounds"
                                  << sub.bounds.min << sub.bounds.max << "offset" << sub.offset;
    }

#if 0 // EXTRA_DEBUG

    int iStride = m_ssgMesh.indexBuffer().componentType == QSSGMesh::Mesh::ComponentType::UnsignedInt16 ? 2 : 4;
    int vStride = m_ssgMesh.vertexBuffer().stride;
    qDebug() << "IDX" << compTypes[int(m_ssgMesh.indexBuffer().componentType)] << m_ssgMesh.indexBuffer().data.size() / iStride;
    const auto ib = m_ssgMesh.indexBuffer().data;
    const auto vb = m_ssgMesh.vertexBuffer().data;

    auto getPoint = [&vb, vStride, this](int idx) -> QVector3D {
        auto *vp = vb.constData() + vStride * idx + m_posOffset;
        return *reinterpret_cast<const QVector3D *>(vp);
        return {};
    };

    if (iStride == 2) {

    } else {
        auto *ip = reinterpret_cast<const uint32_t *>(ib.data());
        int n = ib.size() / iStride;
        for (int i = 0; i < qMin(50,n); i += 3) {

            qDebug() << "    " << ip [i] << ip[i+1] << ip[i+2] << " --- "
                     << getPoint(ip[i]) << getPoint(ip[i+1]) << getPoint(ip[i+2]);
        }
    }
#endif
    if (!m_ssgMesh.isValid())
        qCWarning(lcQuick3dPhysics) << "Could not read mesh from" << m_meshPath;
}

QQuick3DPhysicsMesh *QQuick3DPhysicsMeshManager::getMesh(const QUrl &source, QObject *contextObject)
{
    const QString qmlSource = QQuick3DModel::translateMeshSource(source, contextObject);
    auto *mesh = sourceMeshHash.value(qmlSource);
    if (!mesh) {
        mesh = new QQuick3DPhysicsMesh(qmlSource);
        sourceMeshHash[qmlSource] = mesh;
    }
    mesh->ref();
    return mesh;
}

QQuick3DPhysicsMesh *QQuick3DPhysicsMeshManager::getMesh(QQuick3DGeometry *source)
{
    auto *mesh = geometryMeshHash.value(source);
    if (!mesh) {
        mesh = new QQuick3DPhysicsMesh(source);
        geometryMeshHash.insert(source, mesh);
    }
    mesh->ref();
    return mesh;
}

void QQuick3DPhysicsMeshManager::releaseMesh(QQuick3DPhysicsMesh *mesh)
{
    if (mesh == nullptr || mesh->deref() > 0)
        return;

    qCDebug(lcQuick3dPhysics()) << "deleting mesh" << mesh;
    erase_if(sourceMeshHash, [mesh](std::pair<const QString &, QQuick3DPhysicsMesh *&> h) {
        return h.second == mesh;
    });
    erase_if(geometryMeshHash, [mesh](std::pair<QQuick3DGeometry *, QQuick3DPhysicsMesh *&> h) {
        return h.second == mesh;
    });
    delete mesh;
}

QHash<QString, QQuick3DPhysicsMesh *> QQuick3DPhysicsMeshManager::sourceMeshHash;
QHash<QQuick3DGeometry *, QQuick3DPhysicsMesh *> QQuick3DPhysicsMeshManager::geometryMeshHash;

/////////////////////////////////////////////////////////////////////////////

QMeshShape::~QMeshShape()
{
    delete m_convexGeometry;
    if (m_mesh)
        QQuick3DPhysicsMeshManager::releaseMesh(m_mesh);
}

physx::PxGeometry *QMeshShape::getPhysXGeometry()
{
    if (m_dirtyPhysx || m_scaleDirty)
        updatePhysXGeometry();
    if (shapeType() == MeshType::CONVEX)
        return m_convexGeometry;
    if (shapeType() == MeshType::TRIANGLE)
        return m_triangleGeometry;

    Q_UNREACHABLE_RETURN(nullptr);
}

void QMeshShape::updatePhysXGeometry()
{
    delete m_convexGeometry;
    delete m_triangleGeometry;
    m_convexGeometry = nullptr;
    m_triangleGeometry = nullptr;

    if (!m_mesh)
        return;

    auto *convexMesh = shapeType() == MeshType::CONVEX ? m_mesh->convexMesh() : nullptr;
    auto *triangleMesh = shapeType() == MeshType::TRIANGLE ? m_mesh->triangleMesh() : nullptr;
    if (!convexMesh && !triangleMesh)
        return;

    auto meshScale = sceneScale();
    physx::PxMeshScale scale(physx::PxVec3(meshScale.x(), meshScale.y(), meshScale.z()),
                             physx::PxQuat(physx::PxIdentity));

    if (convexMesh)
        m_convexGeometry = new physx::PxConvexMeshGeometry(convexMesh, scale);
    if (triangleMesh)
        m_triangleGeometry = new physx::PxTriangleMeshGeometry(triangleMesh, scale);

    m_dirtyPhysx = false;
}

const QUrl &QMeshShape::source() const
{
    return m_meshSource;
}

void QMeshShape::setSource(const QUrl &newSource)
{
    if (m_meshSource == newSource)
        return;
    m_meshSource = newSource;

    // If we get a new source and our mesh was from the old source
    // (meaning it was NOT from a geometry) we deref
    if (m_geometry == nullptr) {
        QQuick3DPhysicsMeshManager::releaseMesh(m_mesh);
        m_mesh = nullptr;
    }

    // Load new mesh only if we don't have a geometry as source
    if (m_geometry == nullptr && !newSource.isEmpty())
        m_mesh = QQuick3DPhysicsMeshManager::getMesh(m_meshSource, this);

    updatePhysXGeometry();
    m_dirtyPhysx = true;

    emit needsRebuild(this);
    emit sourceChanged();
}

QQuick3DGeometry *QMeshShape::geometry() const
{
    return m_geometry;
}

void QMeshShape::setGeometry(QQuick3DGeometry *newGeometry)
{
    if (m_geometry == newGeometry)
        return;
    if (m_geometry)
        m_geometry->disconnect(this);

    m_geometry = newGeometry;

    if (m_geometry != nullptr) {
        connect(m_geometry, &QObject::destroyed, this, &QMeshShape::geometryDestroyed);
        connect(m_geometry, &QQuick3DGeometry::geometryChanged, this,
                &QMeshShape::geometryContentChanged);
    }

    // New geometry means we get a new mesh so deref the old one
    QQuick3DPhysicsMeshManager::releaseMesh(m_mesh);
    m_mesh = nullptr;
    if (m_geometry != nullptr)
        m_mesh = QQuick3DPhysicsMeshManager::getMesh(m_geometry);
    else if (!m_meshSource.isEmpty())
        m_mesh = QQuick3DPhysicsMeshManager::getMesh(m_meshSource, this);

    updatePhysXGeometry();
    m_dirtyPhysx = true;
    emit needsRebuild(this);
    emit geometryChanged();
}

void QMeshShape::geometryDestroyed(QObject *geometry)
{
    Q_ASSERT(m_geometry == geometry);
    // Set geometry to null and the old one will be disconnected and dereferenced
    setGeometry(nullptr);
}

void QMeshShape::geometryContentChanged()
{
    Q_ASSERT(m_geometry != nullptr);
    QQuick3DPhysicsMeshManager::releaseMesh(m_mesh);
    m_mesh = QQuick3DPhysicsMeshManager::getMesh(m_geometry);

    updatePhysXGeometry();
    m_dirtyPhysx = true;
    emit needsRebuild(this);
}

QT_END_NAMESPACE
