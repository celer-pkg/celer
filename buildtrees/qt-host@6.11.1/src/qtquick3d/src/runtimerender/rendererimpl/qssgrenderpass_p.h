// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERPASS_H
#define QSSGRENDERPASS_H

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

#include <QtCore/qglobal.h>

#include <ssg/qssgrenderhelpers.h>
#include <QtQuick3DUtils/private/qssgaosettings_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableobjects_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermotionvectormap_p.h>
#include "qssgrenderer_p.h"

QT_BEGIN_NAMESPACE

class QSSGRenderShadowMap;
class QSSGRenderReflectionMap;
class QSSGLayerRenderData;
class QSSGRenderCamera;
class QSGRenderer;
class QSSGRenderExtension;
class QSSGRenderUserPass;

class QRhiTexture;

class QSSGRenderPass
{
public:
    enum class Type
    {
        Standalone,
        Main,
        Extension
    };
    // Input:

    virtual ~QSSGRenderPass();
    virtual void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) = 0;
    virtual void renderPass(QSSGRenderer &renderer) = 0;
    virtual Type passType() const = 0;
    virtual void resetForFrame() = 0;

    // Output:

    // Flags: Debug markers(?)

    // Dependency
};

class MotionVectorMapPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    QSSGRhiRenderableTexture *rhiMotionVectorTexture = nullptr;

    static constexpr int MaxBuckets = 8; // (int(skin) << 2) | (int(instance) << 1) | int(morph);
    QSSGRenderableObjectList motionVectorPassObjects[MaxBuckets];
    QSSGRenderCamera *camera = nullptr;
    QSSGRhiGraphicsPipelineState ps;
    bool enabled = false;
    QSSGRenderMotionVectorMapPtr motionVectorMapManager;
};

class ShadowMapPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    std::shared_ptr<QSSGRenderShadowMap> shadowMapManager;
    QSSGRenderableObjectList shadowPassObjects;
    QSSGShaderLightList globalLights;
    QSSGRenderCamera *camera = nullptr;
    std::unique_ptr<QSSGRenderCamera> debugCamera;
    QSSGRhiGraphicsPipelineState ps;
    QSSGBounds3 castingObjectsBox;
    QSSGBounds3 receivingObjectsBox;
    bool enabled = false;
};

class ReflectionMapPass : public QSSGRenderPass
{
public:
    ReflectionMapPass();
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    std::shared_ptr<QSSGRenderReflectionMap> reflectionMapManager;
    QList<QSSGRenderReflectionProbe *> reflectionProbes;
    QSSGRenderableObjectList reflectionPassObjects;
    QSSGRhiGraphicsPipelineState ps;
    bool m_includeSTO = false; // Compatibility flag to include STO objects in reflection map rendering.
};

class ZPrePassPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRenderableObjectList renderedDepthWriteObjects;
    QSSGRenderableObjectList renderedOpaqueDepthPrepassObjects;
    QSSGRhiGraphicsPipelineState ps;
    bool active = false;
};

class SSAOMapPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    const QSSGRhiRenderableTexture *rhiDepthTexture = nullptr;
    const QSSGRenderCamera *camera = nullptr;
    QSSGAmbientOcclusionSettings aoSettings;
    QSSGRhiGraphicsPipelineState ps;
    QSSGRhiRenderableTexture *rhiAoTexture = nullptr;
    QSSGRhiShaderPipelinePtr ssaoShaderPipeline;
};

class Q_QUICK3DRUNTIMERENDER_EXPORT DepthMapPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;
    void setMultisamplingEnabled(bool enabled) { m_multisampling = enabled; }
    bool isMultisamplingEnabled() const { return m_multisampling; }

    QSSGRenderableObjectList sortedOpaqueObjects;
    QSSGRenderableObjectList sortedTransparentObjects;
    QSSGRhiGraphicsPipelineState ps;
    QSSGRhiRenderableTexture *rhiDepthTexture = nullptr;
    bool m_multisampling = false;
};

class Q_QUICK3DRUNTIMERENDER_EXPORT NormalPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    QSSGRenderableObjectList sortedOpaqueObjects;
    QSSGRhiGraphicsPipelineState ps;
    QSSGRhiRenderableTexture *normalTexture = nullptr;
    QSSGRhiRenderableTexture *depthBuffer = nullptr;
};

class SkyboxPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRenderLayer *layer = nullptr;
    QRhiRenderPassDescriptor *rpDesc = nullptr;
    QSSGRhiGraphicsPipelineState ps;
    bool skipTonemapping = false;
    bool skipPrep = false;
};

class SkyboxCubeMapPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRhiShaderPipelinePtr skyBoxCubeShader;
    QSSGRenderLayer *layer = nullptr;
    QRhiRenderPassDescriptor *rpDesc = nullptr;
    QSSGRhiGraphicsPipelineState ps;
    bool skipTonemapping = false;
};

class ScreenMapPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    QSSGRhiRenderableTexture *rhiScreenTexture = nullptr;
    std::optional<SkyboxPass> skyboxPass;
    std::optional<SkyboxCubeMapPass> skyboxCubeMapPass;
    QSSGShaderFeatures shaderFeatures;
    QSSGRenderableObjectList sortedOpaqueObjects;
    QSSGRhiGraphicsPipelineState ps;
    QColor clearColor{Qt::transparent};
    bool wantsMips = false;
};

class ScreenReflectionPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRenderableObjectList sortedScreenTextureObjects;
    const QSSGRhiRenderableTexture *rhiScreenTexture = nullptr;
    QSSGRhiGraphicsPipelineState ps {};
};

class OpaquePass : public QSSGRenderPass
{
public:
    static void prep(const QSSGRenderContextInterface &ctx,
                     QSSGLayerRenderData &data,
                     QSSGPassKey passKey,
                     QSSGRhiGraphicsPipelineState &ps,
                     QSSGShaderFeatures shaderFeatures,
                     QRhiRenderPassDescriptor *rpDesc,
                     const QSSGRenderableObjectList &sortedOpaqueObjects);

    static void render(const QSSGRenderContextInterface &ctx,
                       const QSSGRhiGraphicsPipelineState &ps,
                       const QSSGRenderableObjectList &sortedOpaqueObjects);

    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRenderableObjectList sortedOpaqueObjects;
    QSSGRhiGraphicsPipelineState ps;
    QSSGShaderFeatures shaderFeatures;
};

struct QSSGCameraRenderData;

class TransparentPass : public QSSGRenderPass
{
public:
    static void prep(const QSSGRenderContextInterface &ctx,
                     QSSGLayerRenderData &data,
                     QSSGPassKey passKey,
                     QSSGRhiGraphicsPipelineState &ps,
                     QSSGShaderFeatures shaderFeatures,
                     QRhiRenderPassDescriptor *rpDesc,
                     const QSSGRenderableObjectList &sortedTransparentObjects,
                     bool oit = false);

    static void render(const QSSGRenderContextInterface &ctx,
                       const QSSGRhiGraphicsPipelineState &ps,
                       const QSSGRenderableObjectList &sortedTransparentObjects);


    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRenderableObjectList sortedTransparentObjects;
    QSSGRhiGraphicsPipelineState ps;
    QSSGShaderFeatures shaderFeatures;
};

class Item2DPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

protected:
    std::vector<QSGRenderer *> prepdItem2DRenderers;
};

class InfiniteGridPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRhiShaderPipelinePtr gridShader;
    QSSGRhiGraphicsPipelineState ps {};
    QSSGRenderLayer *layer = nullptr;
};

class DebugDrawPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;

    QSSGRhiShaderPipelinePtr debugObjectShader;
    QSSGRhiGraphicsPipelineState ps;
};

class UserExtensionPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Extension; }
    void resetForFrame() final;

    bool hasData() const { return extensions.size() != 0; }

    QList<QSSGRenderExtension *> extensions;
};

class UserRenderPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Standalone; }
    void resetForFrame() final;

    bool hasData() const { return (userPasses.size() != 0); }

    QList<QSSGRenderUserPass *> userPasses;

