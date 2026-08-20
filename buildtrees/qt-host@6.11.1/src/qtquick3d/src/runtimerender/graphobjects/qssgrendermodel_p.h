// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_MODEL_H
#define QSSG_RENDER_MODEL_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendergeometry_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderskeleton_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderskin_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderinstancetable_p.h>

#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

struct QSSGRenderDefaultMaterial;
struct QSSGParticleBuffer;
class QSSGBufferManager;
class QRhiTexture;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderModel : public QSSGRenderNode
{
    QVector<QSSGRenderGraphObject *> materials;
    QVector<QSSGRenderGraphObject *> morphTargets;
    QSSGRenderGeometry *geometry = nullptr;
    QSSGRenderPath meshPath;
    QSSGRenderSkeleton *skeleton = nullptr;
    QSSGRenderSkin *skin = nullptr;
    QVector<QMatrix4x4> inverseBindPoses;
    float m_depthBiasSq = 0.0f; // Depth bias is expected to be squared!
    bool castsShadows = true;
    bool receivesShadows = true;
    float instancingLodMin = -1;
    float instancingLodMax = -1;
    mutable int instanceLodCount = -1;

    QSSGRenderInstanceTable *instanceTable = nullptr;
    int instanceCount() const { return instanceTable ? (instanceLodCount >= 0) ? instanceLodCount : instanceTable->count() : 0; }
    bool instancing() const { return instanceTable;}

    QSSGParticleBuffer *particleBuffer = nullptr;
    QMatrix4x4 particleMatrix;
    bool hasTransparency = false;

    QVector<float> morphWeights;
    QVector<quint32> morphAttributes;

    bool receivesReflections = false;
    bool castsReflections = true;
    bool usedInBakedLighting = false;
    QString lightmapKey;
    float texelsPerUnit = 0.0f;
    bool motionVectorEnabled = true;
    float motionVectorScale = 1.0f;
    bool hasLightmap() const { return !lightmapKey.isEmpty(); }
    bool usesBoneTexture() const { return ((skin != nullptr) || (skeleton != nullptr)); }

    float levelOfDetailBias = 1.0f; // values < 1.0 will decrease usage of LODs, values > 1.0 will increase usage of LODs

    QSSGRenderModelHandle mh;

    QSSGRenderModel();
};
QT_END_NAMESPACE

#endif
