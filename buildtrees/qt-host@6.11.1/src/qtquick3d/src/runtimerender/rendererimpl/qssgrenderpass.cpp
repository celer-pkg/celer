// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderpass_p.h"
#include "qssgrhiquadrenderer_p.h"
#include "qssglayerrenderdata_p.h"
#include "qssgrendercontextcore.h"
#include "qssgdebugdrawsystem_p.h"
#include "extensionapi/qssgrenderextensions.h"
#include "qssgrenderhelpers_p.h"
#include "qssgrendercommands_p.h"

#include "../utils/qssgassert_p.h"

#include <QtQuick/private/qsgrenderer_p.h>
#include <qtquick3d_tracepoints_p.h>

QT_BEGIN_NAMESPACE

static const char defaultFragOutputs[][12] = {
    "fragOutput",
    "fragOutput1",
    "fragOutput2",
    "fragOutput3",
};

static QByteArrayView getDefaultOutputName(size_t index)
{
    QSSG_ASSERT(std::size(defaultFragOutputs) > index, return defaultFragOutputs[0]);
    return defaultFragOutputs[index];
}

static inline QMatrix4x4 correctMVPForScissor(QRectF viewportRect, QRect scissorRect, bool isYUp) {
    const auto &scissorCenter = scissorRect.center();
    const auto &viewCenter = viewportRect.center();
    const float scaleX = viewportRect.width() / float(scissorRect.width());
    const float scaleY = viewportRect.height() / float(scissorRect.height());
    const float dx = 2 * (viewCenter.x() - scissorCenter.x()) / scissorRect.width();
    const float dyRect = isYUp ? (scissorCenter.y() - viewCenter.y())
                                : (viewCenter.y() - scissorCenter.y());
    const float dy = 2 * dyRect / scissorRect.height();

    return QMatrix4x4(scaleX, 0.0f, 0.0f, dx,
                      0.0f, scaleY, 0.0f, dy,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}

QSSGRenderPass::~QSSGRenderPass()
{

}


// MOTION VECTOR PASS

void MotionVectorMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    Q_UNUSED(renderer)
    using namespace RenderHelpers;

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    camera = data.renderedCameras[0];
    QMatrix4x4 viewProjection(Qt::Uninitialized);
    QMatrix4x4 cameraGlobalTransform(Qt::Uninitialized);
    cameraGlobalTransform = data.getGlobalTransform(*camera);
    data.renderedCameras[0]->calculateViewProjectionMatrix(cameraGlobalTransform, viewProjection);

    const auto &layerPrepResult = data.layerPrepResult;
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();

    const auto &renderedOpaques = data.getSortedOpaqueRenderableObjects(*camera);
    const auto &renderedTransparent = data.getSortedTransparentRenderableObjects(*camera);

    rhiMotionVectorTexture = data.getRenderResult(QSSGRenderResult::Key::MotionVectorTexture);
    bool textureReady = rhiMotionVectorTexture &&
            rhiPrepareMotionVectorTexture(rhiCtx.get(), layerPrepResult.textureDimensions(), rhiMotionVectorTexture);

    if (!textureReady) {
        rhiMotionVectorTexture = nullptr;
        enabled = false;
        return;
    }

    motionVectorMapManager = data.requestMotionVectorMapManager();
    ps = data.getPipelineState();
    ps.flags |= { QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled,
                  QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled };
    ps.samples = 1;
    ps.viewCount = data.layer.viewCount;

    for (int i = 0; i < MaxBuckets; ++i)
        motionVectorPassObjects[i].clear();
    enabled = false;

    for (const auto &handles : { &renderedOpaques, &renderedTransparent }) {
        for (const auto &handle : *handles) {
            if (handle.obj->type == QSSGRenderableObject::Type::DefaultMaterialMeshSubset ||
                handle.obj->type == QSSGRenderableObject::Type::CustomMaterialMeshSubset) {
                QSSGSubsetRenderable *renderable(static_cast<QSSGSubsetRenderable *>(handle.obj));
                if (handle.obj->renderableFlags.isMotionVectorParticipant()) {
                    bool skin = renderable->modelContext.model.usesBoneTexture();
                    bool instance = renderable->modelContext.model.instanceCount() > 0;
                    bool morph = renderable->modelContext.model.morphTargets.size() > 0;
                    int bucketIndex = (int(skin) << 2) | (int(instance) << 1) | int(morph);
                    motionVectorPassObjects[bucketIndex].push_back(handle);

                    QSSGRhiGraphicsPipelineState tempPs = ps;

                    rhiPrepareMotionVectorRenderable(rhiCtx.get(),
                                                     this,
                                                     data,
                                                     viewProjection,
                                                     *handle.obj,
                                                     rhiMotionVectorTexture->rpDesc,
                                                     &tempPs,
                                                     *motionVectorMapManager);
                    enabled = true;
                }
            }
        }
    }
}

void MotionVectorMapPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;
    if (enabled) {
        const auto &rhiCtx = renderer.contextInterface()->rhiContext();
        QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
        QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
        cb->debugMarkBegin(QByteArrayLiteral("Quick3D motion vector map"));
        Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D motion vector map"));

        if (Q_LIKELY(rhiMotionVectorTexture && rhiMotionVectorTexture->isValid())) {
            cb->beginPass(rhiMotionVectorTexture->rt, QColor(0, 0, 0, 0), { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
            QSSGRHICTX_STAT(rhiCtx, beginRenderPass(rhiMotionVectorTexture->rt));
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);

            rhiRenderMotionVector(rhiCtx.get(),
                                  ps,
                                  motionVectorPassObjects,
                                  MaxBuckets);

            cb->endPass();
            QSSGRHICTX_STAT(rhiCtx, endRenderPass());
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("motion_vector_map"));
        }
        cb->debugMarkEnd();
    }
}

void MotionVectorMapPass::resetForFrame()
{
    QSSG_CHECK(motionVectorMapManager);
    motionVectorMapManager->endFrame();
    camera = nullptr;
    ps = {};
    rhiMotionVectorTexture = nullptr;
    for (int i = 0; i < MaxBuckets; ++i)
        motionVectorPassObjects[i].clear();
}

// SHADOW PASS

void ShadowMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    Q_UNUSED(renderer)
    using namespace RenderHelpers;

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    camera = data.renderedCameras[0];

    QSSG_ASSERT(shadowPassObjects.isEmpty(), shadowPassObjects.clear());

    data.getShadowCastingObjects(*camera, shadowPassObjects, castingObjectsBox, receivingObjectsBox);

    globalLights = data.globalLights;
    enabled = !shadowPassObjects.isEmpty() || !globalLights.isEmpty();

    if (enabled) {
        shadowMapManager = data.requestShadowMapManager();

        ps = data.getPipelineState();
        ps.flags |= { QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled, QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled };
        // Try reducing self-shadowing and artifacts.
        ps.depthBias = 2;
        ps.slopeScaledDepthBias = 1.5f;

        if (!debugCamera) {
            debugCamera = std::make_unique<QSSGRenderCamera>(QSSGRenderGraphObject::Type::OrthographicCamera);
        }
    }
}

void ShadowMapPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    // INPUT: Sorted opaque and transparent + depth, global lights (scoped lights not supported) and camera.

    // DEPENDECY: None

    // OUTPUT: Texture (Shadowmap Texture Atlas)

    // CONDITION: Lights (shadowPassObjects)

    if (enabled) {
        const auto &rhiCtx = renderer.contextInterface()->rhiContext();
        QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
        QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
        cb->debugMarkBegin(QByteArrayLiteral("Quick3D shadow map"));
        Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D shadow map"));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);

        QSSG_CHECK(shadowMapManager);
        rhiRenderShadowMap(rhiCtx.get(),
                           this,
                           ps,
                           *shadowMapManager,
                           *camera,
                           debugCamera.get(),
                           globalLights, // scoped lights are not relevant here
                           shadowPassObjects,
                           renderer,
                           castingObjectsBox,
                           receivingObjectsBox);

        cb->debugMarkEnd();
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("shadow_map"));
    }
}

void ShadowMapPass::resetForFrame()
{
    enabled = false;
    camera = nullptr;
    castingObjectsBox = {};
    receivingObjectsBox = {};
    ps = {};
    shadowPassObjects.clear();
    globalLights.clear();
}

// REFLECTIONMAP PASS

ReflectionMapPass::ReflectionMapPass()
{
    // Read the environment variable to check if the old behavior of including the screen texture objects
    // in the reflection pass should be kept for compatibility reasons.
    // Besides being expensive, we shouldn't be using the main screen texture in the probes at all.
    m_includeSTO = qEnvironmentVariableIntValue("QT_QUICK3D_REFLECTION_PASS_INCLUDE_SCREEN_TEXTURE_OBJECTS") != 0;
}

void ReflectionMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    Q_UNUSED(renderer);
    Q_UNUSED(data);

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    ps = data.getPipelineState();
    ps.flags |= { QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled,
                  QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled,
                  QSSGRhiGraphicsPipelineState::Flag::BlendEnabled };

    reflectionProbes = { data.reflectionProbesView.begin(), data.reflectionProbesView.end() };
    reflectionMapManager = data.requestReflectionMapManager();

    const auto &sortedOpaqueObjects = data.getSortedOpaqueRenderableObjects(*camera);
    const auto &sortedTransparentObjects = data.getSortedTransparentRenderableObjects(*camera);
    QSSGRenderableObjectList emptyList{};
    const auto &sortedScreenTextureObjects = m_includeSTO ? data.getSortedScreenTextureRenderableObjects(*camera) : emptyList;

    QSSG_ASSERT(reflectionPassObjects.isEmpty(), reflectionPassObjects.clear());

    // NOTE: We should consider keeping track of the reflection casting objects to avoid
    // filtering this list on each prep.
    for (const auto &handles : { &sortedOpaqueObjects, &sortedTransparentObjects, &sortedScreenTextureObjects }) {
        for (const auto &handle : *handles) {
            if (handle.obj->renderableFlags.testFlag(QSSGRenderableObjectFlag::CastsReflections))
                reflectionPassObjects.push_back(handle);
        }
    }
}

void ReflectionMapPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    // INPUT: Reflection probes, sorted opaque and transparent

    // DEPENDECY: None

    // OUTPUT: Cube maps (1 per probe)

    // NOTE: Full pass with a sky box pass

    // CONDITION: Probes and sorted opaque and transparent

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    const auto *layerData = QSSGLayerRenderData::getCurrent(renderer);
    QSSG_ASSERT(layerData, return);

    QSSG_CHECK(reflectionMapManager);
    if (!reflectionPassObjects.isEmpty() || !reflectionProbes.isEmpty()) {
        cb->debugMarkBegin(QByteArrayLiteral("Quick3D reflection map"));
        Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D reflection map"));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        rhiRenderReflectionMap(rhiCtx.get(),
                               this,
                               *layerData,
                               &ps,
                               *reflectionMapManager,
                               reflectionProbes,
                               reflectionPassObjects,
                               renderer);

        cb->debugMarkEnd();
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("reflection_map"));
    }
}

void ReflectionMapPass::resetForFrame()
{
    ps = {};
    reflectionProbes.clear();
    reflectionPassObjects.clear();
}

// ZPrePass
void ZPrePassPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    using namespace RenderHelpers;

    // INPUT: Item2Ds + depth write + depth prepass

    // DEPENDECY: none

    // OUTPUT: Depth buffer attchment for current target

    // NOTE: Could we make the depth pass more complete and just do a blit here?
    //
    // 1. If we have a depth map, just do a blit and then update with the rest
    // 2. If we don't have a depth map (and/or SSAO) consider using a lower lod level.

    // CONDITION: Input + globally enabled or ?

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    ps = data.getPipelineState();

    renderedDepthWriteObjects = data.getSortedRenderedDepthWriteObjects(*camera);
    renderedOpaqueDepthPrepassObjects = data.getSortedrenderedOpaqueDepthPrepassObjects(*camera);

    cb->debugMarkBegin(QByteArrayLiteral("Quick3D prepare Z prepass"));
    Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D prepare Z prepass"));
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    active = rhiPrepareDepthPass(rhiCtx.get(), this, ps, rhiCtx->mainRenderPassDescriptor(), data,
                                         renderedDepthWriteObjects, renderedOpaqueDepthPrepassObjects,
                                         rhiCtx->mainPassSampleCount(), data.layer.viewCount);
    data.setZPrePassPrepResult(active);
    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("prepare_z_prepass"));
}

void ZPrePassPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);

    bool needsSetViewport = true;
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    if (active) {
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D render Z prepass"));
        cb->debugMarkBegin(QByteArrayLiteral("Quick3D render Z prepass"));
        rhiRenderDepthPass(rhiCtx.get(), ps, renderedDepthWriteObjects, renderedOpaqueDepthPrepassObjects, &needsSetViewport);
        cb->debugMarkEnd();
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("render_z_prepass"));
    }
}

void ZPrePassPass::resetForFrame()
{
    renderedDepthWriteObjects.clear();
    renderedOpaqueDepthPrepassObjects.clear();
    ps = {};
    active = false;
}

// SSAO PASS
void SSAOMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    using namespace RenderHelpers;

    // Assumption for now is that all passes are keept alive and only reset once a frame is done.
    // I.e., holding data like this should be safe (If that's no longer the case we need to do ref counting
    // for shared data).

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);

    rhiAoTexture = data.getRenderResult(QSSGRenderResult::Key::AoTexture);
    rhiDepthTexture = data.getRenderResult(QSSGRenderResult::Key::DepthTexture);
    QSSG_ASSERT_X(!data.renderedCameras.isEmpty(), "Preparing AO pass failed, missing camera", return);
    camera = data.renderedCameras[0];
    QSSG_ASSERT_X((rhiDepthTexture && rhiDepthTexture->isValid()), "Preparing AO pass failed, missing equired texture(s)", return);

    const auto &shaderCache = renderer.contextInterface()->shaderCache();
    ssaoShaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiSsaoShader(data.layer.viewCount);
    aoSettings = { data.layer.aoStrength, data.layer.aoDistance, data.layer.aoSoftness, data.layer.aoBias, data.layer.aoSamplerate, data.layer.aoDither };

    ps = data.getPipelineState();
    const auto &layerPrepResult = data.layerPrepResult;
    const bool ready = rhiAoTexture && rhiPrepareAoTexture(rhiCtx.get(), layerPrepResult.textureDimensions(), rhiAoTexture, data.layer.viewCount);

    if (Q_UNLIKELY(!ready))
        rhiAoTexture = nullptr;
}

void SSAOMapPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    // INPUT: Camera + depth map

    // DEPENDECY: Depth map (zprepass)

    // OUTPUT: AO Texture

    // NOTE:

    // CONDITION: SSAO enabled
    QSSG_ASSERT(camera && rhiDepthTexture && rhiDepthTexture->isValid(), return);

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->debugMarkBegin(QByteArrayLiteral("Quick3D SSAO map"));
    Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D SSAO map"));
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);

    if (Q_LIKELY(rhiAoTexture && rhiAoTexture->isValid())) {
        rhiRenderAoTexture(rhiCtx.get(),
                           this,
                           renderer,
                           *ssaoShaderPipeline,
                           ps,
                           aoSettings,
                           *rhiAoTexture,
                           *rhiDepthTexture,
                           *camera);
    }

    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("ssao_map"));
}

void SSAOMapPass::resetForFrame()
{
    rhiDepthTexture = nullptr;
    rhiAoTexture = nullptr;
    camera = nullptr;
    ps = {};
    aoSettings = {};
}

// DEPTH TEXTURE PASS
void DepthMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    using namespace RenderHelpers;

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    const auto &layerPrepResult = data.layerPrepResult;
    bool ready = false;
    ps = data.getPipelineState();

    if (m_multisampling) {
        ps.samples = rhiCtx->mainPassSampleCount();
        rhiDepthTexture = data.getRenderResult(QSSGRenderResult::Key::DepthTextureMS);
    } else {
        ps.samples = 1;
        rhiDepthTexture = data.getRenderResult(QSSGRenderResult::Key::DepthTexture);
    }

    if (Q_LIKELY(rhiDepthTexture && rhiPrepareDepthTexture(rhiCtx.get(), layerPrepResult.textureDimensions(), rhiDepthTexture, data.layer.viewCount, ps.samples))) {
        sortedOpaqueObjects = data.getSortedOpaqueRenderableObjects(*camera);
        sortedTransparentObjects = data.getSortedTransparentRenderableObjects(*camera);
        // the depth texture is always non-MSAA, but is a 2D array with multiview
        ready = rhiPrepareDepthPass(rhiCtx.get(), this, ps, rhiDepthTexture->rpDesc, data,
                                    sortedOpaqueObjects, sortedTransparentObjects,
                                    ps.samples, data.layer.viewCount);
    }

    if (Q_UNLIKELY(!ready))
        rhiDepthTexture = nullptr;
}

void DepthMapPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    // INPUT: sorted objects (opaque + transparent) (maybe...)

    // DEPENDECY: If this is only used for the AO case, that dictates if this should be done or not.

    // OUTPUT: Texture

    // NOTE: Why are we prepping opaque + transparent object if we're not using them? And why are we staying compatible with 5.15?
    //       Only used for AO? Merge into the AO pass?

    // NOTES:
    //
    // 1: If requested, use this and blit it in the z-pre pass.
    // 2. Why are we handling the transparent objects in the render prep (only)?

    // CONDITION:

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->debugMarkBegin(QByteArrayLiteral("Quick3D depth texture"));

    if (Q_LIKELY(rhiDepthTexture && rhiDepthTexture->isValid())) {
        bool needsSetViewport = true;
        cb->beginPass(rhiDepthTexture->rt, Qt::transparent, { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
        QSSGRHICTX_STAT(rhiCtx, beginRenderPass(rhiDepthTexture->rt));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        // NB! We do not pass sortedTransparentObjects in the 4th
        // argument to stay compatible with the 5.15 code base,
        // which also does not include semi-transparent objects in
        // the depth texture. In addition, capturing after the
        // opaque pass, not including transparent objects, is part
        // of the contract for screen reading custom materials,
        // both for depth and color.
        rhiRenderDepthPass(rhiCtx.get(), ps, sortedOpaqueObjects, {}, &needsSetViewport);
        cb->endPass();
        QSSGRHICTX_STAT(rhiCtx, endRenderPass());
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("depth_texture"));
    }

    cb->debugMarkEnd();
}

void DepthMapPass::resetForFrame()
{
    rhiDepthTexture = nullptr;
    sortedOpaqueObjects.clear();
    sortedTransparentObjects.clear();
    ps = {};
}

// NORMAL TEXTURE PASS

void NormalPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    using namespace RenderHelpers;

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QRhi *rhi = rhiCtx->rhi();
    QSSG_ASSERT(rhi->isRecordingFrame(), return);
    const auto &layerPrepResult = data.layerPrepResult;

    // the normal texture is not multiview-dependent and it is always a 2D texture
    // (so not an array with multiview either)

    // the normal texture is always non-MSAA

    ps = data.getPipelineState();
    ps.samples = 1;
    ps.viewCount = 1;

    sortedOpaqueObjects = data.getSortedOpaqueRenderableObjects(*camera);

    // transparent objects are not included in the normal texture pass

    QSSGShaderFeatures shaderFeatures = data.getShaderFeatures();
    shaderFeatures.set(QSSGShaderFeatures::Feature::NormalPass, true);

    normalTexture = data.getRenderResult(QSSGRenderResult::Key::NormalTexture);

    const QSize size = layerPrepResult.textureDimensions();
    bool needsBuild = false;

    if (!normalTexture->texture) {
        QRhiTexture::Format format = QRhiTexture::RGBA16F;
        if (!rhi->isTextureFormatSupported(format)) {
            qWarning("No float formats, not great");
            format = QRhiTexture::RGBA8;
        }
        normalTexture->texture = rhiCtx->rhi()->newTexture(format, size, 1, QRhiTexture::RenderTarget);
        needsBuild = true;
        normalTexture->texture->setName(QByteArrayLiteral("Normal texture"));
    } else if (normalTexture->texture->pixelSize() != size) {
        normalTexture->texture->setPixelSize(size);
        needsBuild = true;
    }

    if (!normalTexture->depthStencil) {
        normalTexture->depthStencil = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, size);
        needsBuild = true;
    } else if (normalTexture->depthStencil->pixelSize() != size) {
        normalTexture->depthStencil->setPixelSize(size);
        needsBuild = true;
    }

    if (needsBuild) {
        if (!normalTexture->texture->create()) {
            qWarning("Failed to build normal texture (size %dx%d, format %d)",
                     size.width(), size.height(), int(normalTexture->texture->format()));
            normalTexture->reset();
            return;
        }

        if (!normalTexture->depthStencil->create()) {
            qWarning("Failed to build depth-stencil buffer for normal texture (size %dx%d)",
                     size.width(), size.height());
            normalTexture->reset();
            return;
        }

        normalTexture->resetRenderTarget();

        QRhiTextureRenderTargetDescription rtDesc;
        QRhiColorAttachment colorAttachment(normalTexture->texture);
        rtDesc.setColorAttachments({ colorAttachment });
        rtDesc.setDepthStencilBuffer(normalTexture->depthStencil);

        normalTexture->rt = rhi->newTextureRenderTarget(rtDesc);
        normalTexture->rt->setName(QByteArrayLiteral("Normal texture RT"));
        normalTexture->rpDesc = normalTexture->rt->newCompatibleRenderPassDescriptor();
        normalTexture->rt->setRenderPassDescriptor(normalTexture->rpDesc);
        if (!normalTexture->rt->create()) {
            qWarning("Failed to build render target for normal texture");
            normalTexture->reset();
            return;
        }
    }

    rhiPrepareNormalPass(rhiCtx.get(), this, ps, normalTexture->rpDesc, data, sortedOpaqueObjects);
}

void NormalPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->debugMarkBegin(QByteArrayLiteral("Quick3D normal texture"));

    if (Q_LIKELY(normalTexture && normalTexture->isValid())) {
         bool needsSetViewport = true;
         cb->beginPass(normalTexture->rt, Qt::transparent, { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
         QSSGRHICTX_STAT(rhiCtx, beginRenderPass(normalTexture->rt));
         Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);

        rhiRenderNormalPass(rhiCtx.get(), ps, sortedOpaqueObjects, &needsSetViewport);

        cb->endPass();
        QSSGRHICTX_STAT(rhiCtx, endRenderPass());
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("normal_texture"));
    }

    cb->debugMarkEnd();
}

void NormalPass::resetForFrame()
{
    normalTexture = nullptr;
    depthBuffer = nullptr;
    sortedOpaqueObjects.clear();
    ps = {};
}

// SCREEN TEXTURE PASS

void ScreenMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    using namespace RenderHelpers;

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    rhiScreenTexture = data.getRenderResult(QSSGRenderResult::Key::ScreenTexture);
    auto &layer = data.layer;
    const auto &layerPrepResult = data.layerPrepResult;
    wantsMips = layerPrepResult.getFlags().requiresMipmapsForScreenTexture();
    sortedOpaqueObjects = data.getSortedOpaqueRenderableObjects(*camera);
    ps = data.getPipelineState();
    ps.samples = 1; // screen texture is always non-MSAA
    ps.viewCount = data.layer.viewCount; // but is a 2D texture array when multiview

    if (layer.background == QSSGRenderLayer::Background::Color)
        clearColor = QColor::fromRgbF(layer.clearColor.x(), layer.clearColor.y(), layer.clearColor.z());

    if (rhiCtx->rhi()->isFeatureSupported(QRhi::TexelFetch)) {
        if (layer.background == QSSGRenderLayer::Background::SkyBoxCubeMap && layer.skyBoxCubeMap) {
            if (!skyboxCubeMapPass)
                skyboxCubeMapPass = SkyboxCubeMapPass();

            skyboxCubeMapPass->skipTonemapping = true;
            skyboxCubeMapPass->renderPrep(renderer, data);

            // The pass expects to output to the main render target, but we have
            // our own texture here, possibly with a differing sample count, so
            // override the relevant settings once renderPrep() is done.
            skyboxCubeMapPass->ps.samples = ps.samples;

            skyboxPass = std::nullopt;
        } else if (layer.background == QSSGRenderLayer::Background::SkyBox && layer.lightProbe) {
            if (!skyboxPass)
                skyboxPass = SkyboxPass();

            skyboxPass->skipTonemapping = true;
            skyboxPass->renderPrep(renderer, data);

            skyboxPass->ps.samples = ps.samples;

            skyboxCubeMapPass = std::nullopt;
        }
    }

    bool ready = false;
    if (Q_LIKELY(rhiScreenTexture && rhiPrepareScreenTexture(rhiCtx.get(), layerPrepResult.textureDimensions(), wantsMips, rhiScreenTexture, layer.viewCount))) {
        ready = true;
        if (skyboxCubeMapPass)
            skyboxCubeMapPass->rpDesc = rhiScreenTexture->rpDesc;
        if (skyboxPass)
            skyboxPass->rpDesc = rhiScreenTexture->rpDesc;
        // NB: not compatible with disabling LayerEnableDepthTest
        // because there are effectively no "opaque" objects then.
        // Disable Tonemapping for all materials in the screen pass texture
        shaderFeatures = data.getShaderFeatures();
        shaderFeatures.disableTonemapping();
        const auto &sortedOpaqueObjects = data.getSortedOpaqueRenderableObjects(*camera);
        for (const auto &handle : sortedOpaqueObjects) {
            // Reflection cube maps are not available at this point, make sure they are turned off.
            bool recRef = handle.obj->renderableFlags.receivesReflections();
            handle.obj->renderableFlags.setReceivesReflections(false);
            rhiPrepareRenderableForScreenMapPass(rhiCtx.get(), this, data, *handle.obj, rhiScreenTexture->rpDesc, &ps, shaderFeatures, 1, data.layer.viewCount);
            handle.obj->renderableFlags.setReceivesReflections(recRef);
        }
    }

    if (Q_UNLIKELY(!ready))
        rhiScreenTexture = nullptr;
}

void ScreenMapPass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    // INPUT: Sorted opaque objects + depth objects

    // DEPENDECY: Depth pass (if enabled)

    // OUTPUT: Texture (screen texture).

    // NOTE: Used for refrection and effects (?)

    // CONDITION:

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    cb->debugMarkBegin(QByteArrayLiteral("Quick3D screen texture"));

    if (Q_LIKELY(rhiScreenTexture && rhiScreenTexture->isValid())) {
        cb->beginPass(rhiScreenTexture->rt, clearColor, { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
        QSSGRHICTX_STAT(rhiCtx, beginRenderPass(rhiScreenTexture->rt));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);

        bool needsSetViewport = true;
        for (const auto &handle : std::as_const(sortedOpaqueObjects))
            rhiRenderRenderable(rhiCtx.get(), ps, *handle.obj, &needsSetViewport);

        if (skyboxCubeMapPass)
            skyboxCubeMapPass->renderPass(renderer);
        else if (skyboxPass)
            skyboxPass->renderPass(renderer);

        QRhiResourceUpdateBatch *rub = nullptr;
        if (wantsMips) {
            rub = rhiCtx->rhi()->nextResourceUpdateBatch();
            rub->generateMips(rhiScreenTexture->texture);
        }
        cb->endPass(rub);
        QSSGRHICTX_STAT(rhiCtx, endRenderPass());
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("screen_texture"));
    }

    cb->debugMarkEnd();
}

void ScreenMapPass::resetForFrame()
{
    rhiScreenTexture = nullptr;
    if (skyboxPass)
        skyboxPass->resetForFrame();
    if (skyboxCubeMapPass)
        skyboxCubeMapPass->resetForFrame();
    ps = {};
    wantsMips = false;
    clearColor = Qt::transparent;
    shaderFeatures = {};
    sortedOpaqueObjects.clear();
}

void ScreenReflectionPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    rhiScreenTexture = data.getRenderResult(QSSGRenderResult::Key::ScreenTexture);
    QSSG_ASSERT_X(rhiScreenTexture && rhiScreenTexture->isValid(), "Invalid screen texture!", return);

    const auto &layer = data.layer;
    const auto shaderFeatures = data.getShaderFeatures();
    const bool layerEnableDepthTest = layer.layerFlags.testFlag(QSSGRenderLayer::LayerFlag::EnableDepthTest);

    QRhiRenderPassDescriptor *mainRpDesc = rhiCtx->mainRenderPassDescriptor();
    const int samples = rhiCtx->mainPassSampleCount();
    const int viewCount = data.layer.viewCount;

    // NOTE: We're piggybacking on the screen map pass for now, but we could do better.
    ps = data.getPipelineState();
    const bool depthTestEnabled = (data.screenMapPass.ps.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled));
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled, depthTestEnabled);
    const bool depthWriteEnabled = (data.screenMapPass.ps.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled));
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, depthWriteEnabled);
    sortedScreenTextureObjects = data.getSortedScreenTextureRenderableObjects(*camera);
    for (const auto &handle : std::as_const(sortedScreenTextureObjects)) {
        QSSGRenderableObject *theObject = handle.obj;
        const auto depthWriteMode = theObject->depthWriteMode;
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, theObject->renderableFlags.hasTransparency());
        const bool curDepthWriteEnabled = !(depthWriteMode == QSSGDepthDrawMode::Never || depthWriteMode == QSSGDepthDrawMode::OpaquePrePass
                                     || data.isZPrePassActive() || !layerEnableDepthTest);
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, curDepthWriteEnabled);
        RenderHelpers::rhiPrepareRenderable(rhiCtx.get(), this, data, *theObject, mainRpDesc, &ps, shaderFeatures, samples, viewCount);
    }
}

void ScreenReflectionPass::renderPass(QSSGRenderer &renderer)
{
    if (QSSG_GUARD(rhiScreenTexture && rhiScreenTexture->isValid())) {
        const auto &rhiCtx = renderer.contextInterface()->rhiContext();
        QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
        QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

        // 3. Screen texture depended objects
        cb->debugMarkBegin(QByteArrayLiteral("Quick3D render screen texture dependent"));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Quick3D render screen texture dependent"));
        bool needsSetViewport = true;
        for (const auto &handle : std::as_const(sortedScreenTextureObjects)) {
            QSSGRenderableObject *theObject = handle.obj;
            RenderHelpers::rhiRenderRenderable(rhiCtx.get(), ps, *theObject, &needsSetViewport);
        }
        cb->debugMarkEnd();
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("screen_texture_dependent"));
        Q_TRACE(QSSG_renderPass_exit);
    }
}

void ScreenReflectionPass::resetForFrame()
{
    sortedScreenTextureObjects.clear();
    rhiScreenTexture = nullptr;
    ps = {};
}

void OpaquePass::prep(const QSSGRenderContextInterface &ctx,
                      QSSGLayerRenderData &data,
                      QSSGPassKey passKey,
                      QSSGRhiGraphicsPipelineState &ps,
                      QSSGShaderFeatures shaderFeatures,
                      QRhiRenderPassDescriptor *rpDesc,
                      const QSSGRenderableObjectList &sortedOpaqueObjects)
{
    const auto &rhiCtx = ctx.rhiContext();
    QSSG_ASSERT(rpDesc && rhiCtx->rhi()->isRecordingFrame(), return);

    const auto &layer = data.layer;
    const bool layerEnableDepthTest = layer.layerFlags.testFlag(QSSGRenderLayer::LayerFlag::EnableDepthTest);

    for (const auto &handle : std::as_const(sortedOpaqueObjects)) {
        QSSGRenderableObject *theObject = handle.obj;
        const auto depthWriteMode = theObject->depthWriteMode;
        const bool curDepthWriteEnabled = !(depthWriteMode == QSSGDepthDrawMode::Never ||
                                            depthWriteMode == QSSGDepthDrawMode::OpaquePrePass ||
                                            data.isZPrePassActive() || !layerEnableDepthTest);
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, curDepthWriteEnabled);
        RenderHelpers::rhiPrepareRenderable(rhiCtx.get(), passKey, data, *theObject, rpDesc, &ps, shaderFeatures, ps.samples, ps.viewCount);
    }
}

void OpaquePass::render(const QSSGRenderContextInterface &ctx,
                        const QSSGRhiGraphicsPipelineState &ps,
                        const QSSGRenderableObjectList &sortedOpaqueObjects)
{
    const auto &rhiCtx = ctx.rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    bool needsSetViewport = true;
    for (const auto &handle : std::as_const(sortedOpaqueObjects)) {
        QSSGRenderableObject *theObject = handle.obj;
        RenderHelpers::rhiRenderRenderable(rhiCtx.get(), ps, *theObject, &needsSetViewport);
    }
}

void OpaquePass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    auto *ctx = renderer.contextInterface();
    const auto &rhiCtx = ctx->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QSSG_ASSERT(!data.renderedCameras.isEmpty() && data.renderedCameraData.has_value() , return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = data.layer.viewCount;
    ps.depthFunc = QRhiGraphicsPipeline::LessOrEqual;
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, false);

    // opaque objects (or, this list is empty when LayerEnableDepthTest is disabled)
    sortedOpaqueObjects = data.getSortedOpaqueRenderableObjects(*camera);
    shaderFeatures = data.getShaderFeatures();

    QRhiRenderPassDescriptor *mainRpDesc = rhiCtx->mainRenderPassDescriptor();
    prep(*ctx, data, this, ps, shaderFeatures, mainRpDesc, sortedOpaqueObjects);
}

void OpaquePass::renderPass(QSSGRenderer &renderer)
{
    auto *ctx = renderer.contextInterface();
    const auto &rhiCtx = ctx->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    cb->debugMarkBegin(QByteArrayLiteral("Quick3D render opaque"));
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Quick3D render opaque"));
    render(*ctx, ps, sortedOpaqueObjects);
    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("opaque_pass"));
    Q_TRACE(QSSG_renderPass_exit);
}

void OpaquePass::resetForFrame()
{
    sortedOpaqueObjects.clear();
    ps = {};
    shaderFeatures = {};
}

void TransparentPass::prep(const QSSGRenderContextInterface &ctx,
                           QSSGLayerRenderData &data,
                           QSSGPassKey passKey,
                           QSSGRhiGraphicsPipelineState &ps,
                           QSSGShaderFeatures shaderFeatures,
                           QRhiRenderPassDescriptor *rpDesc,
                           const QSSGRenderableObjectList &sortedTransparentObjects,
                           bool oit)
{
    const auto &rhiCtx = ctx.rhiContext();
    QSSG_ASSERT(rpDesc && rhiCtx->rhi()->isRecordingFrame(), return);

    const bool zPrePassActive = data.isZPrePassActive();
    for (const auto &handle : std::as_const(sortedTransparentObjects)) {
        QSSGRenderableObject *theObject = handle.obj;
        const auto depthWriteMode = theObject->depthWriteMode;
        const bool curDepthWriteEnabled = (depthWriteMode == QSSGDepthDrawMode::Always && !zPrePassActive);
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, curDepthWriteEnabled);
        if (!(theObject->renderableFlags.isCompletelyTransparent())) {
            RenderHelpers::rhiPrepareRenderable(rhiCtx.get(), passKey, data, *theObject, rpDesc, &ps, shaderFeatures,
                                                ps.samples, ps.viewCount, nullptr, nullptr, QSSGRenderTextureCubeFaceNone, nullptr, oit);
        }
    }
}

void TransparentPass::render(const QSSGRenderContextInterface &ctx,
                             const QSSGRhiGraphicsPipelineState &ps,
                             const QSSGRenderableObjectList &sortedTransparentObjects)
{
    const auto &rhiCtx = ctx.rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    // If scissorRect is set, Item2Ds will be drawn by a workaround of modifying
    // viewport, not using actual 3D scissor test.
    // It means non-opaque objects may be affected by this viewport setting.
    bool needsSetViewport = true;
    for (const auto &handle : std::as_const(sortedTransparentObjects)) {
        QSSGRenderableObject *theObject = handle.obj;
        if (!theObject->renderableFlags.isCompletelyTransparent())
            RenderHelpers::rhiRenderRenderable(rhiCtx.get(), ps, *theObject, &needsSetViewport);
    }
}

void TransparentPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    auto *ctx = renderer.contextInterface();
    const auto &rhiCtx = ctx->rhiContext();

    QSSG_ASSERT(!data.renderedCameras.isEmpty() && data.renderedCameraData.has_value() , return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    QRhiRenderPassDescriptor *mainRpDesc = rhiCtx->mainRenderPassDescriptor();

    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = data.layer.viewCount;

    // transparent objects (or, without LayerEnableDepthTest, all objects)
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, false);

    shaderFeatures = data.getShaderFeatures();
    sortedTransparentObjects = data.getSortedTransparentRenderableObjects(*camera);

    prep(*ctx, data, this, ps, shaderFeatures, mainRpDesc, sortedTransparentObjects);
}

void TransparentPass::renderPass(QSSGRenderer &renderer)
{
    auto *ctx = renderer.contextInterface();
    const auto &rhiCtx = ctx->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    cb->debugMarkBegin(QByteArrayLiteral("Quick3D render alpha"));
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Quick3D render alpha"));
    render(*ctx, ps, sortedTransparentObjects);
    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("transparent_pass"));
    Q_TRACE(QSSG_renderPass_exit);
}

void TransparentPass::resetForFrame()
{
    sortedTransparentObjects.clear();
    ps = {};
    shaderFeatures = {};
}

void SkyboxPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    if (!skipPrep) {
        const auto &rhiCtx = renderer.contextInterface()->rhiContext();
        QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
        QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
        QSSG_ASSERT(data.renderedCameras.count() == data.layer.viewCount, return);
        layer = &data.layer;
        QSSG_ASSERT(layer, return);

        rpDesc = rhiCtx->mainRenderPassDescriptor();
        ps = data.getPipelineState();
        ps.samples = rhiCtx->mainPassSampleCount();
        ps.viewCount = data.layer.viewCount;
        ps.polygonMode = QRhiGraphicsPipeline::Fill;

        // When there are effects, then it is up to the last pass of the
        // last effect to perform tonemapping, neither the skybox nor the
        // main render pass should alter the colors then.
        skipTonemapping = layer->firstEffect != nullptr;

        RenderHelpers::rhiPrepareSkyBox(rhiCtx.get(), this, *layer, data.renderedCameras, renderer);
        skipPrep = true;
    }
}

void SkyboxPass::renderPass(QSSGRenderer &renderer)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QSSG_ASSERT(layer, return);

    QRhiShaderResourceBindings *srb = layer->skyBoxSrb;
    QSSG_ASSERT(srb, return);

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D render skybox"));

    // Note: We get the shader here, as the screen map pass might modify the state of
    // the tonemap mode.

    QSSGRenderLayer::TonemapMode tonemapMode = skipTonemapping && (layer->tonemapMode != QSSGRenderLayer::TonemapMode::Custom) ?  QSSGRenderLayer::TonemapMode::None : layer->tonemapMode;
    const auto &shaderCache = renderer.contextInterface()->shaderCache();
    auto shaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiSkyBoxShader(tonemapMode, layer->skyBoxIsRgbe8, layer->viewCount);
    QSSG_CHECK(shaderPipeline);
    QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, shaderPipeline.get());
    renderer.rhiQuadRenderer()->recordRenderQuad(rhiCtx.get(), &ps, srb, rpDesc, { QSSGRhiQuadRenderer::DepthTest | QSSGRhiQuadRenderer::RenderBehind });
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("skybox_map"));
}

void SkyboxPass::resetForFrame()
{
    ps = {};
    layer = nullptr;
    skipPrep = false;
}

void SkyboxCubeMapPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSG_ASSERT(data.renderedCameras.count() == data.layer.viewCount, return);
    layer = &data.layer;
    QSSG_ASSERT(layer, return);

    rpDesc = rhiCtx->mainRenderPassDescriptor();
    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = data.layer.viewCount;
    ps.polygonMode = QRhiGraphicsPipeline::Fill;

    QSSGRenderLayer::TonemapMode tonemapMode = skipTonemapping && (layer->tonemapMode != QSSGRenderLayer::TonemapMode::Custom) ?  QSSGRenderLayer::TonemapMode::None : layer->tonemapMode;

    const auto &shaderCache = renderer.contextInterface()->shaderCache();
    skyBoxCubeShader = shaderCache->getBuiltInRhiShaders().getRhiSkyBoxCubeShader(tonemapMode, !data.layer.skyBoxIsSrgb, data.layer.viewCount);

    RenderHelpers::rhiPrepareSkyBox(rhiCtx.get(), this, *layer, data.renderedCameras, renderer, uint(tonemapMode));
}

void SkyboxCubeMapPass::renderPass(QSSGRenderer &renderer)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QSSG_ASSERT(layer && skyBoxCubeShader, return);

    QRhiShaderResourceBindings *srb = layer->skyBoxSrb;
    QSSG_ASSERT(srb, return);

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D render cubemap skybox"));

    QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, skyBoxCubeShader.get());
    renderer.rhiCubeRenderer()->recordRenderCube(rhiCtx.get(), &ps, srb, rpDesc, { QSSGRhiQuadRenderer::DepthTest | QSSGRhiQuadRenderer::RenderBehind });
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("skybox_cube"));
}

void SkyboxCubeMapPass::resetForFrame()
{
    ps = {};
    layer = nullptr;
}

void Item2DPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    const auto &layer = data.layer;

    const auto &item2Ds = data.getRenderableItem2Ds();
    prepdItem2DRenderers.reserve(size_t(item2Ds.size()));
    // NOTE: This marks the start of the 2D sub-scene rendering as it might result in
    // a nested 3D scene to be rendered and if we don't save the state here, we can
    // end up with a mismatched state in the QtQuick3D renderer.
    // See the end of this function for the corresponding end call (endSubLayerRender()).
    renderer.beginSubLayerRender(data);
    for (const auto *item2D: std::as_const(item2Ds)) {
        // Find data for item
        auto item2DData = data.getItem2DRenderer(*item2D);
        const auto &mvps = data.getItem2DMvps(*item2D);
        QSGRenderer *renderer2d = item2DData;

        QSSG_ASSERT(renderer2d, continue);

        // NOTE: We shouldn't get into this state...
        if (renderer2d && renderer2d->currentRhi() != rhiCtx->rhi()) {
            static bool contextWarningShown = false;
            if (!contextWarningShown) {
                qWarning () << "Scene with embedded 2D content can only be rendered in one window.";
                contextWarningShown = true;
            }
            continue;
        }

        // Set the projection matrix

        auto layerPrepResult = data.layerPrepResult;

        QRhiRenderTarget *renderTarget = rhiCtx->renderTarget();
        auto *rpd = renderTarget->renderPassDescriptor();
        renderer2d->setDevicePixelRatio(renderTarget->devicePixelRatio());
        const QRect deviceRect(QPoint(0, 0), renderTarget->pixelSize());
        const int viewCount = data.layer.viewCount;
        if (layer.scissorRect.isValid()) {
            QRect effScissor = layer.scissorRect & layerPrepResult.getViewport().toRect();
            QMatrix4x4 correctionMat = correctMVPForScissor(layerPrepResult.getViewport(),
                                                            effScissor,
                                                            rhiCtx->rhi()->isYUpInNDC());
            for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex) {
                const QMatrix4x4 projectionMatrix = correctionMat * mvps[viewIndex];
                renderer2d->setProjectionMatrix(projectionMatrix, viewIndex);
            }
            renderer2d->setViewportRect(effScissor);
        } else {
            for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex)
                renderer2d->setProjectionMatrix(mvps[viewIndex], viewIndex);
            renderer2d->setViewportRect(RenderHelpers::correctViewportCoordinates(layerPrepResult.getViewport(), deviceRect));
        }
        renderer2d->setDeviceRect(deviceRect);
        QSGRenderTarget sgRt(renderTarget, rpd, rhiCtx->commandBuffer());
        sgRt.multiViewCount = data.layer.viewCount;
        renderer2d->setRenderTarget(sgRt);
        renderer2d->prepareSceneInline();
        prepdItem2DRenderers.push_back(renderer2d);
    }
    renderer.endSubLayerRender(data);
}

void Item2DPass::renderPass(QSSGRenderer &renderer)
{
    QSSG_ASSERT(prepdItem2DRenderers.size() > 0, return);

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    cb->debugMarkBegin(QByteArrayLiteral("Quick3D render 2D sub-scene"));
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D render 2D sub-scene"));
    QSSGLayerRenderData *data = QSSGLayerRenderData::getCurrent(renderer);
    renderer.beginSubLayerRender(*data);
    for (QSGRenderer *renderer2d : std::as_const(prepdItem2DRenderers))
        renderer2d->renderSceneInline();
    renderer.endSubLayerRender(*data);
    cb->debugMarkEnd();
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("2D_sub_scene"));
}

void Item2DPass::resetForFrame()
{
    prepdItem2DRenderers.clear();
}

void InfiniteGridPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSG_ASSERT(data.renderedCameras.count() == data.layer.viewCount, return);
    layer = &data.layer;
    QSSG_ASSERT(layer, return);

    const auto &shaderCache = renderer.contextInterface()->shaderCache();
    gridShader = shaderCache->getBuiltInRhiShaders().getRhiGridShader(data.layer.viewCount);

    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = data.layer.viewCount;
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
    ps.polygonMode = QRhiGraphicsPipeline::Fill;

    RenderHelpers::rhiPrepareGrid(rhiCtx.get(), this, *layer, data.renderedCameras, renderer);
}

void InfiniteGridPass::renderPass(QSSGRenderer &renderer)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(gridShader && rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    cb->debugMarkBegin(QByteArrayLiteral("Quick3D render grid"));
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
    Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick3D render grid"));
    QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, gridShader.get());
    QRhiShaderResourceBindings *srb = layer->gridSrb;
    QRhiRenderPassDescriptor *rpDesc = rhiCtx->mainRenderPassDescriptor();
    renderer.rhiQuadRenderer()->recordRenderQuad(rhiCtx.get(), &ps, srb, rpDesc, { QSSGRhiQuadRenderer::DepthTest });
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("render_grid"));
}

void InfiniteGridPass::resetForFrame()
{
    ps = {};
    layer = nullptr;
}

void DebugDrawPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());
    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    QSSG_ASSERT(data.renderedCameras.count() == data.layer.viewCount, return);

    const auto &shaderCache = renderer.contextInterface()->shaderCache();
    debugObjectShader = shaderCache->getBuiltInRhiShaders().getRhiDebugObjectShader(data.layer.viewCount);
    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = data.layer.viewCount;

    // debug objects
    const auto &debugDraw = renderer.contextInterface()->debugDrawSystem();
    if (debugDraw && debugDraw->hasContent()) {
        QRhi *rhi = rhiCtx->rhi();
        QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
        debugDraw->prepareGeometry(rhiCtx.get(), rub);
        QSSGRhiDrawCallData &dcd = rhiCtxD->drawCallData({ this, nullptr, nullptr, 0 });
        if (!dcd.ubuf) {
            dcd.ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64 * data.renderedCameras.count());
            dcd.ubuf->create();
        }
        char *ubufData = dcd.ubuf->beginFullDynamicBufferUpdateForCurrentFrame();
        QMatrix4x4 viewProjection(Qt::Uninitialized);
        QMatrix4x4 cameraGlobalTransform(Qt::Uninitialized);
        for (qsizetype viewIdx = 0; viewIdx < data.renderedCameras.count(); ++viewIdx) {
            cameraGlobalTransform = data.getGlobalTransform(*data.renderedCameras[viewIdx]);
            data.renderedCameras[viewIdx]->calculateViewProjectionMatrix(cameraGlobalTransform, viewProjection);
            viewProjection = rhi->clipSpaceCorrMatrix() * viewProjection;
            memcpy(ubufData, viewProjection.constData() + viewIdx * 64, 64);
        }
        dcd.ubuf->endFullDynamicBufferUpdateForCurrentFrame();

        QSSGRhiShaderResourceBindingList bindings;
        bindings.addUniformBuffer(0, QRhiShaderResourceBinding::VertexStage, dcd.ubuf);
        dcd.srb = rhiCtxD->srb(bindings);

        rhiCtx->commandBuffer()->resourceUpdate(rub);
    }
}

