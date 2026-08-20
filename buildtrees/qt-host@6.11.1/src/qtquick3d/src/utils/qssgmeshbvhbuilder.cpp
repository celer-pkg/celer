// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgmeshbvhbuilder_p.h"
#include <QtQuick3DUtils/private/qssgassert_p.h>

QT_BEGIN_NAMESPACE

static constexpr quint32 QSSG_MAX_TREE_DEPTH = 40;
static constexpr quint32 QSSG_MAX_LEAF_TRIANGLES = 10;

QSSGMeshBVHBuilder::QSSGMeshBVHBuilder(const QSSGMesh::Mesh &mesh)
    : m_mesh(mesh)
{
    const QSSGMesh::Mesh::VertexBuffer vb = mesh.vertexBuffer();
    const QSSGMesh::Mesh::IndexBuffer ib = mesh.indexBuffer();
    m_vertexBufferData = vb.data;
    m_indexBufferData = ib.data;
    m_indexBufferComponentType = QSSGRenderComponentType(ib.componentType);
    if (m_indexBufferComponentType == QSSGRenderComponentType::Int16)
        m_indexBufferComponentType = QSSGRenderComponentType::UnsignedInt16;
    else if (m_indexBufferComponentType == QSSGRenderComponentType::Int32)
        m_indexBufferComponentType = QSSGRenderComponentType::UnsignedInt32;

    // Get VertexBuffer Information
    // When using the texture coordinates, UV0 has priority but if the mesh has
    // UV1 without UV0, UV1 will be used instead of UV0.
    for (quint32 entryIdx = 0, entryEnd = vb.entries.size(); entryIdx < entryEnd; ++entryIdx) {
        QSSGRenderVertexBufferEntry entry = vb.entries[entryIdx].toRenderVertexBufferEntry();
        if (!strcmp(entry.m_name, QSSGMesh::MeshInternal::getPositionAttrName())) {
            m_hasPositionData = true;
            m_vertexPosOffset = entry.m_firstItemOffset;
        } else if (!strcmp(entry.m_name, QSSGMesh::MeshInternal::getUV0AttrName())) {
            m_hasUVData = true;
            m_vertexUVOffset = entry.m_firstItemOffset;
        } else if (!m_hasUVData && !strcmp(entry.m_name, QSSGMesh::MeshInternal::getUV1AttrName())) {
            m_hasUVData = true;
            m_vertexUVOffset = entry.m_firstItemOffset;
        }
    }
    m_vertexStride = vb.stride;
}

QSSGMeshBVHBuilder::QSSGMeshBVHBuilder(const QByteArray &vertexBuffer,
                                       int stride,
                                       int posOffset,
                                       bool hasUV,
                                       int uvOffset,
                                       bool hasIndexBuffer,
                                       const QByteArray &indexBuffer,
                                       QSSGRenderComponentType indexBufferType)
{
    m_vertexBufferData = vertexBuffer;
    m_vertexStride = stride;
    m_hasPositionData = true;
    m_vertexPosOffset = posOffset;
    m_hasUVData = hasUV;
    m_vertexUVOffset = uvOffset;
    m_hasIndexBuffer = hasIndexBuffer;
    m_indexBufferData = indexBuffer;
    m_indexBufferComponentType = indexBufferType;
    if (m_indexBufferComponentType == QSSGRenderComponentType::Int16)
        m_indexBufferComponentType = QSSGRenderComponentType::UnsignedInt16;
    else if (m_indexBufferComponentType == QSSGRenderComponentType::Int32)
        m_indexBufferComponentType = QSSGRenderComponentType::UnsignedInt32;
}

