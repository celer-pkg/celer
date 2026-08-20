// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGLIGHTMAPUVGENERATOR_P_H
#define QSSGLIGHTMAPUVGENERATOR_P_H

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

#include <QtQuick3DUtils/private/qssgmesh_p.h>

QT_BEGIN_NAMESPACE

struct QSSGLightmapUVGeneratorResult
{
    QByteArray lightmapUVChannel;
    QVector<quint32> vertexMap;
    QByteArray indexData;
    quint32 lightmapWidth = 0;
    quint32 lightmapHeight = 0;

    bool isValid() const {
        return !lightmapUVChannel.isEmpty() && !vertexMap.isEmpty();
    }
};

class Q_QUICK3DUTILS_EXPORT QSSGLightmapUVGenerator
{
public:
    // Takes in position, normals, UV0, indices and returns a new lightmap UV
    // channel, new index data, and a mapping to original vertices.
    //
    // The topology must be triangles. The position data is expected to contain
    // 3 component float positions. normals is expected to contain 3 component
    // float normal vectors. uv0 is expected to contain 2 component float UV
    // coordinates. When not available, normals and uv0 can be left empty.
    //
    // The resulting index data has always the same number of indices as the
    // original, but regardless of the original component type, the new data
    // always has a component type of uint32.
    //
    // The resulting lightmapUVChannel contains 2 component floats. There can
    // be more lightmap UVs than input positions, because the unwrapping
    // process may add extra vertices to avoid seams. The new vertices always
    // correspond to an original vertex. That is why the result also has a list
    // that has one element for each UV in the lightmap UV data, the value
    // being an index of a vertex in the original position channel. This
    // mapping must be used by the caller to grow and rewrite all vertex input
    // data (position, normals, UVs, etc.) so that the count of their elements
    // matches the lightmap UV channel.
    //
    // texelsPerUnit
    // Unit to texel scale. e.g. a 1x1 quad with texelsPerUnit of 32 will take
    // up approximately 32x32 texels in the atlas.
    //
    QSSGLightmapUVGeneratorResult run(const QByteArray &positions,
                                      const QByteArray &normals,
                                      const QByteArray &uv0,
                                      const QByteArray &index,
                                      QSSGMesh::Mesh::ComponentType indexComponentType,
                                      float texelsPerUnit);

    // source is of N elements of componentCount * sizeof(T) bytes each. The
    // returned data is M elements of componentCount * sizeof(T) bytes each, where
    // M >= N. vertexMap is the mapping table with M elements where each element
    // is an index in range [0, N-1].
    template<typename T>
    static QByteArray remap(const QByteArray &source, const QVector<quint32> &vertexMap, int componentCount)
    {
        if (source.isEmpty())
            return QByteArray();
        const T *src = reinterpret_cast<const T *>(source.constData());
        const int byteStride = sizeof(T) * componentCount;
        QByteArray result(vertexMap.size() * byteStride, Qt::Uninitialized);
        T *dst = reinterpret_cast<T *>(result.data());
        for (int i = 0, count = vertexMap.size(); i != count; ++i) {
            const quint32 originalVertexIndex = vertexMap[i];
            for (int j = 0; j < componentCount; ++j)
                *dst++ = src[originalVertexIndex * componentCount + j];
        }
        return result;
    }
};

QT_END_NAMESPACE

#endif // QSSGLIGHTMAPUVGENERATOR_P_H
