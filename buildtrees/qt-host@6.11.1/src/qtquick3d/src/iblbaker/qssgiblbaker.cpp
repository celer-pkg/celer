// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgiblbaker_p.h"
#include <QFile>
#include <QFileInfo>
#include <QScopeGuard>

#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderloadedtexture_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>

#if QT_CONFIG(opengl)
#include <QOffscreenSurface>
#include <QOpenGLContext>
#endif

QT_BEGIN_NAMESPACE

#define GL_FLOAT 0x1406
#define GL_HALF_FLOAT 0x140B
#define GL_UNSIGNED_BYTE 0x1401
#define GL_RGBA 0x1908
#define GL_RGBA8 0x8058
#define GL_RGBA16F 0x881A
#define GL_RGBA32F 0x8814

static constexpr QSSGRenderTextureFormat FORMAT(QSSGRenderTextureFormat::RGBA16F);

const QStringList QSSGIblBaker::inputExtensions() const
{
    return { QStringLiteral("hdr"), QStringLiteral("exr")};
}

const QString QSSGIblBaker::outputExtension() const
{
    return QStringLiteral(".ktx");
}

namespace {
void writeUInt32(QIODevice &device, quint32 value)
{
    device.write(reinterpret_cast<char *>(&value), sizeof(qint32));
}

void appendBinaryVector(QVector<char> &dest, const quint32 src)
{
    qsizetype oldsize = dest.size();
    dest.resize(dest.size() + sizeof(src));
    memcpy(dest.data() + oldsize, &src, sizeof(src));
}

void appendBinaryVector(QVector<char> &dest, const std::string &src)
{
    qsizetype oldsize = dest.size();
    dest.resize(dest.size() + src.size() + 1);
    memcpy(dest.data() + oldsize, src.c_str(), src.size() + 1);
}
}

// Vertex data for rendering environment cube map
static const float cube[] = {
    -1.0f, -1.0f, -1.0f, // -X side
    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f, // -Z side
    1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, // -Y side
    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, // +Y side
    -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,

    1.0f,  1.0f,  -1.0f, // +X side
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,

    -1.0f, 1.0f,  1.0f, // +Z side
    -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,

    0.0f,  1.0f, // -X side
    1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,

    1.0f,  1.0f, // -Z side
    0.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,

    1.0f,  0.0f, // -Y side
    1.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    1.0f,  0.0f, // +Y side
    0.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,

    1.0f,  0.0f, // +X side
    0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,

    0.0f,  0.0f, // +Z side
    0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,
};

