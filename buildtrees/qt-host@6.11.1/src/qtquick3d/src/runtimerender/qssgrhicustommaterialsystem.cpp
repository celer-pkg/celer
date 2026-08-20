// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrhicustommaterialsystem_p.h"

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include "qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgvertexpipelineimpl_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgruntimerenderlogging_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhiparticles_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderhelpers_p.h>
#include <qtquick3d_tracepoints_p.h>

#include <QtCore/qbitarray.h>

QT_BEGIN_NAMESPACE

Q_TRACE_POINT(qtquick3d, QSSG_generateShader_entry)
Q_TRACE_POINT(qtquick3d, QSSG_generateShader_exit)

QSSGCustomMaterialSystem::QSSGCustomMaterialSystem() = default;

QSSGCustomMaterialSystem::~QSSGCustomMaterialSystem()
{
}

bool QSSGCustomMaterialSystem::prepareForRender(const QSSGRenderModel &,
                                          const QSSGRenderSubset &,
                                          QSSGRenderCustomMaterial &inMaterial)
{
    return inMaterial.isDirty();
}

void QSSGCustomMaterialSystem::setRenderContextInterface(QSSGRenderContextInterface *inContext)
{
    context = inContext;
}

void QSSGCustomMaterialSystem::releaseCachedResources()
{
    shaderMap.clear();
}

QSSGRhiShaderPipelinePtr QSSGCustomMaterialSystem::shadersForCustomMaterial(QSSGRhiGraphicsPipelineState *ps,
                                                                            const QSSGRenderCustomMaterial &material,
                                                                            QSSGSubsetRenderable &renderable,
                                                                            const QSSGShaderDefaultMaterialKeyProperties &defaultMaterialShaderKeyProperties,
                                                                            const QSSGShaderFeatures &featureSet,
                                                                            const QSSGUserShaderAugmentation &shaderAugmentation)
{
    QElapsedTimer timer;
    timer.start();

    QSSGRhiShaderPipelinePtr shaderPipeline;

    const bool multiView = featureSet.isSet(QSSGShaderFeatures::Feature::DisableMultiView)
        ? false
        : defaultMaterialShaderKeyProperties.m_viewCount.getValue(renderable.shaderDescription) >= 2;
    const QByteArray shaderPathKey = material.m_shaderPathKey[multiView ? QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex
                                                                        : QSSGRenderCustomMaterial::RegularShaderPathKeyIndex] + shaderAugmentation.preamble + shaderAugmentation.body;

    // This just references inFeatureSet and inRenderable.shaderDescription -
    // cheap to construct and is good enough for the find(). This is the first
    // level, fast lookup. (equivalent to what
    // QSSGRenderer::getShaderPipelineForDefaultMaterial does for the
    // default/principled material)
    QSSGShaderMapKey skey = QSSGShaderMapKey(shaderPathKey,
                                             featureSet,
                                             renderable.shaderDescription);
    auto it = shaderMap.find(skey);
    if (it == shaderMap.end()) {
        // NB this key calculation must replicate exactly what the generator does in generateMaterialRhiShader()
        QByteArray shaderString = shaderPathKey;
        QSSGShaderDefaultMaterialKey matKey(renderable.shaderDescription);
        matKey.toString(shaderString, defaultMaterialShaderKeyProperties);

        // Try the persistent (disk-based) cache.
        const QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(shaderString, QQsbCollection::toFeatureSet(featureSet));
        shaderPipeline = context->shaderCache()->tryNewPipelineFromPersistentCache(qsbcKey, shaderPathKey, featureSet);

        if (!shaderPipeline) {
            // Have to generate the shaders and send it all through the shader conditioning pipeline.
            Q_TRACE_SCOPE(QSSG_generateShader);
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DGenerateShader);
            QSSGMaterialVertexPipeline vertexPipeline(*context->shaderProgramGenerator(),
                                                      defaultMaterialShaderKeyProperties,
                                                      material.adapter);

            shaderPipeline = QSSGMaterialShaderGenerator::generateMaterialRhiShader(shaderPathKey,
                                                                                    vertexPipeline,
                                                                                    renderable.shaderDescription,
                                                                                    defaultMaterialShaderKeyProperties,
                                                                                    featureSet,
                                                                                    material,
                                                                                    *context->shaderLibraryManager(),
                                                                                    *context->shaderCache(),
                                                                                    shaderAugmentation);
            Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DGenerateShader, 0, material.profilingId);
        }

        // make skey useable as a key for the QHash (makes a copy of the materialKey, instead of just referencing)
        skey.detach();
        // insert it no matter what, no point in trying over and over again
        shaderMap.insert(skey, shaderPipeline);
    } else {
        shaderPipeline = it.value();
    }

    if (shaderPipeline) {
        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(*ps, shaderPipeline.get());
        shaderPipeline->resetExtraTextures();
    }

    QSSGRhiContextStats::get(*context->rhiContext()).registerMaterialShaderGenerationTime(timer.elapsed());

    return shaderPipeline;
}

