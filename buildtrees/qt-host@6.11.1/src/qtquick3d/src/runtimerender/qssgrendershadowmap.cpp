// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadowmap_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include "qssgrendercontextcore.h"

QT_BEGIN_NAMESPACE

namespace AtlasHelpers {

struct ShelfPacker {
    struct Shelf {
        int y;
        int height;
        int curX;
    };

    struct ShelfPage {
        int pageIndex;
        int width;
        int height;
        int curY;
        std::vector<Shelf> shelves;
    };

    struct AtlasPlacement {
        bool success;
        int pageIndex;
        int x;
        int y;
    };
    const int pageWidth;
    const int pageHeight;
    std::vector<ShelfPage> pages;

    ShelfPacker(int pageWidth, int pageHeight)
        : pageWidth(pageWidth)
        , pageHeight(pageHeight)
    {
    }

    AtlasPlacement addRectangle(int sizeNeeded) {
        // Try each page in turn
        for (auto &page : pages) {
            AtlasPlacement placement = placeOnPage(page, sizeNeeded);
            if (placement.success)
                return placement;
        }

        // If we get here, we need a new page
        ShelfPage newPage;
        newPage.pageIndex = int(pages.size());
        newPage.width = pageWidth;
        newPage.height = pageHeight;
        newPage.curY = 0;
        newPage.shelves.clear();
        pages.push_back(newPage);

        // now place in the new page
        return placeOnPage(pages.back(), sizeNeeded);
    }

    AtlasPlacement placeOnPage(ShelfPage &page, int sizeNeeded)
    {
        AtlasPlacement result;
        result.success = false;

        // Iterate over shelves to see if we can place the rect
        for (auto &shelf : page.shelves) {
            // check if it fits horizontally
            if (shelf.curX + sizeNeeded <= page.width) {
                // also check if the shelf's height is enough
                // but here we store the shelf's "height" as the max of the rectangles' heights
                // If each rect is the same "sizeNeeded," we just need to see if there's room in page.height

                // not enough vertical space
                if (shelf.y + shelf.height >= page.height)
                    continue;

                // place here
                result.success = true;
                result.pageIndex = page.pageIndex;
                result.x = shelf.curX;
                result.y = shelf.y;
                // update shelf
                shelf.curX += sizeNeeded;
                if (sizeNeeded > shelf.height)
                    shelf.height = sizeNeeded;

                return result;
            }
        }

        // No existing shelf had space, so start a new shelf
        int newShelfY = 0;
        if (!page.shelves.empty()) {
            const Shelf &lastShelf = page.shelves.back();
            newShelfY = lastShelf.y + lastShelf.height;
        }

        // check if there's enough space left in the page vertically
        if (newShelfY + sizeNeeded > page.height) {
            // no space in this page
            return result; // success = false
        }

        // create a new shelf
        Shelf newShelf;
        newShelf.y = newShelfY;
        newShelf.height = sizeNeeded;
        newShelf.curX = 0;
        page.shelves.push_back(newShelf);

        // place rect at top-left of this shelf
        Shelf &shelfRef = page.shelves.back();
        result.success = true;
        result.pageIndex = page.pageIndex;
        result.x = shelfRef.curX;
        result.y = shelfRef.y;
        shelfRef.curX += sizeNeeded;

        return result;
    }

    int pagesNeeded() const { return int(pages.size()); }
};

}

static QRhiTexture *allocateRhiShadowTexture(QRhi *rhi, QRhiTexture::Format format, const QSize &size, quint32 numLayers, QRhiTexture::Flags flags)
{
    auto texture = rhi->newTexture(format, size, 1, flags);
    if (flags & QRhiTexture::TextureArray)
        texture->setArraySize(numLayers);
    if (!texture->create())
        qWarning("Failed to create shadow map texture of size %dx%d", size.width(), size.height());
    return texture;
}

