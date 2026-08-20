// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QSSG_RENDER_MOTIONVECTORMAP_H
#define QSSG_RENDER_MOTIONVECTORMAP_H

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

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtCore/QHash>
#include <QtCore/QByteArray>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableobjects_p.h>

QT_BEGIN_NAMESPACE

class QSSGRhiContext;
class QSSGRenderContextInterface;
class QRhiRenderBuffer;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiTexture;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderMotionVectorMap
{
    Q_DISABLE_COPY(QSSGRenderMotionVectorMap)
public:
    struct MotionResultData {
        QMatrix4x4 prevModelViewProjection;
        QMatrix4x4 prevInstanceLocal;
        QMatrix4x4 prevInstanceGlobal;
        QSSGRenderImageTexture prevBoneTexture;
        QSSGRenderImageTexture prevInstanceTexture;
        QSSGRenderImageTexture currentMorphWeightTexture;
        QSSGRenderImageTexture prevMorphWeightTexture;
    };

    explicit QSSGRenderMotionVectorMap(const QSSGRenderContextInterface &inContext);
    ~QSSGRenderMotionVectorMap();

    void endFrame();
    MotionResultData trackMotionData(const void *modelKey,
                                     const QMatrix4x4 &currentModelViewProjection,
                                     const QMatrix4x4 &currentInstanceLocal,
                                     const QMatrix4x4 &currentInstanceGlobal,
                                     QSSGRenderTextureData *currentBoneTextureData,
                                     QSSGRenderInstanceTable *currentInstanceTable,
                                     const QVector<float> &currentMorphWeights);

    void releaseCachedResources();

private:
    struct MotionStoreData {
        QMatrix4x4 prevModelViewProjection;
        QMatrix4x4 prevInstanceLocal;
        QMatrix4x4 prevInstanceGlobal;
        std::shared_ptr<QSSGRenderTextureData> prevBoneTextureData;
        std::shared_ptr<QSSGRenderTextureData> prevInstanceTextureData;
        std::shared_ptr<QSSGRenderTextureData> currentMorphWeightTextureData;
        std::shared_ptr<QSSGRenderTextureData> prevMorphWeightTextureData;
        QSize lastPrevBoneTextureDataSize;
        QSize lastPrevInstanceTextureDataSize;
        QSize lastPrevMorphWeightTextureDataSize;
        QSize lastCurrentMorphWeightTextureDataSize;
        int frameAge = 0;
        bool visitedThisFrame = false;
        MotionResultData cachedResult;
    };

    const QSSGRenderContextInterface &m_context;
    QHash<const void*, MotionStoreData> m_cache;

    void cleanupStaleEntries();
    void releaseEntryResources(MotionStoreData& entry);
    QSSGRenderImageTexture loadAndReleaseIfNeeded(const std::shared_ptr<QSSGRenderTextureData>& textureData, QSize lastSize);
};

using QSSGRenderMotionVectorMapPtr = std::shared_ptr<QSSGRenderMotionVectorMap>;

QT_END_NAMESPACE

#endif // QSSG_RENDER_MOTIONVECTORMAP_H