void QSSGCustomMaterialSystem::updateUniformsForCustomMaterial(QSSGRhiShaderPipeline &shaderPipeline,
                                                               QSSGRhiContext *rhiCtx,
                                                               const QSSGLayerRenderData &inData,
                                                               char *ubufData,
                                                               QSSGRhiGraphicsPipelineState *ps,
                                                               const QSSGRenderCustomMaterial &material,
                                                               QSSGSubsetRenderable &renderable,
                                                               const QSSGRenderCameraList &cameras,
                                                               const QVector2D *depthAdjust,
                                                               const QMatrix4x4 *alteredModelViewProjection)
{
    QSSGRenderMvpArray alteredMvpList;
    if (alteredModelViewProjection)
        alteredMvpList[0] = *alteredModelViewProjection;

    const QMatrix4x4 clipSpaceCorrMatrix = rhiCtx->rhi()->clipSpaceCorrMatrix();
    QRhiTexture *lightmapTexture = inData.getLightmapTexture(renderable.modelContext);

    const auto &modelNode = renderable.modelContext.model;
    const auto &[localInstanceTransform, globalInstanceTransform] = inData.getInstanceTransforms(modelNode);

    const auto &defaultMaterialShaderKeyProperties = inData.getDefaultMaterialPropertyTable();

    const QMatrix4x4 &modelMatrix(modelNode.usesBoneTexture() ? QMatrix4x4() : renderable.modelContext.globalTransform);

    QSSGMaterialShaderGenerator::setRhiMaterialProperties(*context,
                                                          shaderPipeline,
                                                          ubufData,
                                                          ps,
                                                          material,
                                                          renderable.shaderDescription,
                                                          defaultMaterialShaderKeyProperties,
                                                          cameras,
                                                          alteredModelViewProjection ? alteredMvpList : renderable.modelContext.modelViewProjections,
                                                          renderable.modelContext.normalMatrix,
                                                          modelMatrix,
                                                          clipSpaceCorrMatrix,
                                                          localInstanceTransform,
                                                          globalInstanceTransform,
                                                          toDataView(modelNode.morphWeights),
                                                          renderable.firstImage,
                                                          renderable.opacity,
                                                          inData,
                                                          renderable.lights,
                                                          renderable.reflectionProbe,
                                                          true,
                                                          renderable.renderableFlags.receivesReflections(),
                                                          depthAdjust,
                                                          lightmapTexture);
}

static const QRhiShaderResourceBinding::StageFlags CUSTOM_MATERIAL_VISIBILITY_ALL =
        QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

