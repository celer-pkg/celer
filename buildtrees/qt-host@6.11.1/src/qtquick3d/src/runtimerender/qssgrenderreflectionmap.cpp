// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderreflectionprobe_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include "qssgrendercontextcore.h"

QT_BEGIN_NAMESPACE

const int prefilterSampleCount = 16;

QSSGRenderReflectionMap::QSSGRenderReflectionMap(const QSSGRenderContextInterface &inContext)
    : m_context(inContext)
{
}

QSSGRenderReflectionMap::~QSSGRenderReflectionMap()
{
    releaseCachedResources();
}

void QSSGRenderReflectionMap::releaseCachedResources()
{
    for (QSSGReflectionMapEntry &entry : m_reflectionMapList)
        entry.destroyRhiResources();

    m_reflectionMapList.clear();
}

static QRhiTexture *allocateRhiReflectionTexture(QRhi *rhi,
                                                 QRhiTexture::Format format,
                                                 const QSize &size,
                                                 QRhiTexture::Flags flags = {})
{
    auto texture = rhi->newTexture(format, size, 1, flags);
    if (!texture->create())
        qWarning("Failed to create reflection map texture of size %dx%d", size.width(), size.height());
    return texture;
}

static QRhiRenderBuffer *allocateRhiReflectionRenderBuffer(QRhi *rhi,
                                                           QRhiRenderBuffer::Type type,
                                                           const QSize &size)
{
    auto renderBuffer = rhi->newRenderBuffer(type, size, 1);
    if (!renderBuffer->create())
        qWarning("Failed to build depth-stencil buffer of size %dx%d", size.width(), size.height());
    return renderBuffer;
}


