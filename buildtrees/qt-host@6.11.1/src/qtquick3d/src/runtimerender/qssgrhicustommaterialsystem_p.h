// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RHI_CUSTOM_MATERIAL_SYSTEM_H
#define QSSG_RHI_CUSTOM_MATERIAL_SYSTEM_H

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
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableobjects_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermapkey_p.h>
#include <QtCore/qhash.h>
#include <QtGui/QMatrix4x4>

QT_BEGIN_NAMESPACE

struct QSSGRenderCustomMaterial;
struct QSSGRenderSubset;
struct QSSGRenderModel;
class QSSGLayerRenderData;
struct QSSGRenderableImage;
struct QSSGRenderLayer;
struct QSSGRenderLight;
class QSSGRenderCamera;
struct QSSGReflectionMapEntry;
class QRhiTexture;


class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGCustomMaterialSystem
{
    Q_DISABLE_COPY(QSSGCustomMaterialSystem)
    typedef QPair<QByteArray, QByteArray> TStrStrPair;
    typedef QHash<QSSGShaderMapKey, QSSGRhiShaderPipelinePtr> TShaderMap;

    QSSGRenderContextInterface *context = nullptr;
    TShaderMap shaderMap;

    void setShaderResources(char *ubufData,
                            const QSSGRenderCustomMaterial &inMaterial,
                            const QByteArray &inPropertyName,
                            const QVariant &propertyValue,
                            QSSGRenderShaderValue::Type inPropertyType,
                            QSSGRhiShaderPipeline &shaderPipeline);

public:
    QSSGCustomMaterialSystem();
    ~QSSGCustomMaterialSystem();

    void setRenderContextInterface(QSSGRenderContextInterface *inContext);

    void releaseCachedResources();

    // Returns true if the material is dirty and thus will produce a different render result
    // than previously.  This effects things like progressive AA.
    bool prepareForRender(const QSSGRenderModel &inModel,
                          const QSSGRenderSubset &inSubset,
                          QSSGRenderCustomMaterial &inMaterial);

    QSSGRhiShaderPipelinePtr shadersForCustomMaterial(QSSGRhiGraphicsPipelineState *ps,
                                                      const QSSGRenderCustomMaterial &material,
                                                      QSSGSubsetRenderable &renderable,
                                                      const QSSGShaderDefaultMaterialKeyProperties &defaultMaterialShaderKeyProperties,
                                                      const QSSGShaderFeatures &featureSet,
                                                      const QSSGUserShaderAugmentation &shaderAugmentation = {});

    void updateUniformsForCustomMaterial(QSSGRhiShaderPipeline &shaderPipeline,
                                         QSSGRhiContext *rhiCtx,
                                         const QSSGLayerRenderData &inData,
                                         char *ubufData,
                                         QSSGRhiGraphicsPipelineState *ps,
                                         const QSSGRenderCustomMaterial &material,
                                         QSSGSubsetRenderable &renderable,
                                         const QSSGRenderCameraList &cameras,
                                         const QVector2D *depthAdjust,
                                         const QMatrix4x4 *alteredModelViewProjection);

    void rhiPrepareRenderable(QSSGRhiGraphicsPipelineState *ps,
                              QSSGPassKey passKey,
                              QSSGSubsetRenderable &renderable,
                              const QSSGShaderFeatures &featureSet,
                              const QSSGRenderCustomMaterial &material,
                              const QSSGLayerRenderData &layerData,
                              QRhiRenderPassDescriptor *renderPassDescriptor,
                              int samples,
                              int viewCount,
                              bool screenMapPass,
                              QSSGRenderCamera *camera = nullptr,
                              QSSGRenderTextureCubeFace cubeFace = QSSGRenderTextureCubeFaceNone,
                              QMatrix4x4 *modelViewProjection = nullptr,
                              QSSGReflectionMapEntry *entry = nullptr,
                              bool oit = false);
    void applyRhiShaderPropertyValues(char *ubufData,
                                      const QSSGRenderCustomMaterial &inMaterial,
                                      QSSGRhiShaderPipeline &shaderPipeline);
    void rhiRenderRenderable(QSSGRhiContext *rhiCtx,
                             QSSGSubsetRenderable &renderable,
                             bool *needsSetViewport,
                             QSSGRenderTextureCubeFace cubeFace,
                             const QSSGRhiGraphicsPipelineState &state,
                             qsizetype userPassIndex = -1);
};

QT_END_NAMESPACE

#endif
