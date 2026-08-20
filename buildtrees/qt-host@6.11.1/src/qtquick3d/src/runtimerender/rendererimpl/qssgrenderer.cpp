// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderer_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderitem2d_p.h>
#include "../qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include "../qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgperframeallocator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhiquadrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhiparticles_p.h>
#include <QtQuick3DRuntimeRender/private/qssgvertexpipelineimpl_p.h>
#include "../qssgshadermapkey_p.h"
#include "../qssgrenderpickresult_p.h"
#include "../graphobjects/qssgrenderroot_p.h"

#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>
#include <QtQuick3DUtils/private/qssgdataref_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3DUtils/private/qssgfrustum_p.h>
#include <qtquick3d_tracepoints_p.h>

#include <QtQuick/private/qsgcontext_p.h>
#include <QtQuick/private/qsgrenderer_p.h>

#include <QtCore/QMutexLocker>
#include <QtCore/QBitArray>

#include <cstdlib>
#include <algorithm>
#include <limits>

/*
    Rendering is done is several steps, these are:

    1. \l{QSSGRenderer::beginFrame(){beginFrame()} - set's up the renderer to start a new frame.

    2. Now that the renderer is reset, values for the \l{QSSGRenderer::setViewport}{viewport}, \l{QSSGRenderer::setDpr}{dpr},
    \l{QSSGRenderer::setScissorRect}{scissorRect} etc. should be updated.

    3. \l{QSSGRenderer::prepareLayerForRender()} - At this stage the scene tree will be traversed
    and state for the renderer needed to render gets collected. This includes, but is not limited to,
    calculating global transforms, loading of meshes, preparing materials and setting up the rendering
    steps needed for the frame (opaque and transparent pass etc.)
    If the there are custom \l{QQuick3DRenderExtension}{render extensions} added to to \l{View3D::extensions}{View3D}
    then they will get their first chance to modify or react to the collected data here.
    If the users have implemented the virtual function \l{QSSGRenderExtension::prepareData()}{prepareData} it will be
    called after all active nodes have been collected and had their global data updated, but before any mesh or material
    has been loaded.

    4. \l{QSSGRenderer::rhiPrepare()} - Starts rendering necessary sub-scenes and prepare resources.
    Sub-scenes, or sub-passes that are to be done in full, will be done at this stage.

    5. \l{QSSGRenderer::rhiRender()} - Renders the scene to the main target.

    6. \l{QSSGRenderer::endFrame()} - Marks the frame as done and cleans-up dirty states and
    uneeded resources.
*/

QT_BEGIN_NAMESPACE

struct QSSGRenderableImage;
class QSSGSubsetRenderable;

void QSSGRenderer::releaseCachedResources()
{
    m_rhiQuadRenderer.reset();
    m_rhiCubeRenderer.reset();
}

void QSSGRenderer::registerItem2DData(QSSGRenderItem2DData &data)
{
    // Check if data is already in the m_item2DDatas list, if not insert it.
    for (const auto *item2DData : m_item2DDatas) {
        if (item2DData == &data)
            return;
    }

    m_item2DDatas.push_back(&data);
}

void QSSGRenderer::unregisterItem2DData(QSSGRenderItem2DData &data)
{
    const auto foundIt = std::find(m_item2DDatas.begin(), m_item2DDatas.end(), &data);
    if (foundIt != m_item2DDatas.end())
        m_item2DDatas.erase(foundIt);
}

void QSSGRenderer::releaseItem2DData(const QSSGRenderItem2D &item2D)
{
    for (auto *item2DData : m_item2DDatas)
        item2DData->releaseRenderData(item2D);
}

QSSGRenderer::QSSGRenderer() = default;

QSSGRenderer::~QSSGRenderer()
{
    m_contextInterface = nullptr;
    releaseCachedResources();
}

void QSSGRenderer::cleanupUnreferencedBuffers(QSSGRenderLayer *inLayer)
{
    // Now check for unreferenced buffers and release them if necessary
    m_contextInterface->bufferManager()->cleanupUnreferencedBuffers(m_frameCount, inLayer);
}

void QSSGRenderer::resetResourceCounters(QSSGRenderLayer *inLayer)
{
    m_contextInterface->bufferManager()->resetUsageCounters(m_frameCount, inLayer);
}

bool QSSGRenderer::prepareLayerForRender(QSSGRenderLayer &inLayer)
{
    QSSGLayerRenderData *theRenderData = getOrCreateLayerRenderData(inLayer);
    Q_ASSERT(theRenderData);

    // Need to check if the world root node is dirty and if we need to trigger
    // a reindex of the world root node.
    Q_ASSERT(inLayer.rootNode);
    if (inLayer.rootNode->isDirty(QSSGRenderRoot::DirtyFlag::TreeDirty))
        inLayer.rootNode->reindex(); // Clears TreeDirty flag

    beginLayerRender(*theRenderData);
    theRenderData->resetForFrame();
    theRenderData->prepareForRender();
    endLayerRender();
    return theRenderData->layerPrepResult.getFlags().wasDirty();
}

// Phase 1: prepare. Called when the renderpass is not yet started on the command buffer.
void QSSGRenderer::rhiPrepare(QSSGRenderLayer &inLayer)
{
    QSSGLayerRenderData *theRenderData = getOrCreateLayerRenderData(inLayer);
    QSSG_ASSERT(theRenderData && !theRenderData->renderedCameras.isEmpty(), return);

    const auto layerPrepResult = theRenderData->layerPrepResult;
    if (layerPrepResult.isLayerVisible()) {
        ///
        QSSGRhiContext *rhiCtx = contextInterface()->rhiContext().get();
        QSSG_ASSERT(rhiCtx->isValid() && rhiCtx->rhi()->isRecordingFrame(), return);
        beginLayerRender(*theRenderData);
        theRenderData->maybeProcessLightmapBaking();
        // Process active passes. "PreMain" passes are individual passes
        // that does can and should be done in the rhi prepare phase.
        // It is assumed that passes are sorted in the list with regards to
        // execution order.
        const auto &activePasses = theRenderData->activePasses;
        for (const auto &pass : activePasses) {
            pass->renderPrep(*this, *theRenderData);
            if (pass->passType() == QSSGRenderPass::Type::Standalone)
                pass->renderPass(*this);
        }

        endLayerRender();
    }
}