protected:
    struct UserPassData
    {
        QSSGRhiGraphicsPipelineState ps;
        QSSGRenderableObjectList renderables;
        QSSGRhiRenderableTextureV2Ptr renderableTexture;
        QSSGShaderDefineList shaderDefines;
        QColor clearColor = Qt::black;
        QRhiDepthStencilClearValue depthStencilClearValue = { };
        qsizetype index = 0;

        std::optional<SkyboxCubeMapPass> skyboxCubeMapPass;
        std::optional<SkyboxPass> skyboxPass;
        std::optional<Item2DPass> item2DPass;
        std::vector<UserPassData> subPassData;
    };

    void prepareTopLevelPass(QSSGRenderer &renderer, QSSGLayerRenderData &data, QSSGRenderUserPass *passNode)
    {
        visitedPasses.clear(); // clear visited passes for each top-level pass
        preparePassImpl(renderer, data, passNode, userPassData, {/* null */});
    }

    void prepareSubPass(QSSGRenderer &renderer,
                        QSSGLayerRenderData &data,
                        QSSGRenderUserPass *subPassNode,
                        std::vector<UserPassData> &subPassData,
                        const QSSGRhiRenderableTextureV2Ptr &renderableTexture)
    {
        Q_ASSERT(renderableTexture != nullptr);
        preparePassImpl(renderer, data, subPassNode, subPassData, renderableTexture);
    }

    void preparePassImpl(QSSGRenderer &renderer,
                         QSSGLayerRenderData &data,
                         QSSGRenderUserPass *passNode,
                         std::vector<UserPassData> &outData,
                         QSSGRhiRenderableTextureV2Ptr renderableTexture);
    std::vector<UserPassData> userPassData;
    std::set<QSSGResourceId> visitedPasses; // For circular dependency detection
    static constexpr size_t MAX_SUBPASS_DEPTH = 16;
};

class OITRenderPass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final;
    void resetForFrame() final;
    void setMethod(QSSGRenderLayer::OITMethod m) { method = m; }

    QSSGRenderLayer::OITMethod method;

    QSSGRhiShaderPipelinePtr clearPipeline;
    QRhiShaderResourceBindings *clearSrb = nullptr;

    QSSGRenderableObjectList sortedTransparentObjects;
    QSSGRhiGraphicsPipelineState ps;
    QSSGShaderFeatures shaderFeatures;
    union {
        QSSGRhiRenderableTexture *rhiAccumTexture = nullptr;
        QSSGRhiRenderableTexture *rhiABufferImage;
        QRhiBuffer *rhiABuffer;
    };
    union {
        QSSGRhiRenderableTexture *rhiRevealageTexture = nullptr;
        QSSGRhiRenderableTexture *rhiAuxiliaryImage;
        QRhiBuffer *rhiAuxBuffer;
    };
    QSSGRhiRenderableTexture *rhiCounterImage = nullptr;
    QRhiBuffer *rhiCounterBuffer = nullptr;
    QRhiTexture *readbackImage = nullptr;
    QSSGRhiRenderableTexture *rhiDepthTexture = nullptr;
    QRhiTextureRenderTarget *renderTarget = nullptr;
    quint32 reportedNodeCount = 0;
    quint32 currentNodeCount = 0;
    QList<QRhiReadbackResult* > results;
    QRhiResourceUpdateBatch *rub = nullptr;
};

class OITCompositePass : public QSSGRenderPass
{
public:
    void renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data) final;
    void renderPass(QSSGRenderer &renderer) final;
    Type passType() const final { return Type::Main; }
    void resetForFrame() final;
    void setMethod(QSSGRenderLayer::OITMethod m) { method = m; }
    QSSGRenderLayer::OITMethod method;
    QRhiShaderResourceBindings *compositeSrb = nullptr;
    QSSGRhiGraphicsPipelineState ps;
    QSSGShaderFeatures shaderFeatures;
    QSSGRhiShaderPipelinePtr compositeShaderPipeline;
    union {
        QSSGRhiRenderableTexture *rhiAccumTexture = nullptr;
        QSSGRhiRenderableTexture *rhiABufferImage;
        QRhiBuffer *rhiABuffer;
    };
    union {
        QSSGRhiRenderableTexture *rhiRevealageTexture = nullptr;
        QSSGRhiRenderableTexture *rhiAuxiliaryImage;
        QRhiBuffer *rhiAuxBuffer;
    };
};

QT_END_NAMESPACE

#endif // QSSGRENDERPASS_H