static QRhiRenderBuffer *allocateRhiShadowRenderBuffer(QRhi *rhi, QRhiRenderBuffer::Type type, const QSize &size)
{
    auto renderBuffer = rhi->newRenderBuffer(type, size, 1);
    if (!renderBuffer->create())
        qWarning("Failed to build depth-stencil buffer of size %dx%d", size.width(), size.height());
    return renderBuffer;
}


static bool checkCompatibility(QSSGShadowMapEntry *entry, const QVector<QSSGShadowMapEntry::AtlasEntry> &atlasEntries) {
    if (!entry)
        return false;

    for (int i = 0; i < atlasEntries.size(); ++i) {
        const auto &atlasEntry = atlasEntries.at(i);

        if (!(entry->m_atlasInfo[i].layerIndex == atlasEntry.layerIndex &&
              qFuzzyCompare(entry->m_atlasInfo[i].uOffset, atlasEntry.uOffset) &&
              qFuzzyCompare(entry->m_atlasInfo[i].vOffset, atlasEntry.vOffset) &&
              qFuzzyCompare(entry->m_atlasInfo[i].uvScale, atlasEntry.uvScale)))
            return false;
    }

    return true;
}

static QVector<QSSGShadowMapEntry::AtlasEntry> createAtlasEntries(const QVarLengthArray<AtlasHelpers::ShelfPacker::AtlasPlacement, 4> &atlasPlacements, int entrySize, int atlasPageSize) {
    QVector<QSSGShadowMapEntry::AtlasEntry> atlasEntries;
    atlasEntries.reserve(atlasPlacements.size());
    for (int i = 0; i < atlasPlacements.size(); ++i) {
        const auto &placement = atlasPlacements.at(i);
        const float x = float(placement.x) / float(atlasPageSize);
        const float y = float(placement.y) / float(atlasPageSize);
        const float uvScale = float(entrySize) / float(atlasPageSize);
        atlasEntries.append({placement.pageIndex, x, y, uvScale});
    }
    return atlasEntries;
}


QSSGRenderShadowMap::QSSGRenderShadowMap(const QSSGRenderContextInterface &inContext)
    : m_context(inContext)
{
}

QSSGRenderShadowMap::~QSSGRenderShadowMap()
{
    releaseCachedResources();
}

void QSSGRenderShadowMap::releaseCachedResources()
{
    for (QSSGShadowMapEntry &entry : m_shadowMapList)
        entry.destroyRhiResources();

    if (m_shadowMapAtlasTexture)
        m_shadowMapAtlasTexture.reset();

    if (m_shadowMapBlueNoiseTexture)
        m_shadowMapBlueNoiseTexture.reset();

    qDeleteAll(m_layerDepthStencilBuffers);
    m_layerDepthStencilBuffers.clear();
    qDeleteAll(m_layerRenderTargets);
    m_layerRenderTargets.clear();
    qDeleteAll(m_layerRenderPassDescriptors);
    m_layerRenderPassDescriptors.clear();

    m_shadowMapList.clear();
}