void QSSGRenderReflectionMap::addReflectionMapEntry(qint32 probeIdx, const QSSGRenderReflectionProbe &probe)
{
    QRhi *rhi = m_context.rhiContext()->rhi();
    // Bail out if there is no QRhi, since we can't add entries without it
    if (!rhi)
        return;

    QRhiTexture::Format rhiFormat = QRhiTexture::RGBA16F;

    const QByteArray rtName = probe.debugObjectName.toLatin1();

    const int mapRes = 1 << probe.reflectionMapRes;
    QSize pixelSize(mapRes, mapRes);
    QSSGReflectionMapEntry *pEntry = reflectionMapEntry(probeIdx);

    if (!pEntry) {
        QRhiRenderBuffer *depthStencil = allocateRhiReflectionRenderBuffer(rhi, QRhiRenderBuffer::DepthStencil, pixelSize);
        QRhiTexture *map = allocateRhiReflectionTexture(rhi, rhiFormat, pixelSize, QRhiTexture::RenderTarget | QRhiTexture::CubeMap
                                                                    | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);
        QRhiTexture *prefiltered = allocateRhiReflectionTexture(rhi, rhiFormat, pixelSize, QRhiTexture::RenderTarget | QRhiTexture::CubeMap
                                                                            | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);
        m_reflectionMapList.push_back(QSSGReflectionMapEntry::withRhiCubeMap(probeIdx, map, prefiltered, depthStencil));

        pEntry = &m_reflectionMapList.back();
    }

    if (pEntry) {
        pEntry->m_needsRender = true;

        if (probe.hasScheduledUpdate)
            pEntry->m_rendered = false;

        if (!pEntry->m_rhiDepthStencil || mapRes != pEntry->m_rhiCube->pixelSize().width()) {
            pEntry->destroyRhiResources();
            pEntry->m_rhiDepthStencil = allocateRhiReflectionRenderBuffer(rhi, QRhiRenderBuffer::DepthStencil, pixelSize);
            pEntry->m_rhiCube = allocateRhiReflectionTexture(rhi, rhiFormat, pixelSize, QRhiTexture::RenderTarget | QRhiTexture::CubeMap
                                                                         | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);
            pEntry->m_rhiPrefilteredCube = allocateRhiReflectionTexture(rhi, rhiFormat, pixelSize, QRhiTexture::RenderTarget | QRhiTexture::CubeMap
                                                                                    | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips);
        }

        // Additional graphics resources: samplers, render targets.
        if (pEntry->m_rhiRenderTargets.isEmpty()) {
            pEntry->m_rhiRenderTargets.resize(6);
            for (int i = 0; i < 6; ++i)
                pEntry->m_rhiRenderTargets[i] = nullptr;
        }
        Q_ASSERT(pEntry->m_rhiRenderTargets.size() == 6);

        if (pEntry->m_skyBoxSrbs.isEmpty()) {
            pEntry->m_skyBoxSrbs.resize(6);
            for (int i = 0; i < 6; ++i)
                pEntry->m_skyBoxSrbs[i] = nullptr;
        }


        for (const auto face : QSSGRenderTextureCubeFaces) {
            QRhiTextureRenderTarget *&rt(pEntry->m_rhiRenderTargets[quint8(face)]);
            if (!rt) {
                QRhiColorAttachment att(pEntry->m_rhiCube);
                att.setLayer(quint8(face)); // 6 render targets, each referencing one face of the cubemap
                QRhiTextureRenderTargetDescription rtDesc;
                rtDesc.setColorAttachments({ att });
                rtDesc.setDepthStencilBuffer(pEntry->m_rhiDepthStencil);
                rt = rhi->newTextureRenderTarget(rtDesc);
                rt->setDescription(rtDesc);
                if (!pEntry->m_rhiRenderPassDesc)
                    pEntry->m_rhiRenderPassDesc = rt->newCompatibleRenderPassDescriptor();
                rt->setRenderPassDescriptor(pEntry->m_rhiRenderPassDesc);
                if (!rt->create())
                    qWarning("Failed to build reflection map render target");
            }
            rt->setName(rtName + QByteArrayLiteral(" reflection cube face: ") + QSSGBaseTypeHelpers::displayName(face));
        }

        if (!pEntry->m_prefilterPipeline) {
            const QSize mapSize = pEntry->m_rhiCube->pixelSize();

            int mipmapCount = rhi->mipLevelsForSize(mapSize);
            mipmapCount = qMin(mipmapCount, 6);  // don't create more than 6 mip levels

            // Create a renderbuffer for each mip level
            for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
                const QSize levelSize = QSize(mapSize.width() * std::pow(0.5, mipLevel),
                                              mapSize.height() * std::pow(0.5, mipLevel));
                pEntry->m_prefilterMipLevelSizes.insert(mipLevel, levelSize);
                // Setup Render targets (6 * mipmapCount)
                QVarLengthArray<QRhiTextureRenderTarget *, 6> renderTargets;
                for (const auto face : QSSGRenderTextureCubeFaces) {
                    QRhiColorAttachment att(pEntry->m_rhiPrefilteredCube);
                    att.setLayer(quint8(face));
                    att.setLevel(mipLevel);
                    QRhiTextureRenderTargetDescription rtDesc;
                    rtDesc.setColorAttachments({att});
                    auto renderTarget = rhi->newTextureRenderTarget(rtDesc);
                    renderTarget->setName(rtName + QByteArrayLiteral(" reflection prefilter mip/face ")
                                          + QByteArray::number(mipLevel) + QByteArrayLiteral("/") + QSSGBaseTypeHelpers::displayName(face));
                    renderTarget->setDescription(rtDesc);
                    if (!pEntry->m_rhiPrefilterRenderPassDesc)
                        pEntry->m_rhiPrefilterRenderPassDesc = renderTarget->newCompatibleRenderPassDescriptor();
                    renderTarget->setRenderPassDescriptor(pEntry->m_rhiPrefilterRenderPassDesc);
                    if (!renderTarget->create())
                        qWarning("Failed to build prefilter cube map render target");
                    renderTargets << renderTarget;
                }
                pEntry->m_rhiPrefilterRenderTargetsMap.insert(mipLevel, renderTargets);
            }

            const auto &prefilterShaderStages = m_context.shaderCache()->getBuiltInRhiShaders().getRhiReflectionprobePreFilterShader();

            const QSSGRhiSamplerDescription samplerMipMapDesc {
                QRhiSampler::Linear,
                QRhiSampler::Linear,
                QRhiSampler::Linear,
                QRhiSampler::ClampToEdge,
                QRhiSampler::ClampToEdge,
                QRhiSampler::Repeat
            };

            const QSSGRhiSamplerDescription samplerDesc {
                QRhiSampler::Linear,
                QRhiSampler::Linear,
                QRhiSampler::None,
                QRhiSampler::ClampToEdge,
                QRhiSampler::ClampToEdge,
                QRhiSampler::Repeat
            };

            QRhiSampler *sampler = m_context.rhiContext()->sampler(samplerDesc);
            QRhiSampler *cubeSampler = m_context.rhiContext()->sampler(samplerMipMapDesc);

            QRhiVertexInputLayout inputLayout;
            inputLayout.setBindings({
                                        { 3 * sizeof(float) }
                                    });
            inputLayout.setAttributes({
                                          { 0, 0, QRhiVertexInputAttribute::Float3, 0 }
                                      });

            int ubufElementSize = rhi->ubufAligned(128);
            pEntry->m_prefilterVertBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufElementSize * 6);
            pEntry->m_prefilterVertBuffer->create();

            const int uBufSamplesSize = 16 * prefilterSampleCount + 8;
            int uBufSamplesElementSize = rhi->ubufAligned(uBufSamplesSize);
            pEntry->m_prefilterFragBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, uBufSamplesElementSize * mipmapCount);
            pEntry->m_prefilterFragBuffer->create();

            pEntry->m_prefilterPipeline = rhi->newGraphicsPipeline();
            pEntry->m_prefilterPipeline->setCullMode(QRhiGraphicsPipeline::Front);
            pEntry->m_prefilterPipeline->setFrontFace(QRhiGraphicsPipeline::CCW);
            pEntry->m_prefilterPipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
            pEntry->m_prefilterPipeline->setShaderStages({
                                    *prefilterShaderStages->vertexStage(),
                                    *prefilterShaderStages->fragmentStage()
                                });

            pEntry->m_prefilterSrb = rhi->newShaderResourceBindings();
            pEntry->m_prefilterSrb->setBindings({
                                  QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage, pEntry->m_prefilterVertBuffer, 128),
                                  QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(2, QRhiShaderResourceBinding::FragmentStage, pEntry->m_prefilterFragBuffer, uBufSamplesSize),
                                  QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, pEntry->m_rhiCube, cubeSampler)
                              });
            pEntry->m_prefilterSrb->create();

            pEntry->m_prefilterPipeline->setVertexInputLayout(inputLayout);
            pEntry->m_prefilterPipeline->setShaderResourceBindings(pEntry->m_prefilterSrb);
            pEntry->m_prefilterPipeline->setRenderPassDescriptor(pEntry->m_rhiPrefilterRenderPassDesc);
            if (!pEntry->m_prefilterPipeline->create())
                qWarning("failed to create pre-filter reflection map pipeline state");

            const auto &irradianceShaderStages = m_context.shaderCache()->getBuiltInRhiShaders().getRhienvironmentmapPreFilterShader(false /* isRGBE */);

            pEntry->m_irradiancePipeline = rhi->newGraphicsPipeline();
            pEntry->m_irradiancePipeline->setCullMode(QRhiGraphicsPipeline::Front);
            pEntry->m_irradiancePipeline->setFrontFace(QRhiGraphicsPipeline::CCW);
            pEntry->m_irradiancePipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
            pEntry->m_irradiancePipeline->setShaderStages({
                                     *irradianceShaderStages->vertexStage(),
                                     *irradianceShaderStages->fragmentStage()
                                 });

            int ubufIrradianceSize = rhi->ubufAligned(20);
            pEntry->m_irradianceFragBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufIrradianceSize);
            pEntry->m_irradianceFragBuffer->create();

            pEntry->m_irradianceSrb = rhi->newShaderResourceBindings();
            pEntry->m_irradianceSrb->setBindings({
                                  QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage, pEntry->m_prefilterVertBuffer, 128),
                                  QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(2, QRhiShaderResourceBinding::FragmentStage, pEntry->m_irradianceFragBuffer, 20),
                                  QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, pEntry->m_rhiCube, sampler)
                              });
            pEntry->m_irradianceSrb->create();

            pEntry->m_irradiancePipeline->setShaderResourceBindings(pEntry->m_irradianceSrb);
            pEntry->m_irradiancePipeline->setVertexInputLayout(inputLayout);
            pEntry->m_irradiancePipeline->setRenderPassDescriptor(pEntry->m_rhiPrefilterRenderPassDesc);
            if (!pEntry->m_irradiancePipeline->create())
                qWarning("failed to create irradiance reflection map pipeline state");
        }

        pEntry->m_timeSlicing = probe.timeSlicing;
        pEntry->m_probeIndex = probeIdx;
        Q_QUICK3D_PROFILE_ASSIGN_ID(&probe, pEntry);
    }
}