// Phase 2: render. Called within an active renderpass on the command buffer.
void QSSGRenderer::rhiRender(QSSGRenderLayer &inLayer)
{
    QSSGLayerRenderData *theRenderData = getOrCreateLayerRenderData(inLayer);
    QSSG_ASSERT(theRenderData && !theRenderData->renderedCameras.isEmpty(), return);
    if (theRenderData->layerPrepResult.isLayerVisible()) {
        beginLayerRender(*theRenderData);
        const auto &activePasses = theRenderData->activePasses;
        for (const auto &pass : activePasses) {
            if (pass->passType() == QSSGRenderPass::Type::Main || pass->passType() == QSSGRenderPass::Type::Extension)
                pass->renderPass(*this);
        }
        endLayerRender();
    }
}

template<typename Container>
static void cleanupResourcesImpl(const QSSGRenderContextInterface &rci, const Container &resources)
{
    const auto &rhiCtx = rci.rhiContext();
    if (!rhiCtx->isValid())
        return;

    const auto &bufferManager = rci.bufferManager();

    for (const auto &resource : resources) {
        if (resource->type == QSSGRenderGraphObject::Type::Geometry) {
            auto geometry = static_cast<QSSGRenderGeometry*>(resource);
            bufferManager->releaseGeometry(geometry);
        } else if (resource->type == QSSGRenderGraphObject::Type::Model) {
            auto model = static_cast<QSSGRenderModel*>(resource);
            QSSGRhiContextPrivate::get(rhiCtx.get())->cleanupDrawCallData(model);
            delete model->particleBuffer;
        } else if (resource->type == QSSGRenderGraphObject::Type::TextureData || resource->type == QSSGRenderGraphObject::Type::Skin) {
            static_assert(std::is_base_of_v<QSSGRenderTextureData, QSSGRenderSkin>, "QSSGRenderSkin is expected to be a QSSGRenderTextureData type!");
            auto textureData = static_cast<QSSGRenderTextureData *>(resource);
            bufferManager->releaseTextureData(textureData);
        } else if (resource->type == QSSGRenderGraphObject::Type::RenderExtension) {
            auto *rext = static_cast<QSSGRenderExtension *>(resource);
            bufferManager->releaseExtensionResult(*rext);
        } else if (resource->type == QSSGRenderGraphObject::Type::ModelInstance) {
            auto *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx.get());
            auto *table = static_cast<QSSGRenderInstanceTable *>(resource);
            rhiCtxD->releaseInstanceBuffer(table);
        } else if (resource->type == QSSGRenderGraphObject::Type::Item2D) {
            auto *item2D = static_cast<QSSGRenderItem2D *>(resource);
            rci.renderer()->releaseItem2DData(*item2D);
        } else if (resource->type == QSSGRenderGraphObject::Type::RenderPass) {
            auto *userPass = static_cast<QSSGRenderUserPass *>(resource);
            bufferManager->releaseUserRenderPass(*userPass);
        }

        // ### There might be more types that need to be supported

        delete resource;
    }
}

void QSSGRenderer::cleanupResources(QList<QSSGRenderGraphObject *> &resources)
{
    cleanupResourcesImpl(*m_contextInterface, resources);
    resources.clear();
}

void QSSGRenderer::cleanupResources(QSet<QSSGRenderGraphObject *> &resources)
{
    cleanupResourcesImpl(*m_contextInterface, resources);
    resources.clear();
}

QSSGLayerRenderData *QSSGRenderer::getOrCreateLayerRenderData(QSSGRenderLayer &layer)
{
    if (layer.renderData == nullptr)
        layer.renderData = new QSSGLayerRenderData(layer, *this);

    return layer.renderData;
}

void QSSGRenderer::addMaterialDirtyClear(QSSGRenderGraphObject *material)
{
    m_materialClearDirty.insert(material);
}

static QByteArray rendererLogPrefix() { return QByteArrayLiteral("mesh default material pipeline-- "); }


QSSGRhiShaderPipelinePtr QSSGRendererPrivate::generateRhiShaderPipelineImpl(QSSGSubsetRenderable &renderable,
                                                                            QSSGShaderLibraryManager &shaderLibraryManager,
                                                                            QSSGShaderCache &shaderCache,
                                                                            QSSGProgramGenerator &shaderProgramGenerator,
                                                                            const QSSGShaderDefaultMaterialKeyProperties &shaderKeyProperties,
                                                                            const QSSGShaderFeatures &featureSet,
                                                                            const QSSGUserShaderAugmentation &shaderAugmentation,
                                                                            QByteArray &shaderString)
{
    shaderString = rendererLogPrefix();
    QSSGShaderDefaultMaterialKey theKey(renderable.shaderDescription);

    // This is not a cheap operation. This function assumes that it will not be
    // hit for every material for every model in every frame (except of course
    // for materials that got changed). In practice this is ensured by the
    // cheaper-to-lookup cache in getShaderPipelineForDefaultMaterial().
    theKey.toString(shaderString, shaderKeyProperties);

    // Check the in-memory, per-QSSGShaderCache (and so per-QQuickWindow)
    // runtime cache. That may get cleared upon an explicit call to
    // QQuickWindow::releaseResources(), but will otherwise store all
    // encountered shader pipelines in any View3D in the window.
    if (const auto &maybePipeline = shaderCache.tryGetRhiShaderPipeline(shaderString, featureSet))
        return maybePipeline;

    // Check if there's a pre-built (offline generated) shader for available.
    const QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(shaderString, QQsbCollection::toFeatureSet(featureSet));
    const QQsbCollection::EntryMap &pregenEntries = shaderLibraryManager.m_preGeneratedShaderEntries;
    if (!pregenEntries.isEmpty()) {
        const auto foundIt = pregenEntries.constFind(QQsbCollection::Entry(qsbcKey));
        if (foundIt != pregenEntries.cend())
            return shaderCache.newPipelineFromPregenerated(shaderString, featureSet, *foundIt, renderable.material);
    }

    // Try the persistent (disk-based) cache then.
    if (const auto &maybePipeline = shaderCache.tryNewPipelineFromPersistentCache(qsbcKey, shaderString, featureSet))
        return maybePipeline;

    // Otherwise, build new shader code and run the resulting shaders through
    // the shader conditioning pipeline.
    const auto &material = static_cast<const QSSGRenderDefaultMaterial &>(renderable.getMaterial());
    QSSGMaterialVertexPipeline vertexPipeline(shaderProgramGenerator,
                                              shaderKeyProperties,
                                              material.adapter);

    return QSSGMaterialShaderGenerator::generateMaterialRhiShader(rendererLogPrefix(),
                                                                  vertexPipeline,
                                                                  renderable.shaderDescription,
                                                                  shaderKeyProperties,
                                                                  featureSet,
                                                                  renderable.material,
                                                                  shaderLibraryManager,
                                                                  shaderCache,
                                                                  shaderAugmentation);
}

