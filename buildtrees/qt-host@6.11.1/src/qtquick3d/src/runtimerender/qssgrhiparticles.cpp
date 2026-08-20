// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrhiparticles_p.h"
#include "qssgrhicontext_p.h"

#include <qfloat16.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderhelpers_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshaderresourcemergecontext_p.h>

#include <ssg/qssgrendercontextcore.h>
#include "qssgrendershadercodegenerator_p.h"

#include "qtquick3d_tracepoints_p.h"

QT_BEGIN_NAMESPACE

static const QRhiShaderResourceBinding::StageFlags VISIBILITY_ALL =
        QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

struct ParticleLightData
{
    QVector4D pointLightPos[4];
    float pointLightConstantAtt[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float pointLightLinearAtt[4] = {0.0f};
    float pointLightQuadAtt[4] = {0.0f};
    QVector4D pointLightColor[4];
    QVector4D spotLightPos[4];
    float spotLightConstantAtt[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float spotLightLinearAtt[4] = {0.0f};
    float spotLightQuadAtt[4] = {0.0f};
    QVector4D spotLightColor[4];
    QVector4D spotLightDir[4];
    float spotLightConeAngle[4] = {0.0f};
    float spotLightInnerConeAngle[4] = {0.0f};
};

static bool s_shaderCacheEnabled = true;
void QSSGParticleRenderer::setShaderCacheEnabled(bool enabled)
{
    s_shaderCacheEnabled = enabled;
}

void QSSGParticleRenderer::updateUniformsForParticles(const QSSGLayerRenderData &inData,
                                                      QSSGRhiShaderPipeline &shaders,
                                                      QSSGRhiContext *rhiCtx,
                                                      char *ubufData,
                                                      QSSGParticlesRenderable &renderable,
                                                      const QSSGRenderCameraList &cameras)
{
    const QMatrix4x4 clipSpaceCorrMatrix = rhiCtx->rhi()->clipSpaceCorrMatrix();

    QSSGRhiShaderPipeline::CommonUniformIndices &cui = shaders.commonUniformIndices;

    const int viewCount = cameras.count();

    // Pull the camera transforms from the render data once.
    QMatrix4x4 camGlobalTransforms[2] { QMatrix4x4{Qt::Uninitialized}, QMatrix4x4{Qt::Uninitialized} };
    if (viewCount < 2) {
        camGlobalTransforms[0] = inData.getGlobalTransform(*cameras[0]);
    } else {
        for (size_t viewIndex = 0; viewIndex != std::size(camGlobalTransforms); ++viewIndex)
            camGlobalTransforms[viewIndex] = inData.getGlobalTransform(*cameras[viewIndex]);
    }

    if (viewCount < 2) {
        const QMatrix4x4 projection = clipSpaceCorrMatrix * cameras[0]->projection;
        shaders.setUniform(ubufData, "qt_projectionMatrix", projection.constData(), 16 * sizeof(float), &cui.projectionMatrixIdx);
        const QMatrix4x4 viewMatrix = camGlobalTransforms[0].inverted();
        shaders.setUniform(ubufData, "qt_viewMatrix", viewMatrix.constData(), 16 * sizeof(float), &cui.viewMatrixIdx);
    } else {
        QVarLengthArray<QMatrix4x4, 2> projectionMatrices(viewCount);
        QVarLengthArray<QMatrix4x4, 2> viewMatrices(viewCount);
        for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex) {
            projectionMatrices[viewIndex] = clipSpaceCorrMatrix * cameras[viewIndex]->projection;
            viewMatrices[viewIndex] = camGlobalTransforms[viewIndex].inverted();
        }
        shaders.setUniformArray(ubufData, "qt_projectionMatrix", projectionMatrices.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.projectionMatrixIdx);
        shaders.setUniformArray(ubufData, "qt_viewMatrix", viewMatrices.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.viewMatrixIdx);
    }

    const QMatrix4x4 &modelMatrix = renderable.globalTransform;
    shaders.setUniform(ubufData, "qt_modelMatrix", modelMatrix.constData(), 16 * sizeof(float), &cui.modelMatrixIdx);

    const QVector2D camProperties(cameras[0]->clipPlanes);
    shaders.setUniform(ubufData, "qt_cameraProperties", &camProperties, 2 * sizeof(float), &cui.cameraPropertiesIdx);

    QVector2D oneOverSize = QVector2D(1.0f, 1.0f);
    auto &particleBuffer = renderable.particles.m_particleBuffer;
    const quint32 particlesPerSlice = particleBuffer.particlesPerSlice();
    oneOverSize = QVector2D(1.0f / particleBuffer.size().width(), 1.0f / particleBuffer.size().height());
    shaders.setUniform(ubufData, "qt_oneOverParticleImageSize", &oneOverSize, 2 * sizeof(float));
    shaders.setUniform(ubufData, "qt_countPerSlice", &particlesPerSlice, 1 * sizeof(quint32));

    // Global opacity of the particles node
    shaders.setUniform(ubufData, "qt_opacity", &renderable.opacity, 1 * sizeof(float));

    float blendImages = renderable.particles.m_blendImages ? 1.0f : 0.0f;
    float imageCount = float(renderable.particles.m_spriteImageCount);
    float ooImageCount = 1.0f / imageCount;

    QVector4D spriteConfig(imageCount, ooImageCount, 0.0f, blendImages);
    shaders.setUniform(ubufData, "qt_spriteConfig", &spriteConfig, 4 * sizeof(float));

    const float billboard = renderable.particles.m_billboard ? 1.0f : 0.0f;
    shaders.setUniform(ubufData, "qt_billboard", &billboard, 1 * sizeof(float));

    // Lights
    QVector3D theLightAmbientTotal;
    bool hasLights = !renderable.particles.m_lights.isEmpty();
    int pointLight = 0;
    int spotLight = 0;
    if (hasLights) {
        ParticleLightData lightData;
        auto &lights = renderable.lights;
        for (quint32 lightIdx = 0, lightEnd = lights.size();
             lightIdx < lightEnd && lightIdx < QSSG_MAX_NUM_LIGHTS; ++lightIdx) {
            QSSGRenderLight *theLight(lights[lightIdx].light);
            // Ignore lights which are not specified for the particle
            if (!renderable.particles.m_lights.contains(theLight))
                continue;

            const QMatrix4x4 lightGlobalTransform = inData.getGlobalTransform(*theLight);

            if (theLight->type == QSSGRenderLight::Type::DirectionalLight) {
                theLightAmbientTotal += theLight->m_diffuseColor * theLight->m_brightness;
            } else if (theLight->type == QSSGRenderLight::Type::PointLight && pointLight < 4) {
                lightData.pointLightColor[pointLight] = QVector4D(theLight->m_diffuseColor * theLight->m_brightness, 1.0f);
                lightData.pointLightPos[pointLight] = QVector4D(QSSGRenderNode::getGlobalPos(lightGlobalTransform), 1.0f);
                lightData.pointLightConstantAtt[pointLight] = QSSGUtils::aux::translateConstantAttenuation(theLight->m_constantFade);
                lightData.pointLightLinearAtt[pointLight] = QSSGUtils::aux::translateLinearAttenuation(theLight->m_linearFade);
                lightData.pointLightQuadAtt[pointLight] = QSSGUtils::aux::translateQuadraticAttenuation(theLight->m_quadraticFade);
                pointLight++;
            } else if (theLight->type == QSSGRenderLight::Type::SpotLight && spotLight < 4) {
                lightData.spotLightColor[spotLight] = QVector4D(theLight->m_diffuseColor * theLight->m_brightness, 1.0f);
                lightData.spotLightPos[spotLight] = QVector4D(QSSGRenderNode::getGlobalPos(lightGlobalTransform), 1.0f);
                lightData.spotLightDir[spotLight] = QVector4D(lights[lightIdx].direction, 0.0f);
                lightData.spotLightConstantAtt[spotLight] = QSSGUtils::aux::translateConstantAttenuation(theLight->m_constantFade);
                lightData.spotLightLinearAtt[spotLight] = QSSGUtils::aux::translateLinearAttenuation(theLight->m_linearFade);
                lightData.spotLightQuadAtt[spotLight] = QSSGUtils::aux::translateQuadraticAttenuation(theLight->m_quadraticFade);
                float coneAngle = theLight->m_coneAngle;
                // Inner cone angle must always be < cone angle, to not have possible undefined behavior for shader smoothstep
                float innerConeAngle = std::min(theLight->m_innerConeAngle, coneAngle - 0.01f);
                lightData.spotLightConeAngle[spotLight] = qDegreesToRadians(coneAngle);
                lightData.spotLightInnerConeAngle[spotLight] = qDegreesToRadians(innerConeAngle);
                spotLight++;
            }
            theLightAmbientTotal += theLight->m_ambientColor;
        }
        // Copy light data
        int lightOffset = shaders.offsetOfUniform("lightData");
        if (lightOffset >= 0)
            memcpy(ubufData + lightOffset, &lightData, sizeof(ParticleLightData));
    }
    shaders.setUniform(ubufData, "qt_light_ambient_total", &theLightAmbientTotal, 3 * sizeof(float), &cui.light_ambient_totalIdx);
    int enablePointLights = pointLight > 0 ? 1 : 0;
    int enableSpotLights = spotLight > 0 ? 1 : 0;
    shaders.setUniform(ubufData, "qt_pointLights", &enablePointLights, sizeof(int));
    shaders.setUniform(ubufData, "qt_spotLights", &enableSpotLights, sizeof(int));

    // Line particle uniform
    int segmentCount = particleBuffer.segments();
    if (segmentCount) {
        shaders.setUniform(ubufData, "qt_lineSegmentCount", &segmentCount, sizeof(int));
        float alphaFade = renderable.particles.m_alphaFade;
        float sizeModifier = renderable.particles.m_sizeModifier;
        float texcoordScale = renderable.particles.m_texcoordScale;
        auto image = renderable.firstImage;
        if (image && image->m_texture.m_texture) {
            const auto size = image->m_texture.m_texture->pixelSize();
            texcoordScale *= float(size.height()) / float(size.width());
        }
        shaders.setUniform(ubufData, "qt_alphaFade", &alphaFade, sizeof(float));
        shaders.setUniform(ubufData, "qt_sizeModifier", &sizeModifier, sizeof(float));
        shaders.setUniform(ubufData, "qt_texcoordScale", &texcoordScale, sizeof(float));
    }

#ifdef QSSG_OIT_USE_BUFFERS
    shaders.setOITImages((QRhiTexture*)inData.oitRenderContext.aBuffer,
                         (QRhiTexture*)inData.oitRenderContext.auxBuffer,
                         (QRhiTexture*)inData.oitRenderContext.counterBuffer);
    if (inData.oitRenderContext.aBuffer) {
#else
    const QSSGRhiRenderableTexture *abuf = inData.getRenderResult(QSSGRenderResult::Key::ABufferImage);
    const QSSGRhiRenderableTexture *aux = inData.getRenderResult(QSSGRenderResult::Key::AuxiliaryImage);
    const QSSGRhiRenderableTexture *counter = inData.getRenderResult(QSSGRenderResult::Key::CounterImage);
    shaders.setOITImages(abuf->texture, aux->texture, counter->texture);
    if (abuf->texture) {
#endif
        int abufWidth = RenderHelpers::rhiCalculateABufferSize(inData.layer.oitNodeCount);
        int listNodeCount = abufWidth * abufWidth;
        shaders.setUniform(ubufData, "qt_ABufImageWidth", &abufWidth, sizeof(int), &cui.abufImageWidth);
        shaders.setUniform(ubufData, "qt_listNodeCount", &listNodeCount, sizeof(int), &cui.listNodeCount);
        int viewSize[2] = {inData.layerPrepResult.textureDimensions().width(), inData.layerPrepResult.textureDimensions().height()};
        shaders.setUniform(ubufData, "qt_viewSize", viewSize, sizeof(int) * 2, &cui.viewSize);
        int samples = rhiCtx->mainPassSampleCount();
        shaders.setUniform(ubufData, "qt_samples", &samples, sizeof(int), &cui.samples);
    }

}

void QSSGParticleRenderer::updateUniformsForParticleModel(QSSGRhiShaderPipeline &shaderPipeline,
                                                          char *ubufData,
                                                          const QSSGRenderModel *model,
                                                          quint32 offset)
{
    auto &particleBuffer = *model->particleBuffer;
    const quint32 particlesPerSlice = particleBuffer.particlesPerSlice();
    const QVector2D oneOverSize = QVector2D(1.0f / particleBuffer.size().width(), 1.0f / particleBuffer.size().height());
    shaderPipeline.setUniform(ubufData, "qt_oneOverParticleImageSize", &oneOverSize, 2 * sizeof(float));
    shaderPipeline.setUniform(ubufData, "qt_countPerSlice", &particlesPerSlice, sizeof(quint32));
    const QMatrix4x4 &particleMatrix = model->particleMatrix;
    shaderPipeline.setUniform(ubufData, "qt_particleMatrix", &particleMatrix, 16 * sizeof(float));
    shaderPipeline.setUniform(ubufData, "qt_particleIndexOffset", &offset, sizeof(quint32));
}

static void fillTargetBlend(QRhiGraphicsPipeline::TargetBlend &targetBlend, QSSGRenderParticles::BlendMode mode)
{
    switch (mode) {
    case QSSGRenderParticles::BlendMode::Screen:
        targetBlend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
        targetBlend.dstColor = QRhiGraphicsPipeline::One;
        targetBlend.srcAlpha = QRhiGraphicsPipeline::One;
        targetBlend.dstAlpha = QRhiGraphicsPipeline::One;
        break;
    case QSSGRenderParticles::BlendMode::Multiply:
        targetBlend.srcColor = QRhiGraphicsPipeline::DstColor;
        targetBlend.dstColor = QRhiGraphicsPipeline::Zero;
        targetBlend.srcAlpha = QRhiGraphicsPipeline::One;
        targetBlend.dstAlpha = QRhiGraphicsPipeline::One;
        break;
    default:
        // Source over as default
        targetBlend.srcColor = QRhiGraphicsPipeline::SrcAlpha;
        targetBlend.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        targetBlend.srcAlpha = QRhiGraphicsPipeline::One;
        targetBlend.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        break;

    }
}

static void sortParticles(const QSSGLayerRenderData &renderData, QByteArray &result, QList<QSSGRhiSortData> &sortData,
                          const QSSGParticleBuffer &buffer, const QSSGRenderParticles &particles,
                          const QVector3D &cameraDirection, bool animatedParticles)
{
    const QMatrix4x4 &modelMatrix = renderData.getGlobalTransform(particles);
    const QMatrix4x4 &invModelMatrix = modelMatrix.inverted();
    QVector3D dir = invModelMatrix.map(cameraDirection);
    QVector3D n = dir.normalized();
    const auto segments = buffer.segments();
    auto particleCount = buffer.particleCount();
    const bool lineParticles = segments > 0;
    if (lineParticles)
        particleCount /= segments;
    sortData.resize(particleCount);
    sortData.fill({});

    const auto srcParticlePointer = [](int line, int segment, int sc, int ss, int pps, const char *source) -> const QSSGLineParticle * {
        int pi = (line * sc + segment) / pps;
        int i = (line * sc + segment) % pps;
        const QSSGLineParticle *sp = reinterpret_cast<const QSSGLineParticle *>(source + pi * ss);
        return sp + i;
    };

    // create sort data
    {
        const auto slices = buffer.sliceCount();
        const auto ss = buffer.sliceStride();
        const auto pps = buffer.particlesPerSlice();

        QSSGRhiSortData *dst = sortData.data();
        const char *source = buffer.pointer();
        const char *begin = source;
        int i = 0;
        if (lineParticles) {
            for (i = 0; i < particleCount; i++) {
                QSSGRhiSortData lineData;
                const QSSGLineParticle *lineBegin = srcParticlePointer(i, 0, segments, ss, pps, source);
                lineData.indexOrOffset = i;
                lineData.d = QVector3D::dotProduct(lineBegin->position, n);
                for (int j = 1; j < buffer.segments(); j++) {
                    const QSSGLineParticle *p = srcParticlePointer(i, j, segments, ss, pps, source);
                    lineData.d = qMin(lineData.d, QVector3D::dotProduct(p->position, n));
                }
                *dst++ = lineData;
            }
        } else if (animatedParticles) {
            for (int s = 0; s < slices; s++) {
                const QSSGParticleAnimated *sp = reinterpret_cast<const QSSGParticleAnimated *>(source);
                for (int p = 0; p < pps && i < particleCount; p++) {
                    *dst = { QVector3D::dotProduct(sp->position, n), int(reinterpret_cast<const char *>(sp) - begin)};
                    sp++;
                    dst++;
                    i++;
                }
                source += ss;
            }
        } else {
            for (int s = 0; s < slices; s++) {
                const QSSGParticleSimple *sp = reinterpret_cast<const QSSGParticleSimple *>(source);
                for (int p = 0; p < pps && i < particleCount; p++) {
                    *dst = { QVector3D::dotProduct(sp->position, n), int(reinterpret_cast<const char *>(sp) - begin)};
                    sp++;
                    dst++;
                    i++;
                }
                source += ss;
            }
        }
    }

    // sort
    result.resize(buffer.bufferSize());
    std::sort(sortData.begin(), sortData.end(), [](const QSSGRhiSortData &a, const QSSGRhiSortData &b){
        return a.d > b.d;
    });

    auto copyParticles = [&](QByteArray &dst, const QList<QSSGRhiSortData> &data, const QSSGParticleBuffer &buffer) {
        const auto slices = buffer.sliceCount();
        const auto ss = buffer.sliceStride();
        const auto pps = buffer.particlesPerSlice();
        const QSSGRhiSortData *sdata = data.data();
        char *dest = dst.data();
        const char *source = buffer.pointer();
        int i = 0;
        if (lineParticles) {
            int seg = 0;
            for (int s = 0; s < slices; s++) {
                QSSGLineParticle *dp = reinterpret_cast<QSSGLineParticle *>(dest);
                for (int p = 0; p < pps && i < particleCount; p++) {
                    *dp = *srcParticlePointer(sdata->indexOrOffset, seg, segments, ss, pps, source);
                    dp++;
                    seg++;
                    if (seg == segments) {
                        sdata++;
                        i++;
                        seg = 0;
                    }
                }
                dest += ss;
            }
        } else if (animatedParticles) {
            for (int s = 0; s < slices; s++) {
                QSSGParticleAnimated *dp = reinterpret_cast<QSSGParticleAnimated *>(dest);
                for (int p = 0; p < pps && i < particleCount; p++) {
                    *dp = *reinterpret_cast<const QSSGParticleAnimated *>(source + sdata->indexOrOffset);
                    dp++;
                    sdata++;
                    i++;
                }
                dest += ss;
            }
        } else {
            for (int s = 0; s < slices; s++) {
                QSSGParticleSimple *dp = reinterpret_cast<QSSGParticleSimple *>(dest);
                for (int p = 0; p < pps && i < particleCount; p++) {
                    *dp = *reinterpret_cast<const QSSGParticleSimple *>(source + sdata->indexOrOffset);
                    dp++;
                    sdata++;
                    i++;
                }
                dest += ss;
            }
        }
    };

    // write result
    copyParticles(result, sortData, buffer);
}

static QByteArray convertParticleData(QByteArray &dest, const QByteArray &data, bool convert)
{
    if (!convert)
        return data;
    int count = data.size() / 4;
    if (dest.size() != count * 2)
        dest.resize(2 * count);
    qFloatToFloat16(reinterpret_cast<qfloat16 *>(dest.data()), reinterpret_cast<const float *>(data.constData()), count);
    return dest;
}

void QSSGParticleRenderer::rhiPrepareRenderable(QSSGRhiShaderPipeline &shaderPipeline,
                                                QSSGPassKey passKey,
                                                QSSGRhiContext *rhiCtx,
                                                QSSGRhiGraphicsPipelineState *ps,
                                                QSSGParticlesRenderable &renderable,
                                                const QSSGLayerRenderData &inData,
                                                QRhiRenderPassDescriptor *renderPassDescriptor,
                                                int samples,
                                                int viewCount,
                                                QSSGRenderCamera *alteredCamera,
                                                QSSGRenderTextureCubeFace cubeFace,
                                                QSSGReflectionMapEntry *entry,
                                                bool oit)
{
    const void *node = &renderable.particles;
    const bool needsConversion = !rhiCtx->rhi()->isTextureFormatSupported(QRhiTexture::RGBA32F);

    const auto cubeFaceIdx = QSSGBaseTypeHelpers::indexOfCubeFace(cubeFace);
    QSSGRhiDrawCallData &dcd = QSSGRhiContextPrivate::get(rhiCtx)->drawCallData({ passKey, node, entry, cubeFaceIdx });
    shaderPipeline.ensureUniformBuffer(&dcd.ubuf);

    char *ubufData = dcd.ubuf->beginFullDynamicBufferUpdateForCurrentFrame();
    if (!alteredCamera) {
        updateUniformsForParticles(inData, shaderPipeline, rhiCtx, ubufData, renderable, inData.renderedCameras);
    } else {
        QSSGRenderCameraList cameras({ alteredCamera });
        updateUniformsForParticles(inData, shaderPipeline, rhiCtx, ubufData, renderable, cameras);
    }
    dcd.ubuf->endFullDynamicBufferUpdateForCurrentFrame();

    QSSGRhiParticleData &particleData = QSSGRhiContextPrivate::get(rhiCtx)->particleData(&renderable.particles);
    const QSSGParticleBuffer &particleBuffer = renderable.particles.m_particleBuffer;
    int particleCount = particleBuffer.particleCount();
    if (particleData.texture == nullptr || particleData.particleCount != particleCount) {
        QSize size(particleBuffer.size());
        if (!particleData.texture) {
            particleData.texture = rhiCtx->rhi()->newTexture(needsConversion ? QRhiTexture::RGBA16F : QRhiTexture::RGBA32F, size);
            particleData.texture->create();
        } else {
            particleData.texture->setPixelSize(size);
            particleData.texture->create();
        }
        particleData.particleCount = particleCount;
    }

    bool sortingChanged = particleData.sorting != renderable.particles.m_depthSorting;
    if (sortingChanged && !renderable.particles.m_depthSorting) {
        particleData.sortData.clear();
        particleData.sortedData.clear();
    }
    particleData.sorting = renderable.particles.m_depthSorting;

    QByteArray uploadData;

    if (renderable.particles.m_depthSorting) {
        bool animatedParticles = renderable.particles.m_featureLevel == QSSGRenderParticles::FeatureLevel::Animated;
        if (!alteredCamera) {
            sortParticles(inData, particleData.sortedData, particleData.sortData, particleBuffer, renderable.particles, inData.renderedCameraData.value()[0].direction, animatedParticles);
        } else {
            const QMatrix4x4 globalTransform = inData.getGlobalTransform(*alteredCamera);
            sortParticles(inData, particleData.sortedData, particleData.sortData, particleBuffer, renderable.particles, QSSGRenderNode::getScalingCorrectDirection(globalTransform), animatedParticles);
        }
        uploadData = convertParticleData(particleData.convertData, particleData.sortedData, needsConversion);
    } else {
        uploadData = convertParticleData(particleData.convertData, particleBuffer.data(), needsConversion);
    }

    QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
    QRhiTextureSubresourceUploadDescription upload;
    upload.setData(uploadData);
    QRhiTextureUploadDescription uploadDesc(QRhiTextureUploadEntry(0, 0, upload));
    rub->uploadTexture(particleData.texture, uploadDesc);
    rhiCtx->commandBuffer()->resourceUpdate(rub);

    auto &ia = QSSGRhiInputAssemblerStatePrivate::get(*ps);
    ia.topology = QRhiGraphicsPipeline::TriangleStrip;
    ia.inputLayout = QRhiVertexInputLayout();
    ia.inputs.clear();

    ps->samples = samples;
    ps->viewCount = viewCount;
    ps->cullMode = QRhiGraphicsPipeline::None;

    if (inData.orderIndependentTransparencyEnabled == false) {
        if (renderable.renderableFlags.hasTransparency())
            fillTargetBlend(ps->targetBlend[0], renderable.particles.m_blendMode);
        else
            ps->targetBlend[0] = QRhiGraphicsPipeline::TargetBlend();
    }

    QSSGRhiShaderResourceBindingList bindings;
    bindings.addUniformBuffer(0, VISIBILITY_ALL, dcd.ubuf, 0, shaderPipeline.ub0Size());

    // Texture maps
    // we only have one image
    QSSGRenderableImage *renderableImage = renderable.firstImage;

    int samplerBinding = shaderPipeline.bindingForTexture("qt_sprite");
    if (samplerBinding >= 0) {
        QRhiTexture *texture = renderableImage ? renderableImage->m_texture.m_texture : nullptr;
        if (samplerBinding >= 0 && texture) {
            const bool mipmapped = texture->flags().testFlag(QRhiTexture::MipMapped);
            QRhiSampler *sampler = rhiCtx->sampler({ QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_minFilterType),
                                                     QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_magFilterType),
                                                     mipmapped ? QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_mipFilterType) : QRhiSampler::None,
                                                     QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_horizontalTilingMode),
                                                     QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_verticalTilingMode),
                                                     QSSGRhiHelpers::toRhi(renderableImage->m_imageNode.m_depthTilingMode)
                                                   });
            bindings.addTexture(samplerBinding, QRhiShaderResourceBinding::FragmentStage, texture, sampler);
        } else {
            QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
            QRhiTexture *texture = rhiCtx->dummyTexture({}, rub, QSize(4, 4), Qt::white);
            rhiCtx->commandBuffer()->resourceUpdate(rub);
            QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                     QRhiSampler::Nearest,
                                                     QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::Repeat
                                                   });
            bindings.addTexture(samplerBinding, QRhiShaderResourceBinding::FragmentStage, texture, sampler);
        }
    }

    samplerBinding = shaderPipeline.bindingForTexture("qt_particleTexture");
    if (samplerBinding >= 0) {
        QRhiTexture *texture = particleData.texture;
        if (samplerBinding >= 0 && texture) {
            QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                     QRhiSampler::Nearest,
                                                     QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::Repeat
                                                   });
            bindings.addTexture(samplerBinding, QRhiShaderResourceBinding::VertexStage, texture, sampler);
        }
    }

    samplerBinding = shaderPipeline.bindingForTexture("qt_colorTable");
    if (samplerBinding >= 0) {
        bool hasTexture = false;
        if (renderable.colorTable) {
            QRhiTexture *texture = renderable.colorTable->m_texture.m_texture;
            if (texture) {
                hasTexture = true;
                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                         QRhiSampler::Nearest,
                                                         QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge,
                                                         QRhiSampler::ClampToEdge,
                                                         QRhiSampler::Repeat
                                                       });
                bindings.addTexture(samplerBinding, QRhiShaderResourceBinding::FragmentStage, texture, sampler);
            }
        }

        if (!hasTexture) {
            QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
            QRhiTexture *texture = rhiCtx->dummyTexture({}, rub, QSize(4, 4), Qt::white);
            rhiCtx->commandBuffer()->resourceUpdate(rub);
            QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                     QRhiSampler::Nearest,
                                                     QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::Repeat
                                                   });
            bindings.addTexture(samplerBinding, QRhiShaderResourceBinding::FragmentStage, texture, sampler);
        }
    }

    if (oit && inData.layer.oitMethod == QSSGRenderLayer::OITMethod::LinkedList)
        RenderHelpers::addAccumulatorImageBindings(&shaderPipeline, bindings);

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);

    // TODO: This is identical to other renderables. Make into a function?
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