void QSSGRenderReflectionMap::addTexturedReflectionMapEntry(qint32 probeIdx, const QSSGRenderReflectionProbe &probe)
{
    QSSGReflectionMapEntry *pEntry = reflectionMapEntry(probeIdx);
    const QSSGRenderImageTexture probeTexture = m_context.bufferManager()->loadRenderImage(probe.texture, QSSGBufferManager::MipModeFollowRenderImage);
    if (!pEntry) {
        if (probeTexture.m_texture)
            m_reflectionMapList.push_back(QSSGReflectionMapEntry::withRhiTexturedCubeMap(probeIdx, probeTexture.m_texture));
        else
            addReflectionMapEntry(probeIdx, probe);
    } else {
        if (pEntry->m_rhiDepthStencil)
            pEntry->destroyRhiResources();
        if (probeTexture.m_texture)
            pEntry->m_rhiPrefilteredCube = probeTexture.m_texture;
    }
}

QSSGReflectionMapEntry *QSSGRenderReflectionMap::reflectionMapEntry(int probeIdx)
{
    Q_ASSERT(probeIdx >= 0);

    for (int i = 0; i < m_reflectionMapList.size(); i++) {
        QSSGReflectionMapEntry *pEntry = &m_reflectionMapList[i];
        if (pEntry->m_probeIndex == quint32(probeIdx))
            return pEntry;
    }

    return nullptr;
}