QSSGRhiShaderPipelinePtr QSSGRendererPrivate::generateRhiShaderPipeline(QSSGRenderer &renderer,
                                                                        QSSGSubsetRenderable &inRenderable,
                                                                        const QSSGShaderFeatures &inFeatureSet,
                                                                        const QSSGUserShaderAugmentation &shaderAugmentation = {})
{
    auto *currentLayer = renderer.m_currentLayer;
    auto &generatedShaderString = currentLayer->generatedShaderString;
    const auto &m_contextInterface = renderer.m_contextInterface;
    const auto &theCache = m_contextInterface->shaderCache();
    const auto &shaderProgramGenerator = m_contextInterface->shaderProgramGenerator();
    const auto &shaderLibraryManager = m_contextInterface->shaderLibraryManager();
    return QSSGRendererPrivate::generateRhiShaderPipelineImpl(inRenderable, *shaderLibraryManager, *theCache, *shaderProgramGenerator, currentLayer->defaultMaterialShaderKeyProperties, inFeatureSet, shaderAugmentation, generatedShaderString);
}

void QSSGRenderer::beginFrame(QSSGRenderLayer &layer, bool allowRecursion)
{
    const bool executeBeginFrame = !(allowRecursion && (m_activeFrameRef++ != 0));
    if (executeBeginFrame) {
        m_contextInterface->perFrameAllocator()->reset();
        QSSGRHICTX_STAT(m_contextInterface->rhiContext().get(), start(&layer));
        resetResourceCounters(&layer);
    }
}

bool QSSGRenderer::endFrame(QSSGRenderLayer &layer, bool allowRecursion)
{
    const bool executeEndFrame = !(allowRecursion && (--m_activeFrameRef != 0));
    if (executeEndFrame) {
        cleanupUnreferencedBuffers(&layer);

               // We need to do this endFrame(), as the material nodes might not exist after this!
        for (auto *matObj : std::as_const(m_materialClearDirty)) {
            if (matObj->type == QSSGRenderGraphObject::Type::CustomMaterial) {
                static_cast<QSSGRenderCustomMaterial *>(matObj)->clearDirty();
            } else if (matObj->type == QSSGRenderGraphObject::Type::DefaultMaterial ||
                       matObj->type == QSSGRenderGraphObject::Type::PrincipledMaterial ||
                       matObj->type == QSSGRenderGraphObject::Type::SpecularGlossyMaterial) {
                static_cast<QSSGRenderDefaultMaterial *>(matObj)->clearDirty();
            }
        }
        m_materialClearDirty.clear();

        QSSGRHICTX_STAT(m_contextInterface->rhiContext().get(), stop(&layer));

        ++m_frameCount;
    }

    return executeEndFrame;
}

QSSGRendererPrivate::PickResultList QSSGRendererPrivate::syncPickAll(const QSSGRenderContextInterface &ctx,
                                                                     const QSSGRenderLayer &layer,
                                                                     const QSSGRenderRay &ray)
{
    const auto &bufferManager = ctx.bufferManager();
    const bool isGlobalPickingEnabled = QSSGRendererPrivate::isGlobalPickingEnabled(*ctx.renderer());
    PickResultList pickResults;
    Q_ASSERT(layer.getGlobalState(QSSGRenderNode::GlobalState::Active));
    getLayerHitObjectList(layer, *bufferManager, ray, isGlobalPickingEnabled, pickResults);
    // Things are rendered in a particular order and we need to respect that ordering.
    std::stable_sort(pickResults.begin(), pickResults.end(), [](const QSSGRenderPickResult &lhs, const QSSGRenderPickResult &rhs) {
        return lhs.m_distanceSq < rhs.m_distanceSq;
    });
    return pickResults;
}

QSSGRendererPrivate::PickResultList QSSGRendererPrivate::syncPick(const QSSGRenderContextInterface &ctx,
                                                                  const QSSGRenderLayer &layer,
                                                                  const QSSGRenderRay &ray,
                                                                  QSSGRenderNode *target)
{
    const auto &bufferManager = ctx.bufferManager();
    const bool isGlobalPickingEnabled = QSSGRendererPrivate::isGlobalPickingEnabled(*ctx.renderer());

    Q_ASSERT(layer.getGlobalState(QSSGRenderNode::GlobalState::Active));
    PickResultList pickResults;
    if (target)
        intersectRayWithSubsetRenderable(layer, *bufferManager, ray, *target, pickResults);
    else
        getLayerHitObjectList(layer, *bufferManager, ray, isGlobalPickingEnabled, pickResults);

    std::stable_sort(pickResults.begin(), pickResults.end(), [](const QSSGRenderPickResult &lhs, const QSSGRenderPickResult &rhs) {
        return lhs.m_distanceSq < rhs.m_distanceSq;
    });
    return pickResults;
}