std::unique_ptr<QSSGMeshBVH> QSSGMeshBVHBuilder::buildTree()
{
    // This only works with triangles
    if (m_mesh.isValid() && m_mesh.drawMode() != QSSGMesh::Mesh::DrawMode::Triangles)
        return nullptr;

    auto meshBvh = std::make_unique<QSSGMeshBVH>();
    auto &roots = meshBvh->m_roots;
    auto &triangleBounds = meshBvh->m_triangles;

    // Calculate the bounds for each triangle in whole mesh once
    quint32 indexCount = 0;
    if (m_hasIndexBuffer)
        indexCount = quint32(m_indexBufferData.size() / QSSGBaseTypeHelpers::getSizeOfType(m_indexBufferComponentType));
    else
        indexCount = m_vertexBufferData.size() / m_vertexStride;
    triangleBounds = calculateTriangleBounds(0, indexCount);

    // For each submesh, generate a root bvh node
    if (m_mesh.isValid()) {
        const QVector<QSSGMesh::Mesh::Subset> subsets = m_mesh.subsets();
        roots.reserve(subsets.size());
        for (quint32 subsetIdx = 0, subsetEnd = subsets.size(); subsetIdx < subsetEnd; ++subsetIdx) {
            const QSSGMesh::Mesh::Subset &source(subsets[subsetIdx]);
            QSSGMeshBVHNode::Handle root = meshBvh->newHandle();
            // Offsets provided by subset are for the index buffer
            // Convert them to work with the triangle bounds list
            const quint32 triangleOffset = source.offset / 3;
            const quint32 triangleCount = source.count / 3;
            root->boundingData = getBounds(*meshBvh, triangleOffset, triangleCount);
            // Recursively split the mesh into a tree of smaller bounding volumns
            root = splitNode(*meshBvh, root, triangleOffset, triangleCount);
            roots.push_back(root);
        }
    } else {
        // Custom Geometry only has one subset
        QSSGMeshBVHNode::Handle root = meshBvh->newHandle();
        root->boundingData = getBounds(*meshBvh, 0, quint32(triangleBounds.size()));
        root = splitNode(*meshBvh, root, 0, quint32(triangleBounds.size()));
        roots.push_back(root);
    }

    return meshBvh;
}


template <QSSGRenderComponentType ComponentType>
static inline quint32 getIndexBufferValue(quint32 index, const quint32 indexCount, const QByteArray &indexBufferData)
{
    Q_ASSERT(index < indexCount);
    Q_STATIC_ASSERT(ComponentType == QSSGRenderComponentType::UnsignedInt16 || ComponentType == QSSGRenderComponentType::UnsignedInt32);

    quint32 result = 0;
    if constexpr (ComponentType == QSSGRenderComponentType::UnsignedInt16) {
        QSSGDataView<quint16> shortIndex(reinterpret_cast<const quint16 *>(indexBufferData.begin()), indexCount);
        result = shortIndex[index];
    } else if (ComponentType == QSSGRenderComponentType::UnsignedInt32) {
        QSSGDataView<quint32> longIndex(reinterpret_cast<const quint32 *>(indexBufferData.begin()), indexCount);
        result = longIndex[index];
    }

    return result;
}

static inline QVector3D getVertexBufferValuePosition(quint32 index, const quint32 vertexStride, const quint32 vertexPosOffset, const QByteArray &vertexBufferData)
{
    const quint32 offset = index * vertexStride + vertexPosOffset;
    Q_ASSERT(qsizetype(offset) < vertexBufferData.size());
    const QVector3D *position = reinterpret_cast<const QVector3D *>(vertexBufferData.begin() + offset);

    return *position;
}

static inline QVector2D getVertexBufferValueUV(quint32 index, const quint32 vertexStride, const quint32 vertexUVOffset, const QByteArray &vertexBufferData)
{
    const quint32 offset = index * vertexStride + vertexUVOffset;
    Q_ASSERT(qsizetype(offset) < vertexBufferData.size());
    const QVector2D *uv = reinterpret_cast<const QVector2D *>(vertexBufferData.begin() + offset);

    return *uv;
}