void QSSGRenderShadowMap::addShadowMaps(const QSSGShaderLightList &renderableLights)
{
    QRhi *rhi = m_context.rhiContext()->rhi();
    // Bail out if there is no QRhi, since we can't add entries without it
    if (!rhi)
        return;

    // Handle blue noise texture
    if (!m_shadowMapBlueNoiseTexture) {
        // The blue noise image is based on two random blue noise images with values v0 and v1.
        // Then the cos and sin of these are stored to avoid trigonmoetric functions
        // in the shadow mapping shader.
        // R = cos(v0)
        // G = sin(v0)
        // B = cos(v1)
        // A = sin(v1)
        QImage blueNoiseImage(QStringLiteral(":/res/textures/blue_noise.png"));
        if (blueNoiseImage.isNull()) {
            qWarning("Failed to load blue noise texture!");
        } else {
            Q_ASSERT(blueNoiseImage.size() == QSize(64, 64));
            Q_ASSERT(blueNoiseImage.format() == QImage::Format_ARGB32);
            blueNoiseImage = blueNoiseImage.convertToFormat(QImage::Format_RGBA8888);
            m_shadowMapBlueNoiseTexture.reset(allocateRhiShadowTexture(rhi, QRhiTexture::RGBA8, QSize(64, 64), 0, {}));
            if (!m_shadowMapBlueNoiseTexture->create()) {
                qWarning("Failed to create blue noise texture");
            } else {
                QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
                QRhiTextureSubresourceUploadDescription subresDesc(blueNoiseImage);
                rub->uploadTexture(m_shadowMapBlueNoiseTexture.get(),
                                   QRhiTextureUploadDescription({ QRhiTextureUploadEntry(0, 0, subresDesc) }));
                QRhiCommandBuffer *cb = m_context.rhiContext()->commandBuffer();
                cb->resourceUpdate(rub);
            }
        }
    }

    const quint32 numLights = renderableLights.size();
    qsizetype numShadows = 0;
    const bool supports32BitTextures = rhi->isTextureFormatSupported(QRhiTexture::R32F);
    QRhiTexture::Format format = QRhiTexture::R16F;
    quint32 mapSize = 0;
    QHash<quint32, QVector<QSSGShadowMapEntry::AtlasEntry>> lightIndexToAtlasEntries;

    // Get format and maximum texture size needed
    for (quint32 lightIndex = 0; lightIndex < numLights; ++lightIndex) {
        const QSSGShaderLight &shaderLight = renderableLights.at(lightIndex);
        if (!shaderLight.shadows)
            continue;

        // Force R32F format if any light requests it
        if (shaderLight.light->m_use32BitShadowmap && supports32BitTextures)
            format = QRhiTexture::R32F;

        // Find the largest shadow map size needed
        if (mapSize < shaderLight.light->m_shadowMapRes)
            mapSize = shaderLight.light->m_shadowMapRes;

        numShadows += 1;
    }

    auto atlasPacker = AtlasHelpers::ShelfPacker(mapSize, mapSize);


    // Figure out the number of layers needed for the atlas
    // including where everthing will go in the atlas
    for (quint32 lightIndex = 0; lightIndex < numLights; ++lightIndex) {
        const QSSGShaderLight &shaderLight = renderableLights.at(lightIndex);
        if (!shaderLight.shadows)
            continue;

        quint8 mapsNeeded = 0;

        if (shaderLight.light->type == QSSGRenderLight::Type::DirectionalLight)
            mapsNeeded = shaderLight.light->m_csmNumSplits + 1;
        else if (shaderLight.light->type == QSSGRenderLight::Type::SpotLight)
            mapsNeeded = 1;
        else if (shaderLight.light->type == QSSGRenderLight::Type::PointLight)
            mapsNeeded = 2;

        QVarLengthArray<AtlasHelpers::ShelfPacker::AtlasPlacement, 4> atlasPlacements;
        for (quint8 i = 0; i < mapsNeeded; ++i) {
            const int size = shaderLight.light->m_shadowMapRes;
            auto result = atlasPacker.addRectangle(size);
            if (result.success)
                atlasPlacements.push_back(result);
        }
        lightIndexToAtlasEntries.insert(lightIndex, createAtlasEntries(atlasPlacements, shaderLight.light->m_shadowMapRes, mapSize));
    }
    const QSize texSize = QSize(mapSize, mapSize);
    const quint32 layersNeeded = atlasPacker.pagesNeeded();

    // Check if we need a rebuild
    bool needsRebuild = numShadows != shadowMapEntryCount();
    if (!m_shadowMapAtlasTexture ||
        m_shadowMapAtlasTexture->pixelSize() != texSize ||
        m_shadowMapAtlasTexture->arraySize() != int(layersNeeded) ||
        m_shadowMapAtlasTexture->format() != format) {

        // If we need a new texture as well
        if (m_shadowMapAtlasTexture) {
            m_shadowMapAtlasTexture.reset();
        }

        m_shadowMapAtlasTexture.reset(allocateRhiShadowTexture(rhi, format, texSize, layersNeeded, QRhiTexture::RenderTarget | QRhiTexture::TextureArray));

        qDeleteAll(m_layerDepthStencilBuffers);
        m_layerDepthStencilBuffers.clear();
        qDeleteAll(m_layerRenderTargets);
        m_layerRenderTargets.clear();
        qDeleteAll(m_layerRenderPassDescriptors);
        m_layerRenderPassDescriptors.clear();

        for (quint32 i = 0; i < layersNeeded; ++i) {
            // Recreate per layer RenderBuffers, TextureRenderTarget, and RenderPassDescriptors
            QRhiRenderBuffer *depthStencilBuffer = allocateRhiShadowRenderBuffer(rhi, QRhiRenderBuffer::DepthStencil, texSize);
            QRhiTextureRenderTargetDescription rtDesc;
            QRhiColorAttachment attachment(m_shadowMapAtlasTexture.get());
            attachment.setLayer(i);
            rtDesc.setColorAttachments({ attachment });
            rtDesc.setDepthStencilBuffer(depthStencilBuffer);
            QRhiTextureRenderTarget *rt = rhi->newTextureRenderTarget(rtDesc);
            rt->setDescription(rtDesc);
            rt->setFlags(QRhiTextureRenderTarget::PreserveColorContents); // Don't clear between passes since this is an atlas
            QRhiRenderPassDescriptor *rpDesc = rt->newCompatibleRenderPassDescriptor();
            rt->setRenderPassDescriptor(rpDesc);
            if (!rt->create())
                qWarning("Failed to build shadow map render target");
            rt->setName(QByteArrayLiteral("shadow map atlas layer") + QByteArray::number(i));
            m_layerDepthStencilBuffers.append(depthStencilBuffer);
            m_layerRenderTargets.append(rt);
            m_layerRenderPassDescriptors.append(rpDesc);
        }

        needsRebuild = true;
    }

    // If we need to allocate the RHI resources
    if (!m_sharedFrontCubeToAtlasUniformBuffer || !m_sharedBackCubeToAtlasUniformBuffer) {
        const quint32 uniformValueFront = 0;
        const quint32 uniformValueBack = 1;

        QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
        if (!m_sharedFrontCubeToAtlasUniformBuffer) {
            m_sharedFrontCubeToAtlasUniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(uint32_t)));
            m_sharedFrontCubeToAtlasUniformBuffer->create();
            rub->updateDynamicBuffer(m_sharedFrontCubeToAtlasUniformBuffer.get(), 0, sizeof(quint32), &uniformValueFront);
        }

        if (!m_sharedBackCubeToAtlasUniformBuffer) {
            m_sharedBackCubeToAtlasUniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(uint32_t)));
            m_sharedBackCubeToAtlasUniformBuffer->create();
            rub->updateDynamicBuffer(m_sharedBackCubeToAtlasUniformBuffer.get(), 0, sizeof(quint32), &uniformValueBack);
        }
        QRhiCommandBuffer *cb = m_context.rhiContext()->commandBuffer();
        cb->resourceUpdate(rub);
    }
    if (!m_sharedCubeToAtlasSampler) {
        m_sharedCubeToAtlasSampler.reset(rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        m_sharedCubeToAtlasSampler->create();
    }
    if (!m_shadowClearSrb) {
        m_shadowClearSrb.reset(rhi->newShaderResourceBindings());
        m_shadowClearSrb->create();
    }


    if (!needsRebuild) {
        // Check if relevant shadow properties has changed
        for (quint32 lightIndex = 0; lightIndex < numLights; ++lightIndex) {
            const QSSGShaderLight &shaderLight = renderableLights.at(lightIndex);
            if (!shaderLight.shadows)
                continue;
            QSSGShadowMapEntry *pEntry = shadowMapEntry(lightIndex);
            if (!pEntry) {
                needsRebuild = true;
                break;
            }

            const auto &atlasEntires = lightIndexToAtlasEntries.value(lightIndex);
            if (!checkCompatibility(pEntry, atlasEntires)) {
                needsRebuild = true;
                break;
            }
        }
    }

    if (!needsRebuild)
        return;

    // Rebuild then
    for (QSSGShadowMapEntry &entry : m_shadowMapList)
        entry.destroyRhiResources();
    m_shadowMapList.clear();

    for (quint32 lightIndex = 0; lightIndex < numLights; ++lightIndex) {
        const QSSGShaderLight &shaderLight = renderableLights.at(lightIndex);
        if (!shaderLight.shadows)
            continue;
        addShadowMap(lightIndex,
                     texSize,
                     lightIndexToAtlasEntries.value(lightIndex),
                     shaderLight.light->m_csmNumSplits,
                     format,
                     shaderLight.light->type == QSSGRenderLight::Type::PointLight,
                     shaderLight.light->debugObjectName);
    }

}