using RenderableList = QVarLengthArray<const QSSGRenderNode *>;
static void getPickableRecursive(const QSSGRenderNode &node, RenderableList &renderables, bool pickEverything = false)
{
    if (QSSGRenderGraphObject::isRenderable(node.type) && (pickEverything || node.getLocalState(QSSGRenderNode::LocalState::Pickable))) {
        renderables.push_back(&node);
    }

    for (const auto &child : node.children)
        getPickableRecursive(child, renderables, pickEverything);
}

std::optional<QSSGRenderPickResult> QSSGRendererPrivate::syncPickClosestPoint(const QSSGRenderContextInterface &ctx,
                                                                              const QSSGRenderLayer &layer,
                                                                              const QVector3D &center, const float radiusSquared,
                                                                              QSSGRenderNode *target)
{
    const auto &bufferManager = ctx.bufferManager();

    Q_ASSERT(layer.getGlobalState(QSSGRenderNode::GlobalState::Active));
    std::optional<QSSGRenderPickResult> result = std::nullopt;
    if (target) {
        result = closestPointOnSubsetRenderable(layer, *bufferManager, center, radiusSquared, *target);
    } else {
        const bool pickEverything = QSSGRendererPrivate::isGlobalPickingEnabled(*ctx.renderer());
        RenderableList renderables;
        for (const auto &childNode : layer.children)
            getPickableRecursive(childNode, renderables, pickEverything);
        float bestDistSquared = radiusSquared;
        for (const auto &childNode : renderables) {
            const auto res = closestPointOnSubsetRenderable(layer, *bufferManager, center, bestDistSquared, *childNode);
            if (res.has_value()) {
                bestDistSquared = res.value().m_distanceSq;
                result = res;
            }
        }
    }

    return result;
}

QSSGRendererPrivate::PickResultList QSSGRendererPrivate::syncPickSubset(const QSSGRenderLayer &layer,
                                                                        QSSGBufferManager &bufferManager,
                                                                        const QSSGRenderRay &ray,
                                                                        QVarLengthArray<QSSGRenderNode*> subset)
{
    QSSGRendererPrivate::PickResultList pickResults;
    Q_ASSERT(layer.getGlobalState(QSSGRenderNode::GlobalState::Active));

    for (auto target : subset)
        intersectRayWithSubsetRenderable(layer, bufferManager, ray, *target, pickResults);

    std::stable_sort(pickResults.begin(), pickResults.end(), [](const QSSGRenderPickResult &lhs, const QSSGRenderPickResult &rhs) {
        return lhs.m_distanceSq < rhs.m_distanceSq;
    });
    return pickResults;
}

void QSSGRendererPrivate::setGlobalPickingEnabled(QSSGRenderer &renderer, bool isEnabled)
{
    renderer.m_globalPickingEnabled = isEnabled;
}

void QSSGRendererPrivate::setRenderContextInterface(QSSGRenderer &renderer, QSSGRenderContextInterface *ctx)
{
    renderer.m_contextInterface = ctx;
}

void QSSGRendererPrivate::setSgRenderContext(QSSGRenderer &renderer, QSGRenderContext *sgRenderCtx)
{
    renderer.m_qsgRenderContext = sgRenderCtx;
}

QSGRenderContext *QSSGRendererPrivate::getSgRenderContext(const QSSGRenderer &renderer)
{
    return renderer.m_qsgRenderContext.data();
}

const std::unique_ptr<QSSGRhiQuadRenderer> &QSSGRenderer::rhiQuadRenderer() const
{
    if (!m_rhiQuadRenderer)
        m_rhiQuadRenderer = std::make_unique<QSSGRhiQuadRenderer>();

    return m_rhiQuadRenderer;
}

const std::unique_ptr<QSSGRhiCubeRenderer> &QSSGRenderer::rhiCubeRenderer() const
{
    if (!m_rhiCubeRenderer)
        m_rhiCubeRenderer = std::make_unique<QSSGRhiCubeRenderer>();

    return m_rhiCubeRenderer;

}

void QSSGRenderer::beginSubLayerRender(QSSGLayerRenderData &inLayer)
{
    inLayer.saveRenderState(*this);
    m_currentLayer = nullptr;
}

void QSSGRenderer::endSubLayerRender(QSSGLayerRenderData &inLayer)
{
    inLayer.restoreRenderState(*this);
    m_currentLayer = &inLayer;
}

void QSSGRenderer::beginLayerRender(QSSGLayerRenderData &inLayer)
{
    m_currentLayer = &inLayer;
}
void QSSGRenderer::endLayerRender()
{
    m_currentLayer = nullptr;
}

static void dfs(const QSSGRenderNode &node, RenderableList &renderables)
{
    if (QSSGRenderGraphObject::isRenderable(node.type))
        renderables.push_back(&node);

    for (const auto &child : node.children)
        dfs(child, renderables);
}

void QSSGRendererPrivate::getLayerHitObjectList(const QSSGRenderLayer &layer,
                                                QSSGBufferManager &bufferManager,
                                                const QSSGRenderRay &ray,
                                                bool inPickEverything,
                                                PickResultList &outIntersectionResult)
{
    RenderableList renderables;
    for (const auto &childNode : layer.children)
        dfs(childNode, renderables);

    for (int idx = renderables.size() - 1; idx >= 0; --idx) {
        const auto &pickableObject = renderables.at(idx);
        if (inPickEverything || pickableObject->getLocalState(QSSGRenderNode::LocalState::Pickable))
            intersectRayWithSubsetRenderable(layer, bufferManager, ray, *pickableObject, outIntersectionResult);
    }
}