void QSSGParticleRenderer::prepareParticlesForModel(QSSGRhiShaderPipeline &shaderPipeline,
                                                    QSSGRhiContext *rhiCtx,
                                                    QSSGRhiShaderResourceBindingList &bindings,
                                                    const QSSGRenderModel *model)
{
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    QSSGRhiParticleData &particleData = rhiCtxD->particleData(model);
    const QSSGParticleBuffer &particleBuffer = *model->particleBuffer;
    int particleCount = particleBuffer.particleCount();
    bool update = particleBuffer.serial() != particleData.serial;
    const bool needsConversion = !rhiCtx->rhi()->isTextureFormatSupported(QRhiTexture::RGBA32F);
    if (particleData.texture == nullptr || particleData.particleCount != particleCount) {
        QSize size(particleBuffer.size());
        if (!particleData.texture) {
            particleData.texture = rhiCtx->rhi()->newTexture(needsConversion ? QRhiTexture::RGBA16F : QRhiTexture::RGBA32F, size);
            particleData.texture->create();
        } else {
            particleData.texture->setPixelSize(size);
            particleData.texture->create();
        }
        particleData.particleCount = particleCount;
        update = true;
    }

    if (update) {
        QRhiResourceUpdateBatch *rub = rhiCtx->rhi()->nextResourceUpdateBatch();
        QRhiTextureSubresourceUploadDescription upload;
        upload.setData(convertParticleData(particleData.convertData, particleBuffer.data(), needsConversion));
        QRhiTextureUploadDescription uploadDesc(QRhiTextureUploadEntry(0, 0, upload));
        rub->uploadTexture(particleData.texture, uploadDesc);
        rhiCtx->commandBuffer()->resourceUpdate(rub);
    }
    particleData.serial = particleBuffer.serial();
    int samplerBinding = shaderPipeline.bindingForTexture("qt_particleTexture");
    if (samplerBinding >= 0) {
        QRhiTexture *texture = particleData.texture;
        if (samplerBinding >= 0 && texture) {
            QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Nearest,
                                                     QRhiSampler::Nearest,
                                                     QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::ClampToEdge,
                                                     QRhiSampler::Repeat
                                                   });
            bindings.addTexture(samplerBinding, QRhiShaderResourceBinding::VertexStage, texture, sampler);
        }
    }
}