void DebugDrawPass::renderPass(QSSGRenderer &renderer)
{
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(debugObjectShader && rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());

    const auto &debugDraw = renderer.contextInterface()->debugDrawSystem();
    if (debugDraw && debugDraw->hasContent()) {
        cb->debugMarkBegin(QByteArrayLiteral("Quick 3D debug objects"));
        Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Quick 3D debug objects"));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, debugObjectShader.get());
        QSSGRhiDrawCallData &dcd = rhiCtxD->drawCallData({ this, nullptr, nullptr, 0 });
        QRhiShaderResourceBindings *srb = dcd.srb;
        QRhiRenderPassDescriptor *rpDesc = rhiCtx->mainRenderPassDescriptor();
        debugDraw->recordRenderDebugObjects(rhiCtx.get(), &ps, srb, rpDesc);
        cb->debugMarkEnd();
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("debug_objects"));
    }
}

void DebugDrawPass::resetForFrame()
{
    ps = {};
}

void UserExtensionPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    Q_UNUSED(renderer);
    auto &frameData = data.getFrameData();
    for (const auto &p : std::as_const(extensions)) {
        p->prepareRender(frameData);
        if (p->mode() == QSSGRenderExtension::RenderMode::Standalone)
            p->render(frameData);
    }
}

void UserExtensionPass::renderPass(QSSGRenderer &renderer)
{
    auto *data = QSSGLayerRenderData::getCurrent(renderer);
    QSSG_ASSERT(data, return);
    auto &frameData = data->getFrameData();
    for (const auto &p : std::as_const(extensions)) {
        if (p->mode() == QSSGRenderExtension::RenderMode::Main)
            p->render(frameData);
    }
}

void UserExtensionPass::resetForFrame()
{
    for (const auto &p : std::as_const(extensions))
        p->resetForFrame();

    // TODO: We should track if we need to update this list.
    extensions.clear();
}

static quint32 nextMultipleOf(quint32 value, quint32 multiple)
{
    return multiple * ((value / multiple) + 1);
}

static quint32 ensureFreeNodes(quint32 value, quint32 multiple)
{
    quint32 multipleOf = nextMultipleOf(value, multiple);
    if (multipleOf - value < multiple)
        multipleOf += multiple;
    return multipleOf;
}

void OITRenderPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    auto *ctx = renderer.contextInterface();
    const auto &rhiCtx = ctx->rhiContext();
    auto *rhi = rhiCtx->rhi();

    QSSG_ASSERT(!data.renderedCameras.isEmpty() && data.renderedCameraData.has_value() , return);
    QSSGRenderCamera *camera = data.renderedCameras[0];

    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = rhiCtx->mainPassViewCount();

    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
    ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, false);

    shaderFeatures = data.getShaderFeatures();
    sortedTransparentObjects = data.getSortedTransparentRenderableObjects(*camera);

    if (method == QSSGRenderLayer::OITMethod::WeightedBlended) {
        ps.colorAttachmentCount = 2;

        rhiAccumTexture = data.getRenderResult(QSSGRenderResult::Key::AccumTexture);
        rhiRevealageTexture = data.getRenderResult(QSSGRenderResult::Key::RevealageTexture);
        if (ps.samples > 1)
            rhiDepthTexture = data.getRenderResult(QSSGRenderResult::Key::DepthTextureMS);
        else
            rhiDepthTexture = data.getRenderResult(QSSGRenderResult::Key::DepthTexture);
        if (!rhiDepthTexture->isValid())
            return;
        auto &oitrt = data.getOitRenderContext();
        if (!oitrt.oitRenderTarget || oitrt.oitRenderTarget->pixelSize() != data.layerPrepResult.textureDimensions()
            || rhiDepthTexture->texture != oitrt.oitRenderTarget->description().depthTexture()
            || ps.samples != oitrt.oitRenderTarget->sampleCount()) {
            if (oitrt.oitRenderTarget) {
                rhiAccumTexture->texture->destroy();
                rhiRevealageTexture->texture->destroy();
                oitrt.oitRenderTarget->destroy();
                oitrt.renderPassDescriptor->destroy();
                oitrt.oitRenderTarget = nullptr;
            }
            const QRhiTexture::Flags textureFlags = QRhiTexture::RenderTarget;
            if (ps.viewCount >= 2) {
                rhiAccumTexture->texture = rhi->newTextureArray(QRhiTexture::RGBA16F, ps.viewCount, data.layerPrepResult.textureDimensions(), ps.samples, textureFlags);
                rhiRevealageTexture->texture = rhi->newTextureArray(QRhiTexture::R16F, ps.viewCount, data.layerPrepResult.textureDimensions(), ps.samples, textureFlags);
            } else {
                rhiAccumTexture->texture = rhi->newTexture(QRhiTexture::RGBA16F, data.layerPrepResult.textureDimensions(), ps.samples, textureFlags);
                rhiRevealageTexture->texture = rhi->newTexture(QRhiTexture::R16F, data.layerPrepResult.textureDimensions(), ps.samples, textureFlags);
            }
            rhiAccumTexture->texture->create();
            rhiRevealageTexture->texture->create();

            QRhiTextureRenderTargetDescription desc;
            desc.setColorAttachments({{rhiAccumTexture->texture}, {rhiRevealageTexture->texture}});
            desc.setDepthTexture(rhiDepthTexture->texture);

            if (oitrt.oitRenderTarget == nullptr) {
                oitrt.oitRenderTarget = rhi->newTextureRenderTarget(desc, QRhiTextureRenderTarget::PreserveDepthStencilContents);
                oitrt.renderPassDescriptor = oitrt.oitRenderTarget->newCompatibleRenderPassDescriptor();
                oitrt.oitRenderTarget->setRenderPassDescriptor(oitrt.renderPassDescriptor);
                oitrt.oitRenderTarget->create();

                renderTarget = oitrt.oitRenderTarget;
            }
        }
        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());
        const auto &shaderCache = renderer.contextInterface()->shaderCache();
        clearPipeline = shaderCache->getBuiltInRhiShaders().getRhiClearMRTShader();

        QSSGRhiShaderResourceBindingList bindings;
        QVector4D clearData[2];
        clearData[0] = QVector4D(0.0, 0.0, 0.0, 0.0);
        clearData[1] = QVector4D(1.0, 1.0, 1.0, 1.0);

        QSSGRhiDrawCallData &dcd(rhiCtxD->drawCallData({ this, clearPipeline.get(), nullptr, 0 }));
        QRhiBuffer *&ubuf = dcd.ubuf;
        const int ubufSize = sizeof(clearData);
        if (!ubuf) {
            ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
            ubuf->create();
        }

        QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
        rub->updateDynamicBuffer(ubuf, 0, ubufSize, &clearData);
        renderer.rhiQuadRenderer()->prepareQuad(rhiCtx.get(), rub);

        bindings.addUniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, ubuf);

        clearSrb = rhiCtxD->srb(bindings);

        ps.targetBlend[0].srcAlpha = QRhiGraphicsPipeline::One;
        ps.targetBlend[0].srcColor = QRhiGraphicsPipeline::One;
        ps.targetBlend[0].dstAlpha = QRhiGraphicsPipeline::One;
        ps.targetBlend[0].dstColor = QRhiGraphicsPipeline::One;
        ps.targetBlend[1].srcAlpha = QRhiGraphicsPipeline::Zero;
        ps.targetBlend[1].srcColor = QRhiGraphicsPipeline::Zero;
        ps.targetBlend[1].dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        ps.targetBlend[1].dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;

        TransparentPass::prep(*ctx, data, this, ps, shaderFeatures, oitrt.renderPassDescriptor, sortedTransparentObjects, true);
    } else if (method == QSSGRenderLayer::OITMethod::LinkedList) {
        if (this->rub) {
            rhiCtx->commandBuffer()->resourceUpdate(this->rub);
            this->rub = nullptr;
        }
        // same as transparent pass
        // transparent objects (or, without LayerEnableDepthTest, all objects)
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, false);
        ps.targetBlend[0].srcAlpha = QRhiGraphicsPipeline::One;
        ps.targetBlend[0].srcColor = QRhiGraphicsPipeline::SrcAlpha;
        ps.targetBlend[0].dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        ps.targetBlend[0].dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;

        shaderFeatures = data.getShaderFeatures();
        sortedTransparentObjects = data.getSortedTransparentRenderableObjects(*camera);

#ifdef QSSG_OIT_USE_BUFFERS
        auto &oitCtx = data.getOitRenderContext();
        rhiABuffer = oitCtx.aBuffer;
        rhiAuxBuffer = oitCtx.auxBuffer;
        rhiCounterBuffer = oitCtx.counterBuffer;
#else
        rhiABufferImage = data.getRenderResult(QSSGRenderResult::Key::ABufferImage);
        rhiAuxiliaryImage = data.getRenderResult(QSSGRenderResult::Key::AuxiliaryImage);
        rhiCounterImage = data.getRenderResult(QSSGRenderResult::Key::CounterImage);
#endif
        QSize dim = data.layerPrepResult.textureDimensions();
        dim.setWidth(dim.width() * ps.samples);
        dim.setHeight(dim.height() * ps.viewCount);
#ifdef QSSG_OIT_USE_BUFFERS
        if (!rhiAuxBuffer || rhiAuxBuffer->size() != (dim.width() * dim.height() * 4u) || currentNodeCount == 0 || currentNodeCount != reportedNodeCount)
#else
        if (!rhiAuxiliaryImage->texture || rhiAuxiliaryImage->texture->pixelSize() != dim || currentNodeCount == 0 || currentNodeCount != reportedNodeCount)
#endif
        {
            quint32 extraNodeCount = 0;
#ifdef QSSG_OIT_USE_BUFFERS
            if (rhiAuxBuffer && rhiAuxBuffer->size() != (dim.width() * dim.height() * 4u)) {
                if (rhiAuxBuffer->size() < (dim.width() * dim.height() * 4u))
                    extraNodeCount = ensureFreeNodes((dim.width() * dim.height() * 4u) - rhiAuxBuffer->size(), 32u * 1024u);
                rhiAuxBuffer->destroy();
                rhiAuxBuffer = nullptr;
            }
#else
            if (rhiABufferImage->texture) {
                const auto s = rhiAuxiliaryImage->texture->pixelSize();
                if (s.width() * s.height() < dim.width() * dim.height())
                    extraNodeCount = ensureFreeNodes(s.width() * s.height() - dim.width() * dim.height(), 32u * 1024u);
                rhiABufferImage->texture->destroy();
                rhiAuxiliaryImage->texture->destroy();
            }
#endif

            if (reportedNodeCount) {
                currentNodeCount = reportedNodeCount + extraNodeCount;
            } else {
                quint32 size = RenderHelpers::rhiCalculateABufferSize(data.layerPrepResult.textureDimensions(), 4 * ps.samples * ps.viewCount);
                currentNodeCount = ensureFreeNodes(size * size, 32u * 1024u);
            }
            data.layer.oitNodeCount = currentNodeCount;

#ifdef QSSG_OIT_USE_BUFFERS
            if (rhiABuffer && currentNodeCount * 16 != rhiABuffer->size()) {
                rhiABuffer->destroy();
                rhiABuffer = nullptr;
            }
            if (!rhiABuffer) {
                rhiABuffer = rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, currentNodeCount * 16);
                rhiABuffer->create();
            }
            if (!rhiAuxBuffer) {
                rhiAuxBuffer = rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, dim.width() * dim.height() * 4);
                rhiAuxBuffer->create();
            }
            if (!rhiCounterBuffer) {
                rhiCounterBuffer = rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, 4);
                rhiCounterBuffer->create();
                oitCtx.counterBuffer = rhiCounterBuffer;
            }
            oitCtx.aBuffer = rhiABuffer;
            oitCtx.auxBuffer = rhiAuxBuffer;