namespace  {

static inline QVector3D multiply(const QMatrix3x3& M, const QVector3D& v)
{
    return QVector3D(
            M(0,0) * v.x() + M(0,1) * v.y() + M(0,2) * v.z(),
            M(1,0) * v.x() + M(1,1) * v.y() + M(1,2) * v.z(),
            M(2,0) * v.x() + M(2,1) * v.y() + M(2,2) * v.z()
            );
}

// Return true if G ≈ s^2 I; outputs s2 (>=0). tolerance is relative-ish.
static inline bool isUniformScaleMetric(const QMatrix3x3& G, float& s2, float tolerance = 1e-5f) {
    const float gxx = G(0,0), gyy = G(1,1), gzz = G(2,2);
    const float gxy = G(0,1), gxz = G(0,2), gyz = G(1,2);

    // Average of diagonals as robust estimate of s^2
    s2 = (gxx + gyy + gzz) / 3.0f;

    // Scale for relative tolerance (avoid divide by zero)
    const float scale = std::max({ std::fabs(gxx), std::fabs(gyy), std::fabs(gzz), 1.0f });

    // Off-diagonals should be ~0; diagonals should be ~equal to s2
    const bool offDiagOK = (std::fabs(gxy) <= tolerance * scale) &&
            (std::fabs(gxz) <= tolerance * scale) &&
            (std::fabs(gyz) <= tolerance * scale) &&
            (std::fabs(G(1,0)) <= tolerance * scale) && // in case it's not exactly symmetric
            (std::fabs(G(2,0)) <= tolerance * scale) &&
            (std::fabs(G(2,1)) <= tolerance * scale);

    const bool diagOK = (std::fabs(gxx - s2) <= tolerance * scale) &&
            (std::fabs(gyy - s2) <= tolerance * scale) &&
            (std::fabs(gzz - s2) <= tolerance * scale);

    return offDiagOK && diagOK && (s2 >= 0.0f);
}

struct EuclideanDot
{
    inline float operator()(const QVector3D& u, const QVector3D& v) const {
        return QVector3D::dotProduct(u, v);
    }
};

struct MetricDot
{
    QMatrix3x3 G;
    inline float operator()(const QVector3D& u, const QVector3D& v) const {
        // u^T (G v)
        return QVector3D::dotProduct(u, multiply(G, v));
    }
};

// Closest point on triangle ABC to point p, using metric defined by template class
// This code is based on: https://github.com/RenderKit/embree/blob/master/tutorials/common/math/closest_point.h
// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

template<class Dot>
static QVector3D closestPointOnTriangle(const QVector3D &p,
                                        const QVector3D &a,
                                        const QVector3D &b,
                                        const QVector3D &c,
                                        const Dot &dot,
                                        float &u, float &v, float &w)
{
    const QVector3D ab = b - a;
    const QVector3D ac = c - a;
    const QVector3D ap = p - a;

    // Vertex region A
    const float d1 = dot(ab, ap);
    const float d2 = dot(ac, ap);
    if (d1 <= 0.f && d2 <= 0.f) {
        u = 1.0f; v = 0.0f; w = 0.0f;
        return a;
    }

    // Vertex region B
    const QVector3D bp = p - b;
    const float d3 = dot(ab, bp);
    const float d4 = dot(ac, bp);
    if (d3 >= 0.f && d4 <= d3) {
        u = 0.0f; v = 1.0f; w = 0.0f;
        return b;
    }

    // Edge AB
    const float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f) {
        const float v_edge = d1 / (d1 - d3);
        u = 1.0f - v_edge; v = v_edge; w = 0.0f;
        return a + v_edge * ab;
    }

    // Vertex region C
    const QVector3D cp = p - c;
    const float d5 = dot(ab, cp);
    const float d6 = dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6) {
        u = 0.0f; v = 0.0f; w = 1.0f;
        return c;
    }

    // Edge AC
    const float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f) {
        const float w_edge = d2 / (d2 - d6);
        u = 1.0f - w_edge; v = 0.0f; w = w_edge;
        return a + w_edge * ac;
    }

    // Edge BC
    const float va = d3 * d6 - d5 * d4;
    if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f) {
        const QVector3D bc = c - b;
        const float w_edge = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        u = 0.0f; v = 1.0f - w_edge; w = w_edge;
        return b + w_edge * bc;
    }

    // Inside face region
    const float denom = va + vb + vc;

    // Check for degenerate case
    if (std::abs(denom) < 1e-20f) {
        // Degenerate triangle in metric space: fall back to closest among vertices
        const float da = dot(ap, ap);
        const float db = dot(bp, bp);
        const float dc = dot(cp, cp);
        if (da <= db && da <= dc) {
            u = 1.0f; v = 0.0f; w = 0.0f;
            return a;
        }
        if (db <= dc) {
            u = 0.0f; v = 1.0f; w = 0.0f;
            return b;
        }
        u = 0.0f; v = 0.0f; w = 1.0f;
        return c;
    }

    const float invDenom = 1.0f / denom;
    u = va * invDenom;
    v = vb * invDenom;
    w = vc * invDenom;
    return a + v * ab + w * ac;
}

struct SphereData
{
    QMatrix4x4 globalTransform;  // model -> world
    QMatrix3x3 pullbackMetric;
    QVector3D  centerLocal;      // sphere center in model local space
};

// Create local-space query data from world-space sphere and model transform.
// The local radius uses max column length of the inverse linear part as a cheap,
// conservative bound under non-uniform scaling/shear.
static inline SphereData createSphereData(const QMatrix4x4 &globalTransform,
                                          const QVector3D &centerWorld)
{
    QMatrix4x4 inv = globalTransform.inverted();

    // center in local space
    const QVector3D centerLocal = QSSGUtils::mat44::transform(inv, centerWorld);

    const QMatrix3x3 A = QSSGUtils::mat44::getUpper3x3(globalTransform);
    const QMatrix3x3 G = A.transposed() * A;

    return SphereData{ globalTransform, G, centerLocal };
}

// Squared distance from point to axis-aligned bounding box.
static inline float distanceSqPointTransformedAABB(const QVector3D &localPoint,
                                                   const QSSGBounds3 &localAABB,
                                                   const QMatrix3x3 &G)
{
    // Find the closest point on the AABB in local space
    QVector3D closestLocal(
            qBound(localAABB.minimum.x(), localPoint.x(), localAABB.maximum.x()),
            qBound(localAABB.minimum.y(), localPoint.y(), localAABB.maximum.y()),
            qBound(localAABB.minimum.z(), localPoint.z(), localAABB.maximum.z())
            );

    // Compute the difference vector in local space
    QVector3D localDiff = localPoint - closestLocal;

    // Use the pullback metric to get the squared world-space distance
    // ||v||_world^2 = v^T * G * v where G = A^T * A
    MetricDot dot{G};
    return dot(localDiff, localDiff);
}

