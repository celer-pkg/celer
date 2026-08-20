// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendergeometry_p.h"
#include "qssgrendermesh_p.h"
#include "resourcemanager/qssgrenderbuffermanager_p.h"

QSSGRenderGeometry::QSSGRenderGeometry()
    : QSSGRenderGraphObject(QSSGRenderGraphObject::Type::Geometry, FlagT(Flags::HasGraphicsResources))
{
}

QSSGRenderGeometry::~QSSGRenderGeometry()
{
}

const QByteArray &QSSGRenderGeometry::vertexBuffer() const
{
    return m_meshData.m_vertexBuffer;
}

QByteArray &QSSGRenderGeometry::vertexBuffer()
{
    return m_meshData.m_vertexBuffer;
}

const QByteArray &QSSGRenderGeometry::indexBuffer() const
{
    return m_meshData.m_indexBuffer;
}

QByteArray &QSSGRenderGeometry::indexBuffer()
{
    return m_meshData.m_indexBuffer;
}

int QSSGRenderGeometry::attributeCount() const
{
    return m_meshData.m_attributeCount;
}

QVector3D QSSGRenderGeometry::boundsMin() const
{
    return m_bounds.minimum;
}

QVector3D QSSGRenderGeometry::boundsMax() const
{
    return m_bounds.maximum;
}

int QSSGRenderGeometry::stride() const
{
    return m_meshData.m_stride;
}

QSSGMesh::Mesh::DrawMode QSSGRenderGeometry::primitiveType() const
{
    return m_meshData.m_primitiveType;
}

QSSGRenderGeometry::Attribute QSSGRenderGeometry::attribute(int idx) const
{
    Attribute attr;
    const auto &mattr = m_meshData.m_attributes[idx];
    attr.offset = mattr.offset;
    attr.semantic = mattr.semantic;
    attr.componentType = mattr.componentType;
    return attr;
}

void QSSGRenderGeometry::addAttribute(QSSGMesh::RuntimeMeshData::Attribute::Semantic semantic,
                                      int offset,
                                      QSSGMesh::Mesh::ComponentType componentType)
{
    Attribute attr;
    attr.semantic = semantic;
    attr.offset = offset;
    attr.componentType = componentType;
    addAttribute(attr);
}

void QSSGRenderGeometry::addAttribute(const Attribute &att)
{
    const int index = m_meshData.m_attributeCount;
    if (index == QSSGMesh::RuntimeMeshData::MAX_ATTRIBUTES) {
        qWarning("Maximum number (%d) of vertex attributes in custom geometry has been reached; ignoring extra attributes",
                 QSSGMesh::RuntimeMeshData::MAX_ATTRIBUTES);
        return;
    }
    m_meshData.m_attributes[index].semantic
            = static_cast<QSSGMesh::RuntimeMeshData::Attribute::Semantic>(att.semantic);
    m_meshData.m_attributes[index].offset = att.offset;
    m_meshData.m_attributes[index].componentType = att.componentType;
    ++m_meshData.m_attributeCount;
    markDirty();
}

void QSSGRenderGeometry::addTargetAttribute(quint32 targetId,
                                            QSSGMesh::RuntimeMeshData::Attribute::Semantic semantic,
                                            int offset,
                                            int stride)
{
    TargetAttribute tAttr;
    tAttr.targetId = targetId;
    tAttr.attr.semantic = semantic;
    tAttr.attr.offset = offset;
    tAttr.stride = stride;
    addTargetAttribute(tAttr);
}

void QSSGRenderGeometry::addTargetAttribute(const TargetAttribute &att)
{
    const int index = m_meshData.m_targetAttributeCount;
    if (index == QSSGMesh::RuntimeMeshData::MAX_TARGET_ATTRIBUTES) {
        qWarning("Maximum number (%d) of morph target attributes in custom geometry has been reached; ignoring extra attributes",
                 QSSGMesh::RuntimeMeshData::MAX_TARGET_ATTRIBUTES);
        return;
    }
    m_meshData.m_targetAttributes[index].attr.semantic
            = static_cast<QSSGMesh::RuntimeMeshData::Attribute::Semantic>(att.attr.semantic);
    m_meshData.m_targetAttributes[index].attr.offset = att.attr.offset;
    m_meshData.m_targetAttributes[index].targetId = att.targetId;
    m_meshData.m_targetAttributes[index].stride = att.stride;
    ++m_meshData.m_targetAttributeCount;
    markDirty();
}

void QSSGRenderGeometry::addSubset(quint32 offset, quint32 count, const QVector3D &boundsMin, const QVector3D &boundsMax, const QString &name)
{
    m_meshData.m_subsets.append({name, {boundsMin, boundsMax}, count, offset, {}, {}});
}

void QSSGRenderGeometry::setStride(int stride)
{
    m_meshData.m_stride = stride;
    markDirty();
}

void QSSGRenderGeometry::setPrimitiveType(QSSGMesh::Mesh::DrawMode type)
{
    m_meshData.m_primitiveType = type;
    markDirty();
}

void QSSGRenderGeometry::setBounds(const QVector3D &min, const QVector3D &max)
{
    m_bounds = QSSGBounds3(min, max);
    markDirty();
}

void QSSGRenderGeometry::clear()
{
    m_meshData.clearVertexAndIndex();
    m_meshData.clearTarget();
    m_bounds.setEmpty();
    markDirty();
}

void QSSGRenderGeometry::clearVertexAndIndex()
{
    m_meshData.clearVertexAndIndex();
    m_bounds.setEmpty();
    markDirty();
}


void QSSGRenderGeometry::clearTarget()
{
    m_meshData.clearTarget();
    markDirty();
}

void QSSGRenderGeometry::clearAttributes()
{
    m_meshData.m_attributeCount = 0;
}

uint32_t QSSGRenderGeometry::generationId() const
{
    return m_generationId;
}

const QSSGMesh::RuntimeMeshData &QSSGRenderGeometry::meshData() const
{
    return m_meshData;
}

void QSSGRenderGeometry::setVertexData(const QByteArray &data)
{
    m_meshData.m_vertexBuffer = data;
    markDirty();
}

void QSSGRenderGeometry::setIndexData(const QByteArray &data)
{
    m_meshData.m_indexBuffer = data;
    markDirty();
}

void QSSGRenderGeometry::setTargetData(const QByteArray &data)
{
    m_meshData.m_targetBuffer = data;
    markDirty();
}

void QSSGRenderGeometry::markDirty()
{
    m_generationId++;
}