QSSGShadowMapEntry *QSSGRenderShadowMap::addShadowMap(quint32 lightIdx,
                                                      QSize size,
                                                      QVector<QSSGShadowMapEntry::AtlasEntry> atlasEntries,
                                                      quint32 csmNumSplits,
                                                      QRhiTexture::Format rhiFormat,
                                                      bool isPointLight,
                                                      const QString &renderNodeObjName)
{
    QRhi *rhi = m_context.rhiContext()->rhi();
    QSSGShadowMapEntry *pEntry = shadowMapEntry(lightIdx);

    Q_ASSERT(rhi);
    Q_ASSERT(!pEntry);
    Q_ASSERT(!atlasEntries.isEmpty());

    m_shadowMapList.push_back(QSSGShadowMapEntry::withAtlas(lightIdx));

    pEntry = &m_shadowMapList.back();
    pEntry->m_csmNumSplits = csmNumSplits;

    if (isPointLight) {
        const QSize localSize = size * atlasEntries.first().uvScale;
        // First pass renders to depth cube map
        const QByteArray rtName = renderNodeObjName.toLatin1();

        pEntry->m_rhiDepthCube = allocateRhiShadowTexture(rhi, rhiFormat, localSize, 0, QRhiTexture::RenderTarget | QRhiTexture::CubeMap);
        pEntry->m_rhiDepthStencilCube = allocateRhiShadowRenderBuffer(rhi, QRhiRenderBuffer::DepthStencil, localSize);


        for (const auto face : QSSGRenderTextureCubeFaces) {
            QRhiTextureRenderTarget *&rt(pEntry->m_rhiRenderTargetCube[quint8(face)]);
            Q_ASSERT(!rt);
            QRhiColorAttachment att(pEntry->m_rhiDepthCube);
            att.setLayer(quint8(face)); // 6 render targets, each referencing one face of the cubemap
            QRhiTextureRenderTargetDescription rtDesc;
            rtDesc.setColorAttachments({ att });
            rtDesc.setDepthStencilBuffer(pEntry->m_rhiDepthStencilCube);
            rt = rhi->newTextureRenderTarget(rtDesc);
            rt->setDescription(rtDesc);
            if (!pEntry->m_rhiRenderPassDescCube)
                pEntry->m_rhiRenderPassDescCube = rt->newCompatibleRenderPassDescriptor();
            rt->setRenderPassDescriptor(pEntry->m_rhiRenderPassDescCube);
            if (!rt->create())
                qWarning("Failed to build shadow map render target");
            rt->setName(rtName + QByteArrayLiteral(" shadow cube face: ") + QSSGBaseTypeHelpers::displayName(face));
        }

        if (!pEntry->m_cubeToAtlasFrontSrb) {
            pEntry->m_cubeToAtlasFrontSrb = rhi->newShaderResourceBindings();
            pEntry->m_cubeToAtlasFrontSrb->setBindings({
                QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, m_sharedFrontCubeToAtlasUniformBuffer.get()),
                QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, pEntry->m_rhiDepthCube, m_sharedCubeToAtlasSampler.get())
            });
            pEntry->m_cubeToAtlasFrontSrb->create();
        }

        if (!pEntry->m_cubeToAtlasBackSrb) {
            pEntry->m_cubeToAtlasBackSrb = rhi->newShaderResourceBindings();
            pEntry->m_cubeToAtlasBackSrb->setBindings({
                    QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, m_sharedBackCubeToAtlasUniformBuffer.get()),
                    QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, pEntry->m_rhiDepthCube, m_sharedCubeToAtlasSampler.get())
            });
            pEntry->m_cubeToAtlasBackSrb->create();
        }
    }


    // Additional graphics resources: samplers, render targets.
    const quint32 entriesCount = atlasEntries.size();
    for (quint32 splitIndex = 0; splitIndex < entriesCount; splitIndex++) {
        pEntry->m_atlasInfo[splitIndex].layerIndex = atlasEntries.at(splitIndex).layerIndex;
        pEntry->m_atlasInfo[splitIndex].uOffset = atlasEntries.at(splitIndex).uOffset;
        pEntry->m_atlasInfo[splitIndex].vOffset = atlasEntries.at(splitIndex).vOffset;
        pEntry->m_atlasInfo[splitIndex].uvScale = atlasEntries.at(splitIndex).uvScale;

        quint32 layerId = atlasEntries.at(splitIndex).layerIndex;
        pEntry->m_rhiRenderTargets[splitIndex] = m_layerRenderTargets[layerId];
        pEntry->m_rhiRenderPassDesc[splitIndex] = m_layerRenderPassDescriptors[layerId];
    }
    pEntry->m_lightIndex = lightIdx;

    return pEntry;
}