static QByteArray particlesLogPrefix() { return QByteArrayLiteral("particle material pipeline-- "); }

struct ShaderGeneratorCommon
{
    QSSGStageGeneratorBase *vs;
    QSSGStageGeneratorBase *fs;
    ShaderGeneratorCommon(QSSGProgramGenerator &gen)
        : vs(gen.getStage(QSSGShaderGeneratorStage::Vertex)), fs(gen.getStage(QSSGShaderGeneratorStage::Fragment))
    {

    }
    void addUniform(const QByteArray &name, const QByteArray &type)
    {
        vs->addUniform(name, type);
        fs->addUniform(name, type);
    }
    void addUniformArray(const QByteArray &name, const QByteArray &type, quint32 size)
    {
        vs->addUniformArray(name, type, size);
        fs->addUniformArray(name, type, size);
    }
    void addDefinition(const QByteArray &name, const QByteArray &value)
    {
        vs->addDefinition(name, value);
        fs->addDefinition(name, value);
    }
    void addInterpolant(const QByteArray &name, const QByteArray &type)
    {
        vs->addOutgoing(name, type);
        fs->addIncoming(name, type);
    }
    void addFlatInterpolant(const QByteArray &name, const QByteArray &type)
    {
        vs->addFlatOutgoing(name, type);
        fs->addFlatIncoming(name, type);
    }
    void addPrefix(const QByteArray &prefix)
    {
        vs->addPrefix(prefix);
        fs->addPrefix(prefix);
    }
};