QString renderToKTXFileInternal(const char *name, const QString &inPath, const QString &outPath, QRhi::Implementation impl, QRhiInitParams *initParams)
{
    qDebug() << "Using RHI backend" << name;

    // Open output file
    QFile ktxOutputFile(outPath);
    if (!ktxOutputFile.open(QIODevice::WriteOnly)) {
        return QStringLiteral("Could not open file: %1").arg(outPath);
    }

    QScopedPointer<QRhi> rhi(QRhi::create(impl, initParams, QRhi::Flags(), nullptr));
    if (!rhi)
        return QStringLiteral("Failed to initialize QRhi");

    qDebug() << rhi->driverInfo();

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    const auto rhiContext = std::make_unique<QSSGRhiContext>(rhi.get());
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiContext.get());
    rhiCtxD->setCommandBuffer(cb);

    QScopedPointer<QSSGLoadedTexture> inImage(QSSGLoadedTexture::loadHdrImage(QSSGInputUtil::getStreamForFile(inPath), FORMAT));
    if (!inImage)
        return QStringLiteral("Failed to load hdr file");

    auto shaderCache = std::make_unique<QSSGShaderCache>(*rhiContext);

    // The objective of this method is to take the equirectangular texture
    // provided by inImage and create a cubeMap that contains both pre-filtered
    // specular environment maps, as well as a irradiance map for diffuse
    // operations.
    // To achieve this though we first convert convert the Equirectangular texture
    // to a cubeMap with genereated mip map levels (no filtering) to make the
    // process of creating the prefiltered and irradiance maps eaiser. This
    // intermediate texture as well as the original equirectangular texture are
    // destroyed after this frame completes, and all further associations with
    // the source lightProbe texture are instead associated with the final
    // generated environment map.
    // The intermediate environment cubemap is used to generate the final
    // cubemap. This cubemap will generate 6 mip levels for each face
    // (the remaining faces are unused).  This is what the contents of each
    // face mip level looks like:
    // 0: Pre-filtered with roughness 0 (basically unfiltered)
    // 1: Pre-filtered with roughness 0.25
    // 2: Pre-filtered with roughness 0.5
    // 3: Pre-filtered with roughness 0.75
    // 4: Pre-filtered with rougnness 1.0
    // 5: Irradiance map (ideally at least 16x16)
    // It would be better if we could use a separate cubemap for irradiance, but
    // right now there is a 1:1 association between texture sources on the front-
    // end and backend.

    // Right now minimum face size needs to be 512x512 to be able to have 6 reasonably sized mips
    const int suggestedSize = qMax(512.f, inImage->height * 0.5f);
    const QSize environmentMapSize(suggestedSize, suggestedSize);
    const bool isRGBE = inImage->format.format == QSSGRenderTextureFormat::Format::RGBE8;
    const int colorSpace = inImage->isSRGB ? 1 : 0; // 0 Linear | 1 sRGB

    // Phase 1: Convert the Equirectangular texture to a Cubemap
    QRhiTexture *envCubeMap = rhi->newTexture(QRhiTexture::RGBA16F,
                                              environmentMapSize,
                                              1,
                                              QRhiTexture::RenderTarget | QRhiTexture::CubeMap | QRhiTexture::MipMapped
                                                      | QRhiTexture::UsedWithGenerateMips);
    if (!envCubeMap->create()) {
        return QStringLiteral("Failed to create Environment Cube Map");
    }
    envCubeMap->deleteLater();

    // Create a renderbuffer the size of a the cubeMap face
    QRhiRenderBuffer *envMapRenderBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::Color, environmentMapSize);
    if (!envMapRenderBuffer->create()) {
        return QStringLiteral("Failed to create Environment Map Render Buffer");
    }
    envMapRenderBuffer->deleteLater();

    // Setup the 6 render targets for each cube face
    QVarLengthArray<QRhiTextureRenderTarget *, 6> renderTargets;
    QRhiRenderPassDescriptor *renderPassDesc = nullptr;
    for (int face = 0; face < 6; ++face) {
        QRhiColorAttachment att(envCubeMap);
        att.setLayer(face);
        QRhiTextureRenderTargetDescription rtDesc;
        rtDesc.setColorAttachments({ att });
        auto renderTarget = rhi->newTextureRenderTarget(rtDesc);
        renderTarget->setDescription(rtDesc);
        if (!renderPassDesc)
            renderPassDesc = renderTarget->newCompatibleRenderPassDescriptor();
        renderTarget->setRenderPassDescriptor(renderPassDesc);
        if (!renderTarget->create()) {
            return QStringLiteral("Failed to build env map render target");
        }
        renderTarget->deleteLater();
        renderTargets << renderTarget;
    }
    renderPassDesc->deleteLater();

    // Setup the sampler for reading the equirectangular loaded texture
    QSize size(inImage->width, inImage->height);
    auto *sourceTexture = rhi->newTexture(QRhiTexture::RGBA16F, size, 1);
    if (!sourceTexture->create()) {
        return QStringLiteral("Failed to create source env map texture");
    }
    sourceTexture->deleteLater();

    // Upload the equirectangular texture
    QRhiTextureUploadDescription desc;
    if (inImage->textureFileData.isValid()) {
        desc = { { 0,
                   0,
                   { inImage->textureFileData.data().constData() + inImage->textureFileData.dataOffset(0),
                     quint32(inImage->textureFileData.dataLength(0)) } } };
    } else {
        desc = { { 0, 0, { inImage->data, inImage->dataSizeInBytes } } };
    }
    auto *rub = rhi->nextResourceUpdateBatch();
    rub->uploadTexture(sourceTexture, desc);

    const QSSGRhiSamplerDescription samplerDesc {
        QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat
    };
    QRhiSampler *sampler = rhiContext->sampler(samplerDesc);

    // Load shader and setup render pipeline
    const auto &envMapShaderStages = shaderCache->getBuiltInRhiShaders().getRhiEnvironmentmapShader();

    // Vertex Buffer - Just a single cube that will be viewed from inside
    QRhiBuffer *vertexBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(cube));
    vertexBuffer->create();
    vertexBuffer->deleteLater();
    rub->uploadStaticBuffer(vertexBuffer, cube);

    // Uniform Buffer - 2x mat4
    int ubufElementSize = rhi->ubufAligned(128);
    QRhiBuffer *uBuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufElementSize * 6);
    uBuf->create();
    uBuf->deleteLater();

    int ubufEnvMapElementSize = rhi->ubufAligned(4);
    QRhiBuffer *uBufEnvMap = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufEnvMapElementSize * 6);
    uBufEnvMap->create();
    uBufEnvMap->deleteLater();

    // Shader Resource Bindings
    QRhiShaderResourceBindings *envMapSrb = rhi->newShaderResourceBindings();
    envMapSrb->setBindings(
            { QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage, uBuf, 128),
              QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(2, QRhiShaderResourceBinding::FragmentStage, uBufEnvMap, ubufEnvMapElementSize),
              QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, sourceTexture, sampler) });
    envMapSrb->create();
    envMapSrb->deleteLater();

    // Pipeline
    QRhiGraphicsPipeline *envMapPipeline = rhi->newGraphicsPipeline();
    envMapPipeline->setCullMode(QRhiGraphicsPipeline::Front);
    envMapPipeline->setFrontFace(QRhiGraphicsPipeline::CCW);
    envMapPipeline->setShaderStages({ *envMapShaderStages->vertexStage(), *envMapShaderStages->fragmentStage() });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ { 3 * sizeof(float) } });
    inputLayout.setAttributes({ { 0, 0, QRhiVertexInputAttribute::Float3, 0 } });

    envMapPipeline->setVertexInputLayout(inputLayout);
    envMapPipeline->setShaderResourceBindings(envMapSrb);
    envMapPipeline->setRenderPassDescriptor(renderPassDesc);
    if (!envMapPipeline->create()) {
        return QStringLiteral("Failed to create source env map pipeline state");
    }
    envMapPipeline->deleteLater();

    // Do the actual render passes
    cb->debugMarkBegin("Environment Cubemap Generation");
    const QRhiCommandBuffer::VertexInput vbufBinding(vertexBuffer, 0);

    // Set the Uniform Data
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
    for (int face = 0; face < 6; ++face) {
        rub->updateDynamicBuffer(uBuf, face * ubufElementSize, 64, mvp.constData());
        rub->updateDynamicBuffer(uBuf, face * ubufElementSize + 64, 64, views[face].constData());
        rub->updateDynamicBuffer(uBufEnvMap, face * ubufEnvMapElementSize, 4, &colorSpace);
    }
    cb->resourceUpdate(rub);

    for (int face = 0; face < 6; ++face) {
        cb->beginPass(renderTargets[face], QColor(0, 0, 0, 1), { 1.0f, 0 }, nullptr, rhiContext->commonPassFlags());

        // Execute render pass
        cb->setGraphicsPipeline(envMapPipeline);
        cb->setVertexInput(0, 1, &vbufBinding);
        cb->setViewport(QRhiViewport(0, 0, environmentMapSize.width(), environmentMapSize.height()));
        QVector<QPair<int, quint32>> dynamicOffset = {
            { 0, quint32(ubufElementSize * face) },
            { 2, quint32(ubufEnvMapElementSize * face )}
        };
        cb->setShaderResources(envMapSrb, 2, dynamicOffset.constData());

        cb->draw(36);
        cb->endPass();
    }
    cb->debugMarkEnd();

    if (!isRGBE) {
        // Generate mipmaps for envMap
        rub = rhi->nextResourceUpdateBatch();
        rub->generateMips(envCubeMap);
        cb->resourceUpdate(rub);
    }

    // Phase 2: Generate the pre-filtered environment cubemap
    cb->debugMarkBegin("Pre-filtered Environment Cubemap Generation");
    QRhiTexture *preFilteredEnvCubeMap = rhi->newTexture(QRhiTexture::RGBA16F,
                                                         environmentMapSize,
                                                         1,
                                                         QRhiTexture::RenderTarget | QRhiTexture::CubeMap | QRhiTexture::MipMapped);
    if (!preFilteredEnvCubeMap->create())
        qWarning("Failed to create Pre-filtered Environment Cube Map");
    int mipmapCount = rhi->mipLevelsForSize(environmentMapSize);
    mipmapCount = qMin(mipmapCount, 6); // don't create more than 6 mip levels
    QMap<int, QSize> mipLevelSizes;
    QMap<int, QVarLengthArray<QRhiTextureRenderTarget *, 6>> renderTargetsMap;
    QRhiRenderPassDescriptor *renderPassDescriptorPhase2 = nullptr;

    // Create a renderbuffer for each mip level
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        const QSize levelSize = QSize(environmentMapSize.width() * std::pow(0.5, mipLevel),
                                      environmentMapSize.height() * std::pow(0.5, mipLevel));
        mipLevelSizes.insert(mipLevel, levelSize);
        // Setup Render targets (6 * mipmapCount)
        QVarLengthArray<QRhiTextureRenderTarget *, 6> renderTargets;
        for (int face = 0; face < 6; ++face) {
            QRhiColorAttachment att(preFilteredEnvCubeMap);
            att.setLayer(face);
            att.setLevel(mipLevel);
            QRhiTextureRenderTargetDescription rtDesc;
            rtDesc.setColorAttachments({ att });
            auto renderTarget = rhi->newTextureRenderTarget(rtDesc);
            renderTarget->setDescription(rtDesc);
            if (!renderPassDescriptorPhase2)
                renderPassDescriptorPhase2 = renderTarget->newCompatibleRenderPassDescriptor();
            renderTarget->setRenderPassDescriptor(renderPassDescriptorPhase2);
            if (!renderTarget->create())
                qWarning("Failed to build prefilter env map render target");
            renderTarget->deleteLater();
            renderTargets << renderTarget;
        }
        renderTargetsMap.insert(mipLevel, renderTargets);
        renderPassDescriptorPhase2->deleteLater();
    }

    // Load the prefilter shader stages
    const auto &prefilterShaderStages = shaderCache->getBuiltInRhiShaders().getRhienvironmentmapPreFilterShader(isRGBE);

    // Create a new Sampler
    const QSSGRhiSamplerDescription samplerMipMapDesc {
        QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat
    };

    QRhiSampler *envMapCubeSampler = nullptr;
    // Only use mipmap interpoliation if not using RGBE
    if (!isRGBE)
        envMapCubeSampler = rhiContext->sampler(samplerMipMapDesc);
    else
        envMapCubeSampler = sampler;

    // Reuse Vertex Buffer from phase 1
    // Reuse UniformBuffer from phase 1 (for vertex shader)

    // UniformBuffer
    // float roughness;
    // float resolution;
    // float lodBias;
    // int sampleCount;
    // int distribution;

    int ubufPrefilterElementSize = rhi->ubufAligned(20);
    QRhiBuffer *uBufPrefilter = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufPrefilterElementSize * mipmapCount);
    uBufPrefilter->create();
    uBufPrefilter->deleteLater();

    // Shader Resource Bindings
    QRhiShaderResourceBindings *preFilterSrb = rhi->newShaderResourceBindings();
    preFilterSrb->setBindings({
                          QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(0, QRhiShaderResourceBinding::VertexStage, uBuf, 128),
                          QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(2, QRhiShaderResourceBinding::FragmentStage, uBufPrefilter, 20),
                          QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, envCubeMap, envMapCubeSampler)
                      });
    preFilterSrb->create();
    preFilterSrb->deleteLater();

    // Pipeline
    QRhiGraphicsPipeline *prefilterPipeline = rhi->newGraphicsPipeline();
    prefilterPipeline->setCullMode(QRhiGraphicsPipeline::Front);
    prefilterPipeline->setFrontFace(QRhiGraphicsPipeline::CCW);
    prefilterPipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
    prefilterPipeline->setShaderStages({
                            *prefilterShaderStages->vertexStage(),
                            *prefilterShaderStages->fragmentStage()
                        });
    // same as phase 1
    prefilterPipeline->setVertexInputLayout(inputLayout);
    prefilterPipeline->setShaderResourceBindings(preFilterSrb);
    prefilterPipeline->setRenderPassDescriptor(renderPassDescriptorPhase2);
    if (!prefilterPipeline->create())
        return QStringLiteral("Failed to create pre-filter env map pipeline state");
    prefilterPipeline->deleteLater();

    // Uniform Data
    // set the roughness uniform buffer data
    rub = rhi->nextResourceUpdateBatch();
    const float resolution = environmentMapSize.width();
    const float lodBias = 0.0f;
    const int sampleCount = 1024;
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        Q_ASSERT(mipmapCount - 2);
        const float roughness = float(mipLevel) / float(mipmapCount - 2);
        const int distribution = mipLevel == (mipmapCount - 1) ? 0 : 1; // last mip level is for irradiance
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize, 4, &roughness);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4, 4, &resolution);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4 + 4, 4, &lodBias);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4 + 4 + 4, 4, &sampleCount);
        rub->updateDynamicBuffer(uBufPrefilter, mipLevel * ubufPrefilterElementSize + 4 + 4 + 4 + 4, 4, &distribution);
    }

    cb->resourceUpdate(rub);

    // Render
    for (int mipLevel = 0; mipLevel < mipmapCount; ++mipLevel) {
        for (int face = 0; face < 6; ++face) {
            cb->beginPass(renderTargetsMap[mipLevel][face], QColor(0, 0, 0, 1), { 1.0f, 0 }, nullptr, rhiContext->commonPassFlags());
            cb->setGraphicsPipeline(prefilterPipeline);
            cb->setVertexInput(0, 1, &vbufBinding);
            cb->setViewport(QRhiViewport(0, 0, mipLevelSizes[mipLevel].width(), mipLevelSizes[mipLevel].height()));
            QVector<QPair<int, quint32>> dynamicOffsets = {
                { 0, quint32(ubufElementSize * face) },
                { 2, quint32(ubufPrefilterElementSize * mipLevel) }
            };
            cb->setShaderResources(preFilterSrb, 2, dynamicOffsets.constData());
            cb->draw(36);
            cb->endPass();
        }
    }
    cb->debugMarkEnd();

    // Write ktx

    const quint32 numberOfMipmapLevels = renderTargetsMap.size();
    const quint32 numberOfFaces = 6;

    constexpr size_t KTX_IDENTIFIER_LENGTH = 12;
    constexpr char ktxIdentifier[KTX_IDENTIFIER_LENGTH] = { '\xAB', 'K',    'T',  'X',  ' ',    '1',
                                                            '1',    '\xBB', '\r', '\n', '\x1A', '\n' };
    constexpr quint32 platformEndianIdentifier = 0x04030201;
    QVector<char> keyValueData;

    // Prepare Key/Value array
    {
        // Add a key to the metadata to know it was created by our IBL baker
        static const char key[] = "QT_IBL_BAKER_VERSION";
        static const char value[] = "1";

        constexpr size_t keyAndValueByteSize = sizeof(key) + sizeof(value);   // NB: 2x null terminator
        appendBinaryVector(keyValueData, keyAndValueByteSize);
        appendBinaryVector(keyValueData, key);
        appendBinaryVector(keyValueData, value);

        // Pad until next multiple of 4
        const size_t padding = 3 - ((keyAndValueByteSize + 3) % 4); // Pad until next multiple of 4
        keyValueData.resize(keyValueData.size() + padding);
    }

    // Header

    // identifier
    ktxOutputFile.write(ktxIdentifier, KTX_IDENTIFIER_LENGTH);

    // endianness
    writeUInt32(ktxOutputFile, quint32(platformEndianIdentifier));

    // glType
    writeUInt32(ktxOutputFile, quint32(GL_HALF_FLOAT));

    // glTypeSize (in bytes per component)
    writeUInt32(ktxOutputFile, quint32(FORMAT.getSizeofFormat()) / quint32(FORMAT.getNumberOfComponent()));

    // glFormat
    writeUInt32(ktxOutputFile, quint32(GL_RGBA));

    // glInternalFormat
    writeUInt32(ktxOutputFile, quint32(GL_RGBA16F));

    // glBaseInternalFormat
    writeUInt32(ktxOutputFile, quint32(GL_RGBA));

    // pixelWidth
    writeUInt32(ktxOutputFile, quint32(environmentMapSize.width()));

    // pixelHeight
    writeUInt32(ktxOutputFile, quint32(environmentMapSize.height()));

    // pixelDepth
    writeUInt32(ktxOutputFile, quint32(0));

    // numberOfArrayElements
    writeUInt32(ktxOutputFile, quint32(0));

    // numberOfFaces
    writeUInt32(ktxOutputFile, quint32(numberOfFaces));

    // numberOfMipLevels
    writeUInt32(ktxOutputFile, quint32(numberOfMipmapLevels));

    // bytesOfKeyValueData
    writeUInt32(ktxOutputFile, quint32(keyValueData.size()));

    // Key/Value
    ktxOutputFile.write(keyValueData.data(), keyValueData.size());

    // Images
    for (quint32 mipmap_level = 0; mipmap_level < numberOfMipmapLevels; mipmap_level++) {
        quint32 imageSize = 0;
        for (size_t face = 0; face < numberOfFaces; face++) {
            QRhiTextureRenderTarget *renderTarget = renderTargetsMap[mipmap_level][face];

            // Read back texture
            Q_ASSERT(rhi->isRecordingFrame());

            const auto descr = renderTarget->description();
            const auto texture = descr.cbeginColorAttachments()->texture();

            QRhiReadbackResult result;
            QRhiReadbackDescription readbackDesc(texture); // null src == read from swapchain backbuffer
            readbackDesc.setLayer(int(face));
            readbackDesc.setLevel(mipmap_level);

            QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();
            resourceUpdates->readBackTexture(readbackDesc, &result);

            cb->resourceUpdate(resourceUpdates);
            rhi->finish(); // make sure the readback has finished, stall the pipeline if needed

            // Write imageSize once size is known
            if (imageSize == 0) {
                imageSize = result.data.size();
                writeUInt32(ktxOutputFile, quint32(imageSize));
            }

            ktxOutputFile.write(result.data);
        }
    }

    ktxOutputFile.close();

    preFilteredEnvCubeMap->deleteLater();

    rhi->endOffscreenFrame();
    rhi->finish();

    return {};
}

