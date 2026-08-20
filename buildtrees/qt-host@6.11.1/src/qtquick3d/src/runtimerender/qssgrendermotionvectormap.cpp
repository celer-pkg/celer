// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <ssg/qssgrendercontextcore.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermotionvectormap_p.h>

QT_BEGIN_NAMESPACE

QSSGRenderMotionVectorMap::QSSGRenderMotionVectorMap(const QSSGRenderContextInterface &inContext)
    : m_context(inContext)
{
}

QSSGRenderMotionVectorMap::~QSSGRenderMotionVectorMap()
{
    releaseCachedResources();
}

void QSSGRenderMotionVectorMap::endFrame()
{
    cleanupStaleEntries();
}

static void copyToTextureData(QSSGRenderTextureData *dst, QSSGRenderTextureData *src) {
    dst->setTextureData(src->textureData());
    dst->setDepth(src->depth());
    dst->setFormat(src->format());
    dst->setHasTransparency(src->hasTransparency());
    dst->setSize(src->size());
}

static void copyToTextureData(QSSGRenderTextureData *dst, QSSGRenderInstanceTable *src) {

    dst->setDepth(0);
    dst->setFormat(QSSGRenderTextureFormat::RGBA32F);
    dst->setHasTransparency(false);

    auto instanceCount = src->count();
    int totalPixels = qMax(instanceCount * 4, 1);
    int width = qCeil(qSqrt(static_cast<float>(totalPixels)));
    int height = qCeil(static_cast<float>(totalPixels) / width);
    dst->setSize(QSize(width, height));

    QByteArray instanceBuffer(width * height * 4 * sizeof(float), Qt::Uninitialized);
    float *ptr = reinterpret_cast<float*>(instanceBuffer.data());

    for (int i = 0; i < instanceCount; ++i) {
        memcpy(ptr + i * 16, src->getTransform(i).constData(), 16 * sizeof(float));
    }
    dst->setTextureData(instanceBuffer);
}

static void copyToTextureData(QSSGRenderTextureData *dst, const QVector<float> &src) {

    dst->setDepth(0);
    dst->setFormat(QSSGRenderTextureFormat::R32F);
    dst->setHasTransparency(false);
    dst->setSize(QSize(src.count(), 1));
    QByteArray textureData(reinterpret_cast<const char*>(src.constData()), src.count() * sizeof(float));
    dst->setTextureData(textureData);
}

QSSGRenderImageTexture QSSGRenderMotionVectorMap::loadAndReleaseIfNeeded(const std::shared_ptr<QSSGRenderTextureData>& textureData, QSize lastSize)
{
    if (!textureData)
        return QSSGRenderImageTexture();

    auto texture = m_context.bufferManager()->loadTextureData(textureData.get(), QSSGBufferManager::MipModeDisable);

    // the only thing might to force to recreate new texture is changing the texture data size
    // so we should release last one
    if (lastSize != textureData->size()) {
        m_context.bufferManager()->releaseTextureData(
                QSSGBufferManager::CustomImageCacheKey{
                                                         textureData.get(),
                                                         lastSize,
                                                         QSSGBufferManager::MipMode::MipModeDisable});
    }
    return texture;
}