void QSSGCustomMaterialSystem::rhiPrepareRenderable(QSSGRhiGraphicsPipelineState *ps,
                                                    QSSGPassKey passKey,
                                                    QSSGSubsetRenderable &renderable,
                                                    const QSSGShaderFeatures &featureSet,
                                                    const QSSGRenderCustomMaterial &material,
                                                    const QSSGLayerRenderData &layerData,
                                                    QRhiRenderPassDescriptor *renderPassDescriptor,
                                                    int samples,
                                                    int viewCount,
                                                    bool screenMapPass,
                                                    QSSGRenderCamera *alteredCamera,
                                                    QSSGRenderTextureCubeFace cubeFace,
                                                    QMatrix4x4 *alteredModelViewProjection,
                                                    QSSGReflectionMapEntry *entry,
                                                    bool oit)
{
    QSSGRhiContext *rhiCtx = context->rhiContext().get();

    QRhiGraphicsPipeline::TargetBlend blend; // no blending by default
    if (material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::Blending)) {
        blend.enable = true;
        blend.srcColor = material.m_srcBlend;
        blend.srcAlpha = material.m_srcAlphaBlend;
        blend.dstColor = material.m_dstBlend;
        blend.dstAlpha = material.m_dstAlphaBlend;
    }

    const QSSGCullFaceMode cullMode = material.m_cullMode;

    const auto &defaultMaterialShaderKeyProperties = layerData.getDefaultMaterialPropertyTable();

    const bool blendParticles = defaultMaterialShaderKeyProperties.m_blendParticles.getValue(renderable.shaderDescription);

    const auto &shaderPipeline = shadersForCustomMaterial(ps, material, renderable, defaultMaterialShaderKeyProperties, featureSet);

    if (shaderPipeline) {
        QSSGRhiShaderResourceBindingList bindings;
        const auto &modelNode = renderable.modelContext.model;

        // NOTE:
        // - entryIdx should 0 for QSSGRenderTextureCubeFaceNone.
        // In all other cases the entryIdx is a combination of the cubeface idx and the subset offset, where the lower bits
        // are the cubeface idx.
        const auto cubeFaceIdx = QSSGBaseTypeHelpers::indexOfCubeFace(cubeFace);
        const quintptr entryIdx = quintptr(cubeFace != QSSGRenderTextureCubeFaceNone) * (cubeFaceIdx + (quintptr(renderable.subset.offset) << 3));
        // As the entry might be null we create an entry key consisting of the entry and the material.
        const auto entryPartA = reinterpret_cast<quintptr>(&material);
        const auto entryPartB = reinterpret_cast<quintptr>(entry);
        const void *entryKey = reinterpret_cast<const void *>(entryPartA ^ entryPartB);

        QSSGRhiDrawCallData &dcd = QSSGRhiContextPrivate::get(rhiCtx)->drawCallData({ passKey, &modelNode, entryKey, entryIdx });

        shaderPipeline->ensureCombinedUniformBuffer(&dcd.ubuf);
        char *ubufData = dcd.ubuf->beginFullDynamicBufferUpdateForCurrentFrame();
        if (!alteredCamera) {
            updateUniformsForCustomMaterial(*shaderPipeline, rhiCtx, layerData, ubufData, ps, material, renderable, layerData.renderedCameras, nullptr, nullptr);
        } else {
            QSSGRenderCameraList cameras({ alteredCamera });
            updateUniformsForCustomMaterial(*shaderPipeline, rhiCtx, layerData, ubufData, ps, material, renderable, cameras, nullptr, alteredModelViewProjection);
        }
        if (blendParticles)
            QSSGParticleRenderer::updateUniformsForParticleModel(*shaderPipeline, ubufData, &renderable.modelContext.model, renderable.subset.offset);
        dcd.ubuf->endFullDynamicBufferUpdateForCurrentFrame();

        if (blendParticles)
            QSSGParticleRenderer::prepareParticlesForModel(*shaderPipeline, rhiCtx, bindings, &renderable.modelContext.model);
        bool instancing = false;
        if (!alteredCamera) {
            const QSSGRenderCameraDataList &cameraDatas(*layerData.renderedCameraData);
            instancing = QSSGLayerRenderData::prepareInstancing(rhiCtx, &renderable, cameraDatas[0].direction, cameraDatas[0].position, renderable.instancingLodMin, renderable.instancingLodMax);
        } else {
            const auto &altCamTransform = layerData.getGlobalTransform(*alteredCamera);
            instancing = QSSGLayerRenderData::prepareInstancing(rhiCtx, &renderable, QSSGRenderNode::getScalingCorrectDirection(altCamTransform), QSSGRenderNode::getGlobalPos(altCamTransform), renderable.instancingLodMin, renderable.instancingLodMax);
        }

        ps->samples = samples;
        ps->viewCount = viewCount;

        ps->cullMode = QSSGRhiHelpers::toCullMode(cullMode);

        if (!oit)
            ps->targetBlend[0] = blend;

        auto &ia = QSSGRhiInputAssemblerStatePrivate::get(*ps);

        ia = renderable.subset.rhi.ia;

        //### Copied code from default materials
        int instanceBufferBinding = 0;
        if (instancing) {
            // Need to setup new bindings for instanced buffers
            const quint32 stride = renderable.modelContext.model.instanceTable->stride();
            QVarLengthArray<QRhiVertexInputBinding, 8> bindings;
            std::copy(ia.inputLayout.cbeginBindings(),
                      ia.inputLayout.cendBindings(),
                      std::back_inserter(bindings));
            bindings.append({ stride, QRhiVertexInputBinding::PerInstance });
            instanceBufferBinding = bindings.size() - 1;
            ia.inputLayout.setBindings(bindings.cbegin(), bindings.cend());
        }

        QSSGRhiHelpers::bakeVertexInputLocations(&ia, *shaderPipeline, instanceBufferBinding);

        QRhiResourceUpdateBatch *resourceUpdates = rhiCtx->rhi()->nextResourceUpdateBatch();
        QRhiTexture *dummyTexture = rhiCtx->dummyTexture({}, resourceUpdates);
        QRhiTexture *dummyTexture3D = rhiCtx->dummyTexture(QRhiTexture::ThreeDimensional, resourceUpdates);
        QRhiTexture *dummyCubeTexture = rhiCtx->dummyTexture(QRhiTexture::CubeMap, resourceUpdates);
        rhiCtx->commandBuffer()->resourceUpdate(resourceUpdates);

        bindings.addUniformBuffer(0, CUSTOM_MATERIAL_VISIBILITY_ALL, dcd.ubuf, 0, shaderPipeline->ub0Size());
        bindings.addUniformBuffer(1, CUSTOM_MATERIAL_VISIBILITY_ALL, dcd.ubuf, shaderPipeline->ub0LightDataOffset(), sizeof(QSSGShaderLightsUniformData));
        bindings.addUniformBuffer(2, CUSTOM_MATERIAL_VISIBILITY_ALL, dcd.ubuf, shaderPipeline->ub0DirectionalLightDataOffset(), sizeof(QSSGShaderDirectionalLightsUniformData));

        QVector<QShaderDescription::InOutVariable> samplerVars =
                shaderPipeline->fragmentStage()->shader().description().combinedImageSamplers();
        for (const QShaderDescription::InOutVariable &var : shaderPipeline->vertexStage()->shader().description().combinedImageSamplers()) {
            auto it = std::find_if(samplerVars.cbegin(), samplerVars.cend(),
                                   [&var](const QShaderDescription::InOutVariable &v) { return var.binding == v.binding; });
            if (it == samplerVars.cend())
                samplerVars.append(var);
        }

        int maxSamplerBinding = -1;
        for (const QShaderDescription::InOutVariable &var : samplerVars)
            maxSamplerBinding = qMax(maxSamplerBinding, var.binding);

        // Will need to set unused image-samplers to something dummy
        // because the shader code contains all custom property textures,
        // and not providing a binding for all of them is invalid with some
        // graphics APIs (and will need a real texture because setting a
        // null handle or similar is not permitted with some of them so the
        // srb does not accept null QRhiTextures either; but first let's
        // figure out what bindings are unused in this frame)
        QBitArray samplerBindingsSpecified(maxSamplerBinding + 1);

        if (blendParticles)
            samplerBindingsSpecified.setBit(shaderPipeline->bindingForTexture("qt_particleTexture"));

        // Skinning
        if (QRhiTexture *boneTexture = layerData.getBonemapTexture(renderable.modelContext)) {
            int binding = shaderPipeline->bindingForTexture("qt_boneTexture");
            if (binding >= 0) {
                QRhiSampler *boneSampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                             QRhiSampler::Nearest,
                                                             QRhiSampler::None,
                                                             QRhiSampler::ClampToEdge,
                                                             QRhiSampler::ClampToEdge,
                                                             QRhiSampler::Repeat
                                                           });
                bindings.addTexture(binding,
                                    QRhiShaderResourceBinding::VertexStage,
                                    boneTexture,
                                    boneSampler);
                samplerBindingsSpecified.setBit(binding);
            }
        }

        // Morphing
        auto *targetsTexture = renderable.subset.rhi.targetsTexture;
        if (targetsTexture) {
            int binding = shaderPipeline->bindingForTexture("qt_morphTargetTexture");
            if (binding >= 0) {
                QRhiSampler *targetsSampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                                QRhiSampler::Nearest,
                                                                QRhiSampler::None,
                                                                QRhiSampler::ClampToEdge,
                                                                QRhiSampler::ClampToEdge,
                                                                QRhiSampler::ClampToEdge
                                                           });
                bindings.addTexture(binding, QRhiShaderResourceBinding::VertexStage, renderable.subset.rhi.targetsTexture, targetsSampler);
                samplerBindingsSpecified.setBit(binding);
            }
        }

        // Prioritize reflection texture over Light Probe texture because
        // reflection texture also contains the irradiance and pre filtered
        // values for the light probe.
        if (featureSet.isSet(QSSGShaderFeatures::Feature::ReflectionProbe)) {
            int reflectionSampler = shaderPipeline->bindingForTexture("qt_reflectionMap");
            QRhiTexture* reflectionTexture = layerData.getReflectionMapManager()->reflectionMapEntry(renderable.reflectionProbeIndex)->m_rhiPrefilteredCube;
            const auto mipMapFilter = reflectionTexture && reflectionTexture->flags().testFlag(QRhiTexture::Flag::MipMapped)
                    ? QRhiSampler::Linear
                    : QRhiSampler::None;
            QRhiSampler *sampler = rhiCtx->sampler(
                    { QRhiSampler::Linear, QRhiSampler::Linear, mipMapFilter, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
            if (reflectionSampler >= 0 && reflectionTexture) {
                bindings.addTexture(reflectionSampler, QRhiShaderResourceBinding::FragmentStage, reflectionTexture, sampler);
                samplerBindingsSpecified.setBit(reflectionSampler);
            }
        } else if (shaderPipeline->lightProbeTexture()) {
            int binding = shaderPipeline->bindingForTexture("qt_lightProbe", int(QSSGRhiSamplerBindingHints::LightProbe));
            if (binding >= 0) {
                samplerBindingsSpecified.setBit(binding);
                QPair<QSSGRenderTextureCoordOp, QSSGRenderTextureCoordOp> tiling = shaderPipeline->lightProbeTiling();
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::Linear, // enables mipmapping
                                                         QSSGRhiHelpers::toRhi(tiling.first), QSSGRhiHelpers::toRhi(tiling.second), QRhiSampler::Repeat });
                bindings.addTexture(binding,
                                    QRhiShaderResourceBinding::FragmentStage,
                                    shaderPipeline->lightProbeTexture(), sampler);
            } // else ignore, not an error (for example, an unshaded material's fragment shader will not have this sampler)
        }

        if (shaderPipeline->screenTexture()) {
            const int screenTextureBinding = shaderPipeline->bindingForTexture("qt_screenTexture", int(QSSGRhiSamplerBindingHints::ScreenTexture));
            const int screenTextureArrayBinding = shaderPipeline->bindingForTexture("qt_screenTextureArray", int(QSSGRhiSamplerBindingHints::ScreenTextureArray));
            if (screenTextureBinding >= 0 || screenTextureArrayBinding >= 0) {
                QRhiTexture *screenTexture = shaderPipeline->screenTexture();
                // If we're called as part of the screen map pass there's obviously no screen texture available, but the shader may still expect it, so bind a dummy texture.
                if (screenMapPass) {
                    QRhiResourceUpdateBatch *resourceUpdates = rhiCtx->rhi()->nextResourceUpdateBatch();
                    // Just mirror what is set up by the ScreenMap pass.
                    const QRhiTexture::Flags flags = screenTexture->flags();
                    // and set the dummy texture so there's something for the shader to sample.
                    screenTexture = rhiCtx->dummyTexture(flags, resourceUpdates);
                    rhiCtx->commandBuffer()->resourceUpdate(resourceUpdates);
                }
                // linear min/mag, mipmap filtering depends on the
                // texture, with SCREEN_TEXTURE there are no mipmaps, but
                // once SCREEN_MIP_TEXTURE is seen the texture (the same
                // one) has mipmaps generated.
                QRhiSampler::Filter mipFilter = shaderPipeline->screenTexture()->flags().testFlag(QRhiTexture::MipMapped)
                        ? QRhiSampler::Linear : QRhiSampler::None;
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, mipFilter,
                                                         QRhiSampler::Repeat, QRhiSampler::Repeat, QRhiSampler::Repeat });
                if (screenTextureBinding >= 0) {
                    samplerBindingsSpecified.setBit(screenTextureBinding);
                    bindings.addTexture(screenTextureBinding,
                                        QRhiShaderResourceBinding::FragmentStage,
                                        screenTexture, sampler);
                }
                if (screenTextureArrayBinding >= 0) {
                    samplerBindingsSpecified.setBit(screenTextureArrayBinding);
                    bindings.addTexture(screenTextureArrayBinding,
                                        QRhiShaderResourceBinding::FragmentStage,
                                        screenTexture, sampler);
                }
            } // else ignore, not an error
        }

        if (shaderPipeline->depthTexture()) {
            const int depthTextureBinding = shaderPipeline->bindingForTexture("qt_depthTexture", int(QSSGRhiSamplerBindingHints::DepthTexture));
            const int depthTextureArrayBinding = shaderPipeline->bindingForTexture("qt_depthTextureArray", int(QSSGRhiSamplerBindingHints::DepthTextureArray));
            if (depthTextureBinding >= 0 || depthTextureArrayBinding >= 0) {
                // nearest min/mag, no mipmap
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                if (depthTextureBinding >= 0) {
                    samplerBindingsSpecified.setBit(depthTextureBinding);
                    bindings.addTexture(depthTextureBinding,
                                        CUSTOM_MATERIAL_VISIBILITY_ALL,
                                        shaderPipeline->depthTexture(), sampler);
                }
                if (depthTextureArrayBinding >= 0) {
                    samplerBindingsSpecified.setBit(depthTextureArrayBinding);
                    bindings.addTexture(depthTextureArrayBinding,
                                        CUSTOM_MATERIAL_VISIBILITY_ALL,
                                        shaderPipeline->depthTexture(), sampler);
                }
            } // else ignore, not an error
        }

        if (shaderPipeline->normalTexture()) {
            const int normalTextureBinding = shaderPipeline->bindingForTexture("qt_normalTexture", int(QSSGRhiSamplerBindingHints::NormalTexture));
            if (normalTextureBinding >= 0) {
                // nearest min/mag, no mipmap
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                samplerBindingsSpecified.setBit(normalTextureBinding);
                bindings.addTexture(normalTextureBinding, CUSTOM_MATERIAL_VISIBILITY_ALL, shaderPipeline->normalTexture(), sampler);
            } // else ignore, not an error
        }

        if (shaderPipeline->ssaoTexture()) {
            const int ssaoTextureBinding = shaderPipeline->bindingForTexture("qt_aoTexture", int(QSSGRhiSamplerBindingHints::AoTexture));
            const int ssaoTextureArrayBinding = shaderPipeline->bindingForTexture("qt_aoTextureArray", int(QSSGRhiSamplerBindingHints::AoTextureArray));
            if (ssaoTextureBinding >= 0 || ssaoTextureArrayBinding >= 0) {
                // linear min/mag, no mipmap
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                if (ssaoTextureBinding >= 0) {
                    samplerBindingsSpecified.setBit(ssaoTextureBinding);
                    bindings.addTexture(ssaoTextureBinding,
                                        QRhiShaderResourceBinding::FragmentStage,
                                        shaderPipeline->ssaoTexture(), sampler);
                }
                if (ssaoTextureArrayBinding >= 0) {
                    samplerBindingsSpecified.setBit(ssaoTextureArrayBinding);
                    bindings.addTexture(ssaoTextureArrayBinding,
                                        QRhiShaderResourceBinding::FragmentStage,
                                        shaderPipeline->ssaoTexture(), sampler);
                }
            } // else ignore, not an error
        }

        if (shaderPipeline->lightmapTexture()) {
            int binding = shaderPipeline->bindingForTexture("qt_lightmap", int(QSSGRhiSamplerBindingHints::LightmapTexture));
            if (binding >= 0) {
                samplerBindingsSpecified.setBit(binding);
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                bindings.addTexture(binding,
                                    QRhiShaderResourceBinding::FragmentStage,
                                    shaderPipeline->lightmapTexture(), sampler);
            } // else ignore, not an error
        }

        if (shaderPipeline->MotionVectorTexture()) {
            int binding = shaderPipeline->bindingForTexture("qt_motionVectorTexture", int(QSSGRhiSamplerBindingHints::MotionVectorTexture));
            if (binding >= 0) {
                samplerBindingsSpecified.setBit(binding);
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                bindings.addTexture(binding,
                                    QRhiShaderResourceBinding::FragmentStage,
                                    shaderPipeline->MotionVectorTexture(), sampler);

            }
        }

        // Shadow map atlas
        auto shadowMapAtlas = shaderPipeline->shadowMapAtlasTexture();
        if (shadowMapAtlas) {
            int binding = shaderPipeline->bindingForTexture("qt_shadowmap_texture");
            if (binding >= 0) {
                samplerBindingsSpecified.setBit(binding);
                QRhiTexture *texture = shadowMapAtlas;
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                Q_ASSERT(texture && sampler);
                bindings.addTexture(binding, QRhiShaderResourceBinding::FragmentStage, texture, sampler);
            }
        }

        // Shadow map blue noise
        if (auto shadowMapBlueNoise = shaderPipeline->shadowMapBlueNoiseTexture()) {
            int binding = shaderPipeline->bindingForTexture("qt_shadowmap_blue_noise_texture");
            if (binding >= 0) {
                samplerBindingsSpecified.setBit(binding);
                QRhiTexture *texture = shadowMapBlueNoise;
                QRhiSampler *sampler = rhiCtx->sampler(
                        { QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None, QRhiSampler::Repeat, QRhiSampler::Repeat, QRhiSampler::Repeat });
                Q_ASSERT(texture && sampler);
                bindings.addTexture(binding, QRhiShaderResourceBinding::FragmentStage, texture, sampler);
            }
        }

        QSSGRenderableImage *renderableImage = renderable.firstImage;
        while (renderableImage) {
            const char *samplerName = QSSGMaterialShaderGenerator::getSamplerName(renderableImage->m_mapType);
            const int samplerHint = int(renderableImage->m_mapType);
            int samplerBinding = shaderPipeline->bindingForTexture(samplerName, samplerHint);
            if (samplerBinding >= 0) {
                QRhiTexture *texture = renderableImage->m_texture.m_texture;
                if (samplerBinding >= 0 && texture) {
                    const bool mipmapped = texture->flags().testFlag(QRhiTexture::MipMapped);
                    QSSGRhiSamplerDescription samplerDesc = {
                        QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_minFilterType),
                        QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_magFilterType),
                        mipmapped ? QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_mipFilterType) : QRhiSampler::None,
                        QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_horizontalTilingMode),
                        QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_verticalTilingMode),
                        QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_depthTilingMode)
                    };
                    rhiCtx->checkAndAdjustForNPoT(texture, &samplerDesc);
                    QRhiSampler *sampler = rhiCtx->sampler(samplerDesc);
                    samplerBindingsSpecified.setBit(samplerBinding);
                    bindings.addTexture(samplerBinding,
                                        CUSTOM_MATERIAL_VISIBILITY_ALL,
                                        texture, sampler);
                }
            } // else this is not necessarily an error, e.g. having metalness/roughness maps with metalness disabled
            renderableImage = renderableImage->m_nextImage;
        }

        if (maxSamplerBinding >= 0) {
            // custom property textures
            int customTexCount = shaderPipeline->extraTextureCount();
            for (int i = 0; i < customTexCount; ++i) {
                QSSGRhiTexture &t(shaderPipeline->extraTextureAt(i));
                const int samplerBinding = shaderPipeline->bindingForTexture(t.name);
                if (samplerBinding >= 0) {
                    samplerBindingsSpecified.setBit(samplerBinding);
                    rhiCtx->checkAndAdjustForNPoT(t.texture, &t.samplerDesc);
                    QRhiSampler *sampler = rhiCtx->sampler(t.samplerDesc);
                    bindings.addTexture(samplerBinding,
                                        CUSTOM_MATERIAL_VISIBILITY_ALL,
                                        t.texture,
                                        sampler);
                }
            }

            // use a dummy texture for the unused samplers in the shader
            QRhiSampler *dummySampler = rhiCtx->sampler({ QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                                          QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });

            for (const QShaderDescription::InOutVariable &var : samplerVars) {
                if (!samplerBindingsSpecified.testBit(var.binding)) {
                    QRhiTexture *t = nullptr;
                    if (var.type == QShaderDescription::SamplerCube)
                        t = dummyCubeTexture;
                    else if (var.type == QShaderDescription::Sampler3D)
                        t = dummyTexture3D;
                    else
                        t = dummyTexture;
                    bindings.addTexture(var.binding, CUSTOM_MATERIAL_VISIBILITY_ALL, t, dummySampler);
                }
            }
        }

        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);

        if (oit && layerData.layer.oitMethod == QSSGRenderLayer::OITMethod::LinkedList)
            RenderHelpers::addAccumulatorImageBindings(shaderPipeline.get(), bindings);

        // do the same srb lookup acceleration as default materials
        QRhiShaderResourceBindings *&srb = dcd.srb;
        bool srbChanged = false;
        if (!srb || bindings != dcd.bindings) {
            srb = rhiCtxD->srb(bindings);
            rhiCtxD->releaseCachedSrb(dcd.bindings);
            dcd.bindings = bindings;
            srbChanged = true;
        }

        if (cubeFace == QSSGRenderTextureCubeFaceNone)
            renderable.rhiRenderData.mainPass.srb = srb;
        else
            renderable.rhiRenderData.reflectionPass.srb[cubeFaceIdx] = srb;

        const auto pipelineKey = QSSGGraphicsPipelineStateKey::create(*ps, renderPassDescriptor, srb);
        if (dcd.pipeline
                && !srbChanged
                && dcd.renderTargetDescriptionHash == pipelineKey.extra.renderTargetDescriptionHash
                && dcd.renderTargetDescription == pipelineKey.renderTargetDescription
                && dcd.ps == *ps)
        {
            if (cubeFace == QSSGRenderTextureCubeFaceNone)
                renderable.rhiRenderData.mainPass.pipeline = dcd.pipeline;
            else
                renderable.rhiRenderData.reflectionPass.pipeline = dcd.pipeline;
        } else {
            if (cubeFace == QSSGRenderTextureCubeFaceNone) {
                renderable.rhiRenderData.mainPass.pipeline = rhiCtxD->pipeline(pipelineKey,
                                                                               renderPassDescriptor,
                                                                               srb);
                dcd.pipeline = renderable.rhiRenderData.mainPass.pipeline;
            } else {
                renderable.rhiRenderData.reflectionPass.pipeline = rhiCtxD->pipeline(pipelineKey,
                                                                                     renderPassDescriptor,
                                                                                     srb);
                dcd.pipeline = renderable.rhiRenderData.reflectionPass.pipeline;
            }

            dcd.renderTargetDescriptionHash = pipelineKey.extra.renderTargetDescriptionHash;
            dcd.renderTargetDescription = pipelineKey.renderTargetDescription;
            dcd.ps = *ps;
        }
    }
}