QSSGReflectionMapEntry::QSSGReflectionMapEntry()
    : m_probeIndex(std::numeric_limits<quint32>::max())
{
}

// Vertex data for rendering reflection cube map
static const float cube[] = {
    -1.0f,-1.0f,-1.0f,  // -X side
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Z side
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Y side
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f,  // +Y side
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,

    1.0f, 1.0f,-1.0f,  // +X side
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f, 1.0f, 1.0f,  // +Z side
    -1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,

    0.0f, 1.0f,  // -X side
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,  // -Z side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // -Y side
    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // +Y side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,  // +X side
    0.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,  // +Z side
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

float radicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

QVector2D hammersley(uint i, uint N)
{
    return QVector2D(float(i) / float(N), radicalInverseVdC(i));
}

QVector3D importanceSampleGGX(QVector2D xi, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0f * M_PI * xi.x();
    float cosTheta = sqrt((1.0f - xi.y()) / (1.0f + (a*a - 1.0f) * xi.y()));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    return QVector3D(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

float distributionGGX(float nDotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float nDotH2 = nDotH * nDotH;

    float nom = a2;
    float denom = nDotH2 * (a2 - 1.0f) + 1.0f;
    denom = M_PI * denom * denom;

    return nom / denom;
}

void fillPrefilterValues(float roughness, float resolution,
                         QVarLengthArray<QVector4D, prefilterSampleCount> &sampleDirections,
                         float &invTotalWeight, uint &sampleCount)
{
    for (int i = 0; i < prefilterSampleCount * 8; ++i) {
        const QVector2D xi = hammersley(i, prefilterSampleCount);
        const QVector3D half = importanceSampleGGX(xi, roughness);
        QVector3D light = 2.0f * half.z() * half - QVector3D(0, 0, 1);
        light.normalize();
        const float D = distributionGGX(half.z(), roughness);
        const float pdf = D * half.z() / (4.0f * half.z()) + 0.0001f;
        const float saTexel = 4.0f * M_PI / (6.0f * resolution * resolution);
        const float saSample = 1.0f / (float(prefilterSampleCount) * pdf + 0.0001f);
        float mipLevel = roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);
        if (light.z() > 0) {
            sampleDirections.append(QVector4D(light, mipLevel));
            invTotalWeight += light.z();
            sampleCount++;
            if (sampleCount >= prefilterSampleCount)
                break;
        }
    }
    invTotalWeight = 1.0f / invTotalWeight;
}

void QSSGReflectionMapEntry::renderMips(QSSGRhiContext *rhiCtx)
{
    auto *rhi = rhiCtx->rhi();
    auto *cb = rhiCtx->commandBuffer();

    auto *rub = rhi->nextResourceUpdateBatch();
    rub->generateMips(m_rhiCube);
    QRhiBuffer *vertexBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(cube));
    vertexBuffer->create();
    vertexBuffer->deleteLater();
    rub->uploadStaticBuffer(vertexBuffer, cube);
    cb->resourceUpdate(rub);

    const QRhiCommandBuffer::VertexInput vbufBinding(vertexBuffer, 0);

    int ubufElementSize = rhi->ubufAligned(128);

    const int uBufSamplesSize = 16 * prefilterSampleCount + 8;
    int uBufSamplesElementSize = rhi->ubufAligned(uBufSamplesSize);
    int uBufIrradianceElementSize = rhi->ubufAligned(20);

    // Uniform Data
    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix();
    mvp.perspective(90.0f, 1.0f, 0.1f, 10.0f);

    auto lookAt = [](const QVector3D &eye, const QVector3D &center, const QVector3D &up) {
        QMatrix4x4 viewMatrix;
        viewMatrix.lookAt(eye, center, up);
        return viewMatrix;
    };
    QVarLengthArray<QMatrix4x4, 6> views;
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0, 0.0, 0.0), QVector3D(0.0f, -1.0f, 0.0f)));
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(-1.0, 0.0, 0.0), QVector3D(0.0f, -1.0f, 0.0f)));
    if (rhi->isYUpInFramebuffer()) {
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 1.0, 0.0), QVector3D(0.0f, 0.0f, 1.0f)));
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, -1.0, 0.0), QVector3D(0.0f, 0.0f, -1.0f)));
    } else {
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, -1.0, 0.0), QVector3D(0.0f, 0.0f, -1.0f)));
        views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 1.0, 0.0), QVector3D(0.0f, 0.0f, 1.0f)));
    }
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 0.0, 1.0), QVector3D(0.0f, -1.0f, 0.0f)));
    views.append(lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0, 0.0, -1.0), QVector3D(0.0f, -1.0f, 0.0f)));

    rub = rhi->nextResourceUpdateBatch();
    for (const auto face : QSSGRenderTextureCubeFaces) {
        rub->updateDynamicBuffer(m_prefilterVertBuffer, quint8(face) * ubufElementSize, 64, mvp.constData());
        rub->updateDynamicBuffer(m_prefilterVertBuffer, quint8(face) * ubufElementSize + 64, 64, views[quint8(face)].constData());
    }

    const QSize mapSize = m_rhiCube->pixelSize();

    int mipmapCount = rhi->mipLevelsForSize(mapSize);
    mipmapCount = qMin(mipmapCount, 6);

    const float resolution = mapSize.width();
    QVarLengthArray<QVector4D, prefilterSampleCount> sampleDirections;

    // set the samples uniform buffer data
    for (int mipLevel = 0; mipLevel < mipmapCount - 1; ++mipLevel) {
        Q_ASSERT(mipmapCount - 2);
        const float roughness = float(mipLevel) / float(mipmapCount - 2);
        float invTotalWeight = 0.0f;
        uint sampleCount = 0;

        sampleDirections.clear();
        fillPrefilterValues(roughness, resolution, sampleDirections, invTotalWeight, sampleCount);

        rub->updateDynamicBuffer(m_prefilterFragBuffer, mipLevel * uBufSamplesElementSize, 16 * prefilterSampleCount, sampleDirections.constData());
        rub->updateDynamicBuffer(m_prefilterFragBuffer, mipLevel * uBufSamplesElementSize + 16 * prefilterSampleCount, 4, &invTotalWeight);
        rub->updateDynamicBuffer(m_prefilterFragBuffer, mipLevel * uBufSamplesElementSize + 16 * prefilterSampleCount + 4, 4, &sampleCount);
    }
    {
        const float roughness = 0.0f; // doesn't matter for irradiance
        const float lodBias = 0.0f;
        const int distribution = 0;
        const int sampleCount = resolution / 4;

        rub->updateDynamicBuffer(m_irradianceFragBuffer, 0, 4, &roughness);
        rub->updateDynamicBuffer(m_irradianceFragBuffer, 4, 4, &resolution);
        rub->updateDynamicBuffer(m_irradianceFragBuffer, 4 + 4, 4, &lodBias);
        rub->updateDynamicBuffer(m_irradianceFragBuffer, 4 + 4 + 4, 4, &sampleCount);
        rub->updateDynamicBuffer(m_irradianceFragBuffer, 4 + 4 + 4 + 4, 4, &distribution);
    }

    cb->resourceUpdate(rub);

    // Render
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        if (mipLevel > 0 && m_timeSlicing == QSSGRenderReflectionProbe::ReflectionTimeSlicing::AllFacesAtOnce)
            mipLevel = m_timeSliceFrame;

        for (auto face : QSSGRenderTextureCubeFaces) {
            if (m_timeSlicing == QSSGRenderReflectionProbe::ReflectionTimeSlicing::IndividualFaces)
                face = m_timeSliceFace;

            cb->beginPass(m_rhiPrefilterRenderTargetsMap[mipLevel][quint8(face)], QColor(0, 0, 0, 1), { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
            QSSGRHICTX_STAT(rhiCtx, beginRenderPass(m_rhiPrefilterRenderTargetsMap[mipLevel][quint8(face)]));
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
            if (mipLevel < mipmapCount - 1) {
                // Specular pre-filtered Cube Map levels
                cb->setGraphicsPipeline(m_prefilterPipeline);
                cb->setVertexInput(0, 1, &vbufBinding);
                cb->setViewport(QRhiViewport(0, 0, m_prefilterMipLevelSizes[mipLevel].width(), m_prefilterMipLevelSizes[mipLevel].height()));
                QVector<QPair<int, quint32>> dynamicOffsets = {
                    { 0, quint32(ubufElementSize * quint8(face)) },
                    { 2, quint32(uBufSamplesElementSize * mipLevel) }
                };
                cb->setShaderResources(m_prefilterSrb, 2, dynamicOffsets.constData());
            } else {
                // Diffuse Irradiance
                cb->setGraphicsPipeline(m_irradiancePipeline);
                cb->setVertexInput(0, 1, &vbufBinding);
                cb->setViewport(QRhiViewport(0, 0, m_prefilterMipLevelSizes[mipLevel].width(), m_prefilterMipLevelSizes[mipLevel].height()));
                QVector<QPair<int, quint32>> dynamicOffsets = {
                    { 0, quint32(ubufElementSize * quint8(face)) },
                    { 2, quint32(uBufIrradianceElementSize) }
                };
                cb->setShaderResources(m_irradianceSrb, 1, dynamicOffsets.constData());
            }
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);
            cb->draw(36);
            QSSGRHICTX_STAT(rhiCtx, draw(36, 1));
            Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DRenderCall, 36llu | (1llu << 32), profilingId);
            cb->endPass();
            QSSGRHICTX_STAT(rhiCtx, endRenderPass());
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QSSG_RENDERPASS_NAME("reflection_map", mipLevel, face));

            if (m_timeSlicing == QSSGRenderReflectionProbe::ReflectionTimeSlicing::IndividualFaces)
                break;
        }

        if (mipLevel > 0 && m_timeSlicing == QSSGRenderReflectionProbe::ReflectionTimeSlicing::AllFacesAtOnce) {
            m_timeSliceFrame++;
            if (m_timeSliceFrame >= mipmapCount)
                m_timeSliceFrame = 1;
            break;
        }
    }
    cb->debugMarkEnd();
}