struct ClosestPointResult
{
    bool       found = false;
    float      distSq = std::numeric_limits<float>::max();
    QVector3D  localPoint;
    QVector3D  scenePoint;
    QVector3D  faceNormal;
    QVector3D  sceneNormal;
    QVector2D  uv;
    int subset = -1;
    int instanceIndex = -1;
};

static void closestPointBVHLeafNode(const SphereData &data,
                                    const QSSGMeshBVHNode *node,
                                    const QSSGRenderMesh *mesh,
                                    int subset,
                                    int instanceIndex,
                                    ClosestPointResult &best)
{
    const int begin = node->offset;
    const int end = begin + node->count;
    const auto &triangles = mesh->bvh->triangles();

    // Determine if we can use faster Euclidean distance computation
    float uniformScaleFactor = 1.0f;
    const bool isUniformScale = isUniformScaleMetric(data.pullbackMetric, uniformScaleFactor);
    const MetricDot metricDot { data.pullbackMetric };

    for (int i = begin; i < end; ++i) {
        const auto &triangle = triangles[i];

        // Micro-pruning: skip triangles whose bounds are already farther than current best
        const float triangleDistSq = distanceSqPointTransformedAABB(data.centerLocal, triangle.bounds, data.pullbackMetric);
        if (triangleDistSq >= best.distSq)
            continue;

        // Find closest point on triangle
        float u, v, w;
        QVector3D closestPoint;

        if (isUniformScale) {
            closestPoint = closestPointOnTriangle(data.centerLocal,
                                                  triangle.vertex1, triangle.vertex2, triangle.vertex3,
                                                  EuclideanDot{},
                                                  u, v, w);
        } else {
            closestPoint = closestPointOnTriangle(data.centerLocal,
                                                  triangle.vertex1, triangle.vertex2, triangle.vertex3,
                                                  metricDot,
                                                  u, v, w);
        }

        // Compute squared distance in metric space
        const QVector3D delta = data.centerLocal - closestPoint;
        const float distSq = metricDot(delta, delta);

        // Update best result if this is closer
        if (distSq < best.distSq) {
            best.distSq = distSq;
            best.localPoint = closestPoint;
            best.scenePoint = QSSGUtils::mat44::transform(data.globalTransform, closestPoint);
            best.subset = subset;
            best.instanceIndex = instanceIndex;
            best.found = true;

            // Interpolate UV coordinates using barycentric coordinates.
            // Note that we're using a different definition of u, v, w than intersectWithBVHTriangles
            best.uv = u * triangle.uvCoord1 + v * triangle.uvCoord2 + w * triangle.uvCoord3;

            // Compute face normal in local space
            const QVector3D edge1 = triangle.vertex2 - triangle.vertex1;
            const QVector3D edge2 = triangle.vertex3 - triangle.vertex1;
            best.faceNormal = QVector3D::normal(edge1, edge2).normalized();
            const QMatrix3x3 normalMatrix = data.globalTransform.normalMatrix();
            best.sceneNormal = QSSGUtils::mat33::transform(normalMatrix, best.faceNormal);
        }
    }
}

static void closestPointBVH(const SphereData &data,
                            const QSSGMeshBVHNode *node,
                            const QSSGRenderMesh *mesh,
                            int subset,
                            int instanceIndex,
                            ClosestPointResult &best)
{
    if (!node || !mesh || !mesh->bvh)
        return;

    // Prune by AABB distance vs. current best
    const float aabbDistSq = distanceSqPointTransformedAABB(data.centerLocal, node->boundingData, data.pullbackMetric);
    if (aabbDistSq >= best.distSq)
        return;

    // Leaf node: compute closest point on each triangle
    if (node->count != 0) {
        closestPointBVHLeafNode(data, node, mesh, subset, instanceIndex, best);
        return;
    }

    // Internal node: visit children in order of increasing AABB distance
    const auto *leftChild = static_cast<const QSSGMeshBVHNode *>(node->left);
    const auto *rightChild = static_cast<const QSSGMeshBVHNode *>(node->right);

    // Compute AABB distances for both children
    const float leftDistSq = leftChild
            ? distanceSqPointTransformedAABB(data.centerLocal, leftChild->boundingData, data.pullbackMetric)
            : std::numeric_limits<float>::max();
    const float rightDistSq = rightChild
            ? distanceSqPointTransformedAABB(data.centerLocal, rightChild->boundingData, data.pullbackMetric)
            : std::numeric_limits<float>::max();

    // Visit children in order of increasing distance (closer child first for better pruning)
    if (leftDistSq < rightDistSq) {
        if (leftDistSq < best.distSq)
            closestPointBVH(data, leftChild, mesh, subset, instanceIndex, best);
        if (rightDistSq < best.distSq)
            closestPointBVH(data, rightChild, mesh, subset, instanceIndex, best);
    } else {
        if (rightDistSq < best.distSq)
            closestPointBVH(data, rightChild, mesh, subset, instanceIndex, best);
        if (leftDistSq < best.distSq)
            closestPointBVH(data, leftChild, mesh, subset, instanceIndex, best);
    }
}
} // namespace (anonymous)

