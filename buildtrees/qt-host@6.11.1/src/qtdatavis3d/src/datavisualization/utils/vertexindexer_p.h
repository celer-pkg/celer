// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef VERTEXINDEXER_P_H
#define VERTEXINDEXER_P_H

#include "datavisualizationglobal_p.h"

#include <QtCore/QList>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

class VertexIndexer
{
public:
    struct PackedVertex {
        QVector3D position;
        QVector2D uv;
        QVector3D normal;
        bool operator<(const PackedVertex that) const {
            return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
        }
    };

    static void indexVBO(const QList<QVector3D> &in_vertices, const QList<QVector2D> &in_uvs,
                         const QList<QVector3D> &in_normals, QList<GLuint> &out_indices,
                         QList<QVector3D> &out_vertices, QList<QVector2D> &out_uvs,
                         QList<QVector3D> &out_normals);

private:
    static bool getSimilarVertexIndex_fast(const PackedVertex &packed,
                                           QMap<PackedVertex, GLuint> &VertexToOutIndex,
                                           GLuint &result);
};

QT_END_NAMESPACE

#endif