#else
            quint32 sizeWithLayers = RenderHelpers::rhiCalculateABufferSize(currentNodeCount);
            const QRhiTexture::Flags textureFlags = QRhiTexture::UsedWithLoadStore;
            rhiABufferImage->texture = rhi->newTexture(QRhiTexture::RGBA32UI, QSize(sizeWithLayers, sizeWithLayers), 1, textureFlags);
            rhiABufferImage->texture->create();
            rhiAuxiliaryImage->texture = rhi->newTexture(QRhiTexture::R32UI, dim, 1, textureFlags);
            rhiAuxiliaryImage->texture->create();
            if (!rhiCounterImage->texture) {
                rhiCounterImage->texture = rhi->newTexture(QRhiTexture::R32UI, QSize(1, 1), 1, textureFlags | QRhiTexture::UsedAsTransferSource);
                rhiCounterImage->texture->create();

                auto &oitrt = data.getOitRenderContext();
                readbackImage = rhi->newTexture(QRhiTexture::R32UI, QSize(1, 1), 1, QRhiTexture::UsedAsTransferSource);
                readbackImage->create();
                oitrt.copyTexture = readbackImage;
            }
#endif
        }
        QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();

        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());
        const auto &shaderCache = renderer.contextInterface()->shaderCache();
#ifdef QSSG_OIT_USE_BUFFERS
        clearPipeline = shaderCache->getBuiltInRhiShaders().getRhiClearBufferShader();
#else
        clearPipeline = shaderCache->getBuiltInRhiShaders().getRhiClearImageShader();
#endif
        QSSGRhiShaderResourceBindingList bindings;
        quint32 clearImageData[8] = {0};

        QSSGRhiDrawCallData &dcd(rhiCtxD->drawCallData({ this, clearPipeline.get(), nullptr, 0 }));
        QRhiBuffer *&ubuf = dcd.ubuf;
        const int ubufSize = sizeof(clearImageData);
        if (!ubuf) {
            ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
            ubuf->create();
        }

        clearImageData[4] = data.layerPrepResult.textureDimensions().width();
        clearImageData[5] = data.layerPrepResult.textureDimensions().height();
        clearImageData[6] = ps.samples;
        clearImageData[7] = ps.viewCount;

        rub->updateDynamicBuffer(ubuf, 0, ubufSize, clearImageData);
        quint32 zero = 0;
#ifdef QSSG_OIT_USE_BUFFERS
        rub->uploadStaticBuffer(rhiCounterBuffer, 0, 4, &zero);
#else
        rub->uploadTexture(rhiCounterImage->texture, {{0, 0, {&zero, 4}}});
#endif

        bindings.addUniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, ubuf);
#ifdef QSSG_OIT_USE_BUFFERS
        bindings.addStorageBuffer(1, QRhiShaderResourceBinding::FragmentStage, rhiAuxBuffer);
#else
        bindings.addImageStore(1, QRhiShaderResourceBinding::FragmentStage, rhiAuxiliaryImage->texture, 0);
#endif

        clearSrb = rhiCtxD->srb(bindings);

        renderer.rhiQuadRenderer()->prepareQuad(rhiCtx.get(), rub);

        TransparentPass::prep(*ctx, data, this, ps, shaderFeatures, rhiCtx->mainRenderPassDescriptor(), sortedTransparentObjects, true);
    }
}

void OITRenderPass::renderPass(QSSGRenderer &renderer)
{
    auto *ctx = renderer.contextInterface();
    const auto &rhiCtx = ctx->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    if (method == QSSGRenderLayer::OITMethod::WeightedBlended) {
        if (Q_LIKELY(renderTarget)) {
            cb->beginPass(renderTarget, Qt::black, {});

            QRhiShaderResourceBindings *srb = clearSrb;
            QSSG_ASSERT(srb, return);
            ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, false);
            QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, clearPipeline.get());
            renderer.rhiQuadRenderer()->recordRenderQuad(rhiCtx.get(), &ps, srb, renderTarget->renderPassDescriptor(), {});
            ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);

            cb->debugMarkBegin(QByteArrayLiteral("Quick3D render order-independent alpha"));
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
            Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Quick3D render order-independent alpha"));
            ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled, true);
            ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, false);
            TransparentPass::render(*ctx, ps, sortedTransparentObjects);
            cb->debugMarkEnd();
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("transparent_order_independent_pass"));
            Q_TRACE(QSSG_renderPass_exit);

            cb->endPass();
        }
    } else if (method == QSSGRenderLayer::OITMethod::LinkedList) {
        cb->debugMarkBegin(QByteArrayLiteral("Quick3D render alpha"));
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        Q_TRACE(QSSG_renderPass_entry, QStringLiteral("Quick3D render alpha"));

        QRhiShaderResourceBindings *srb = clearSrb;
        QSSG_ASSERT(srb, return);
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, clearPipeline.get());
        renderer.rhiQuadRenderer()->recordRenderQuad(rhiCtx.get(), &ps, srb, rhiCtx->mainRenderPassDescriptor(), {});

        TransparentPass::render(*ctx, ps, sortedTransparentObjects);
#ifdef QSSG_OIT_USE_BUFFERS
        QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
        QRhiReadbackResult *result = nullptr;
        if (results.size() > 1) {
            result = results.takeLast();
            result->pixelSize = {};
            result->data = {};
            result->format = QRhiTexture::UnknownFormat;
        }  else  {
            result = new QRhiReadbackResult();
        }
        const auto completedFunc = [this, result](){
            if (result) {
                const quint32 *d = reinterpret_cast<const quint32 *>(result->data.constData());
                quint32 nodeCount = *d;
                if (nodeCount)
                    this->reportedNodeCount = ensureFreeNodes(nodeCount, 32u * 1024u);
                this->results.append(result);
            }
        };
        result->completed = completedFunc;
        rub->readBackBuffer(rhiCounterBuffer, 0, 4, result);
        this->rub = rub;
#else
        QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
        rub->copyTexture(readbackImage, rhiCounterImage->texture);
        QRhiReadbackDescription rbdesc;
        rbdesc.setTexture(readbackImage);
        QRhiReadbackResult *result = nullptr;
        if (results.size() > 1) {
            result = results.takeLast();
            result->pixelSize = {};
            result->data = {};
            result->format = QRhiTexture::UnknownFormat;
        }  else  {
            result = new QRhiReadbackResult();
        }
        const auto completedFunc = [this, result](){
            if (result) {
                const quint32 *d = reinterpret_cast<const quint32 *>(result->data.constData());
                quint32 nodeCount = *d;
                if (nodeCount)
                    this->reportedNodeCount = ensureFreeNodes(nodeCount, 32u * 1024u);
                this->results.append(result);
            }
        };
        result->completed = completedFunc;
        rub->readBackTexture(rbdesc, result);
        this->rub = rub;
#endif
        cb->debugMarkEnd();
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("transparent_order_independent_pass"));
        Q_TRACE(QSSG_renderPass_exit);
    }
}

QSSGRenderPass::Type OITRenderPass::passType() const
{
    if (method == QSSGRenderLayer::OITMethod::WeightedBlended)
        return Type::Standalone;
    return Type::Main;
}


void OITRenderPass::resetForFrame()
{
    sortedTransparentObjects.clear();
    ps = {};
    shaderFeatures = {};
    rhiAccumTexture = nullptr;
    rhiRevealageTexture = nullptr;
    rhiDepthTexture = nullptr;
    rhiCounterImage = nullptr;
}

void OITCompositePass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    using namespace RenderHelpers;

    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    const auto &shaderCache = renderer.contextInterface()->shaderCache();

    ps = data.getPipelineState();
    ps.samples = rhiCtx->mainPassSampleCount();
    ps.viewCount = rhiCtx->mainPassViewCount();

    if (method == QSSGRenderLayer::OITMethod::WeightedBlended) {
        rhiAccumTexture = data.getRenderResult(QSSGRenderResult::Key::AccumTexture);
        rhiRevealageTexture = data.getRenderResult(QSSGRenderResult::Key::RevealageTexture);
        compositeShaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiOitCompositeShader(method, ps.samples > 1 ? true : false);
    } else if (method == QSSGRenderLayer::OITMethod::LinkedList) {
#ifdef QSSG_OIT_USE_BUFFERS
        rhiABuffer = data.getOitRenderContext().aBuffer;
        rhiAuxBuffer = data.getOitRenderContext().auxBuffer;
        compositeShaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiOitCompositeShader(method, ps.samples > 1 ? true : false, true);
#else
        rhiABufferImage = data.getRenderResult(QSSGRenderResult::Key::ABufferImage);
        rhiAuxiliaryImage = data.getRenderResult(QSSGRenderResult::Key::AuxiliaryImage);
        compositeShaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiOitCompositeShader(method, ps.samples > 1 ? true : false);
#endif

        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());
        QSSGRhiDrawCallData &dcd(rhiCtxD->drawCallData({ this, nullptr, nullptr, 0 }));
        QRhiBuffer *&ubuf = dcd.ubuf;
        const int ubufSize = 6 * sizeof(quint32);
        if (!ubuf) {
            ubuf = rhiCtx->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
            ubuf->create();
        }
#ifdef QSSG_OIT_USE_BUFFERS
        const quint32 values[6] = { 0,
                                    quint32(data.layer.oitNodeCount),
                                    quint32(ps.samples),
                                    0,
                                    quint32(data.layerPrepResult.textureDimensions().width()),
                                    quint32(data.layerPrepResult.textureDimensions().height())
        };
#else
        const quint32 sizeWithLayers = rhiABufferImage->texture->pixelSize().width();
        const quint32 values[6] = { sizeWithLayers,
                                    sizeWithLayers * sizeWithLayers,
                                    quint32(ps.samples),
                                    0,
                                    quint32(data.layerPrepResult.textureDimensions().width()),
                                    quint32(data.layerPrepResult.textureDimensions().height())
                                    };
#endif
        QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
        rub->updateDynamicBuffer(ubuf, 0, ubufSize, values);
        renderer.rhiQuadRenderer()->prepareQuad(rhiCtx.get(), rub);
    }
}

void OITCompositePass::renderPass(QSSGRenderer &renderer)
{
    using namespace RenderHelpers;

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());

    if (!rhiAccumTexture->texture || !rhiRevealageTexture->texture)
        return;

    if (method == QSSGRenderLayer::OITMethod::WeightedBlended) {
        QSSGRhiShaderResourceBindingList bindings;

        QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                 QRhiSampler::Nearest,
                                                 QRhiSampler::None,
                                                 QRhiSampler::ClampToEdge,
                                                 QRhiSampler::ClampToEdge,
                                                 QRhiSampler::ClampToEdge });
        bindings.addTexture(1, QRhiShaderResourceBinding::FragmentStage, rhiAccumTexture->texture, sampler);
        bindings.addTexture(2, QRhiShaderResourceBinding::FragmentStage, rhiRevealageTexture->texture, sampler);

        compositeSrb = rhiCtxD->srb(bindings);

        QRhiShaderResourceBindings *srb = compositeSrb;
        QSSG_ASSERT(srb, return);

        cb->debugMarkBegin(QByteArrayLiteral("Quick3D revealage"));
        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, compositeShaderPipeline.get());
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
        renderer.rhiQuadRenderer()->recordRenderQuad(rhiCtx.get(), &ps, srb, rhiCtx->mainRenderPassDescriptor(),
                                                     { QSSGRhiQuadRenderer::UvCoords | QSSGRhiQuadRenderer::DepthTest | QSSGRhiQuadRenderer::PremulBlend});
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("revealage"));
        cb->debugMarkEnd();
    } else if (method == QSSGRenderLayer::OITMethod::LinkedList) {
        QSSGRhiShaderResourceBindingList bindings;

        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());
        QSSGRhiDrawCallData &dcd(rhiCtxD->drawCallData({ this, nullptr, nullptr, 0 }));
        QRhiBuffer *&ubuf = dcd.ubuf;

        bindings.addUniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, ubuf);