struct AutoFormatGenerator
{
    QSSGStageGeneratorBase &gen;
    int indent = 0;
    AutoFormatGenerator(QSSGStageGeneratorBase &g)
        : gen(g)
    {

    }
    AutoFormatGenerator &operator<<(const QByteArray &data)
    {
        for (int i = 0; i < indent; i++)
            gen << "    ";
        gen << data << "\n";
        return *this;
    }
    void incIndent()
    {
        indent++;
    }
    void decIndent()
    {
        indent--;
        Q_ASSERT(indent >= 0);
    }
};

static const char *s_lightPrefix = {
    "struct ParticleLightData\n" \
    "{\n"
    "    vec4 qt_pointLightPosition[4];\n" \
    "    vec4 qt_pointLightConstantAtt;\n" \
    "    vec4 qt_pointLightLinearAtt;\n" \
    "    vec4 qt_pointLightQuadAtt;\n" \
    "    vec4 qt_pointLightColor[4];\n" \
    "    vec4 qt_spotLightPosition[4];\n" \
    "    vec4 qt_spotLightConstantAtt;\n" \
    "    vec4 qt_spotLightLinearAtt;\n" \
    "    vec4 qt_spotLightQuadAtt;\n" \
    "    vec4 qt_spotLightColor[4];\n" \
    "    vec4 qt_spotLightDirection[4];\n" \
    "    vec4 qt_spotLightConeAngle;\n" \
    "    vec4 qt_spotLightInnerConeAngle;\n" \
    "};\n"
};

