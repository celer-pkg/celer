// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGMESHBVH_H
#define QSSGMESHBVH_H

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

#include <QtQuick3DUtils/private/qtquick3dutilsglobal_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>

#include <QtGui/QVector2D>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QSSGMeshBVH;

class Q_QUICK3DUTILS_EXPORT QSSGMeshBVHNode
{
public:
    class Handle
    {
    public:
        Handle() = default;
        bool isNull() const { return !m_owner || m_idx < size_t(FallbackIndex::Count); }

        inline explicit operator const QSSGMeshBVHNode *() const;
        inline QSSGMeshBVHNode *operator->() const;
    private:
        friend class QSSGMeshBVH;
        Handle(QSSGMeshBVH *owner, size_t idx)
            : m_owner(owner)
            , m_idx(idx)
        {}
        QSSGMeshBVH *m_owner = nullptr;
        size_t m_idx = 0;
    };

    // Internal
    Handle left;
    Handle right;
    QSSGBounds3 boundingData;
    //splitAxis

    // Leaf
    int offset = 0;
    int count = 0;

private:
    friend class QSSGMeshBVH;
    friend class QSSGMeshBVHBuilder;

    enum class FallbackIndex : quint8
    {
        InvalidRead  = 0,
        InvalidWrite = 1,
        Count
    };
};

struct Q_QUICK3DUTILS_EXPORT QSSGMeshBVHTriangle
{
    QSSGBounds3 bounds;
    QVector3D vertex1;
    QVector3D vertex2;
    QVector3D vertex3;
    QVector2D uvCoord1;
    QVector2D uvCoord2;
    QVector2D uvCoord3;
};

using QSSGMeshBVHTriangles = std::vector<QSSGMeshBVHTriangle>;
using QSSGMeshBVHRoots = std::vector<QSSGMeshBVHNode::Handle>;
using QSSGMeshBVHNodes = std::vector<QSSGMeshBVHNode>;

class Q_QUICK3DUTILS_EXPORT QSSGMeshBVH
{
public:
    QSSGMeshBVH() = default;
    ~QSSGMeshBVH();

    [[nodiscard]] QSSGMeshBVHNode::Handle newHandle()
    {
        m_nodes.emplace_back();
        return { this, m_nodes.size() - 1 };
    }

    [[nodiscard]] const QSSGMeshBVHTriangles &triangles() const { return m_triangles; }
    [[nodiscard]] const QSSGMeshBVHRoots &roots() const { return m_roots; }
    [[nodiscard]] const QSSGMeshBVHNodes &nodes() const { return m_nodes; }

private:
    friend class QSSGMeshBVHNode::Handle;
    friend class QSSGMeshBVHBuilder;
    using FallbackIndex = QSSGMeshBVHNode::FallbackIndex;
    size_t getNodeIndex(size_t idx, FallbackIndex op) const
    {
        const bool valid = (idx >= size_t(FallbackIndex::Count) && idx < m_nodes.size());
        return  (valid * idx) + (!valid * size_t(op));
    }

    QSSGMeshBVHNode &mutableValue(qsizetype idx)
    {
        return m_nodes[getNodeIndex(idx, FallbackIndex::InvalidWrite)];
    }

    const QSSGMeshBVHNode &value(qsizetype idx) const
    {
        return m_nodes[getNodeIndex(idx, FallbackIndex::InvalidRead)];
    }

    QSSGMeshBVHRoots m_roots;
    QSSGMeshBVHNodes m_nodes { { /* 0 - reserved for invalid reads */ }, { /* 1 - reserved for invalid writes */ } };
    QSSGMeshBVHTriangles m_triangles;
};

QSSGMeshBVHNode::Handle::operator const QSSGMeshBVHNode *() const
{
    return &m_owner->value(m_idx);
}

QSSGMeshBVHNode *QSSGMeshBVHNode::Handle::operator->() const
{
    return &m_owner->mutableValue(m_idx);
}

QT_END_NAMESPACE

#endif // QSSGMESHBVH_H