std::optional<QSSGRenderPickResult>
QSSGRendererPrivate::closestPointOnSubsetRenderable(const QSSGRenderLayer& layer,
                                                    QSSGBufferManager& bufferManager,
                                                    const QVector3D& center,
                                                    const float radiusSquared,
                                                    const QSSGRenderNode& node)
{
    if (!layer.renderData)
        return std::nullopt;

    const auto *renderData = layer.renderData;

    // Note: If we want to extend this to also handling Item2D, this is where we would do it.
    // if (node.type == QSSGRenderGraphObject::Type::Item2D) {
    //     ...
    // }

    if (node.type != QSSGRenderGraphObject::Type::Model)
        return std::nullopt;

    const auto &model = static_cast<const QSSGRenderModel &>(node);

    // We have to have a guard here, as the meshes are usually loaded on the render thread,
    // and we assume all meshes are loaded before picking and none are removed, which
    // is usually true, except for custom geometry which can be updated at any time. So this
    // guard should really only be locked whenever a custom geometry buffer is being updated
    // on the render thread.  Still naughty though because this can block the render thread.

    QMutexLocker mutexLocker(bufferManager.meshUpdateMutex());

    auto mesh = bufferManager.getMeshForPicking(model, model.hasLightmap() ? layer.lightmapSource : QString());
    if (!mesh)
        return std::nullopt;

    // Early culling: check if sphere can reach model bounds
    QSSGBounds3 modelBounds;
    for (const auto &subset : mesh->subsets)
        modelBounds.include(subset.bounds);

    if (modelBounds.isEmpty())
        return std::nullopt;

    const bool instancing = model.instancing();
    int instanceCount = instancing ? model.instanceTable->count() : 1;
    const auto instanceTransforms = instancing ? renderData->getInstanceTransforms(model) : QSSGLayerRenderData::InstanceTransforms{};

    ClosestPointResult best;
    best.distSq = radiusSquared; // Start with sphere radius as max distance

    for (int i = 0; i < instanceCount; ++i) {
        int instanceIndex = 0;
        QMatrix4x4 modelTransform;
        if (instancing) {
            instanceIndex = i;
            modelTransform = instanceTransforms.global * model.instanceTable->getTransform(instanceIndex) * instanceTransforms.local;
        } else {
            modelTransform = renderData->getGlobalTransform(model);
        }
        const SphereData data = createSphereData(modelTransform, center);

        if (distanceSqPointTransformedAABB(data.centerLocal, modelBounds, data.pullbackMetric) > best.distSq)
            continue;

        for (int subsetIndex = 0; subsetIndex < mesh->subsets.size(); ++subsetIndex) {
            const auto &subset = mesh->subsets[subsetIndex];

            // Cull subset if its bounds are beyond our current best distance
            if (distanceSqPointTransformedAABB(data.centerLocal, subset.bounds, data.pullbackMetric) >= best.distSq)
                continue;

            if (!subset.bvhRoot.isNull()) {
                const auto *bvhRoot = static_cast<const QSSGMeshBVHNode *>(subset.bvhRoot);
                closestPointBVH(data, bvhRoot, mesh, subsetIndex, instanceIndex, best);
            }
        }
    }
    if (best.found) {
        return QSSGRenderPickResult{
            &model,
            best.distSq,
            best.uv,
            best.scenePoint,
            best.localPoint,
            best.faceNormal,
            best.sceneNormal,
            best.subset,
            best.instanceIndex
        };
    }

    return std::nullopt;
}

void QSSGRendererPrivate::intersectRayWithSubsetRenderable(const QSSGRenderLayer &layer,
                                                           QSSGBufferManager &bufferManager,
                                                           const QSSGRenderRay &inRay,
                                                           const QSSGRenderNode &node,
                                                           PickResultList &outIntersectionResultList)
{
    if (!layer.renderData)
        return;

    const auto *renderData = layer.renderData;

    // Item2D's requires special handling
    if (node.type == QSSGRenderGraphObject::Type::Item2D) {
        const QSSGRenderItem2D &item2D = static_cast<const QSSGRenderItem2D &>(node);
        intersectRayWithItem2D(layer, inRay, item2D, outIntersectionResultList);
        return;
    }

    if (node.type != QSSGRenderGraphObject::Type::Model)
        return;

    const QSSGRenderModel &model = static_cast<const QSSGRenderModel &>(node);

    // We have to have a guard here, as the meshes are usually loaded on the render thread,
    // and we assume all meshes are loaded before picking and none are removed, which
    // is usually true, except for custom geometry which can be updated at any time. So this
    // guard should really only be locked whenever a custom geometry buffer is being updated
    // on the render thread.  Still naughty though because this can block the render thread.
    QMutexLocker mutexLocker(bufferManager.meshUpdateMutex());
    auto mesh = bufferManager.getMeshForPicking(model, model.hasLightmap() ? layer.lightmapSource : QString());
    if (!mesh)
        return;

    const auto &subMeshes = mesh->subsets;
    QSSGBounds3 modelBounds;
    for (const auto &subMesh : subMeshes)
        modelBounds.include(subMesh.bounds);

    if (modelBounds.isEmpty())
        return;

    const bool instancing = model.instancing(); // && instancePickingEnabled
    int instanceCount = instancing ? model.instanceTable->count() : 1;

    const auto instanceTransforms = instancing ? renderData->getInstanceTransforms(model) : QSSGLayerRenderData::InstanceTransforms{};

    for (int instanceIndex = 0; instanceIndex < instanceCount; ++instanceIndex) {

        QMatrix4x4 modelTransform;
        if (instancing) {
            modelTransform = instanceTransforms.global * model.instanceTable->getTransform(instanceIndex) * instanceTransforms.local;
        } else {
            modelTransform = renderData->getGlobalTransform(model);
        }
        auto rayData = QSSGRenderRay::createRayData(modelTransform, inRay);

        auto hit = QSSGRenderRay::intersectWithAABBv2(rayData, modelBounds);

        // If we don't intersect with the model at all, then there's no need to go furher down!
        if (!hit.intersects())
            continue;

        // Check each submesh to find the closest intersection point
        float minRayLength = std::numeric_limits<float>::max();
        QSSGRenderRay::IntersectionResult intersectionResult;
        QVector<QSSGRenderRay::IntersectionResult> results;

        int subset = 0;
        int resultSubset = 0;
        for (const auto &subMesh : subMeshes) {
            QSSGRenderRay::IntersectionResult result;
            if (!subMesh.bvhRoot.isNull()) {
                hit = QSSGRenderRay::intersectWithAABBv2(rayData, subMesh.bvhRoot->boundingData);
                if (hit.intersects()) {
                    results.clear();
                    inRay.intersectWithBVH(rayData, static_cast<const QSSGMeshBVHNode *>(subMesh.bvhRoot), mesh, results);
                    float subMeshMinRayLength = std::numeric_limits<float>::max();
                    for (const auto &subMeshResult : std::as_const(results)) {
                        if (subMeshResult.rayLengthSquared < subMeshMinRayLength) {
                            result = subMeshResult;
                            subMeshMinRayLength = result.rayLengthSquared;
                        }
                    }
                }
            } else {
                hit = QSSGRenderRay::intersectWithAABBv2(rayData, subMesh.bounds);
                if (hit.intersects())
                    result = QSSGRenderRay::createIntersectionResult(rayData, hit);
            }
            if (result.intersects && result.rayLengthSquared < minRayLength) {
                intersectionResult = result;
                minRayLength = intersectionResult.rayLengthSquared;
                resultSubset = subset;
            }
            subset++;
        }

        if (intersectionResult.intersects)
            outIntersectionResultList.push_back(QSSGRenderPickResult { &model,
                                                                       intersectionResult.rayLengthSquared,
                                                                       intersectionResult.relXY,
                                                                       intersectionResult.scenePosition,
                                                                       intersectionResult.localPosition,
                                                                       intersectionResult.faceNormal,
                                                                       intersectionResult.sceneFaceNormal,
                                                                       resultSubset,
                                                                       instanceIndex
                                                });
    }
}