QSSGShaderFeatures QSSGParticleRenderer::particleShaderFeatures(const QSSGShaderFeatures& features)
{
#define COPY_FEATURE(f) ret.set(f, features.isSet(f));
    QSSGShaderFeatures ret;
    COPY_FEATURE(QSSGShaderFeatures::Feature::LinearTonemapping);
    COPY_FEATURE(QSSGShaderFeatures::Feature::AcesTonemapping);
    COPY_FEATURE(QSSGShaderFeatures::Feature::HejlDawsonTonemapping);
    COPY_FEATURE(QSSGShaderFeatures::Feature::FilmicTonemapping);
    COPY_FEATURE(QSSGShaderFeatures::Feature::DisableMultiView);
#undef COPY_FEATURE
    return ret;
}

QSSGRhiShaderPipelinePtr QSSGParticleRenderer::generateRhiShaderPipeline(QSSGRenderer &renderer,
                                                                         QSSGParticlesRenderable &inRenderable,
                                                                         const QSSGShaderFeatures &inFeatureSet,
                                                                         QByteArray &shaderString,
                                                                         const QSSGShaderParticleMaterialKeyProperties &shaderKeyProperties)
{
    const auto &m_contextInterface = renderer.m_contextInterface;
    const auto &shaderCache = m_contextInterface->shaderCache();
    const auto &shaderProgramGenerator = m_contextInterface->shaderProgramGenerator();
    const auto &shaderLibraryManager = m_contextInterface->shaderLibraryManager();

    shaderString = particlesLogPrefix();
    QSSGShaderParticleMaterialKey theKey(inRenderable.shaderDescription);

    theKey.toString(shaderString, shaderKeyProperties);

    if (s_shaderCacheEnabled) {
        if (const auto &maybePipeline = shaderCache->tryGetRhiShaderPipeline(shaderString, inFeatureSet))
            return maybePipeline;

        const QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(shaderString, QQsbCollection::toFeatureSet(inFeatureSet));
        // Check if there's a pre-built (offline generated) shader for available.
        if (renderer.m_currentLayer) {
            QQsbCollection::EntryMap &pregenEntries = renderer.m_currentLayer->m_particleShaderEntries;
            if (pregenEntries.isEmpty()) {
                renderer.m_currentLayer->m_particleShaderEntries = shaderLibraryManager->getParticleShaderEntries();
                pregenEntries = renderer.m_currentLayer->m_particleShaderEntries;
            }
            if (!pregenEntries.isEmpty()) {
                const auto foundIt = pregenEntries.constFind(QQsbCollection::Entry(qsbcKey));
                if (foundIt != pregenEntries.cend())
                    return shaderCache->newPipelineFromPregenerated(shaderString, inFeatureSet, *foundIt, inRenderable.particles);
            }
        }

        // Try the persistent (disk-based) cache then.
        if (const auto &maybePipeline = shaderCache->tryNewPipelineFromPersistentCache(qsbcKey, shaderString, inFeatureSet))
            return maybePipeline;
    }

    // generate shader code
    shaderProgramGenerator->beginProgram();
    auto &vertex = *shaderProgramGenerator->getStage(QSSGShaderGeneratorStage::Vertex);
    auto &fragment = *shaderProgramGenerator->getStage(QSSGShaderGeneratorStage::Fragment);
    ShaderGeneratorCommon common(*shaderProgramGenerator);
    const bool isSpriteLinear = shaderKeyProperties.m_isSpriteLinear.getValue(theKey);
    const bool isColorTableLinear = shaderKeyProperties.m_isColorTableLinear.getValue(theKey);
    const bool lighting = shaderKeyProperties.m_hasLighting.getValue(theKey);
    const bool isMapped = shaderKeyProperties.m_isMapped.getValue(theKey);
    const bool isLineParticle = shaderKeyProperties.m_isLineParticle.getValue(theKey);
    const bool isAnimated = shaderKeyProperties.m_isAnimated.getValue(theKey);
    const int viewCount = inFeatureSet.isSet(QSSGShaderFeatures::Feature::DisableMultiView)
            ? 1 : shaderKeyProperties.m_viewCount.getValue(theKey);
    const int oit = shaderKeyProperties.m_orderIndependentTransparency.getValue(theKey);
    const bool oitMSAA = shaderKeyProperties.m_oitMSAA.getValue(theKey);

    if (isAnimated)
        common.addDefinition("QSSG_PARTICLES_ENABLE_ANIMATED", "1");
    if (isLineParticle)
        common.addDefinition("QSSG_PARTICLES_ENABLE_LINE_PARTICLE", "1");
    if (oit)
        common.addDefinition("QSSG_OIT_METHOD", QStringLiteral("%1").arg(oit).toUtf8());
    if (oitMSAA)
        common.addDefinition("QSSG_MULTISAMPLE", "1");

    common.addUniform("qt_modelMatrix", "mat4");
    if (viewCount >= 2) {
        common.addUniformArray("qt_viewMatrix", "mat4", viewCount);
        common.addUniformArray("qt_projectionMatrix", "mat4", viewCount);
    } else {
        common.addUniform("qt_viewMatrix", "mat4");
        common.addUniform("qt_projectionMatrix", "mat4");
    }
    if (lighting) {
        common.addPrefix(s_lightPrefix);
        common.addUniform("lightData", "ParticleLightData");
    }
    common.addUniform("qt_spriteConfig", "vec4");
    common.addUniform("qt_light_ambient_total", "vec3");
    common.addUniform("qt_oneOverParticleImageSize", "vec2");
    common.addUniform("qt_countPerSlice", "uint");
    if (isLineParticle)
        common.addUniform("qt_lineSegmentCount", "uint");
    common.addUniform("qt_billboard", "float");
    common.addUniform("qt_opacity", "float");
    if (isLineParticle) {
        common.addUniform("qt_alphaFade", "float");
        common.addUniform("qt_sizeModifier", "float");
        common.addUniform("qt_texcoordScale", "float");
    }
    if (lighting) {
        common.addUniform("qt_pointLights", "bool");
        common.addUniform("qt_spotLights", "bool");
    }
    if (oit == (int)QSSGRenderLayer::OITMethod::WeightedBlended) {
        common.addUniform("qt_cameraProperties", "vec2");
    } else if (oit == (int)QSSGRenderLayer::OITMethod::LinkedList) {
        common.addUniform("qt_viewSize", "ivec2");
        common.addUniform("qt_ABufImageWidth", "uint");
        common.addUniform("qt_listNodeCount", "uint");
#ifdef QSSG_OIT_USE_BUFFERS
        common.addUniform("qt_samples", "uint");
#endif
    }

    if (lighting)
        vertex.addInclude("particleLighting.glsllib");

    vertex.addUniform("qt_particleTexture", "sampler2D");

    vertex.addInclude("particleLoading.glsllib");

    common.addInterpolant("color", "vec4");
    common.addInterpolant("spriteData", "vec2");
    common.addInterpolant("texcoord", "vec2");
    common.addFlatInterpolant("instanceIndex", "uint");
    if (oit == (int)QSSGRenderLayer::OITMethod::LinkedList && viewCount >= 2)
        common.addFlatInterpolant("v_viewIndex", "uint");

    AutoFormatGenerator vg(vertex);
    if (!isLineParticle)
        vg << "const vec2 corners[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};";

    // vertex shader main
    vg << "out gl_PerVertex {"
       << "    vec4 gl_Position;"
       << "};";

    vg << "void main() {";
    vg.incIndent();
    if (isLineParticle) {
        vg << "uint segmentIndex = gl_VertexIndex / 2;"
               << "uint particleIndex = segmentIndex + gl_InstanceIndex * qt_lineSegmentCount;";
    } else {
        vg << "uint particleIndex = gl_InstanceIndex;"
               << "uint cornerIndex = gl_VertexIndex;"
               << "vec2 corner = corners[cornerIndex];";
    }
    vg << "instanceIndex = particleIndex;"
           << "Particle p = qt_loadParticle(particleIndex);";

    if (isLineParticle) {
        vg << "float side = float(mod(gl_VertexIndex, 2)) - 0.5;"
           << "float size = p.size * qt_calcSizeFactor(segmentIndex);"
           << "vec3 o = p.binormal * side * size;"
           << "vec4 worldPos = qt_modelMatrix * vec4(p.position + (1.0 - qt_billboard) * o, 1.0);";
        if (viewCount >= 2) {
            vg << "vec4 viewPos = qt_viewMatrix[gl_ViewIndex] * worldPos;"
               << "vec3 viewBN = (transpose(inverse(qt_viewMatrix[gl_ViewIndex] * qt_modelMatrix)) * vec4(p.binormal, 0.0)).xyz;";
        } else {
            vg << "vec4 viewPos = qt_viewMatrix * worldPos;"
               << "vec3 viewBN = (transpose(inverse(qt_viewMatrix * qt_modelMatrix)) * vec4(p.binormal, 0.0)).xyz;";
        }
        vg << "viewBN.z = 0;"
           << "viewBN = normalize(viewBN) * side * size;"
           << "viewPos = viewPos + vec4(viewBN, 0.0) * qt_billboard;"
           << "texcoord.x = p.segmentLength * qt_texcoordScale;"
           << "texcoord.y = float(mod(gl_VertexIndex, 2));"
           << "color = p.color;"
           << "color.a *= qt_opacity * qt_calcAlphaFade(segmentIndex);";
    } else {
        vg << "mat3 rotMat = qt_fromEulerRotation(p.rotation);"
           << "vec4 worldPos = qt_modelMatrix * vec4(p.position, 1.0);"
           << "vec4 viewPos = worldPos;"
           << "vec3 offset = (p.size * vec3(corner - vec2(0.5), 0.0));"
           << "viewPos.xyz += offset * rotMat * (1.0 - qt_billboard);";
        if (viewCount >= 2)
            vg << "viewPos = qt_viewMatrix[gl_ViewIndex] * viewPos;";
        else
            vg << "viewPos = qt_viewMatrix * viewPos;";
        vg << "viewPos.xyz += rotMat * offset * qt_billboard;"
           << "texcoord = corner;"
           << "color = p.color;"
           << "color.a *= qt_opacity;";
    }
    if (lighting)
        vg << "color.rgb *= qt_calcLightColor(worldPos.xyz);";
    if (viewCount >= 2) {
        if (oit == int(QSSGRenderLayer::OITMethod::LinkedList))
            vg << "v_viewIndex = gl_ViewIndex;";
        vg << "gl_Position = qt_projectionMatrix[gl_ViewIndex] * viewPos;";
    } else {
        vg << "gl_Position = qt_projectionMatrix * viewPos;";
    }
    if (isMapped)
        vg << "spriteData.x = qt_ageToSpriteFactor(p.age);";
    if (isAnimated)
        vg << "spriteData.y = p.animationFrame;";
    vg.decIndent();
    vg << "}";

    // fragment shader main
    fragment.addInclude("tonemapping.glsllib");
    fragment.addInclude("particleSprite.glsllib");
    fragment.addUniform("qt_sprite", "sampler2D");

    AutoFormatGenerator fg(fragment);

    if (oit == int(QSSGRenderLayer::OITMethod::WeightedBlended)) {
        fragment.addInclude("oitweightedblended.glsllib");
        fg << "layout(location = 1) out vec4 revealageOutput;";
    } else if (oit == int(QSSGRenderLayer::OITMethod::LinkedList)) {
#ifdef QSSG_OIT_USE_BUFFERS
        fragment.addInclude("oitlinkedlist_buf.glsllib");
        QSSGShaderResourceMergeContext::setAdditionalBufferAmount(3);
#else
        fragment.addInclude("oitlinkedlist.glsllib");
#endif
    }
    if (isSpriteLinear)
        fg << "#define spriteFunc";
    else
        fg << "#define spriteFunc qt_sRGBToLinear";
    if (isMapped) {
        fragment.addUniform("qt_colorTable", "sampler2D");
        if (isColorTableLinear)
            fg << "#define colorTableFunc";
        else
            fg << "#define colorTableFunc qt_sRGBToLinear";
    }

    fg << "vec4 qt_readSprite() {";
    fg.incIndent();
    if (isAnimated) {
        fg << "vec2 coords[2];"
           << "float factor = qt_spriteCoords(coords);"
           << "return mix(spriteFunc(texture(qt_sprite, coords[0])), spriteFunc(texture(qt_sprite, coords[1])), factor);";
    } else {
        fg << "return spriteFunc(texture(qt_sprite, texcoord));";
    }
    fg.decIndent();
    fg << "}";
    fg << "vec4 qt_readColor() {";
    fg.incIndent();
    if (isMapped)
        fg << "return color * colorTableFunc(texture(qt_colorTable, vec2(spriteData.x, qt_rand(vec2(instanceIndex, 0)))));";
    else
        fg << "return color;";
    fg.decIndent();
    fg << "}";
    fg << "void main() {";
    fg.incIndent();
    fg << "vec4 ret = qt_readColor() * qt_readSprite();";
    if (oit) {
        if (oit == int(QSSGRenderLayer::OITMethod::WeightedBlended)) {
            fg << "float z = abs(gl_FragCoord.z);"
               << "float distWeight = qt_transparencyWeight(z, ret.a, qt_cameraProperties.y);"
               << "fragOutput = distWeight * qt_tonemap(ret);"
               << "revealageOutput = vec4(ret.a);";
        } else if (oit == int(QSSGRenderLayer::OITMethod::LinkedList)) {
#ifdef QSSG_OIT_USE_BUFFERS
            if (viewCount >= 2)
                fg << "fragOutput = qt_oitLinkedList(ret, qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, v_viewIndex, qt_samples);";
            else
                fg << "fragOutput = qt_oitLinkedList(ret, qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, 0, qt_samples);";
#else
            if (viewCount >= 2)
                fg << "fragOutput = qt_oitLinkedList(ret, qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, v_viewIndex);";
            else
                fg << "fragOutput = qt_oitLinkedList(ret, qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, 0);";
#endif
        }
    } else {
        fg << "fragOutput = qt_tonemap(ret);";
    }
    fg.decIndent();
    fg << "}";


    return shaderProgramGenerator->compileGeneratedRhiShader(shaderString,
                                                             inFeatureSet,
                                                             *shaderLibraryManager,
                                                             *shaderCache,
                                                             QSSGRhiShaderPipeline::UsedWithoutIa,
                                                             {},
                                                             shaderKeyProperties.m_viewCount.getValue(inRenderable.shaderDescription),
                                                             false);
}