void adjustToPlatformQuirks(QRhi::Implementation &impl)
{
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    // A macOS VM may not have Metal support at all. We have to decide at this
    // point, it will be too late afterwards, and the only way is to see if
    // MTLCreateSystemDefaultDevice succeeds.
    if (impl == QRhi::Metal) {
        QRhiMetalInitParams rhiParams;
        QRhi *tempRhi = QRhi::create(impl, &rhiParams, {});
        if (!tempRhi) {
            impl = QRhi::OpenGLES2;
            qDebug("Metal does not seem to be supported. Falling back to OpenGL.");
        } else {
            delete tempRhi;
        }
    }
#else
    Q_UNUSED(impl);
#endif
}

QRhi::Implementation getRhiImplementation()
{
    QRhi::Implementation implementation = QRhi::Implementation::Null;

    // check env.vars., fall back to platform-specific defaults when backend is not set
    const QByteArray rhiBackend = qgetenv("QSG_RHI_BACKEND");
    if (rhiBackend == QByteArrayLiteral("gl") || rhiBackend == QByteArrayLiteral("gles2")
        || rhiBackend == QByteArrayLiteral("opengl")) {
        implementation = QRhi::OpenGLES2;
    } else if (rhiBackend == QByteArrayLiteral("d3d11") || rhiBackend == QByteArrayLiteral("d3d")) {
        implementation = QRhi::D3D11;
    } else if (rhiBackend == QByteArrayLiteral("d3d12")) {
        implementation = QRhi::D3D12;
    } else if (rhiBackend == QByteArrayLiteral("vulkan")) {
        implementation = QRhi::Vulkan;
    } else if (rhiBackend == QByteArrayLiteral("metal")) {
        implementation = QRhi::Metal;
    } else if (rhiBackend == QByteArrayLiteral("null")) {
        implementation = QRhi::Null;
    } else {
        if (!rhiBackend.isEmpty()) {
            qWarning("Unknown key \"%s\" for QSG_RHI_BACKEND, falling back to default backend.", rhiBackend.constData());
        }
#if defined(Q_OS_WIN)
        implementation = QRhi::D3D11;
#elif defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        implementation = QRhi::Metal;
#elif QT_CONFIG(opengl)
        implementation = QRhi::OpenGLES2;
#else
        implementation = QRhi::Vulkan;
#endif
    }

    adjustToPlatformQuirks(implementation);

    return implementation;
}

