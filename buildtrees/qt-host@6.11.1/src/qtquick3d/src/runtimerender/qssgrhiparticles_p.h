// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RHI_PARTICLES_H
#define QSSG_RHI_PARTICLES_H

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
#include <QtQuick3DRuntimeRender/private/qssgrenderableobjects_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderparticles_p.h>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>

QT_BEGIN_NAMESPACE

class QSSGLayerRenderData;
struct QSSGRenderableImage;
struct QSSGRenderLayer;
struct QSSGRenderLight;
class QSSGRenderCamera;
struct QSSGReflectionMapEntry;
class QRhiTexture;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGParticleRenderer
{
public:
    static void updateUniformsForParticles(const QSSGLayerRenderData &inData, QSSGRhiShaderPipeline &shaderPipeline,
                                           QSSGRhiContext *rhiCtx,
                                           char *ubufData,
                                           QSSGParticlesRenderable &renderable,
                                           const QSSGRenderCameraList &cameras);
    static void updateUniformsForParticleModel(QSSGRhiShaderPipeline &shaderPipeline,
                                               char *ubufData,
                                               const QSSGRenderModel *model, quint32 offset);

    static QSSGRhiShaderPipelinePtr getShaderPipelineParticles(QSSGRenderer &renderer,
                                                               QSSGParticlesRenderable &inRenderable,
                                                               const QSSGShaderFeatures &inFeatureSet);

    static QSSGRhiShaderPipelinePtr generateRhiShaderPipeline(QSSGRenderer &renderer,
                                                              QSSGParticlesRenderable &inRenderable,
                                                              const QSSGShaderFeatures &inFeatureSet,
                                                              QByteArray &shaderString,
                                                              const QSSGShaderParticleMaterialKeyProperties &shaderKeyProperties);

    static void rhiPrepareRenderable(QSSGRhiShaderPipeline &shaderPipeline, QSSGPassKey passKey,
                                     QSSGRhiContext *rhiCtx,
                                     QSSGRhiGraphicsPipelineState *ps,
                                     QSSGParticlesRenderable &renderable,
                                     const QSSGLayerRenderData &inData,
                                     QRhiRenderPassDescriptor *renderPassDescriptor,
                                     int samples,
                                     int viewCount,
                                     QSSGRenderCamera *alteredCamera = nullptr,
                                     QSSGRenderTextureCubeFace cubeFace = QSSGRenderTextureCubeFaceNone,
                                     QSSGReflectionMapEntry *entry = nullptr,
                                     bool oit = false);
    static void rhiRenderRenderable(QSSGRhiContext *rhiCtx,
                                    QSSGParticlesRenderable &renderable,
                                    bool *needsSetViewport,
                                    QSSGRenderTextureCubeFace cubeFace,
                                    const QSSGRhiGraphicsPipelineState &state);
    static void prepareParticlesForModel(QSSGRhiShaderPipeline &shaderPipeline,
                                         QSSGRhiContext *rhiCtx,
                                         QSSGRhiShaderResourceBindingList &bindings,
                                         const QSSGRenderModel *model);
    static QSSGShaderFeatures particleShaderFeatures(const QSSGShaderFeatures& features);
    static void setShaderCacheEnabled(bool enabled);
};

QT_END_NAMESPACE

#endif
