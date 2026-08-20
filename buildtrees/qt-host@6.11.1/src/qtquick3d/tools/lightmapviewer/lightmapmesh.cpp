// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lightmapmesh.h"

#include <QBuffer>

#include <private/qssgmesh_p.h>
#include <private/qssglightmapio_p.h>

LightmapMesh::LightmapMesh(QQuick3DObject *parent) : QQuick3DGeometry(parent) { }
LightmapMesh::~LightmapMesh() = default;

void LightmapMesh::updateData()
{
    if (!m_source.isValid() || m_key.isEmpty())
        return;

    if (m_currentlyLoadedSource == m_source && m_currentlyLoadedKey == m_key)
        return;

    auto loader = QSSGLightmapLoader::open(m_source.toLocalFile());
    if (!loader) {
        qWarning("Failed to open lightmap source file");
        return;
    }

    const auto keys = loader->getKeys();
    const bool isMeshKey = keys.contains(std::make_pair(m_key, QSSGLightmapIODataTag::Mesh));

    if (!isMeshKey) {
        return;
    }

    QByteArray meshData = loader->readData(m_key, QSSGLightmapIODataTag::Mesh);
    if (meshData.isEmpty())
        return;

    QBuffer buffer(&meshData);
    buffer.open(QIODevice::ReadOnly);
    const QSSGMesh::Mesh mesh = QSSGMesh::Mesh::loadMesh(&buffer, 1);
    if (!mesh.isValid()) {
        qWarning("Mesh load failed");
        return;
    }
    if (!mesh.hasLightmapUVChannel()) {
        qWarning("Mesh does not have lightmap UVs");
        return;
    }

    clear();

    const auto &vb = mesh.vertexBuffer();
    const auto &ib = mesh.indexBuffer();

    setVertexData(vb.data);
    setIndexData(ib.data);
    setStride(vb.stride);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    auto findEntry = [&](const QByteArray &name) -> const QSSGMesh::Mesh::VertexBufferEntry * {
        for (const auto &e : vb.entries)
            if (e.name == name)
                return &e;
        return nullptr;
    };

    const QByteArray posName = QSSGMesh::MeshInternal::getPositionAttrName();
    const QByteArray lmuvName = QSSGMesh::MeshInternal::getLightmapUVAttrName();

    const auto *pos = findEntry(posName);
    const auto *lmuv = findEntry(lmuvName);

    if (!pos) {
        qWarning("Position attribute not found");
        return;
    }
    if (!lmuv) {
        qWarning("Lightmap UV attribute not found");
        return;
    }

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, pos->offset, Attribute::ComponentType::F32Type);

    addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic, lmuv->offset, Attribute::ComponentType::F32Type);

    Attribute::ComponentType ibCompType;
    switch (ib.componentType) {
    case QSSGRenderComponentType::UnsignedInt16:
        ibCompType = Attribute::ComponentType::U16Type;
        break;
    case QSSGRenderComponentType::UnsignedInt32:
        ibCompType = Attribute::ComponentType::U32Type;
        break;
    case QSSGRenderComponentType::Int32:
        ibCompType = Attribute::ComponentType::I32Type;
        break;
    case QSSGRenderComponentType::Float32:
        ibCompType = Attribute::ComponentType::F32Type;
        break;
    default:
        ibCompType = Attribute::ComponentType::U32Type;
    }

    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0, ibCompType);

    m_bounds.bounds.minimum = QVector3D(std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max());
    m_bounds.bounds.maximum = QVector3D(-std::numeric_limits<float>::max(),
                                        -std::numeric_limits<float>::max(),
                                        -std::numeric_limits<float>::max());

    for (const auto &subset : mesh.subsets()) {
        m_bounds.bounds.minimum = QSSGUtils::vec3::minimum(m_bounds.bounds.minimum, subset.bounds.min);
        m_bounds.bounds.maximum = QSSGUtils::vec3::maximum(m_bounds.bounds.maximum, subset.bounds.max);
    }

    setBounds(m_bounds.bounds.minimum, m_bounds.bounds.maximum);

    m_currentlyLoadedSource = m_source;
    m_currentlyLoadedKey = m_key;

    update();
    emit boundsChanged();
}

QUrl LightmapMesh::source() const
{
    return m_source;
}

void LightmapMesh::setSource(const QUrl &newSource)
{
    if (m_source == newSource)
        return;

    m_source = newSource;
    updateData();
    emit sourceChanged();
}

QString LightmapMesh::key() const
{
    return m_key;
}

void LightmapMesh::setKey(const QString &newKey)
{
    if (m_key == newKey)
        return;

    m_key = newKey;
    updateData();
    emit keyChanged();
}

const QQuick3DBounds3 &LightmapMesh::bounds() const
{
    return m_bounds;
}