QSSGRhiShaderPipelinePtr QSSGParticleRenderer::getShaderPipelineParticles(QSSGRenderer &renderer,
                                                           QSSGParticlesRenderable &inRenderable,
                                                           const QSSGShaderFeatures &inFeatureSet)
{
    const auto features = particleShaderFeatures(inFeatureSet);
    auto *m_currentLayer = renderer.m_currentLayer;
    QSSG_ASSERT(m_currentLayer != nullptr, return {});

    auto &shaderMap = m_currentLayer->particleShaderMap;

    QElapsedTimer timer;
    timer.start();

    QSSGRhiShaderPipelinePtr shaderPipeline;

    // This just references inFeatureSet and inRenderable.shaderDescription -
    // cheap to construct and is good enough for the find()
    QSSGParticleShaderMapKey skey = QSSGParticleShaderMapKey(QByteArray(),
                                             features,
                                             inRenderable.shaderDescription);
    auto it = shaderMap.find(skey);
    if (it == shaderMap.end()) {
        Q_TRACE_SCOPE(QSSG_generateShader);
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DGenerateShader);
        auto &shaderString = renderer.m_currentLayer->generatedShaderString;
        shaderPipeline = QSSGParticleRenderer::generateRhiShaderPipeline(renderer, inRenderable, features, shaderString, m_currentLayer->particleMaterialShaderKeyProperties);
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DGenerateShader, 0, inRenderable.particles.profilingId);
        // make skey useable as a key for the QHash (makes a copy of the materialKey, instead of just referencing)
        skey.detach();
        // insert it no matter what, no point in trying over and over again
        shaderMap.insert(skey, shaderPipeline);
    } else {
        shaderPipeline = it.value();
    }

    if (shaderPipeline != nullptr) {
        if (m_currentLayer && !m_currentLayer->renderedCameras.isEmpty())
            m_currentLayer->ensureCachedCameraDatas();
    }

    const auto &rhiContext = renderer.m_contextInterface->rhiContext();
    QSSGRhiContextStats::get(*rhiContext).registerMaterialShaderGenerationTime(timer.elapsed());

    return shaderPipeline;
}

