// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "skingeometry.h"

#include <qmath.h>

struct Vertex {
    QVector3D position;
    qint32 joints[4];
    float weights[4];

    float pad;
};

static const int s_vertexSize = sizeof(Vertex);
Q_STATIC_ASSERT(s_vertexSize == 48);

SkinGeometry::SkinGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::U32Type);
    addAttribute(QQuick3DGeometry::Attribute::JointSemantic, offsetof(Vertex, joints[0]),
                 QQuick3DGeometry::Attribute::ComponentType::I32Type);
    addAttribute(QQuick3DGeometry::Attribute::WeightSemantic, offsetof(Vertex, weights[0]),
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
}

QList<QVector3D> SkinGeometry::positions() const
{
    return m_positions;
}

QList<qint32> SkinGeometry::joints() const
{
    return m_joints;
}

QList<float> SkinGeometry::weights() const
{
    return m_weights;
}

QList<quint32> SkinGeometry::indexes() const
{
    return m_indexes;
}

void SkinGeometry::setPositions(const QList<QVector3D> &positions)
{
    if (positions == m_positions)
        return;
    m_positions = positions;
    emit positionsChanged();
    m_vertexDirty = true;
}

void SkinGeometry::setJoints(const QList<qint32> &joints)
{
    if (joints == m_joints)
        return;
    m_joints = joints;
    emit jointsChanged();
    m_vertexDirty = true;
}

void SkinGeometry::setWeights(const QList<float> &weights)
{
    if (weights == m_weights)
        return;
    m_weights = weights;
    emit weightsChanged();
    m_vertexDirty = true;
}

void SkinGeometry::setIndexes(const QList<quint32> &indexes)
{
    if (indexes == m_indexes)
        return;
    m_indexes = indexes;
    emit indexesChanged();
    m_indexDirty = true;
}

QSSGRenderGraphObject *SkinGeometry::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (m_vertexDirty) {
        m_vertexDirty = false;

        constexpr float maxFloat = std::numeric_limits<float>::max();
        auto boundsMin = QVector3D(maxFloat, maxFloat, maxFloat);
        auto boundsMax = QVector3D(-maxFloat, -maxFloat, -maxFloat);

        const int numVertexes = m_positions.size();
        m_vertexBuffer.resize(numVertexes * s_vertexSize);
        Vertex *vert = reinterpret_cast<Vertex *>(m_vertexBuffer.data());

        for (int i = 0; i < numVertexes; ++i) {
            Vertex &v = vert[i];
            v.position = m_positions[i];
            if (m_joints.size() >= 4 * (i + 1))
                memcpy(v.joints, m_joints.constData() + 4 * i, 4 * sizeof(qint32));
            else
                v.joints[0] = v.joints[1] = v.joints[2] = v.joints[3] = 0;
            if (m_weights.size() >= 4 * (i + 1))
                memcpy(v.weights, m_weights.constData() + 4 * i, 4 * sizeof(float));
            else
                v.weights[0] = v.weights[1] = v.weights[2] = v.weights[3] = 0.0f;

            boundsMin.setX(std::min(boundsMin.x(), v.position.x()));
            boundsMin.setY(std::min(boundsMin.y(), v.position.y()));
            boundsMin.setZ(std::min(boundsMin.z(), v.position.z()));

            boundsMax.setX(std::max(boundsMax.x(), v.position.x()));
            boundsMax.setY(std::max(boundsMax.y(), v.position.y()));
            boundsMax.setZ(std::max(boundsMax.z(), v.position.z()));
        }

        setStride(s_vertexSize);
        setVertexData(m_vertexBuffer);
        setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
        setBounds(boundsMin, boundsMax);
    }

    if (m_indexDirty) {
        m_indexDirty = false;
        m_indexBuffer = QByteArray(reinterpret_cast<char *>(m_indexes.data()), m_indexes.size() * sizeof(quint32));
        setIndexData(m_indexBuffer);
    }

    node = QQuick3DGeometry::updateSpatialNode(node);
    return node;
}