QSSGShadowMapEntry *QSSGRenderShadowMap::shadowMapEntry(int lightIdx)
{
    Q_ASSERT(lightIdx >= 0);

    for (int i = 0; i < m_shadowMapList.size(); i++) {
        QSSGShadowMapEntry *pEntry = &m_shadowMapList[i];
        if (pEntry->m_lightIndex == quint32(lightIdx))
            return pEntry;
    }

    return nullptr;
}

QRhiTextureRenderTarget *QSSGRenderShadowMap::layerRenderTarget(int layerIndex)
{
    if (layerIndex < 0 || layerIndex >= m_layerRenderTargets.size())
        return nullptr;
    return m_layerRenderTargets.at(layerIndex);
}

QRhiRenderPassDescriptor *QSSGRenderShadowMap::layerRenderPassDescriptor(int layerIndex)
{
    if (layerIndex < 0 || layerIndex >= m_layerRenderPassDescriptors.size())
        return nullptr;
    return m_layerRenderPassDescriptors.at(layerIndex);
}

QRhiTexture *QSSGRenderShadowMap::shadowMapAtlasTexture() const
{
    return m_shadowMapAtlasTexture.get();
}

QRhiTexture *QSSGRenderShadowMap::shadowMapBlueNoiseTexture() const
{
    return m_shadowMapBlueNoiseTexture.get();
}

QSSGShadowMapEntry::QSSGShadowMapEntry()
    : m_lightIndex(std::numeric_limits<quint32>::max())
{
}

QSSGShadowMapEntry QSSGShadowMapEntry::withAtlas(quint32 lightIdx)
{
    QSSGShadowMapEntry e;
    e.m_lightIndex = lightIdx;
    return e;
}

void QSSGShadowMapEntry::destroyRhiResources()
{
    delete m_rhiDepthCube;
    m_rhiDepthCube = nullptr;
    delete m_rhiDepthStencilCube;
    m_rhiDepthStencilCube = nullptr;
    qDeleteAll(m_rhiRenderTargetCube);
    m_rhiRenderTargetCube.fill(nullptr);
    delete m_rhiRenderPassDescCube;
    m_rhiRenderPassDescCube = nullptr;
    delete m_cubeToAtlasFrontSrb;
    m_cubeToAtlasFrontSrb = nullptr;
    delete m_cubeToAtlasBackSrb;
    m_cubeToAtlasBackSrb = nullptr;

    // un-owned references
    m_rhiRenderTargets.fill(nullptr);
    m_rhiRenderPassDesc.fill(nullptr);
}

QT_END_NAMESPACE