QSSGRenderMotionVectorMap::MotionResultData QSSGRenderMotionVectorMap::trackMotionData(
        const void *modelKey,
        const QMatrix4x4 &currentModelViewProjection,
        const QMatrix4x4 &currentInstanceLocal,
        const QMatrix4x4 &currentInstanceGlobal,
        QSSGRenderTextureData *currentBoneTextureData,
        QSSGRenderInstanceTable *currentInstanceTable,
        const QVector<float> &currentMorphWeights)
{
    auto lastFrameDataIterator = m_cache.find(modelKey);

    if (lastFrameDataIterator == m_cache.end()) {
        MotionStoreData dataToStore;
        dataToStore.prevModelViewProjection = currentModelViewProjection;
        dataToStore.prevInstanceLocal = currentInstanceLocal;
        dataToStore.prevInstanceGlobal = currentInstanceGlobal;
        dataToStore.frameAge = 0;
        dataToStore.visitedThisFrame = true;

        if (currentBoneTextureData) {
            dataToStore.prevBoneTextureData = std::make_shared<QSSGRenderTextureData>();
            copyToTextureData(dataToStore.prevBoneTextureData.get(), currentBoneTextureData);
            dataToStore.lastPrevBoneTextureDataSize = dataToStore.prevBoneTextureData->size();
        }

        if (currentInstanceTable) {
            dataToStore.prevInstanceTextureData = std::make_shared<QSSGRenderTextureData>();
            copyToTextureData(dataToStore.prevInstanceTextureData.get(), currentInstanceTable);
            dataToStore.lastPrevInstanceTextureDataSize = dataToStore.prevInstanceTextureData->size();
        }

        if (currentMorphWeights.size()) {
            dataToStore.currentMorphWeightTextureData = std::make_shared<QSSGRenderTextureData>();
            copyToTextureData(dataToStore.currentMorphWeightTextureData.get(), currentMorphWeights);
            dataToStore.lastCurrentMorphWeightTextureDataSize = dataToStore.currentMorphWeightTextureData->size();

            dataToStore.prevMorphWeightTextureData = std::make_shared<QSSGRenderTextureData>();
            copyToTextureData(dataToStore.prevMorphWeightTextureData.get(), currentMorphWeights);
            dataToStore.lastPrevMorphWeightTextureDataSize = dataToStore.prevMorphWeightTextureData->size();
        }

        MotionResultData firstFrameResult = { currentModelViewProjection, currentInstanceLocal, currentInstanceGlobal,
                                              QSSGRenderImageTexture(), QSSGRenderImageTexture(),
                                              QSSGRenderImageTexture(), QSSGRenderImageTexture() };
        dataToStore.cachedResult = firstFrameResult;
        m_cache[modelKey] = dataToStore;
        return firstFrameResult;
    }

    if (lastFrameDataIterator->visitedThisFrame)
        return lastFrameDataIterator->cachedResult;

    MotionResultData motionResult;

    motionResult.prevModelViewProjection = lastFrameDataIterator->prevModelViewProjection;
    lastFrameDataIterator->prevModelViewProjection = currentModelViewProjection;

    motionResult.prevInstanceLocal = lastFrameDataIterator->prevInstanceLocal;
    lastFrameDataIterator->prevInstanceLocal = currentInstanceLocal;

    motionResult.prevInstanceGlobal = lastFrameDataIterator->prevInstanceGlobal;
    lastFrameDataIterator->prevInstanceGlobal = currentInstanceGlobal;

    motionResult.prevBoneTexture = loadAndReleaseIfNeeded(
            lastFrameDataIterator->prevBoneTextureData,
            lastFrameDataIterator->lastPrevBoneTextureDataSize);

    motionResult.prevInstanceTexture = loadAndReleaseIfNeeded(
            lastFrameDataIterator->prevInstanceTextureData,
            lastFrameDataIterator->lastPrevInstanceTextureDataSize);

    motionResult.prevMorphWeightTexture = loadAndReleaseIfNeeded(
            lastFrameDataIterator->prevMorphWeightTextureData,
            lastFrameDataIterator->lastPrevMorphWeightTextureDataSize);

    motionResult.currentMorphWeightTexture = loadAndReleaseIfNeeded(
            lastFrameDataIterator->currentMorphWeightTextureData,
            lastFrameDataIterator->lastCurrentMorphWeightTextureDataSize);

    if (currentBoneTextureData) {
        if (!lastFrameDataIterator->prevBoneTextureData)
            lastFrameDataIterator->prevBoneTextureData = std::make_shared<QSSGRenderTextureData>();

        lastFrameDataIterator->lastPrevBoneTextureDataSize = lastFrameDataIterator->prevBoneTextureData->size();
        copyToTextureData(lastFrameDataIterator->prevBoneTextureData.get(), currentBoneTextureData);
    }
    if (currentInstanceTable) {
        if (!lastFrameDataIterator->prevInstanceTextureData)
            lastFrameDataIterator->prevInstanceTextureData = std::make_shared<QSSGRenderTextureData>();

        lastFrameDataIterator->lastPrevInstanceTextureDataSize = lastFrameDataIterator->prevInstanceTextureData->size();
        copyToTextureData(lastFrameDataIterator->prevInstanceTextureData.get(), currentInstanceTable);
    }

    if (currentMorphWeights.size()) {
        if (!lastFrameDataIterator->prevMorphWeightTextureData)
            lastFrameDataIterator->prevMorphWeightTextureData = std::make_shared<QSSGRenderTextureData>();

        lastFrameDataIterator->lastPrevMorphWeightTextureDataSize = lastFrameDataIterator->prevMorphWeightTextureData->size();
        copyToTextureData(lastFrameDataIterator->prevMorphWeightTextureData.get(), lastFrameDataIterator->currentMorphWeightTextureData.get());

        if (!lastFrameDataIterator->currentMorphWeightTextureData)
            lastFrameDataIterator->currentMorphWeightTextureData = std::make_shared<QSSGRenderTextureData>();

        lastFrameDataIterator->lastCurrentMorphWeightTextureDataSize = lastFrameDataIterator->currentMorphWeightTextureData->size();
        copyToTextureData(lastFrameDataIterator->currentMorphWeightTextureData.get(), currentMorphWeights);
    }

    lastFrameDataIterator->frameAge = 0;
    lastFrameDataIterator->visitedThisFrame = true;
    lastFrameDataIterator->cachedResult = motionResult;

    return motionResult;
}

void QSSGRenderMotionVectorMap::releaseEntryResources(MotionStoreData& entry)
{
    if (entry.prevBoneTextureData)
        m_context.bufferManager()->releaseTextureData(entry.prevBoneTextureData.get());
    if (entry.prevInstanceTextureData)
        m_context.bufferManager()->releaseTextureData(entry.prevInstanceTextureData.get());
    if (entry.prevMorphWeightTextureData)
        m_context.bufferManager()->releaseTextureData(entry.prevMorphWeightTextureData.get());
    if (entry.currentMorphWeightTextureData)
        m_context.bufferManager()->releaseTextureData(entry.currentMorphWeightTextureData.get());
}

void QSSGRenderMotionVectorMap::releaseCachedResources()
{
    for (auto& entry : m_cache)
        releaseEntryResources(entry);
    m_cache.clear();
}

void QSSGRenderMotionVectorMap::cleanupStaleEntries()
{
    constexpr int maxFrameAge = 2;
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        it->visitedThisFrame = false;
        it->frameAge++;
        if (it->frameAge > maxFrameAge) {
            releaseEntryResources(*it);
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }
}

QT_END_NAMESPACE