void QSSGCustomMaterialSystem::setShaderResources(char *ubufData,
                                                  const QSSGRenderCustomMaterial &inMaterial,
                                                  const QByteArray &inPropertyName,
                                                  const QVariant &propertyValue,
                                                  QSSGRenderShaderValue::Type inPropertyType,
                                                  QSSGRhiShaderPipeline &shaderPipeline)
{
    Q_UNUSED(inMaterial);

    if (inPropertyType == QSSGRenderShaderValue::Texture) {
        QSSGRenderCustomMaterial::TextureProperty *textureProperty =
                reinterpret_cast<QSSGRenderCustomMaterial::TextureProperty *>(propertyValue.value<void *>());
        QSSGRenderImage *image = textureProperty->texImage;
        if (image) {
            const auto &theBufferManager(context->bufferManager());
            const QSSGRenderImageTexture texture = theBufferManager->loadRenderImage(image);
            if (texture.m_texture) {
                const QSSGRhiTexture t = {
                    inPropertyName,
                    texture.m_texture,
                    { QSSGRhiHelpers::toRhi(textureProperty->minFilterType),
                      QSSGRhiHelpers::toRhi(textureProperty->magFilterType),
                      textureProperty->mipFilterType != QSSGRenderTextureFilterOp::None ? QSSGRhiHelpers::toRhi(textureProperty->mipFilterType) : QRhiSampler::None,
                      QSSGRhiHelpers::toRhi(textureProperty->horizontalClampType),
                      QSSGRhiHelpers::toRhi(textureProperty->verticalClampType),
                      QSSGRhiHelpers::toRhi(textureProperty->zClampType)
                    }
                };
                shaderPipeline.addExtraTexture(t);
            }
        }
    } else {
        shaderPipeline.setUniformValue(ubufData, inPropertyName, propertyValue, inPropertyType);
    }
}