template <QSSGRenderComponentType ComponentType, bool hasIndexBuffer, bool hasPositionData, bool hasUVData>
static void calculateTriangleBoundsImpl(quint32 indexOffset,
                                        quint32 indexCount,
                                        const QByteArray &indexBufferData,
                                        const QByteArray &vertexBufferData,
                                        [[maybe_unused]] const quint32 vertexStride,
                                        [[maybe_unused]] const quint32 vertexUVOffset,
                                        [[maybe_unused]] const quint32 vertexPosOffset,
                                        QSSGMeshBVHTriangles &triangleBounds)
{
    const quint32 triangleCount = indexCount / 3;
    triangleBounds.reserve(triangleCount);

    for (quint32 i = 0; i < triangleCount; ++i) {
        QSSGMeshBVHTriangle triangle{};
        if constexpr (hasIndexBuffer || hasPositionData || hasUVData) {
            // Get the indices for the triangle
            const quint32 triangleIndex = i * 3 + indexOffset;

            quint32 index1 = triangleIndex + 0;
            quint32 index2 = triangleIndex + 1;
            quint32 index3 = triangleIndex + 2;

            if constexpr (hasIndexBuffer) {
                index1 = getIndexBufferValue<ComponentType>(index1, indexCount, indexBufferData);
                index2 = getIndexBufferValue<ComponentType>(index2, indexCount, indexBufferData);
                index3 = getIndexBufferValue<ComponentType>(index3, indexCount, indexBufferData);
            }

            if constexpr (hasPositionData) {
                triangle.vertex1 = getVertexBufferValuePosition(index1, vertexStride, vertexPosOffset, vertexBufferData);
                triangle.vertex2 = getVertexBufferValuePosition(index2, vertexStride, vertexPosOffset, vertexBufferData);
                triangle.vertex3 = getVertexBufferValuePosition(index3, vertexStride, vertexPosOffset, vertexBufferData);
            }

            if constexpr (hasUVData) {
                triangle.uvCoord1 = getVertexBufferValueUV(index1, vertexStride, vertexUVOffset, vertexBufferData);
                triangle.uvCoord2 = getVertexBufferValueUV(index2, vertexStride, vertexUVOffset, vertexBufferData);
                triangle.uvCoord3 = getVertexBufferValueUV(index3, vertexStride, vertexUVOffset, vertexBufferData);
            }
        }

        triangle.bounds.include(triangle.vertex1);
        triangle.bounds.include(triangle.vertex2);
        triangle.bounds.include(triangle.vertex3);
        triangleBounds.push_back(triangle);
    }
}

QSSGMeshBVHTriangles QSSGMeshBVHBuilder::calculateTriangleBounds(quint32 indexOffset, quint32 indexCount) const
{
    QSSGMeshBVHTriangles data;

    using CalcTriangleBoundsFn = void (*)(quint32, quint32, const QByteArray &, const QByteArray &, const quint32, const quint32, const quint32, QSSGMeshBVHTriangles &);
    static const CalcTriangleBoundsFn calcTriangleBounds16Fns[] { &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, false, false, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, false, false, true>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, false, true, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, false, true, true>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, true, false, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, true, false, true>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, true, true, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt16, true, true, true> };

    static const CalcTriangleBoundsFn calcTriangleBounds32Fns[] { &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, false, false, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, false, false, true>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, false, true, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, false, true, true>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, true, false, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, true, false, true>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, true, true, false>,
                                                                  &calculateTriangleBoundsImpl<QSSGRenderComponentType::UnsignedInt32, true, true, true> };


    const size_t idx = (size_t(m_hasIndexBuffer) << 2u) | (size_t(m_hasPositionData) << 1u) | (size_t(m_hasUVData));

    if (m_indexBufferComponentType == QSSGRenderComponentType::UnsignedInt16)
        calcTriangleBounds16Fns[idx](indexOffset, indexCount, m_indexBufferData, m_vertexBufferData, m_vertexStride, m_vertexUVOffset, m_vertexPosOffset, data);
    else if (m_indexBufferComponentType == QSSGRenderComponentType::UnsignedInt32)
        calcTriangleBounds32Fns[idx](indexOffset, indexCount, m_indexBufferData, m_vertexBufferData, m_vertexStride, m_vertexUVOffset, m_vertexPosOffset, data);
    return data;
}