QSSGReflectionMapEntry QSSGReflectionMapEntry::withRhiTexturedCubeMap(quint32 probeIdx, QRhiTexture *prefiltered)
{
    QSSGReflectionMapEntry e;
    e.m_probeIndex = probeIdx;
    e.m_rhiPrefilteredCube = prefiltered;
    return e;
}

QSSGReflectionMapEntry QSSGReflectionMapEntry::withRhiCubeMap(quint32 probeIdx,
                                                           QRhiTexture *cube,
                                                           QRhiTexture *prefiltered,
                                                           QRhiRenderBuffer *depthStencil)
{
    QSSGReflectionMapEntry e;
    e.m_probeIndex = probeIdx;
    e.m_rhiCube = cube;
    e.m_rhiPrefilteredCube = prefiltered;
    e.m_rhiDepthStencil = depthStencil;
    return e;
}

void QSSGReflectionMapEntry::destroyRhiResources()
{
    delete m_rhiCube;
    m_rhiCube = nullptr;
    // Without depth stencil the prefiltered cubemap is assumed to be not owned here and shouldn't be deleted
    if (m_rhiDepthStencil)
        delete m_rhiPrefilteredCube;
    m_rhiPrefilteredCube = nullptr;
    delete m_rhiDepthStencil;
    m_rhiDepthStencil = nullptr;

    qDeleteAll(m_rhiRenderTargets);
    m_rhiRenderTargets.clear();
    delete m_rhiRenderPassDesc;
    m_rhiRenderPassDesc = nullptr;

    delete m_prefilterPipeline;
    m_prefilterPipeline = nullptr;
    delete m_irradiancePipeline;
    m_irradiancePipeline = nullptr;
    delete m_prefilterSrb;
    m_prefilterSrb = nullptr;
    delete m_irradianceSrb;
    m_irradianceSrb = nullptr;
    delete m_prefilterVertBuffer;
    m_prefilterVertBuffer = nullptr;
    delete m_prefilterFragBuffer;
    m_prefilterFragBuffer = nullptr;
    delete m_irradianceFragBuffer;
    m_irradianceFragBuffer = nullptr;
    delete m_rhiPrefilterRenderPassDesc;
    m_rhiPrefilterRenderPassDesc = nullptr;
    for (const auto &e : std::as_const(m_rhiPrefilterRenderTargetsMap))
        qDeleteAll(e);
    m_rhiPrefilterRenderTargetsMap.clear();
    m_prefilterMipLevelSizes.clear();
}

QT_END_NAMESPACE