QString renderToKTXFile(const QString &inPath, const QString &outPath)
{
    const auto rhiImplementation = getRhiImplementation();

#if QT_CONFIG(opengl)
    if (rhiImplementation == QRhi::OpenGLES2) {
        QRhiGles2InitParams params;
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
            // OpenGL 3.2 or higher
            params.format.setProfile(QSurfaceFormat::CoreProfile);
            params.format.setVersion(3, 2);
        } else {
            // OpenGL ES 3.0 or higher
            params.format.setVersion(3, 0);
        }
        params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
        const QString result = renderToKTXFileInternal("OpenGL", inPath, outPath, QRhi::OpenGLES2, &params);
        delete params.fallbackSurface;
        return result;
    }
#endif

#if QT_CONFIG(vulkan)
    if (rhiImplementation == QRhi::Vulkan) {
        QVulkanInstance vulkanInstance;
        vulkanInstance.create();
        QRhiVulkanInitParams params;
        params.inst = &vulkanInstance;
        return renderToKTXFileInternal("Vulkan", inPath, outPath, QRhi::Vulkan, &params);
    }
#endif

#ifdef Q_OS_WIN
    if (rhiImplementation == QRhi::D3D11) {
        QRhiD3D11InitParams params;
        return renderToKTXFileInternal("Direct3D 11", inPath, outPath, QRhi::D3D11, &params);
    } else if (rhiImplementation == QRhi::D3D12) {
        QRhiD3D12InitParams params;
        return renderToKTXFileInternal("Direct3D 12", inPath, outPath, QRhi::D3D12, &params);
    }
#endif

#if QT_CONFIG(metal)
    if (rhiImplementation == QRhi::Metal) {
        QRhiMetalInitParams params;
        return renderToKTXFileInternal("Metal", inPath, outPath, QRhi::Metal, &params);
    }
#endif

    return QStringLiteral("No RHI backend");
}

const QString QSSGIblBaker::import(const QString &sourceFile, const QDir &savePath, QStringList *generatedFiles)
{
    qDebug() << "IBL lightprobe baker" << sourceFile;

    QString outFileName = savePath.absoluteFilePath(QFileInfo(sourceFile).baseName() + QStringLiteral(".ktx"));

    QString error = renderToKTXFile(sourceFile, outFileName);
    if (!error.isEmpty())
        return error;

    m_generatedFiles.append(outFileName);

    if (generatedFiles)
        *generatedFiles = m_generatedFiles;

    return QString();
}

QT_END_NAMESPACE
