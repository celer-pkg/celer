// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_MESH_H
#define QSSG_RENDER_MESH_H

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

#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>

#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DUtils/private/qssgmeshbvh_p.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderSubset
{
    quint32 count;
    quint32 offset;
    QSSGBounds3 bounds; // Vertex buffer bounds
    QSSGMeshBVHNode::Handle bvhRoot;
    struct {
        QSSGRhiBufferPtr vertexBuffer;
        QSSGRhiBufferPtr indexBuffer;
        QSSGRhiInputAssemblerState ia;
        QRhiTexture *targetsTexture = nullptr;
    } rhi;

    struct Lod {
        quint32 count = 0;
        quint32 offset = 0;
        float distance = 0.0f;
    };
    QVector<Lod> lods;

    QSSGRenderSubset() = default;
    QSSGRenderSubset(const QSSGRenderSubset &inOther)
        : count(inOther.count)
        , offset(inOther.offset)
        , bounds(inOther.bounds)
        , bvhRoot(inOther.bvhRoot)
        , rhi(inOther.rhi)
        , lods(inOther.lods)
    {
    }
    QSSGRenderSubset &operator=(const QSSGRenderSubset &inOther)
    {
        if (this != &inOther) {
            count = inOther.count;
            offset = inOther.offset;
            bounds = inOther.bounds;
            bvhRoot = inOther.bvhRoot;
            rhi = inOther.rhi;
            lods = inOther.lods;
        }
        return *this;
    }

    quint32 lodCount(int lodLevel) const {
        if (lodLevel == 0 || lods.isEmpty())
            return count;
        if (lodLevel > lods.count())
            lodLevel = lods.count() - 1;
        else
            lodLevel -= 1;

        return lods[lodLevel].count;
    }

    quint32 lodOffset(int lodLevel) const {
        if (lodLevel == 0 || lods.isEmpty())
            return offset;
        if (lodLevel > lods.count())
            lodLevel = lods.count() - 1;
        else
            lodLevel -= 1;

        return lods[lodLevel].offset;
    }

};

struct QSSGRenderMesh
{
    Q_DISABLE_COPY(QSSGRenderMesh)

    QVector<QSSGRenderSubset> subsets;
    QSSGRenderDrawMode drawMode;
    QSSGRenderWinding winding;
    std::unique_ptr<QSSGMeshBVH> bvh;
    QSize lightmapSizeHint;

    QSSGRenderMesh(QSSGRenderDrawMode inDrawMode, QSSGRenderWinding inWinding)
        : drawMode(inDrawMode), winding(inWinding)
    {
    }
};
QT_END_NAMESPACE

#endif