#ifdef QSSG_OIT_USE_BUFFERS
        bindings.addStorageBuffer(1, QRhiShaderResourceBinding::FragmentStage, rhiABuffer);
        bindings.addStorageBuffer(2, QRhiShaderResourceBinding::FragmentStage, rhiAuxBuffer);
#else
        bindings.addImageLoad(1, QRhiShaderResourceBinding::FragmentStage, rhiABufferImage->texture, 0);
        bindings.addImageLoad(2, QRhiShaderResourceBinding::FragmentStage, rhiAuxiliaryImage->texture, 0);
#endif

        compositeSrb = rhiCtxD->srb(bindings);

        QRhiShaderResourceBindings *srb = compositeSrb;
        QSSG_ASSERT(srb, return);

        cb->debugMarkBegin(QByteArrayLiteral("Quick3D oit-composite"));
        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, compositeShaderPipeline.get());
        ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, true);
        renderer.rhiQuadRenderer()->recordRenderQuad(rhiCtx.get(), &ps, srb, rhiCtx->mainRenderPassDescriptor(),
                                                     { QSSGRhiQuadRenderer::UvCoords | QSSGRhiQuadRenderer::DepthTest | QSSGRhiQuadRenderer::PremulBlend});
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("oit-composite"));
    }
}

void OITCompositePass::resetForFrame()
{
    ps = {};
    shaderFeatures = {};
    rhiAccumTexture = nullptr;
    rhiRevealageTexture = nullptr;
}

Q_STATIC_LOGGING_CATEGORY(lcUserRenderPass, "qt.quick3d.rhi.userrenderpass")

void UserRenderPass::renderPrep(QSSGRenderer &renderer, QSSGLayerRenderData &data)
{
    QSSG_ASSERT(!data.renderedCameras.isEmpty(), return);
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);

    for (auto *passNode : std::as_const(userPasses)) {
        QSSG_ASSERT(passNode != nullptr, continue);
        prepareTopLevelPass(renderer, data, passNode);
    }
}

void UserRenderPass::renderPass(QSSGRenderer &renderer)
{

    const auto &rhiCtx = renderer.contextInterface()->rhiContext();
    QSSG_ASSERT(rhiCtx->rhi()->isRecordingFrame(), return);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    for (UserPassData &passData : userPassData) {

        const auto &ps = passData.ps;
        const auto &renderables = passData.renderables;
        const auto &clearColor = passData.clearColor;
        const auto &depthStencilClearValue = passData.depthStencilClearValue;
        const auto &renderableTexture = passData.renderableTexture;

        cb->debugMarkBegin(QByteArrayLiteral("Quick3D UserRenderPass"));

        if (Q_LIKELY(renderableTexture && renderableTexture->isValid())) {
            cb->beginPass(renderableTexture->getRenderTarget().get(), clearColor, depthStencilClearValue, nullptr, rhiCtx->commonPassFlags());
            if (passData.skyboxCubeMapPass) {
                passData.skyboxCubeMapPass->renderPass(renderer);
            } else if (passData.skyboxPass) {
                passData.skyboxPass->renderPass(renderer);
            } else if (passData.item2DPass) {
                passData.item2DPass->renderPass(renderer);
            } else {
                // Regular User Passes
                bool needsSetViewport = true;
                if (passData.index >= 0) {
                    for (const auto &handle : std::as_const(renderables))
                        RenderHelpers::rhiRenderRenderable(rhiCtx.get(), ps, *handle.obj, &needsSetViewport, QSSGRenderTextureCubeFaceNone, qsizetype(passData.index));
                }
            }
            QRhiResourceUpdateBatch *rub = nullptr;

            // Sub Passes
            for (auto &subPassData : passData.subPassData) {
                const auto &subPs = subPassData.ps;
                const auto &subRenderables = subPassData.renderables;

                if (subPassData.skyboxCubeMapPass) {
                    subPassData.skyboxCubeMapPass->renderPass(renderer);
                } else if (subPassData.skyboxPass) {
                    subPassData.skyboxPass->renderPass(renderer);
                } else if (subPassData.item2DPass) {
                    subPassData.item2DPass->renderPass(renderer);
                } else {
                    bool needsSetViewport = true;
                    if (subPassData.index >= 0) {
                        for (const auto &handle : std::as_const(subRenderables))
                            RenderHelpers::rhiRenderRenderable(rhiCtx.get(), subPs, *handle.obj, &needsSetViewport, QSSGRenderTextureCubeFaceNone, qsizetype(subPassData.index));
                    }
                }
            }

            cb->endPass(rub);
        }

        cb->debugMarkEnd();
    }
}

void UserRenderPass::resetForFrame()
{
    qCDebug(lcUserRenderPass) << "resetForFrame in UserRenderPass";
    userPasses.clear();

    userPassData.clear();
}

