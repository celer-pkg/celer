// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERHELPERS_P_H
#define QSSGRENDERHELPERS_P_H

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

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderglobal.h>

#include <QtQuick3DUtils/private/qssgaosettings_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableobjects_p.h>

QT_BEGIN_NAMESPACE

class QSSGRhiContext;
class QSSGRenderShadowMap;
class QRhiRenderPassDescriptor;
class QSSGRenderReflectionMap;
class QSSGRenderMotionVectorMap;
class QSSGLayerRenderData;
struct QSSGReflectionMapEntry;
class QSSGRhiShaderPipeline;

namespace RenderHelpers
{

void rhiRenderShadowMap(QSSGRhiContext *rhiCtx,
                        QSSGPassKey passKey,
                        QSSGRhiGraphicsPipelineState &ps,
                        QSSGRenderShadowMap &shadowMapManager,
                        const QSSGRenderCamera &camera,
                        QSSGRenderCamera *debugCamera,
                        const QSSGShaderLightList &globalLights,
                        const QSSGRenderableObjectList &sortedOpaqueObjects,
                        QSSGRenderer &renderer,
                        const QSSGBounds3 &castingObjectsBox,
                        const QSSGBounds3 &receivingObjectsBox);

void rhiRenderReflectionMap(QSSGRhiContext *rhiCtx,
                            QSSGPassKey passKey,
                            const QSSGLayerRenderData &inData,
                            QSSGRhiGraphicsPipelineState *ps,
                            QSSGRenderReflectionMap &reflectionMapManager,
                            const QVector<QSSGRenderReflectionProbe *> &reflectionProbes,
                            const QSSGRenderableObjectList &reflectionPassObjects,
                            QSSGRenderer &renderer);

bool rhiPrepareDepthPass(QSSGRhiContext *rhiCtx,
                         QSSGPassKey passKey,
                         const QSSGRhiGraphicsPipelineState &basePipelineState,
                         QRhiRenderPassDescriptor *rpDesc,
                         QSSGLayerRenderData &inData,
                         const QSSGRenderableObjectList &sortedOpaqueObjects,
                         const QSSGRenderableObjectList &sortedTransparentObjects,
                         int samples,
                         int viewCount);

void rhiRenderDepthPass(QSSGRhiContext *rhiCtx, const QSSGRhiGraphicsPipelineState &ps,
                        const QSSGRenderableObjectList &sortedOpaqueObjects,
                        const QSSGRenderableObjectList &sortedTransparentObjects,
                        bool *needsSetViewport);

bool rhiPrepareNormalPass(QSSGRhiContext *rhiCtx,
                          QSSGPassKey passKey,
                          const QSSGRhiGraphicsPipelineState &basePipelineState,
                          QRhiRenderPassDescriptor *rpDesc,
                          QSSGLayerRenderData &inData,
                          const QSSGRenderableObjectList &sortedOpaqueObjects);

void rhiRenderNormalPass(QSSGRhiContext *rhiCtx, const QSSGRhiGraphicsPipelineState &ps,
                         const QSSGRenderableObjectList &sortedOpaqueObjects,
                         bool *needsSetViewport);

bool rhiPrepareAoTexture(QSSGRhiContext *rhiCtx,
                         const QSize &size,
                         QSSGRhiRenderableTexture *renderableTex,
                         quint8 viewCount);

void rhiRenderAoTexture(QSSGRhiContext *rhiCtx,
                        QSSGPassKey passKey,
                        QSSGRenderer &renderer,
                        QSSGRhiShaderPipeline &shaderPipeline,
                        QSSGRhiGraphicsPipelineState &ps,
                        const QSSGAmbientOcclusionSettings &ao,
                        const QSSGRhiRenderableTexture &rhiAoTexture,
                        const QSSGRhiRenderableTexture &rhiDepthTexture,
                        const QSSGRenderCamera &camera);

bool rhiPrepareScreenTexture(QSSGRhiContext *rhiCtx,
                             const QSize &size,
                             bool wantsMips,
                             QSSGRhiRenderableTexture *renderableTex,
                             quint8 viewCount);

void addAccumulatorImageBindings(QSSGRhiShaderPipeline *shaderPipeline,
                                 QSSGRhiShaderResourceBindingList &bindings);

void rhiPrepareGrid(QSSGRhiContext *rhiCtx,
                    QSSGPassKey passKey,
                    QSSGRenderLayer &layer,
                    QSSGRenderCameraList &cameras,
                    QSSGRenderer &renderer);

void rhiPrepareSkyBox(QSSGRhiContext *rhiCtx,
                      QSSGPassKey passKey,
                      QSSGRenderLayer &layer,
                      QSSGRenderCameraList &cameras,
                      QSSGRenderer &renderer,
                      uint tonemapMode = 0);

void rhiPrepareSkyBoxForReflectionMap(QSSGRhiContext *rhiCtx,
                                      QSSGPassKey passKey,
                                      QSSGRenderLayer &layer,
                                      QSSGRenderCamera &inCamera,
                                      QSSGRenderer &renderer,
                                      QSSGReflectionMapEntry *entry,
                                      QSSGRenderTextureCubeFace cubeFace);

[[nodiscard]] qsizetype rhiPrepareOverrideMaterialUserPass(QSSGRhiContext *rhiCtx,
                                                           QSSGPassKey passKey,
                                                           const QSSGRhiGraphicsPipelineState &basePipelineState,
                                                           QRhiRenderPassDescriptor *rpDesc,
                                                           QSSGRenderGraphObject *overrideMaterial,
                                                           QSSGLayerRenderData &inData,
                                                           QSSGRenderableObjectList &inObjects,
                                                           QSSGShaderFeatures featureSet);

[[nodiscard]] qsizetype rhiPrepareOriginalMaterialUserPass(QSSGRhiContext *rhiCtx,
                                                           QSSGPassKey passKey,
                                                           const QSSGRhiGraphicsPipelineState &basePipelineState,
                                                           QRhiRenderPassDescriptor *rpDesc,
                                                           const QSSGLayerRenderData &inData,
                                                           QSSGRenderableObjectList &inObjects,
                                                           QSSGShaderFeatures featureSet);

[[nodiscard]] qsizetype rhiPrepareAugmentedUserPass(QSSGRhiContext *rhiCtx,
                                                    QSSGPassKey passKey,
                                                    const QSSGRhiGraphicsPipelineState &basePipelineState,
                                                    QRhiRenderPassDescriptor *rpDesc,
                                                    const QSSGUserShaderAugmentation &shaderAugmentation,
                                                    const QSSGLayerRenderData &inData,
                                                    QSSGRenderableObjectList &inObjects,
                                                    QSSGShaderFeatures featureSet);

void rhiRenderUserAugmentedPass(QSSGRhiContext *rhiCtx,
                               QSSGRenderableObjectList &inObjects);

Q_QUICK3DRUNTIMERENDER_EXPORT void rhiPrepareRenderable(QSSGRhiContext *rhiCtx,
                                                        QSSGPassKey passKey,
                                                        const QSSGLayerRenderData &inData,
                                                        QSSGRenderableObject &inObject,
                                                        QRhiRenderPassDescriptor *renderPassDescriptor,
                                                        QSSGRhiGraphicsPipelineState *ps,
                                                        QSSGShaderFeatures featureSet,
                                                        int samples,
                                                        int viewCount,
                                                        QSSGRenderCamera *alteredCamera = nullptr,
                                                        QMatrix4x4 *alteredModelViewProjection = nullptr,
                                                        QSSGRenderTextureCubeFace cubeFace = QSSGRenderTextureCubeFaceNone,
                                                        QSSGReflectionMapEntry *entry = nullptr,
                                                        bool oit = false);

Q_QUICK3DRUNTIMERENDER_EXPORT void rhiPrepareRenderableForScreenMapPass(QSSGRhiContext *rhiCtx,
                                                                        QSSGPassKey passKey,
                                                                        const QSSGLayerRenderData &inData,
                                                                        QSSGRenderableObject &inObject,
                                                                        QRhiRenderPassDescriptor *renderPassDescriptor,
                                                                        QSSGRhiGraphicsPipelineState *ps,
                                                                        QSSGShaderFeatures featureSet,
                                                                        int samples,
                                                                        int viewCount,
                                                                        QSSGRenderCamera *alteredCamera = nullptr,
                                                                        QMatrix4x4 *alteredModelViewProjection = nullptr,
                                                                        QSSGRenderTextureCubeFace cubeFace = QSSGRenderTextureCubeFaceNone,
                                                                        QSSGReflectionMapEntry *entry = nullptr,
                                                                        bool oit = false);

Q_QUICK3DRUNTIMERENDER_EXPORT void rhiRenderRenderable(QSSGRhiContext *rhiCtx,
                                                       const QSSGRhiGraphicsPipelineState &state,
                                                       QSSGRenderableObject &object,
                                                       bool *needsSetViewport,
                                                       QSSGRenderTextureCubeFace cubeFace = QSSGRenderTextureCubeFaceNone,
                                                       qsizetype userPassIndex = -1);

bool rhiPrepareDepthTexture(QSSGRhiContext *rhiCtx,
                            const QSize &size,
                            QSSGRhiRenderableTexture *renderableTex,
                            quint8 viewCount,
                            int samples = 1);

bool rhiPrepareMotionVectorTexture(QSSGRhiContext *rhiCtx,
                                   const QSize &size,
                                   QSSGRhiRenderableTexture *renderableTex);

void rhiPrepareMotionVectorRenderable(QSSGRhiContext *rhiCtx,
                                      QSSGPassKey passKey,
                                      const QSSGLayerRenderData &inData,
                                      const QMatrix4x4 &viewProjection,
                                      QSSGRenderableObject &inObject,
                                      QRhiRenderPassDescriptor *renderPassDescriptor,
                                      QSSGRhiGraphicsPipelineState *ps,
                                      QSSGRenderMotionVectorMap &motionVectorMapManager);

void rhiRenderMotionVector(QSSGRhiContext *rhiCtx,
                           const QSSGRhiGraphicsPipelineState &state,
                           const QSSGRenderableObjectList *motionVectorPassObjects,
                           int bucketsCount);

inline QRect correctViewportCoordinates(const QRectF &layerViewport, const QRect &deviceRect)
{
    const int y = deviceRect.bottom() - layerViewport.bottom() + 1;
    return QRect(layerViewport.x(), y, layerViewport.width(), layerViewport.height());
}

inline quint32 rhiCalculateABufferSize(int nodeCount)
{
    quint32 s = nodeCount;
    quint32 s_sqrt = qSqrt(s);
    quint32 s2 = s_sqrt * s_sqrt;
    while (s2 < s) {
        s_sqrt += 1;
        s2 = s_sqrt * s_sqrt;
    }
    return s_sqrt;
}

inline quint32 rhiCalculateABufferSize(const QSize &size, int levels)
{
    quint32 s = size.width() * size.height() * levels;
    return rhiCalculateABufferSize(s);
}

}

QT_END_NAMESPACE


#endif // QSSGRENDERHELPERS_P_H
