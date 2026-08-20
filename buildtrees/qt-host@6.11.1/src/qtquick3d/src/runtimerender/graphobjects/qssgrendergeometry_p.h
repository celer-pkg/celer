// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_GEOMETRY_H
#define QSSG_RENDER_GEOMETRY_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QtQuick3DUtils/private/qssgmesh_p.h>

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderGeometry : public QSSGRenderGraphObject
{
public:
    struct Attribute {
        QSSGMesh::RuntimeMeshData::Attribute::Semantic semantic = QSSGMesh::RuntimeMeshData::Attribute::PositionSemantic;
        int offset = -1;
        QSSGMesh::Mesh::ComponentType componentType = QSSGMesh::Mesh::ComponentType::Float32;
    };
    struct TargetAttribute {
        quint32 targetId = 0;
        Attribute attr;
        int stride = 0;
    };

    explicit QSSGRenderGeometry();
    virtual ~QSSGRenderGeometry();

    const QByteArray &vertexBuffer() const;
    QByteArray &vertexBuffer();
    const QByteArray &indexBuffer() const;
    QByteArray &indexBuffer();
    int attributeCount() const;
    Attribute attribute(int idx) const;
    QSSGMesh::Mesh::DrawMode primitiveType() const;
    QVector3D boundsMin() const;
    QVector3D boundsMax() const;
    int stride() const;
    int targetStride() const;

    void setVertexData(const QByteArray &data);
    void setIndexData(const QByteArray &data);
    void setStride(int stride);
    void setBounds(const QVector3D &min, const QVector3D &max);
    void setPrimitiveType(QSSGMesh::Mesh::DrawMode type);

    void addAttribute(QSSGMesh::RuntimeMeshData::Attribute::Semantic semantic,
                      int offset,
                      QSSGMesh::Mesh::ComponentType componentType);
    void addAttribute(const Attribute &att);
    void addSubset(quint32 offset, quint32 count, const QVector3D &boundsMin, const QVector3D &boundsMax, const QString &name = {});

    void clear();
    void clearAttributes();

    uint32_t generationId() const;
    const QSSGMesh::RuntimeMeshData &meshData() const;

    QString debugObjectName;

    void clearVertexAndIndex();
    void clearTarget();
    void setTargetData(const QByteArray &data);
    void addTargetAttribute(quint32 targetId,
                            QSSGMesh::RuntimeMeshData::Attribute::Semantic semantic,
                            int offset,
                            int stride = 0);
    void addTargetAttribute(const TargetAttribute &att);

protected:
    Q_DISABLE_COPY(QSSGRenderGeometry)

    void markDirty();

    uint32_t m_generationId = 1;
    QSSGMesh::RuntimeMeshData m_meshData;
    QSSGBounds3 m_bounds;
};

QT_END_NAMESPACE

#endif // QSSG_GEOMETRY_H
