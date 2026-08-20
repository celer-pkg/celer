// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "vertexindexer_p.h"

#include <QtCore/qmath.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

int unique_vertices = 0;

bool VertexIndexer::getSimilarVertexIndex_fast(const PackedVertex &packed,
                                               QMap<PackedVertex, GLuint> &VertexToOutIndex,
                                               GLuint &result)
{
    QMap<PackedVertex, GLuint>::iterator it = VertexToOutIndex.find(packed);
    if (it == VertexToOutIndex.end()) {
        return false;
    } else {
        result = it.value();
        return true;
    }
}

void VertexIndexer::indexVBO(const QList<QVector3D> &in_vertices, const QList<QVector2D> &in_uvs,
                             const QList<QVector3D> &in_normals, QList<GLuint> &out_indices,
                             QList<QVector3D> &out_vertices, QList<QVector2D> &out_uvs,
                             QList<QVector3D> &out_normals)
{
    unique_vertices = 0;
    QMap<PackedVertex, GLuint> VertexToOutIndex;

    // For each input vertex
    for (int i = 0; i < in_vertices.size(); i++) {
        PackedVertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};

        // Try to find a similar vertex in out_XXXX
        GLuint index;
        bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

        if (found) {
            out_indices.append(index);
        } else {
            unique_vertices++;
            out_vertices.append(in_vertices[i]);
            out_uvs.append(in_uvs[i]);
            out_normals.append(in_normals[i]);
            GLuint newindex = (GLuint)out_vertices.size() - 1;
            out_indices.append(newindex);
            VertexToOutIndex[packed] = newindex;
        }
    }
}

QT_END_NAMESPACE