void QSSGCustomMaterialSystem::applyRhiShaderPropertyValues(char *ubufData,
                                                            const QSSGRenderCustomMaterial &material,
                                                            QSSGRhiShaderPipeline &shaderPipeline)
{
    const auto &properties = material.m_properties;
    for (const auto &prop : properties)
        setShaderResources(ubufData, material, prop.name, prop.value, prop.shaderDataType, shaderPipeline);

    const auto textProps = material.m_textureProperties;
    for (const auto &prop : textProps)
        setShaderResources(ubufData, material, prop.name, QVariant::fromValue((void *)&prop), prop.shaderDataType, shaderPipeline);
}

void QSSGCustomMaterialSystem::rhiRenderRenderable(QSSGRhiContext *rhiCtx,
                                                   QSSGSubsetRenderable &renderable,
                                                   bool *needsSetViewport,
                                                   QSSGRenderTextureCubeFace cubeFace,
                                                   const QSSGRhiGraphicsPipelineState &state,
                                                   qsizetype userPassIndex)
{
    QRhiGraphicsPipeline *ps = (userPassIndex >= 0) ? renderable.rhiRenderData.userPassData[userPassIndex].pipeline : renderable.rhiRenderData.mainPass.pipeline;
    QRhiShaderResourceBindings *srb = (userPassIndex >= 0) ? renderable.rhiRenderData.userPassData[userPassIndex].srb : renderable.rhiRenderData.mainPass.srb;

    if (cubeFace != QSSGRenderTextureCubeFaceNone) {
        const auto cubeFaceIdx = QSSGBaseTypeHelpers::indexOfCubeFace(cubeFace);
        ps = renderable.rhiRenderData.reflectionPass.pipeline;
        srb = renderable.rhiRenderData.reflectionPass.srb[cubeFaceIdx];
    }

    if (!ps || !srb)
        return;

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);
    QRhiBuffer *vertexBuffer = renderable.subset.rhi.vertexBuffer->buffer();
    QRhiBuffer *indexBuffer = renderable.subset.rhi.indexBuffer ? renderable.subset.rhi.indexBuffer->buffer() : nullptr;

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->setGraphicsPipeline(ps);
    cb->setShaderResources(srb);

    if (*needsSetViewport) {
        cb->setViewport(state.viewport);
        *needsSetViewport = false;
    }

    QRhiCommandBuffer::VertexInput vertexBuffers[2];
    int vertexBufferCount = 1;
    vertexBuffers[0] = QRhiCommandBuffer::VertexInput(vertexBuffer, 0);
    quint32 instances = 1;
    if (renderable.modelContext.model.instancing()) {
        instances = renderable.modelContext.model.instanceCount();
        vertexBuffers[1] = QRhiCommandBuffer::VertexInput(renderable.instanceBuffer, 0);
        vertexBufferCount = 2;
    }
    if (indexBuffer) {
        cb->setVertexInput(0, vertexBufferCount, vertexBuffers, indexBuffer, 0, renderable.subset.rhi.indexBuffer->indexFormat());
        cb->drawIndexed(renderable.subset.lodCount(renderable.subsetLevelOfDetail), instances, renderable.subset.lodOffset(renderable.subsetLevelOfDetail));
        QSSGRHICTX_STAT(rhiCtx, drawIndexed(renderable.subset.lodCount(renderable.subsetLevelOfDetail), instances));
    } else {
        cb->setVertexInput(0, vertexBufferCount, vertexBuffers);
        cb->draw(renderable.subset.count, instances, renderable.subset.offset);
        QSSGRHICTX_STAT(rhiCtx, draw(renderable.subset.count, instances));
    }
    Q_QUICK3D_PROFILE_END_WITH_IDS(QQuick3DProfiler::Quick3DRenderCall, (renderable.subset.count | quint64(instances) << 32),
                                     QVector<int>({renderable.modelContext.model.profilingId,
                                      renderable.material.profilingId}));
}

QT_END_NAMESPACE