void QSSGParticleRenderer::rhiRenderRenderable(QSSGRhiContext *rhiCtx,
                                               QSSGParticlesRenderable &renderable,
                                               bool *needsSetViewport,
                                               QSSGRenderTextureCubeFace cubeFace,
                                               const QSSGRhiGraphicsPipelineState &state)
{
    QRhiGraphicsPipeline *ps = renderable.rhiRenderData.mainPass.pipeline;
    QRhiShaderResourceBindings *srb = renderable.rhiRenderData.mainPass.srb;

    if (cubeFace != QSSGRenderTextureCubeFaceNone) {
        const auto cubeFaceIdx = QSSGBaseTypeHelpers::indexOfCubeFace(cubeFace);
        ps = renderable.rhiRenderData.reflectionPass.pipeline;
        srb = renderable.rhiRenderData.reflectionPass.srb[cubeFaceIdx];
    }

    if (!ps || !srb)
        return;

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    // QRhi optimizes out unnecessary binding of the same pipline
    cb->setGraphicsPipeline(ps);
    cb->setVertexInput(0, 0, nullptr);
    cb->setShaderResources(srb);

    if (needsSetViewport && *needsSetViewport) {
        cb->setViewport(state.viewport);
        *needsSetViewport = false;
    }
    if (renderable.particles.m_featureLevel >= QSSGRenderParticles::FeatureLevel::Line) {
        // draw triangle strip with 2 * segmentCount vertices N times
        int S = renderable.particles.m_particleBuffer.segments();
        int N = renderable.particles.m_particleBuffer.particleCount() / S;
        cb->draw(2 * S, N);
        QSSGRHICTX_STAT(rhiCtx, draw(2 * S, N));
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DRenderCall, (2 * S | quint64(N) << 32), renderable.particles.profilingId);
    } else {
        // draw triangle strip with 2 triangles N times
        cb->draw(4, renderable.particles.m_particleBuffer.particleCount());
        QSSGRHICTX_STAT(rhiCtx, draw(4, renderable.particles.m_particleBuffer.particleCount()));
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DRenderCall, (4 | quint64(renderable.particles.m_particleBuffer.particleCount()) << 32), renderable.particles.profilingId);
    }
}

QT_END_NAMESPACE