void QSSGRendererPrivate::intersectRayWithItem2D(const QSSGRenderLayer &layer,
                                                 const QSSGRenderRay &inRay,
                                                 const QSSGRenderItem2D &item2D,
                                                 PickResultList &outIntersectionResultList)
{
    const auto &globalTransform = layer.renderData->getGlobalTransform(item2D);

    // Get the plane (and normal) that the item 2D is on
    const QVector3D p0 = QSSGRenderNode::getGlobalPos(globalTransform);
    const QVector3D normal  = -QSSGRenderNode::getDirection(globalTransform);

    const float d = QVector3D::dotProduct(inRay.direction, normal);
    float intersectionTime = 0;
    if (d > 1e-6f) {
        const QVector3D p0l0 = p0 - inRay.origin;
        intersectionTime = QVector3D::dotProduct(p0l0, normal) / d;
        if (intersectionTime >= 0) {
            // Intersection
            const QVector3D intersectionPoint = inRay.origin + inRay.direction * intersectionTime;
            const QMatrix4x4 inverseGlobalTransform = globalTransform.inverted();
            const QVector3D localIntersectionPoint = QSSGUtils::mat44::transform(inverseGlobalTransform, intersectionPoint);
            const QVector2D qmlCoordinate(localIntersectionPoint.x(), -localIntersectionPoint.y());
            outIntersectionResultList.push_back(QSSGRenderPickResult { &item2D,
                                                                       intersectionTime * intersectionTime,
                                                                       qmlCoordinate,
                                                                       intersectionPoint,
                                                                       localIntersectionPoint,
                                                                       -normal, -normal });
        }
    }
}

QSSGRhiShaderPipelinePtr QSSGRendererPrivate::getShaderPipelineForDefaultMaterial(QSSGRenderer &renderer,
                                                                                  QSSGSubsetRenderable &inRenderable,
                                                                                  const QSSGShaderFeatures &inFeatureSet,
                                                                                  const QSSGUserShaderAugmentation &shaderAugmentation)
{
    auto *m_currentLayer = renderer.m_currentLayer;
    QSSG_ASSERT(m_currentLayer != nullptr, return {});

    // This function is the main entry point for retrieving the shaders for a
    // default material, and is called for every material for every model in
    // every frame. Therefore, like with custom materials, employ a first level
    // cache (a simple hash table), with a key that's quick to
    // generate/hash/compare. Even though there are other levels of caching in
    // the components that get invoked from here, those may not be suitable
    // performance wise. So bail out right here as soon as possible.
    auto &shaderMap = m_currentLayer->shaderMap;

    QElapsedTimer timer;
    timer.start();

    QSSGRhiShaderPipelinePtr shaderPipeline;

    // This just references inFeatureSet and inRenderable.shaderDescription -
    // cheap to construct and is good enough for the find()
    // FIXME: Would be good to have some better approach here for the key.
    QByteArray name = shaderAugmentation.preamble + shaderAugmentation.body;
    for (const auto &def : shaderAugmentation.defines)
        name.append(def.name).append(def.value);
    QSSGShaderMapKey skey = QSSGShaderMapKey(name,
                                             inFeatureSet,
                                             inRenderable.shaderDescription);
    auto it = shaderMap.find(skey);
    if (it == shaderMap.end()) {
        Q_TRACE_SCOPE(QSSG_generateShader);
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DGenerateShader);
        shaderPipeline = QSSGRendererPrivate::generateRhiShaderPipeline(renderer, inRenderable, inFeatureSet, shaderAugmentation);
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DGenerateShader, 0, inRenderable.material.profilingId);
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

QList<const QSSGRenderNode *> QSSGRendererPrivate::syncPickInFrustum(const QSSGRenderContextInterface &ctx,
                                                                     const QSSGRenderLayer &layer,
                                                                     const QSSGFrustum &frustum)
{
    if (!layer.renderData)
        return {};

    auto &bufferManager = ctx.bufferManager();
    const bool pickEverything = QSSGRendererPrivate::isGlobalPickingEnabled(*ctx.renderer());

    RenderableList nodes;
    for (const auto &child : layer.children)
        getPickableRecursive(child, nodes, pickEverything);

    QList<const QSSGRenderNode *> ret;
    for (const auto node : nodes) {
        auto aabb = node->getBounds(*bufferManager);
        if (!aabb.isEmpty()) {
            const auto &transform = layer.renderData->getGlobalTransform(*node);
            aabb.transform(transform);
            if (frustum.contains(aabb))
                ret.append(node);
        }
    }

    return ret;
}

QT_END_NAMESPACE