QSSGMeshBVHNode::Handle QSSGMeshBVHBuilder::splitNode(QSSGMeshBVH &bvh, QSSGMeshBVHNode::Handle node, quint32 offset, quint32 count, quint32 depth)
{
    // NOTE: The node handle argument is intentionally copied! We can risk the storage reallocating!
    // Besides, it's a trivial type.

    // Force a leaf node if the there are too few triangles or the tree depth
    // has exceeded the maximum depth
    if (count < QSSG_MAX_LEAF_TRIANGLES || depth >= QSSG_MAX_TREE_DEPTH) {
        node->offset = offset;
        node->count = count;
        return node;
    }

    // Determine where to split the current bounds
    const QSSGMeshBVHBuilder::Split split = getOptimalSplit(bvh, node->boundingData, offset, count);
    // Really this shouldn't happen unless there is invalid bounding data, but if that
    // that does happen make this a leaf node.
    if (split.axis == QSSGMeshBVHBuilder::Axis::None) {
        node->offset = offset;
        node->count = count;
        return node;
    }

    // Create the split by sorting the values in m_triangleBounds between
    // offset - count based on the split axis and position. The returned offset
    // will determine which values go into the left and right nodes.
    const quint32 splitOffset = partition(bvh, offset, count, split);

    // Create the leaf nodes
    if (splitOffset == offset || splitOffset == (offset + count)) {
        // If the split is at the start or end, this is a leaf node now
        // because there is no further branches necessary.
        node->offset = offset;
        node->count = count;
    } else {
        // Create the Left Node
        node->left = bvh.newHandle();
        const quint32 leftOffset = offset;
        const quint32 leftCount = splitOffset - offset;
        node->left->boundingData = getBounds(bvh, leftOffset, leftCount);
        node->left = splitNode(bvh, node->left, leftOffset, leftCount, depth + 1);

        // Create the Right Node
        node->right = bvh.newHandle();
        const quint32 rightOffset = splitOffset;
        const quint32 rightCount = count - leftCount;
        node->right->boundingData = getBounds(bvh, rightOffset, rightCount);
        node->right = splitNode(bvh, node->right, rightOffset, rightCount, depth + 1);
    }

    return node;
}

QSSGBounds3 QSSGMeshBVHBuilder::getBounds(const QSSGMeshBVH &bvh, quint32 offset, quint32 count)
{
    QSSGBounds3 totalBounds;
    const auto &triangleBounds = bvh.triangles();

    for (quint32 i = 0; i < count; ++i) {
        const QSSGBounds3 &bounds = triangleBounds[i + offset].bounds;
        totalBounds.include(bounds);
    }
    return totalBounds;
}

QSSGMeshBVHBuilder::Split QSSGMeshBVHBuilder::getOptimalSplit(const QSSGMeshBVH &bvh, const QSSGBounds3 &nodeBounds, quint32 offset, quint32 count)
{
    QSSGMeshBVHBuilder::Split split;
    split.axis = getLongestDimension(nodeBounds);
    split.pos = 0.f;

    if (split.axis != Axis::None)
        split.pos = getAverageValue(bvh, offset, count, split.axis);

    return split;
}

QSSGMeshBVHBuilder::Axis QSSGMeshBVHBuilder::getLongestDimension(const QSSGBounds3 &nodeBounds)
{
    QSSGMeshBVHBuilder::Axis axis = Axis::None;
    float largestDistance = std::numeric_limits<float>::min();

    if (!nodeBounds.isFinite() || nodeBounds.isEmpty())
        return axis;

    const QVector3D delta = nodeBounds.maximum - nodeBounds.minimum;

    if (delta.x() > largestDistance) {
        axis = Axis::X;
        largestDistance = delta.x();
    }
    if (delta.y() > largestDistance) {
        axis = Axis::Y;
        largestDistance = delta.y();
    }
    if (delta.z() > largestDistance)
        axis = Axis::Z;

    return axis;
}

// Get the average values of triangles for a given axis
float QSSGMeshBVHBuilder::getAverageValue(const QSSGMeshBVH &bvh, quint32 offset, quint32 count, QSSGMeshBVHBuilder::Axis axis)
{
    float average = 0;

    Q_ASSERT(axis != Axis::None);
    Q_ASSERT(count != 0);

    const auto &triangleBounds = bvh.triangles();
    for (quint32 i = 0; i < count; ++i)
        average += triangleBounds[i + offset].bounds.center(int(axis));

    return average / count;
}

quint32 QSSGMeshBVHBuilder::partition(QSSGMeshBVH &bvh, quint32 offset, quint32 count, const QSSGMeshBVHBuilder::Split &split)
{
    int left = offset;
    int right = offset + count - 1;
    const float pos = split.pos;
    const int axis = int(split.axis);

    auto &triangleBounds = bvh.m_triangles;
    while (true) {
        while (left <= right && triangleBounds[left].bounds.center(axis) < pos)
            left++;

        while (left <= right && triangleBounds[right].bounds.center(axis) >= pos)
            right--;

        if (left < right) {
            // Swap triangleBounds at left and right
            std::swap(triangleBounds[left], triangleBounds[right]);
            left++;
            right--;
        } else {
            return left;
        }
    }
    Q_UNREACHABLE();
}

QT_END_NAMESPACE