void UserRenderPass::preparePassImpl(QSSGRenderer &renderer,
                                     QSSGLayerRenderData &data,
                                     QSSGRenderUserPass *passNode,
                                     std::vector<UserPassData> &outData,
                                     QSSGRhiRenderableTextureV2Ptr renderableTexture)
{
    // NOTE: Only top-level passes should create their own renderable texture by passing in a null renderable texture.
    // Ideally we this should be more explict...
    const bool isTopLevelPass = (renderableTexture == nullptr);

    QSSGRenderCamera *camera = data.renderedCameras[0];
    const auto &rhiCtx = renderer.contextInterface()->rhiContext();

    const QSize targetSize = data.layerPrepResult.textureDimensions();

    static const auto needsRebuild = [](QRhiTexture *texture, const QSize &size, QRhiTexture::Format format) {
        return !texture || texture->pixelSize() != size || texture->format() != format;
    };

    qCDebug(lcUserRenderPass, "renderPrep in UserRenderPass");

    const QSSGResourceId currentPassId = QSSGRenderGraphObjectUtils::getResourceId(*passNode);
    if (visitedPasses.find(currentPassId) != visitedPasses.end()) {
        qWarning("UserRenderPass: Circular dependency detected in SubRenderPass chain. Ignoring pass.");
        return;
    }

    // Check max depth
    if (visitedPasses.size() >= MAX_SUBPASS_DEPTH) {
        qWarning("UserRenderPass: Maximum SubRenderPass nesting depth (%zu) exceeded. Ignoring pass.", MAX_SUBPASS_DEPTH);
        return;
    }

    visitedPasses.insert(currentPassId);

    UserPassData currentPassData;
    currentPassData.clearColor = passNode->clearColor;
    currentPassData.depthStencilClearValue = passNode->depthStencilClearValue;
    if (isTopLevelPass)
        renderableTexture = currentPassData.renderableTexture = data.requestUserRenderPassManager()->getOrCreateRenderableTexture(*passNode);
    else
        currentPassData.renderableTexture = renderableTexture;

    QSSGRenderableObjectList &renderables = currentPassData.renderables;
    QSSGRhiGraphicsPipelineState &ps = currentPassData.ps;
    const auto &renderTarget = currentPassData.renderableTexture;

    // Initial pipeline state from layer data
    ps = data.getPipelineState();

    bool renderablesFiltered = false;

    bool needsDepthStencilRenderBuffer = false;
    QSSGAllocateTexturePtr depthTextureAllocCommand;

    QVarLengthArray<QSSGColorAttachment *, 16> colorAttachments;
    QVarLengthArray<QSSGResourceId, 4> subPassIds;

    // Process commands in passNode
    for (const QSSGCommand *theCommand : std::as_const(passNode->commands)) {
        QSSG_ASSERT(theCommand != nullptr, continue);

        qCDebug(lcUserRenderPass) << "Exec. command:    >" << theCommand->typeAsString() << "--" << theCommand->debugString();

        switch (theCommand->m_type) {
        case CommandType::ColorAttachment:
        {
            const QSSGColorAttachment *colorAttachCmd = static_cast<const QSSGColorAttachment *>(theCommand);
            colorAttachments.push_back(const_cast<QSSGColorAttachment *>(colorAttachCmd));
        }
        break;
        case CommandType::DepthTextureAttachment:
        {
            QSSG_ASSERT(depthTextureAllocCommand == nullptr, break);
            const QSSGDepthTextureAttachment *depthAttachCmd = static_cast<const QSSGDepthTextureAttachment *>(theCommand);
            needsDepthStencilRenderBuffer = false;
            depthTextureAllocCommand = depthAttachCmd->m_textureCmd;
        }
        break;
        case CommandType::AddShaderDefine:
        {
            const auto *defineCmd = static_cast<const QSSGAddShaderDefine *>(theCommand);
            const auto &defineName = defineCmd->m_name;
            if (defineName.size() > 0) {
                QByteArray value = QByteArray::number(defineCmd->m_value);
                currentPassData.shaderDefines.push_back({ defineName, value });
            }
        }
        break;
        case CommandType::RenderablesFilter:
        {
            auto filterCommand = static_cast<const QSSGRenderablesFilterCommand *>(theCommand);

            // Use the filter to select which renderables to include
            // renderableTypes can be: None (0x0), Opaque (0x1), Transparent (0x2), or both
            enum RenderableType : quint8 {
                None = 0x0,
                Opaque = 0x1,
                Transparent = 0x2,
            };

            if (filterCommand->renderableTypes & RenderableType::Opaque) // Opaque
                renderables = data.getSortedOpaqueRenderableObjects(*camera, 0, filterCommand->layerMask);
            if (filterCommand->renderableTypes & RenderableType::Transparent) // Transparent
                renderables += data.getSortedTransparentRenderableObjects(*camera, 0, filterCommand->layerMask);

            // NOTE: If renderableTypes is None (0x0), no renderables are added
            // NOTE: If no filter is run, all opaque objects are rendered.
            renderablesFiltered = true;
        }
        break;

        case CommandType::PipelineStateOverride:
        {
            auto pipelineCommand = static_cast<const QSSGPipelineStateOverrideCommand *>(theCommand);
            if (pipelineCommand->m_depthTestEnabled)
                ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled, *pipelineCommand->m_depthTestEnabled);
            if (pipelineCommand->m_depthWriteEnabled)
                ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, *pipelineCommand->m_depthWriteEnabled);
            if (pipelineCommand->m_blendEnabled)
                ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled, *pipelineCommand->m_blendEnabled);
            if (pipelineCommand->m_usesStencilReference)
                ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::UsesStencilRef, *pipelineCommand->m_usesStencilReference);
            if (pipelineCommand->m_usesScissor)
                ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::UsesScissor, *pipelineCommand->m_usesScissor);
            if (pipelineCommand->m_depthFunction)
                ps.depthFunc = *pipelineCommand->m_depthFunction;
            if (pipelineCommand->m_cullMode)
                ps.cullMode = *pipelineCommand->m_cullMode;
            if (pipelineCommand->m_polygonMode)
                ps.polygonMode = *pipelineCommand->m_polygonMode;
            if (pipelineCommand->m_stencilOpFrontState)
                ps.stencilOpFrontState = *pipelineCommand->m_stencilOpFrontState;
            if (pipelineCommand->m_stencilWriteMask)
                ps.stencilWriteMask = *pipelineCommand->m_stencilWriteMask;
            if (pipelineCommand->m_stencilReference)
                ps.stencilRef = *pipelineCommand->m_stencilReference;
            if (pipelineCommand->m_viewport)
                ps.viewport = *pipelineCommand->m_viewport;
            if (pipelineCommand->m_scissor)
                ps.scissor = *pipelineCommand->m_scissor;
            if (pipelineCommand->m_targetBlend0)
                ps.targetBlend[0] = *pipelineCommand->m_targetBlend0;
            if (pipelineCommand->m_targetBlend1)
                ps.targetBlend[1] = *pipelineCommand->m_targetBlend1;
            if (pipelineCommand->m_targetBlend2)
                ps.targetBlend[2] = *pipelineCommand->m_targetBlend2;
            if (pipelineCommand->m_targetBlend3)
                ps.targetBlend[3] = *pipelineCommand->m_targetBlend3;
            if (pipelineCommand->m_targetBlend4)
                ps.targetBlend[4] = *pipelineCommand->m_targetBlend4;
            if (pipelineCommand->m_targetBlend5)
                ps.targetBlend[5] = *pipelineCommand->m_targetBlend5;
            if (pipelineCommand->m_targetBlend6)
                ps.targetBlend[6] = *pipelineCommand->m_targetBlend6;
            if (pipelineCommand->m_targetBlend7)
                ps.targetBlend[7] = *pipelineCommand->m_targetBlend7;
            break;
        }
        case CommandType::DepthStencilAttachment:
            //auto depthStencilCommand = static_cast<const QSSGDepthStencilAttachment *>(theCommand);
            QSSG_ASSERT(depthTextureAllocCommand == nullptr, break);
            needsDepthStencilRenderBuffer = true;
            break;
        case CommandType::SubRenderPass:
        {
            auto subPassCommand = static_cast<const QSSGSubRenderPass *>(theCommand);
            if (subPassCommand && subPassCommand->m_userPassId != QSSGResourceId::Invalid)
                subPassIds.append(subPassCommand->m_userPassId);
            break;
        }
        default:
            qWarning() << "Effect command" << theCommand->typeAsString() << "not implemented";
            break;
        }
    }

    if (colorAttachments.size() > 4) {
        colorAttachments.resize(4);
        qWarning() << "UserRenderPass supports up to 4 color attachments only.";
    }

    // m_passNode contains state for this UserRenderPass

    if (isTopLevelPass) {
        // 1) Setup the render target
        bool needsBuild = !renderTarget->isValid();

        const qsizetype oldAttachmentCount = renderTarget->colorAttachmentCount();
        // If the number of attachments has changed, we need to rebuild.
        if (oldAttachmentCount != colorAttachments.size())
            needsBuild = true;

        // Even if the render target is valid we need to check if the textures are still compatible.
        if (!needsBuild) {
            // Color attachments
            for (int i = 0; i != oldAttachmentCount; ++i) {
                const auto &colorAttachment = colorAttachments.at(i);
                QSSG_ASSERT(colorAttachment != nullptr, continue);
                const auto expectedFormat = QSSGBufferManager::toRhiFormat(colorAttachment->format());
                const auto &texture = renderTarget->getColorTexture(i);
                needsBuild = needsBuild || needsRebuild(&(*texture->texture()), targetSize, expectedFormat);
            }

            // depthStencilBuffer
            if (needsDepthStencilRenderBuffer != (renderTarget->getDepthStencil() != nullptr))
                needsBuild = true;

            // depthTexture
            if (depthTextureAllocCommand) {
                const auto format = QSSGBufferManager::toRhiFormat(depthTextureAllocCommand->format());
                const auto &depthTextureWrapper = renderTarget->getDepthTexture();
                needsBuild = depthTextureWrapper == nullptr;
                needsBuild = needsBuild || needsRebuild(&(*depthTextureWrapper->texture()), targetSize, format);
            } else {
                if (renderTarget->getDepthTexture() != nullptr)
                    needsBuild = true;
            }
        }

        if (needsBuild) {
            renderTarget->reset();

            QRhiTextureRenderTargetDescription rtDesc;

            // If no attachments are specified, create one.
            const qsizetype colorAttachmentCount = qMax<qsizetype>(colorAttachments.size(), 1);

            bool createSucceeded = true;
            bool colorAllocatorsNeedsUpdate = false;

            {
                // Used to set the color attachments in rtDesc below (don't let the raw ptrs leave this scope).
                QVarLengthArray<QRhiTexture *, 4> textures;
                for (qsizetype i = 0; i < colorAttachmentCount && createSucceeded ; ++i) {
                    const auto &colorAttCmd = colorAttachments.at(i);
                    const auto &name = colorAttCmd->m_name;
                    const auto format = QSSGBufferManager::toRhiFormat(colorAttCmd->format());
                    const auto &allocateTexCmd = colorAttCmd->m_textureCmd;
                    if (allocateTexCmd->texture() && !needsRebuild(&(*allocateTexCmd->texture()->texture()), targetSize, format)) {
                        // NOTE: The QRhiTexture is tracked by the QSSGUserRenderPassManager, even if we create a new
                        //       shared pointer wrapper here, it will ask the manager before destroying it.
                        textures.push_back(allocateTexCmd->texture()->texture().get());
                    } else {
                        auto *tex = rhiCtx->rhi()->newTexture(format, targetSize, ps.samples, QRhiTexture::RenderTarget);
                        tex->setName(name);
                        createSucceeded = createSucceeded && tex->create();
                        textures.push_back(tex);
                        // We created a new texture, so we need to update the allocator.
                        // NOTE: Since the render target type will take ownership of the texture, we need to
                        //       request the textures once the render target description is set!
                        colorAllocatorsNeedsUpdate = true;
                    }
                }

                rtDesc.setColorAttachments(textures.cbegin(), textures.cend());
            }

            if (needsDepthStencilRenderBuffer) {
                auto renderBuffer = rhiCtx->rhi()->newRenderBuffer(QRhiRenderBuffer::DepthStencil, targetSize, ps.samples);
                if (renderBuffer->create())
                    rtDesc.setDepthStencilBuffer(renderBuffer);
            } else if (depthTextureAllocCommand) {
                const auto format = QSSGBufferManager::toRhiFormat(depthTextureAllocCommand->format());
                if (depthTextureAllocCommand->texture() && !needsRebuild(&(*depthTextureAllocCommand->texture()->texture()), targetSize, format)) {
                    rtDesc.setDepthTexture(depthTextureAllocCommand->texture()->texture().get());
                } else {
                    // Create new depth texture
                    QRhiTexture *depthTex = rhiCtx->rhi()->newTexture(format, targetSize, ps.samples, QRhiTexture::RenderTarget);
                    if (depthTex->create())
                        rtDesc.setDepthTexture(depthTex);
                }
            }

            if (createSucceeded) {
                // Set description takes ownership of rtDesc and the textures inside it (Color + Depth).
                renderTarget->setDescription(rhiCtx->rhi(), std::move(rtDesc), passNode->renderTargetFlags);

                // Now we can update the allocators for the color attachments that were created here.
                if (colorAllocatorsNeedsUpdate) {
                    for (qsizetype i = 0; i < colorAttachmentCount; ++i) {
                        const auto &colorAttCmd = colorAttachments.at(i);
                        const auto &allocateTexCmd = colorAttCmd->m_textureCmd;
                        allocateTexCmd->setTexture(renderTarget->getColorTexture(i));
                    }
                }

                if (depthTextureAllocCommand)
                    depthTextureAllocCommand->setTexture(renderTarget->getDepthTexture());

            } else {
                renderTarget->resetRenderTarget();
                qWarning() << "Failed to create textures for UserRenderPass";
            }
        }

    }

    Q_ASSERT(renderTarget->isValid());

    ps.colorAttachmentCount = int(renderTarget->colorAttachmentCount());

    // Subpasses
    for (const auto &subPassId : std::as_const(subPassIds)) {
        QSSGRenderUserPass *userPassNode = QSSGRenderGraphObjectUtils::getResource<QSSGRenderUserPass>(subPassId);
        QSSG_ASSERT(userPassNode && userPassNode->type == QSSGRenderGraphObject::Type::RenderPass, continue);
        prepareSubPass(renderer, data, userPassNode, currentPassData.subPassData, currentPassData.renderableTexture);
    }

    if (passNode->passMode == QSSGRenderUserPass::PassModes::UserPass) {
        // If no filter is specified, render all opaque objects
        if (!renderablesFiltered && renderables.isEmpty())
            renderables = data.getSortedOpaqueRenderableObjects(*camera);

        if (passNode->materialMode == QSSGRenderUserPass::MaterialModes::AugmentMaterial) {

            QSSGUserShaderAugmentation shaderAugmentation = passNode->shaderAugmentation;

            QSSG_ASSERT(shaderAugmentation.outputs.size() == 0, shaderAugmentation.outputs.clear());
            shaderAugmentation.defines = std::move(currentPassData.shaderDefines);

            for (int i = 0, end = colorAttachments.size(); i < end; ++i) {
                const auto &colorAttCmd = colorAttachments.at(i);
                const auto &name = colorAttCmd->m_name;
                if (name.size() > 0)
                    shaderAugmentation.outputs.push_back(name);
                else
                    shaderAugmentation.outputs.push_back(getDefaultOutputName(size_t(i)));
            }

            QSSGShaderFeatures shaderFeatures = data.getShaderFeatures();
            shaderFeatures.disableTonemapping();

            currentPassData.index = RenderHelpers::rhiPrepareAugmentedUserPass(&(*rhiCtx), this, ps, renderTarget->getRenderPassDescriptor().get(), shaderAugmentation, data, renderables, shaderFeatures);
        } else if (passNode->materialMode == QSSGRenderUserPass::MaterialModes::OverrideMaterial) {
            // Every renderable will use the override material
            QSSGShaderFeatures shaderFeatures = data.getShaderFeatures();
            shaderFeatures.disableTonemapping();

            currentPassData.index = RenderHelpers::rhiPrepareOverrideMaterialUserPass(&(*rhiCtx), this, ps, renderTarget->getRenderPassDescriptor().get(), passNode->overrideMaterial, data, renderables, shaderFeatures);

        } else {
            // Use original material of the renderables
            QSSGShaderFeatures shaderFeatures = data.getShaderFeatures();
            shaderFeatures.disableTonemapping();
            currentPassData.index = RenderHelpers::rhiPrepareOriginalMaterialUserPass(&(*rhiCtx), this, ps, renderTarget->getRenderPassDescriptor().get(), data, renderables, shaderFeatures);
        }
        outData.push_back(currentPassData);
    } else {
        // Wrapped Built-in Passes
        if (passNode->passMode == QSSGRenderUserPass::PassModes::SkyboxPass) {
            if (rhiCtx->rhi()->isFeatureSupported(QRhi::TexelFetch)) {
                if (data.layer.background == QSSGRenderLayer::Background::SkyBoxCubeMap && data.layer.skyBoxCubeMap) {
                    if (!currentPassData.skyboxCubeMapPass)
                        currentPassData.skyboxCubeMapPass = SkyboxCubeMapPass();

                    currentPassData.skyboxCubeMapPass->skipTonemapping = true;
                    currentPassData.skyboxCubeMapPass->renderPrep(renderer, data);
                    currentPassData.skyboxCubeMapPass->ps.samples = ps.samples;
                    currentPassData.skyboxCubeMapPass->rpDesc = renderTarget->getRenderPassDescriptor().get();

                    currentPassData.skyboxPass = std::nullopt;
                } else if (data.layer.background == QSSGRenderLayer::Background::SkyBox && data.layer.lightProbe) {
                    if (!currentPassData.skyboxPass)
                        currentPassData.skyboxPass = SkyboxPass();

                    currentPassData.skyboxPass->skipTonemapping = true;
                    currentPassData.skyboxPass->renderPrep(renderer, data);
                    currentPassData.skyboxPass->ps.samples = ps.samples;
                    currentPassData.skyboxPass->rpDesc = renderTarget->getRenderPassDescriptor().get();

                    currentPassData.skyboxCubeMapPass = std::nullopt;
                }
                outData.push_back(currentPassData);
            }
        } else if (passNode->passMode == QSSGRenderUserPass::PassModes::Item2DPass) {
            if (!currentPassData.item2DPass)
                currentPassData.item2DPass = Item2DPass();
            const bool hasItem2Ds = (data.item2DsView.size() > 0);
            if (hasItem2Ds) {
                //backup render target
                QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(renderer.contextInterface()->rhiContext().get());
                QRhiRenderTarget *prevRenderTarget = rhiCtx->renderTarget();
                rhiCtxD->setRenderTarget(renderTarget->getRenderTarget().get());
                currentPassData.item2DPass->renderPrep(renderer, data);
                //restore render target
                rhiCtxD->setRenderTarget(prevRenderTarget);
                outData.push_back(currentPassData);
            }
        }
    }
}

QT_END_NAMESPACE
