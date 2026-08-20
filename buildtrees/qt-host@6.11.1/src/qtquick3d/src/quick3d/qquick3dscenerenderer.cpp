// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dcubemaptexture_p.h"
#include "qquick3dscenerenderer_p.h"
#include "qquick3dsceneenvironment_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dnode_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dtexture_p.h"
#include "qquick3dcamera_p.h"
#include "qquick3dpickresult_p.h"
#include "qquick3dmodel_p.h"
#include "qquick3drenderstats_p.h"
#include "qquick3ddebugsettings_p.h"
#include "extensions/qquick3drenderextensions.h"
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>

#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/private/qsgdefaultrendercontext_p.h>
#include <QtQuick/private/qsgtexture_p.h>
#include <QtQuick/private/qsgplaintexture_p.h>
#include <QtQuick/private/qsgrendernode_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhieffectsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhiquadrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgcputonemapper_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderroot_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderuserpass_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>


#include <qtquick3d_tracepoints_p.h>

#include <QtCore/QObject>
#include <QtCore/qqueue.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtquick3d,
    "QT_BEGIN_NAMESPACE" \
    "class QQuick3DViewport;" \
    "QT_END_NAMESPACE"
)

Q_TRACE_POINT(qtquick3d, QSSG_prepareFrame_entry, int width, int height);
Q_TRACE_POINT(qtquick3d, QSSG_prepareFrame_exit);
Q_TRACE_POINT(qtquick3d, QSSG_renderFrame_entry, int width, int height);
Q_TRACE_POINT(qtquick3d, QSSG_renderFrame_exit);
Q_TRACE_POINT(qtquick3d, QSSG_synchronize_entry, QQuick3DViewport *view3D, const QSize &size, float dpr);
Q_TRACE_POINT(qtquick3d, QSSG_synchronize_exit);
Q_TRACE_POINT(qtquick3d, QSSG_renderPass_entry, const QString &renderPass);
Q_TRACE_POINT(qtquick3d, QSSG_renderPass_exit);

static bool dumpRenderTimes()
{
    static bool val = (qEnvironmentVariableIntValue("QT_QUICK3D_DUMP_RENDERTIMES") > 0);
    return val;
}

#if QT_CONFIG(qml_debug)

static inline quint64 statDrawCallCount(const QSSGRhiContextStats &stats)
{
    quint64 count = 0;
    const QSSGRhiContextStats::PerLayerInfo &info(stats.perLayerInfo[stats.layerKey]);
    for (const auto &pass : info.renderPasses)
        count += QSSGRhiContextStats::totalDrawCallCountForPass(pass);
    count += QSSGRhiContextStats::totalDrawCallCountForPass(info.externalRenderPass);
    return count;
}

#define STAT_PAYLOAD(stats) \
    (statDrawCallCount(stats) | (quint64(stats.perLayerInfo[stats.layerKey].renderPasses.size()) << 32))

#endif

template <typename In, typename Out>
static void bfs(In *inExtension, QList<Out *> &outList)
{
    QSSG_ASSERT(inExtension, return);

    QQueue<In *> queue { { inExtension } };
    while (queue.size() > 0) {
        if (auto cur = queue.dequeue()) {
            if (auto *ext = static_cast<Out *>(QQuick3DObjectPrivate::get(cur)->spatialNode))
                outList.push_back(ext);
            for (auto &chld : cur->childItems())
                queue.enqueue(qobject_cast<In *>(chld));
        }
    }
}

SGFramebufferObjectNode::SGFramebufferObjectNode()
    : window(nullptr)
    , renderer(nullptr)
    , renderPending(true)
    , invalidatePending(false)
    , devicePixelRatio(1)
{
    qsgnode_set_description(this, QStringLiteral("fbonode"));
    setFlag(QSGNode::UsePreprocess, true);
}

SGFramebufferObjectNode::~SGFramebufferObjectNode()
{
    delete renderer;
    delete texture();
}

void SGFramebufferObjectNode::scheduleRender()
{
    renderPending = true;
    markDirty(DirtyMaterial);
}

QSGTexture *SGFramebufferObjectNode::texture() const
{
    return QSGSimpleTextureNode::texture();
}

void SGFramebufferObjectNode::preprocess()
{
    render();
}

// QQuickWindow::update() behaves differently depending on whether it's called from the GUI thread
// or the render thread.
// TODO: move this to QQuickWindow::fullUpdate(), if we can't change update()
static void requestFullUpdate(QQuickWindow *window)
{
    if (QThread::currentThread() == QCoreApplication::instance()->thread())
        window->update();
    else
        QCoreApplication::postEvent(window, new QEvent(QEvent::Type(QQuickWindowPrivate::FullUpdateRequest)));
}

void SGFramebufferObjectNode::render()
{
    if (renderPending) {
        if (renderer->renderStats())
            renderer->renderStats()->startRender();

        renderPending = false;

        if (renderer->m_sgContext->rhiContext()->isValid()) {
            QRhiTexture *rhiTexture = renderer->renderToRhiTexture(window);
            bool needsNewWrapper = false;
            if (!texture() || (texture()->textureSize() != renderer->surfaceSize()
                               || texture()->rhiTexture() != rhiTexture))
            {
                needsNewWrapper = true;
            }
            if (needsNewWrapper) {
                delete texture();
                QSGPlainTexture *t = new QSGPlainTexture;
                t->setOwnsTexture(false);
                t->setHasAlphaChannel(true);
                t->setTexture(rhiTexture);
                t->setTextureSize(renderer->surfaceSize());
                setTexture(t);
            }
        }

        markDirty(QSGNode::DirtyMaterial);
        emit textureChanged();

        if (renderer->renderStats())
            renderer->renderStats()->endRender(dumpRenderTimes());

        if (renderer->m_requestedFramesCount > 0) {
            scheduleRender();
            requestFullUpdate(window);
            renderer->m_requestedFramesCount--;
        }
    }
}

void SGFramebufferObjectNode::handleScreenChange()
{
    if (!qFuzzyCompare(window->effectiveDevicePixelRatio(), devicePixelRatio)) {
        renderer->invalidateFramebufferObject();
        quickFbo->update();
    }
}


QQuick3DSceneRenderer::QQuick3DSceneRenderer(const std::shared_ptr<QSSGRenderContextInterface> &rci)
    : m_sgContext(rci)
{
}

QQuick3DSceneRenderer::~QQuick3DSceneRenderer()
{
    const auto &rhiCtx = m_sgContext->rhiContext();
    auto *rhi = rhiCtx->rhi();
    rhi->finish(); // finish active readbacks
    QSSGRhiContextStats::get(*rhiCtx).cleanupLayerInfo(m_layer);
    m_sgContext->bufferManager()->releaseResourcesForLayer(m_layer);

    if (m_layer) {
        // The scene root is created by the scene manager and realeased by the
        // the normal cleanup of scene nodes, since we're deleting the layer
        // at a later point, we need to remove the scene root node from the layer now.

        // If the scene is destroyed outside of the normal destroy sequence(like fom a timer),
        // the scene root node might already have been deallocated and pointing
        // to an invalid memory in which case the winAttacment is also not valid anymore.
        if (m_sceneRootNode && winAttacment)
            removeNodeFromLayer(m_sceneRootNode);

        // There might be nodes queued for cleanup that still reference the layer,
        // so we schedule the layer for cleanup so that it is deleted after the nodes
        // have been cleaned up.
        if (winAttacment)
            winAttacment->queueForCleanup(m_layer);
        else
            delete m_layer;
        m_layer = nullptr;
    }

    delete m_texture;

    releaseAaDependentRhiResources();
    delete m_effectSystem;
}

void QQuick3DSceneRenderer::releaseAaDependentRhiResources()
{
    const auto &rhiCtx = m_sgContext->rhiContext();
    if (!rhiCtx->isValid())
        return;

    delete m_textureRenderTarget;
    m_textureRenderTarget = nullptr;

    delete m_textureRenderPassDescriptor;
    m_textureRenderPassDescriptor = nullptr;

    delete m_depthStencilBuffer;
    m_depthStencilBuffer = nullptr;

    delete m_multiViewDepthStencilBuffer;
    m_multiViewDepthStencilBuffer = nullptr;

    delete m_msaaRenderBufferLegacy;
    m_msaaRenderBufferLegacy = nullptr;

    delete m_msaaRenderTexture;
    m_msaaRenderTexture = nullptr;

    delete m_msaaMultiViewRenderBuffer;
    m_msaaMultiViewRenderBuffer = nullptr;

    delete m_ssaaTexture;
    m_ssaaTexture = nullptr;

    delete m_ssaaTextureToTextureRenderTarget;
    m_ssaaTextureToTextureRenderTarget = nullptr;

    delete m_ssaaTextureToTextureRenderPassDescriptor;
    m_ssaaTextureToTextureRenderPassDescriptor = nullptr;

    delete m_temporalAATexture;
    m_temporalAATexture = nullptr;
    delete m_temporalAARenderTarget;
    m_temporalAARenderTarget = nullptr;
    delete m_temporalAARenderPassDescriptor;
    m_temporalAARenderPassDescriptor = nullptr;

    delete m_prevTempAATexture;
    m_prevTempAATexture = nullptr;
}

// Blend factors are in the form of (frame blend factor, accumulator blend factor)
static const QVector2D s_ProgressiveAABlendFactors[QSSGLayerRenderData::MAX_AA_LEVELS] = {
    QVector2D(0.500000f, 0.500000f), // 1x
    QVector2D(0.333333f, 0.666667f), // 2x
    QVector2D(0.250000f, 0.750000f), // 3x
    QVector2D(0.200000f, 0.800000f), // 4x
    QVector2D(0.166667f, 0.833333f), // 5x
    QVector2D(0.142857f, 0.857143f), // 6x
    QVector2D(0.125000f, 0.875000f), // 7x
    QVector2D(0.111111f, 0.888889f), // 8x
};

static const QVector2D s_TemporalAABlendFactors = { 0.5f, 0.5f };

QRhiTexture *QQuick3DSceneRenderer::renderToRhiTexture(QQuickWindow *qw)
{
    if (!m_layer)
        return nullptr;

    QRhiTexture *currentTexture = m_texture; // the result so far

    if (qw) {
        if (m_renderStats)
            m_renderStats->startRenderPrepare();

        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DPrepareFrame);

        QSSGRhiContext *rhiCtx = m_sgContext->rhiContext().get();
        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);

        rhiCtxD->setMainRenderPassDescriptor(m_textureRenderPassDescriptor);
        rhiCtxD->setRenderTarget(m_textureRenderTarget);

        QRhiCommandBuffer *cb = nullptr;
        QRhiSwapChain *swapchain = qw->swapChain();
        if (swapchain) {
            cb = swapchain->currentFrameCommandBuffer();
            rhiCtxD->setCommandBuffer(cb);
        } else {
            QSGRendererInterface *rif = qw->rendererInterface();
            cb = static_cast<QRhiCommandBuffer *>(
                rif->getResource(qw, QSGRendererInterface::RhiRedirectCommandBuffer));
            if (cb)
                rhiCtxD->setCommandBuffer(cb);
            else {
                qWarning("Neither swapchain nor redirected command buffer are available.");
                return currentTexture;
            }
        }

        // Graphics pipeline objects depend on the MSAA sample count, so the
        // renderer needs to know the value.
        rhiCtxD->setMainPassSampleCount(m_msaaRenderBufferLegacy ? m_msaaRenderBufferLegacy->sampleCount() :
            (m_msaaRenderTexture ? m_msaaRenderTexture->sampleCount() :
                (m_msaaMultiViewRenderBuffer ? m_msaaMultiViewRenderBuffer->sampleCount() : 1)));

        // mainPassViewCount is left unchanged

        int ssaaAdjustedWidth = m_surfaceSize.width();
        int ssaaAdjustedHeight = m_surfaceSize.height();
        if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA) {
            ssaaAdjustedWidth *= m_layer->ssaaMultiplier;
            ssaaAdjustedHeight *= m_layer->ssaaMultiplier;
        }

        Q_TRACE(QSSG_prepareFrame_entry, ssaaAdjustedWidth, ssaaAdjustedHeight);

        float dpr = m_sgContext->renderer()->dpr();
        const QRect vp = QRect(0, 0, ssaaAdjustedWidth, ssaaAdjustedHeight);
        beginFrame();
        rhiPrepare(vp, dpr);

        if (m_renderStats)
            m_renderStats->endRenderPrepare();

        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DPrepareFrame, quint64(ssaaAdjustedWidth) | quint64(ssaaAdjustedHeight) << 32, profilingId);

        Q_TRACE(QSSG_prepareFrame_exit);

        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderFrame);
        Q_TRACE(QSSG_renderFrame_entry, ssaaAdjustedWidth, ssaaAdjustedHeight);
        if (m_renderStats)
            m_renderStats->startRender();

        QColor clearColor = Qt::transparent;
        if (m_backgroundMode == QSSGRenderLayer::Background::Color
                || (m_backgroundMode == QSSGRenderLayer::Background::SkyBoxCubeMap && !m_layer->skyBoxCubeMap)
                || (m_backgroundMode == QSSGRenderLayer::Background::SkyBox && !m_layer->lightProbe))
        {
            // Same logic as with the main render pass and skybox: tonemap
            // based on tonemapMode (unless it is None), unless there are effects.
            clearColor = m_layer->firstEffect ? m_linearBackgroundColor : m_tonemappedBackgroundColor;
        }

        // This is called from the node's preprocess() meaning Qt Quick has not
        // actually began recording a renderpass. Do our own.
        cb->beginPass(m_textureRenderTarget, clearColor, { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
        QSSGRHICTX_STAT(rhiCtx, beginRenderPass(m_textureRenderTarget));
        rhiRender();
        cb->endPass();
        QSSGRHICTX_STAT(rhiCtx, endRenderPass());
        Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, quint64(ssaaAdjustedWidth) | quint64(ssaaAdjustedHeight) << 32, QByteArrayLiteral("main"));

        const bool temporalAA = m_layer->temporalAAIsActive;
        const bool progressiveAA = m_layer->progressiveAAIsActive;
        const bool superSamplingAA = m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA;
        QRhi *rhi = rhiCtx->rhi();

        currentTexture = superSamplingAA ? m_ssaaTexture : m_texture;

        // Do effects before antialiasing
        if (m_effectSystem && m_layer->firstEffect && !m_layer->renderedCameras.isEmpty()) {
            const auto &renderer = m_sgContext->renderer();
            QSSGLayerRenderData *theRenderData = renderer->getOrCreateLayerRenderData(*m_layer);
            Q_ASSERT(theRenderData);
            QRhiTexture *theDepthTexture = theRenderData->getRenderResult(QSSGRenderResult::Key::DepthTexture)->texture;
            QRhiTexture *theNormalTexture = theRenderData->getRenderResult(QSSGRenderResult::Key::NormalTexture)->texture;
            QRhiTexture *theMotionVectorTexture = theRenderData->getRenderResult(QSSGRenderResult::Key::MotionVectorTexture)->texture;

            currentTexture = m_effectSystem->process(*m_layer,
                                                     currentTexture,
                                                     theDepthTexture,
                                                     theNormalTexture,
                                                     theMotionVectorTexture);
        }

        if ((progressiveAA || temporalAA) && m_prevTempAATexture) {
            cb->debugMarkBegin(QByteArrayLiteral("Temporal AA"));
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);
            Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("Temporal AA"));
            QRhiTexture *blendResult;
            uint *aaIndex = progressiveAA ? &m_layer->progAAPassIndex : &m_layer->tempAAPassIndex; // TODO: can we use only one index?

            if (*aaIndex > 0) {
                if ((temporalAA && m_layer->temporalAAMode != QSSGRenderLayer::TAAMode::MotionVector) ||
                    *aaIndex <
                            (temporalAA && m_layer->temporalAAMode == QSSGRenderLayer::TAAMode::MotionVector ?
                            quint32(QSSGLayerRenderData::MAX_AA_LEVELS) :
                            quint32(m_layer->antialiasingQuality))) {
                    const auto &renderer = m_sgContext->renderer();

                    QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
                    QSSGRhiDrawCallData &dcd(rhiCtxD->drawCallData({ m_layer, nullptr, nullptr, 0 }));
                    QRhiBuffer *&ubuf = dcd.ubuf;
                    const int ubufSize = 4 * sizeof(float);
                    if (!ubuf) {
                        ubuf = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, ubufSize);
                        ubuf->create();
                    }
                    int idx = *aaIndex - 1;

                    const QSize textureSize = m_prevTempAATexture->pixelSize();
                    QVector4D bufferData;
                    if (progressiveAA)
                        bufferData = QVector4D(s_ProgressiveAABlendFactors[idx]);
                    else if (m_layer->temporalAAMode == QSSGRenderLayer::TAAMode::Default)
                        bufferData = QVector4D(s_TemporalAABlendFactors);
                    else
                        bufferData = QVector4D(1.0f / qMax(textureSize.width(), 1), 1.0f / qMax(textureSize.height(), 1),
                                               0.9f + (qMin(qMax(m_layer->temporalAAStrength, 0.3f), 1.0f) - 0.3f) / 0.7f * 0.09f, 0.0f); //map it to 0.9 to 0.99

                    rub->updateDynamicBuffer(ubuf, 0, 4 * sizeof(float), &bufferData);
                    QSSGRhiGraphicsPipelineState ps;
                    ps.viewport = QRhiViewport(0, 0, float(textureSize.width()), float(textureSize.height()));

                    QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                             QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
                    QSSGRhiShaderResourceBindingList bindings;
                    bindings.addUniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, ubuf);
                    bindings.addTexture(1, QRhiShaderResourceBinding::FragmentStage, currentTexture, sampler);
                    bindings.addTexture(2, QRhiShaderResourceBinding::FragmentStage, m_prevTempAATexture, sampler);
                    if (m_layer->temporalAAMode == QSSGRenderLayer::TAAMode::MotionVector) {
                        QSSGLayerRenderData *theRenderData = renderer->getOrCreateLayerRenderData(*m_layer);
                        Q_ASSERT(theRenderData);
                        QRhiTexture *theDepthTexture = theRenderData->getRenderResult(QSSGRenderResult::Key::DepthTexture)->texture;
                        QRhiTexture *theMotionVectorTexture = theRenderData->getRenderResult(QSSGRenderResult::Key::MotionVectorTexture)->texture;
                        bindings.addTexture(3, QRhiShaderResourceBinding::FragmentStage, theDepthTexture, sampler);
                        bindings.addTexture(4, QRhiShaderResourceBinding::FragmentStage, theMotionVectorTexture, sampler);
                        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, m_sgContext->shaderCache()->getBuiltInRhiShaders().getRhiTemporalAAShader().get());
                    } else {
                        // The fragment shader relies on per-target compilation and
                        // QSHADER_ macros of qsb, hence no need to communicate a flip
                        // flag from here.
                        QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, m_sgContext->shaderCache()->getBuiltInRhiShaders().getRhiProgressiveAAShader().get());
                    }

                    QRhiShaderResourceBindings *srb = rhiCtxD->srb(bindings);
                    renderer->rhiQuadRenderer()->prepareQuad(rhiCtx, rub);
                    renderer->rhiQuadRenderer()->recordRenderQuadPass(rhiCtx, &ps, srb, m_temporalAARenderTarget, QSSGRhiQuadRenderer::UvCoords);
                    blendResult = m_temporalAATexture;
                } else {
                    blendResult = m_prevTempAATexture;
                }
            } else {
                // For the first frame: no blend, only copy
                blendResult = currentTexture;
            }

            QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

            const bool isMotionVector = m_layer->temporalAAMode == QSSGRenderLayer::TAAMode::MotionVector;
            const quint32 aaLimit = (temporalAA && isMotionVector) ? quint32(QSSGLayerRenderData::MAX_AA_LEVELS) : quint32(m_layer->antialiasingQuality);

            const bool aaActive = (temporalAA && !isMotionVector) || (*aaIndex < aaLimit);
            const bool usePrevTexture = m_prevTempAATexture != nullptr;

            if (aaActive && usePrevTexture) {
                QRhiTexture *copySource = (blendResult && (progressiveAA || isMotionVector)) ? blendResult : currentTexture;

                if (copySource) {
                    auto *rub = rhi->nextResourceUpdateBatch();
                    rub->copyTexture(m_prevTempAATexture, copySource);
                    cb->resourceUpdate(rub);
                }
            }

            (*aaIndex)++;
            cb->debugMarkEnd();
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("temporal_aa"));

            currentTexture = blendResult;
        }

        if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA) {
            // With supersampling antialiasing we at this point have the
            // content rendered at a larger size into m_ssaaTexture. Now scale
            // it down to the expected size into m_texture, using linear
            // filtering. Unlike in the OpenGL world, there is no
            // glBlitFramebuffer equivalent available, because APIs like D3D
            // and Metal have no such operation (the generally supported
            // texture copy operations are 1:1 copies, without support for
            // scaling, which is what we would need here). So draw a quad.

            QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
            const auto &renderer = m_sgContext->renderer();

            cb->debugMarkBegin(QByteArrayLiteral("SSAA downsample"));
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderPass);

            Q_TRACE_SCOPE(QSSG_renderPass, QStringLiteral("SSAA downsample"));

            renderer->rhiQuadRenderer()->prepareQuad(rhiCtx, nullptr);

            // Instead of passing in a flip flag we choose to rely on qsb's
            // per-target compilation mode in the fragment shader. (it does UV
            // flipping based on QSHADER_ macros) This is just better for
            // performance and the shaders are very simple so introducing a
            // uniform block and branching dynamically would be an overkill.
            const auto &shaderPipeline = m_sgContext->shaderCache()->getBuiltInRhiShaders().getRhiSupersampleResolveShader(m_layer->viewCount);

            QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
            QSSGRhiShaderResourceBindingList bindings;
            bindings.addTexture(0, QRhiShaderResourceBinding::FragmentStage, currentTexture, sampler);
            QRhiShaderResourceBindings *srb = rhiCtxD->srb(bindings);

            QSSGRhiGraphicsPipelineState ps;
            ps.viewport = QRhiViewport(0, 0, float(m_surfaceSize.width()), float(m_surfaceSize.height()));
            ps.viewCount = m_layer->viewCount;
            QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, shaderPipeline.get());

            renderer->rhiQuadRenderer()->recordRenderQuadPass(rhiCtx, &ps, srb, m_ssaaTextureToTextureRenderTarget, QSSGRhiQuadRenderer::UvCoords);
            cb->debugMarkEnd();
            Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderPass, 0, QByteArrayLiteral("ssaa_downsample"));

            currentTexture = m_texture;
        }

        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DRenderFrame,
                                      STAT_PAYLOAD(QSSGRhiContextStats::get(*rhiCtx)),
                                      profilingId);
        endFrame();
        if (m_renderStats)
            m_renderStats->endRender(dumpRenderTimes());
        Q_TRACE(QSSG_renderFrame_exit);
    }

    return currentTexture;
}

void QQuick3DSceneRenderer::beginFrame()
{
    m_sgContext->renderer()->beginFrame(*m_layer);
}

void QQuick3DSceneRenderer::endFrame()
{
    m_sgContext->renderer()->endFrame(*m_layer);
}

void QQuick3DSceneRenderer::rhiPrepare(const QRect &viewport, qreal displayPixelRatio)
{
    if (!m_layer)
        return;

    const auto &renderer = m_sgContext->renderer();

    renderer->setDpr(displayPixelRatio);

    renderer->setViewport(viewport);

    renderer->prepareLayerForRender(*m_layer);
    // If sync was called the assumption is that the scene is dirty regardless of what
    // the scene prep function says, we still should verify that we have a camera before
    // we call render prep and render.
    const bool renderReady = !m_layer->renderData->renderedCameras.isEmpty();
    if (renderReady) {
        renderer->rhiPrepare(*m_layer);
        m_prepared = true;
    }
}

void QQuick3DSceneRenderer::rhiRender()
{
    if (m_prepared) {
        // There is no clearFirst flag - the rendering here does not record a
        // beginPass() so it never clears on its own.

        m_sgContext->renderer()->rhiRender(*m_layer);
    }

    m_prepared = false;
}

#if QT_CONFIG(quick_shadereffect)
static QRhiTexture::Format toRhiTextureFormat(QQuickShaderEffectSource::Format format)
{
    switch (format) {
    case QQuickShaderEffectSource::RGBA8:
        return QRhiTexture::RGBA8;
    case QQuickShaderEffectSource::RGBA16F:
        return QRhiTexture::RGBA16F;
    case QQuickShaderEffectSource::RGBA32F:
        return QRhiTexture::RGBA32F;
    default:
        return QRhiTexture::RGBA8;
    }
}
#endif

static QVector3D tonemapRgb(const QVector3D &c, QQuick3DSceneEnvironment::QQuick3DEnvironmentTonemapModes tonemapMode)
{
    switch (tonemapMode) {
    case QQuick3DSceneEnvironment::TonemapModeLinear:
        return QSSGTonemapper::tonemapLinearToSrgb(c);
    case QQuick3DSceneEnvironment::TonemapModeHejlDawson:
        return QSSGTonemapper::tonemapHejlDawson(c);
    case QQuick3DSceneEnvironment::TonemapModeAces:
        return QSSGTonemapper::tonemapAces(c);
    case QQuick3DSceneEnvironment::TonemapModeFilmic:
        return QSSGTonemapper::tonemapFilmic(c);
    default:
        break;
    }
    return c;
}

void QQuick3DSceneRenderer::synchronize(QQuick3DViewport *view3D, const QSize &size, float dpr)
{
    Q_TRACE_SCOPE(QSSG_synchronize, view3D, size, dpr);

    Q_ASSERT(view3D != nullptr); // This is not an option!
    QSSGRhiContext *rhiCtx = m_sgContext->rhiContext().get();
    Q_ASSERT(rhiCtx != nullptr);

    // Generate layer node
    if (!m_layer)
        m_layer = new QSSGRenderLayer();

    bool newRenderStats = false;
    if (!m_renderStats) {
        m_renderStats = view3D->renderStats();
        newRenderStats = true;
    }

    if (m_renderStats)
        m_renderStats->startSync();

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DSynchronizeFrame);

    m_sgContext->renderer()->setDpr(dpr);
    bool layerSizeIsDirty = m_surfaceSize != size;
    m_surfaceSize = size;

    QQuick3DSceneEnvironment *environment = view3D->environment();
    if (environment->lightmapper()) {
        QQuick3DLightmapper *lightmapper = environment->lightmapper();
        lmOptions.opacityThreshold = lightmapper->opacityThreshold();
        lmOptions.bias = lightmapper->bias();
        lmOptions.useAdaptiveBias = lightmapper->isAdaptiveBiasEnabled();
        lmOptions.indirectLightEnabled = lightmapper->isIndirectLightEnabled();
        lmOptions.indirectLightSamples = lightmapper->samples();
        lmOptions.indirectLightWorkgroupSize = lightmapper->indirectLightWorkgroupSize();
        lmOptions.indirectLightBounces = lightmapper->bounces();
        lmOptions.indirectLightFactor = lightmapper->indirectLightFactor();
        lmOptions.sigma = lightmapper->denoiseSigma();
        lmOptions.texelsPerUnit = lightmapper->texelsPerUnit();
    } else {
        lmOptions = {};
    }

    { // Resolve lightmaps source url
        const QQmlContext *context = qmlContext(view3D);
        const QUrl originalSource = environment->lightmapper() ? environment->lightmapper()->source()
                                                               : QUrl::fromLocalFile(QStringLiteral("lightmaps.bin"));
        const auto resolvedUrl = context ? context->resolvedUrl(originalSource) : originalSource;
        const auto qmlSource = QQmlFile::urlToLocalFileOrQrc(resolvedUrl);
        const QString lightmapSource = qmlSource.isEmpty() ? originalSource.path() : qmlSource;
        lmOptions.source = lightmapSource;
        m_layer->lightmapSource = lightmapSource;
        // HACK: this is also set in the render layer but we need to set it here since
        // it is needed below when calculating bounding boxes from the stored lightmap mesh
        m_sgContext->bufferManager()->setLightmapSource(lightmapSource);
        if (QQuick3DSceneManager *sceneManager = QQuick3DObjectPrivate::get(view3D->scene())->sceneManager)
            sceneManager->lightmapSource = lightmapSource;
    }

    // Synchronize scene managers under this window
    QSet<QSSGRenderGraphObject *> resourceLoaders;
    QQuick3DWindowAttachment::SyncResult requestSharedUpdate = QQuick3DWindowAttachment::SyncResultFlag::None;
    if (auto window = view3D->window()) {
        if (!winAttacment || winAttacment->window() != window)
            winAttacment = QQuick3DSceneManager::getOrSetWindowAttachment(*window);

        if (winAttacment && winAttacment->rci() != m_sgContext)
            winAttacment->setRci(m_sgContext);

        QSSGRenderRoot *rootNode = winAttacment->rootNode();
        if (m_layer->rootNode != rootNode) {
            Q_ASSERT(m_layer->rootNode == nullptr);
            rootNode->addChild(*m_layer);
            rootNode->setStartVersion(m_layer->h.version());
            m_layer->ref(rootNode);
        }

        if (winAttacment)
            requestSharedUpdate |= winAttacment->synchronize(resourceLoaders);
    }

    // Import scenes used in a multi-window application...
    QQuick3DNode *importScene = view3D->importScene();
    if (importScene) {
        QQuick3DSceneManager *importSceneManager = QQuick3DObjectPrivate::get(importScene)->sceneManager;
        // If the import scene is used with 3D views under a different window, then we'll
        // need to trigger updates for those as well.
        if (auto window = importSceneManager->window(); window && window != view3D->window()) {
            if (auto winAttacment = importSceneManager->wattached) {
                // Not the same window but backed by the same rhi?
                auto rci = winAttacment->rci();
                const bool inlineSync = (rci && rci->rhi() && (rci->rhi()->thread() == m_sgContext->rhi()->thread()));
                if (inlineSync) {
                    // Given that we're on the same thread, we can do an immediate sync
                    // (rhi instances can differ, e.g., basic renderloop).
                    winAttacment->synchronize(resourceLoaders);
                } else if (rci && !window->isExposed()) { // Forced sync of non-exposed windows
                    // Not exposed, so not rendering (playing with fire here)...
                    winAttacment->synchronize(resourceLoaders);
                } else if (!rci || (requestSharedUpdate & QQuick3DWindowAttachment::SyncResultFlag::SharedResourcesDirty)) {
                    // If there's no RCI for the importscene we'll request an update, which should
                    // mean we only get here once. It also means the update to any secondary windows
                    // will be delayed. Note that calling this function on each sync would cause the
                    // different views to ping-pong for updated forever...
                    winAttacment->requestUpdate();
                }
            }
        }
    }

    // Update the layer node properties
    // Store the view count in the layer. If there are multiple, or nested views, sync is called multiple times and the view count
    // can change (see: updateLayerNode()), so we need to store the value on the layer to make sure we don't end up with a mismatch
    // between between the view count of the views rendering directly to the screen (XrView instance) and the view count of the offscreen
    // rendered View3Ds.
    // See also: preSynchronize(), queryMainRenderPassDescriptorAndCommandBuffer() and queryInlineRenderPassDescriptorAndCommandBuffer()
    // (At this point the mainPassViewCount for this view should be set to the correct value)
    m_layer->viewCount = rhiCtx->mainPassViewCount();
    updateLayerNode(*m_layer, *view3D, resourceLoaders.values());

    // Request extra frames for antialiasing (ProgressiveAA/TemporalAA)

    m_requestedFramesCount = 0;
    if (m_layer->isProgressiveAAEnabled() || m_layer->temporalAAMode == QSSGRenderLayer::TAAMode::MotionVector) {
        // with progressive AA or temporal AA motion vector mode, we need a number of extra frames after the last dirty one
        // if we always reset requestedFramesCount when dirty, we will get the extra frames eventually
        // +1 since we need a normal frame to start with, and we're not copying that from the screen
        // Temporal AA (MotionVector mode) requires ~44 frames to reach 99% convergence when
        // blending with mix(current, previous, 0.9). This exponential accumulation ensures
        // sub-pixel details are properly resolved through temporal super-sampling.
        m_requestedFramesCount = (m_layer->temporalAAMode == QSSGRenderLayer::TAAMode::MotionVector ? 45 :
                                          int(m_layer->antialiasingQuality) + 1);
    } else if (m_layer->isTemporalAAEnabled()) {
        // When temporalAA is on and antialiasing mode changes,
        // layer needs to be re-rendered (at least) MAX_TEMPORAL_AA_LEVELS times
        // depend on the temporalAA mode to generate temporal antialiasing.
        // Also, we need to do an extra render when animation stops
        m_requestedFramesCount = (m_aaIsDirty || m_temporalIsDirty) ? QSSGLayerRenderData::MAX_TEMPORAL_AA_LEVELS : 1;
    }

    // Now that we have the effect list used for rendering, finalize the shader
    // code based on the layer (scene.env.) settings.
    for (QSSGRenderEffect *effectNode = m_layer->firstEffect; effectNode; effectNode = effectNode->m_nextEffect)
        effectNode->finalizeShaders(*m_layer, m_sgContext.get());

    // NOTE: This could be done elewhere, but leaving it here for now.
    if (QQuick3DSceneManager *sm = QQuick3DObjectPrivate::get(view3D->scene())->sceneManager; sm) {
        for (QSSGRenderUserPass *userPass : std::as_const(sm->userRenderPasses))
            userPass->finalizeShaders(*m_sgContext);
    }

    if (newRenderStats)
        m_renderStats->setRhiContext(rhiCtx, m_layer);

    static const auto getStageIndex = [](const QSSGRenderExtension &ext) -> size_t {
        const QSSGRenderExtension::RenderMode mode = ext.mode();
        const QSSGRenderExtension::RenderStage stage = ext.stage();
        // If the mode is 'Standalone' then the stage is irrelevant and we put the
        // extension in the 'TextureProvider' list.
        if (mode == QSSGRenderExtension::RenderMode::Standalone)
            return size_t(QSSGRenderLayer::RenderExtensionStage::TextureProviders);

        switch (stage) {
        case QSSGRenderExtension::RenderStage::PreColor:
            return size_t(QSSGRenderLayer::RenderExtensionStage::Overlay);
        case QSSGRenderExtension::RenderStage::PostColor:
            return size_t(QSSGRenderLayer::RenderExtensionStage::Underlay);
        }

        Q_UNREACHABLE_RETURN(size_t(QSSGRenderLayer::RenderExtensionStage::Underlay));
    };

    // if the list is dirty we rebuild (assumption is that this won't happen frequently).
    // NOTE: We do this is two steps as extensions can be added both via the extensionList
    //       or via auto-registration.
    if (QQuick3DSceneManager *sm = QQuick3DObjectPrivate::get(view3D->scene())->sceneManager; sm) {
        const bool rebuildExtensionLists = (requestSharedUpdate & QQuick3DWindowAttachment::SyncResultFlag::ExtensionsDiry)
                                           || view3D->extensionListDirty()
                                           || sm->autoRegisteredExtensionsDirty;

        // Explicit extensions from the extension list
        // NOTE: If either the explicit or auto-registered extensions are dirty we need to
        //       rebuild the list of active extensions.
        if (rebuildExtensionLists) {
            // Clear existing extensions
            for (size_t i = 0; i != size_t(QSSGRenderLayer::RenderExtensionStage::Count); ++i)
                m_layer->renderExtensions[i].clear();

            // All items in the extension list are root items,
            const auto &extensions = view3D->extensionList();
            for (const auto &ext : extensions) {
                const auto type = QQuick3DObjectPrivate::get(ext)->type;
                if (QSSGRenderGraphObject::isExtension(type)) {
                    if (type == QSSGRenderGraphObject::Type::RenderExtension) {
                        if (auto *renderExt = qobject_cast<QQuick3DRenderExtension *>(ext)) {
                            if (QSSGRenderExtension *ssgExt = static_cast<QSSGRenderExtension *>(QQuick3DObjectPrivate::get(renderExt)->spatialNode)) {
                                const auto stage =  getStageIndex(*ssgExt);
                                auto &list = m_layer->renderExtensions[size_t(stage)];
                                bfs(qobject_cast<QQuick3DRenderExtension *>(ext), list);
                            }
                        }
                    }
                }
            }

            view3D->clearExtensionListDirty();
        }

        // Auto-registered extensions
        if (sm->autoRegisteredExtensionsDirty) {
            for (QSSGRenderExtension *ae : std::as_const(sm->autoRegisteredExtensions))
                m_layer->renderExtensions[getStageIndex(*ae)].push_back(ae);
            sm->autoRegisteredExtensionsDirty = false;
        }
    }

    bool postProcessingNeeded = m_layer->firstEffect;
    bool postProcessingWasActive = m_effectSystem;
    QSSGRenderTextureFormat::Format effectOutputFormatOverride = QSSGRenderTextureFormat::Unknown;
    if (postProcessingNeeded) {
        QSSGRenderEffect *lastEffect = m_layer->firstEffect;
        while (lastEffect->m_nextEffect)
            lastEffect = lastEffect->m_nextEffect;
        effectOutputFormatOverride = QSSGRhiEffectSystem::overriddenOutputFormat(lastEffect);
    }
    const auto layerTextureFormat = [effectOutputFormatOverride, view3D](QRhi *rhi, bool postProc) {
        if (effectOutputFormatOverride != QSSGRenderTextureFormat::Unknown)
            return QSSGBufferManager::toRhiFormat(effectOutputFormatOverride);

        // Our standard choice for the postprocessing input/output textures'
        // format is a floating point one. (unlike intermediate Buffers, which
        // default to RGBA8 unless the format is explicitly specified)
        // This is intentional since a float format allows passing in
        // non-tonemapped content without colors being clamped when written out
        // to the render target.
        //
        // When it comes to the output, this applies to that too due to
        // QSSGRhiEffectSystem picking it up unless overridden (with a Buffer
        // an empty 'name'). Here too a float format gives more flexibility:
        // the effect may or may not do its own tonemapping and this approach
        // is compatible with potential future on-screen HDR output support.

        const QRhiTexture::Format preferredPostProcFormat = QRhiTexture::RGBA16F;
        if (postProc && rhi->isTextureFormatSupported(preferredPostProcFormat))
            return preferredPostProcFormat;

#if QT_CONFIG(quick_shadereffect)
        const QRhiTexture::Format preferredView3DFormat = toRhiTextureFormat(view3D->renderFormat());
        if (rhi->isTextureFormatSupported(preferredView3DFormat))
            return preferredView3DFormat;
#endif

        return QRhiTexture::RGBA8;
    };
    bool postProcessingStateDirty = postProcessingNeeded != postProcessingWasActive;

    // Store from the layer properties the ones we need to handle ourselves (with the RHI code path)
    m_backgroundMode = QSSGRenderLayer::Background(view3D->environment()->backgroundMode());

    // This is stateful since we only want to recalculate the tonemapped color
    // when the color changes, not in every frame.
    QColor currentUserBackgroundColor = view3D->environment()->clearColor();
    if (m_userBackgroundColor != currentUserBackgroundColor) {
        m_userBackgroundColor = currentUserBackgroundColor;
        m_linearBackgroundColor = QSSGUtils::color::sRGBToLinearColor(m_userBackgroundColor);
        const QVector3D tc = tonemapRgb(QVector3D(m_linearBackgroundColor.redF(),
                                                  m_linearBackgroundColor.greenF(),
                                                  m_linearBackgroundColor.blueF()),
                                        view3D->environment()->tonemapMode());
        m_tonemappedBackgroundColor = QColor::fromRgbF(tc.x(), tc.y(), tc.z(), m_linearBackgroundColor.alphaF());
    }
    m_layer->scissorRect = QRect(view3D->environment()->scissorRect().topLeft() * dpr,
                                 view3D->environment()->scissorRect().size() * dpr);

    // Add the scene root node for the scene to the layer
    // NOTE: The scene root is not the same as THE root node.
    // The scene root is the root of the scene in a view (There can be multiple views.)
    // THE root node, which there's only one of, is the root for all nodes in the window.
    auto sceneRootNode = static_cast<QSSGRenderNode*>(QQuick3DObjectPrivate::get(view3D->scene())->spatialNode);
    if (sceneRootNode != m_sceneRootNode) {
        if (m_sceneRootNode)
            removeNodeFromLayer(m_sceneRootNode);

        if (sceneRootNode)
            addNodeToLayer(sceneRootNode);

        m_sceneRootNode = sceneRootNode;
    }

    // Add the referenced scene root node to the layer as well if available
    QSSGRenderNode *importSceneRootNode = nullptr;
    if (importScene)
        importSceneRootNode = static_cast<QSSGRenderNode*>(QQuick3DObjectPrivate::get(importScene)->spatialNode);

    if (importSceneRootNode != m_importSceneRootNode) {
        if (m_importSceneRootNode)
            m_layer->removeImportScene(*m_importSceneRootNode);

        if (importSceneRootNode) {
            // if importScene has the rendered viewport as ancestor, it probably means
            // "importScene: MyScene { }" type of inclusion.
            // In this case don't duplicate content by adding it again.
            QObject *sceneParent = importScene->parent();
            bool isEmbedded = false;
            while (sceneParent) {
                if (sceneParent == view3D) {
                    isEmbedded = true;
                    break;
                }
                sceneParent = sceneParent->parent();
            }
            if (!isEmbedded)
                m_layer->setImportScene(*importSceneRootNode);
        }

        m_importSceneRootNode = importSceneRootNode;
    }

    // If the tree is dirty, we need to mark all layers as dirty
    // so that they get updated.
    // The _layer_ dirty flag is cleared in the layer prep function and the reindex and
    // root dirty flag is cleared right before the first layer is prepared (see: prepareLayerForRender().
    {
        QSSGRenderRoot *rootNode = winAttacment->rootNode();
        if (rootNode->isDirty(QSSGRenderRoot::DirtyFlag::TreeDirty)) {
            rootNode->reindex(); // Clears TreeDirty flag
            for (QSSGRenderNode &layer : rootNode->children) {
                if (QSSG_GUARD_X(layer.type == QSSGRenderGraphObject::Type::Layer, "Layer type mismatch"))
                    static_cast<QSSGRenderLayer &>(layer).markDirty(QSSGRenderLayer::DirtyFlag::TreeDirty);
            }

            // We exploit the fact that we can use the nodes indexes to establish a dependency order
            // for user passes by using the parent node's index.
            if (QQuick3DSceneManager *sm = QQuick3DObjectPrivate::get(view3D->scene())->sceneManager; sm) {
                for (QSSGRenderUserPass *userPass : std::as_const(sm->userRenderPasses)) {
                    if (const auto *fo = sm->lookUpNode(userPass); fo && fo->parentItem()) {
                        const auto *pi = fo->parentItem();
                        if (const QSSGRenderGraphObject *parentNode = QQuick3DObjectPrivate::get(pi)->spatialNode; parentNode && QSSGRenderGraphObject::isNodeType(parentNode->type))
                            userPass->setDependencyIndex(static_cast<const QSSGRenderNode *>(parentNode)->h.index());
                        else
                            userPass->setDependencyIndex(0); // 0 means no dependency.
                    }
                }
            }
        }
    }

    maybeSetupLightmapBaking(view3D);

    if (m_useFBO && rhiCtx->isValid()) {
        QRhi *rhi = rhiCtx->rhi();
        const QSize renderSize = m_layer->isSsaaEnabled() ? m_surfaceSize * m_layer->ssaaMultiplier : m_surfaceSize;

        if (m_texture) {
            // the size changed, or the AA settings changed, or toggled between some effects - no effect
            if (layerSizeIsDirty || postProcessingStateDirty) {
                m_texture->setPixelSize(m_surfaceSize);
                m_texture->setFormat(layerTextureFormat(rhi, postProcessingNeeded));
                m_texture->create();

                // If AA settings changed, then we drop and recreate all
                // resources, otherwise use a lighter path if just the size
                // changed.
                if (!m_aaIsDirty) {
                    // A special case: when toggling effects and AA is on,
                    // use the heavier AA path because the renderbuffer for
                    // MSAA and texture for SSAA may need a different
                    // format now since m_texture's format could have
                    // changed between RBGA8 and RGBA16F (due to layerTextureFormat()).
                    if (postProcessingStateDirty && (m_layer->antialiasingMode != QSSGRenderLayer::AAMode::NoAA || m_layer->isTemporalAAEnabled())) {
                        releaseAaDependentRhiResources();
                    } else {
                        if (m_ssaaTexture) {
                            m_ssaaTexture->setPixelSize(renderSize);
                            m_ssaaTexture->create();
                        }
                        if (m_depthStencilBuffer) {
                            m_depthStencilBuffer->setPixelSize(renderSize);
                            m_depthStencilBuffer->create();
                        }
                        if (m_multiViewDepthStencilBuffer) {
                            m_multiViewDepthStencilBuffer->setPixelSize(renderSize);
                            m_multiViewDepthStencilBuffer->create();
                        }
                        if (m_msaaRenderBufferLegacy) {
                            m_msaaRenderBufferLegacy->setPixelSize(renderSize);
                            m_msaaRenderBufferLegacy->create();
                        }
                        if (m_msaaRenderTexture) {
                            m_msaaRenderTexture->setPixelSize(renderSize);
                            m_msaaRenderTexture->create();
                        }
                        if (m_msaaMultiViewRenderBuffer) {
                            m_msaaMultiViewRenderBuffer->setPixelSize(renderSize);
                            m_msaaMultiViewRenderBuffer->create();
                        }
                        // Toggling effects on and off will change the format
                        // (assuming effects default to a floating point
                        // format) and that needs on a different renderpass on
                        // Vulkan. Hence renewing m_textureRenderPassDescriptor as well.
                        if (postProcessingStateDirty) {
                            delete m_textureRenderPassDescriptor;
                            m_textureRenderPassDescriptor = m_textureRenderTarget->newCompatibleRenderPassDescriptor();
                            m_textureRenderTarget->setRenderPassDescriptor(m_textureRenderPassDescriptor);
                        }
                        m_textureRenderTarget->create();
                        if (m_ssaaTextureToTextureRenderTarget)
                            m_ssaaTextureToTextureRenderTarget->create();

                        if (m_temporalAATexture) {
                            m_temporalAATexture->setPixelSize(renderSize);
                            m_temporalAATexture->create();
                        }
                        if (m_prevTempAATexture) {
                            m_prevTempAATexture->setPixelSize(renderSize);
                            m_prevTempAATexture->create();
                        }
                        if (m_temporalAARenderTarget)
                            m_temporalAARenderTarget->create();
                    }
                }
            } else if (m_aaIsDirty && rhi->backend() == QRhi::Metal) { // ### to avoid garbage upon enabling MSAA with macOS 10.14 (why is this needed?)
                m_texture->create();
            }

            if (m_aaIsDirty)
                releaseAaDependentRhiResources();
        }

        const QRhiTexture::Flags textureFlags = QRhiTexture::RenderTarget
                | QRhiTexture::UsedAsTransferSource; // transfer source is for progressive/temporal AA
        const QRhiTexture::Format textureFormat = layerTextureFormat(rhi, postProcessingNeeded);

        if (!m_texture) {
            if (m_layer->viewCount >= 2)
                m_texture = rhi->newTextureArray(textureFormat, m_layer->viewCount, m_surfaceSize, 1, textureFlags);
            else
                m_texture = rhi->newTexture(textureFormat, m_surfaceSize, 1, textureFlags);
            m_texture->create();
        }

        if (!m_ssaaTexture && m_layer->isSsaaEnabled()) {
            if (m_layer->viewCount >= 2)
                m_ssaaTexture = rhi->newTextureArray(textureFormat, m_layer->viewCount, renderSize, 1, textureFlags);
            else
                m_ssaaTexture = rhi->newTexture(textureFormat, renderSize, 1, textureFlags);
            m_ssaaTexture->create();
        }

        if (m_timeBasedAA && !m_temporalAATexture) {
            m_temporalAATexture = rhi->newTexture(textureFormat, renderSize, 1, textureFlags);
            m_temporalAATexture->create();
            m_prevTempAATexture = rhi->newTexture(textureFormat, renderSize, 1, textureFlags);
            m_prevTempAATexture->create();
        }

        // we need to re-render time-based AA not only when AA state changes, but also when resized
        if (m_aaIsDirty || layerSizeIsDirty)
            m_layer->tempAAPassIndex = m_layer->progAAPassIndex = 0;

        if (m_aaIsDirty) {
            m_samples = 1;
            if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::MSAA) {
                if (rhi->isFeatureSupported(QRhi::MultisampleRenderBuffer)) {
                    m_samples = qMax(1, int(m_layer->antialiasingQuality));
                    // The Quick3D API exposes high level values such as
                    // Medium, High, VeryHigh instead of direct sample
                    // count values. Therefore, be nice and find a sample
                    // count that's actually supported in case the one
                    // associated by default is not.
                    const QVector<int> supported = rhi->supportedSampleCounts(); // assumed to be sorted
                    if (!supported.contains(m_samples)) {
                        if (!supported.isEmpty()) {
                            auto it = std::lower_bound(supported.cbegin(), supported.cend(), m_samples);
                            m_samples = it == supported.cend() ? supported.last() : *it;
                        } else {
                            m_samples = 1;
                        }
                    }
                } else {
                    static bool warned = false;
                    if (!warned) {
                        warned = true;
                        qWarning("Multisample renderbuffers are not supported, disabling MSAA for Offscreen View3D");
                    }
                }
            }
        }

        if (m_layer->viewCount >= 2) {
            if (!m_multiViewDepthStencilBuffer) {
                const auto format = rhi->isTextureFormatSupported(QRhiTexture::D24S8) ? QRhiTexture::D24S8 : QRhiTexture::D32FS8;
                m_multiViewDepthStencilBuffer = rhi->newTextureArray(format, m_layer->viewCount, renderSize,
                                                                     m_samples, QRhiTexture::RenderTarget);
                m_multiViewDepthStencilBuffer->create();
            }
        } else {
            if (!m_depthStencilBuffer) {
                m_depthStencilBuffer = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, renderSize, m_samples);
                m_depthStencilBuffer->create();
            }
        }

        if (!m_textureRenderTarget) {
            QRhiTextureRenderTargetDescription rtDesc;
            QRhiColorAttachment att;
            if (m_samples > 1) {
                if (m_layer->viewCount >= 2) {
                    m_msaaMultiViewRenderBuffer = rhi->newTextureArray(textureFormat, m_layer->viewCount, renderSize, m_samples, QRhiTexture::RenderTarget);
                    m_msaaMultiViewRenderBuffer->create();
                    att.setTexture(m_msaaMultiViewRenderBuffer);
                } else {
                    if (!rhi->isFeatureSupported(QRhi::MultisampleTexture)) {
                        // pass in the texture's format (which may be a floating point one!) as the preferred format hint
                        m_msaaRenderBufferLegacy = rhi->newRenderBuffer(QRhiRenderBuffer::Color, renderSize, m_samples, {}, m_texture->format());
                        m_msaaRenderBufferLegacy->create();
                        att.setRenderBuffer(m_msaaRenderBufferLegacy);
                    } else {
                        // The texture-backed MSAA path has the benefit of being able to use
                        // GL_EXT_multisampled_render_to_texture on OpenGL ES when supported (Mali
                        // and Qualcomm GPUs typically), potentially giving significant performance
                        // gains. Whereas with 3D APIs other than OpenGL there is often no
                        // difference between QRhiRenderBuffer and QRhiTexture anyway.
                        m_msaaRenderTexture = rhi->newTexture(textureFormat, renderSize, m_samples, QRhiTexture::RenderTarget);
                        m_msaaRenderTexture->create();
                        att.setTexture(m_msaaRenderTexture);
                    }
                }
                att.setResolveTexture(m_texture);
            } else {
                if (m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA)
                    att.setTexture(m_ssaaTexture);
                else
                    att.setTexture(m_texture);
            }
            att.setMultiViewCount(m_layer->viewCount);
            rtDesc.setColorAttachments({ att });
            if (m_depthStencilBuffer)
                rtDesc.setDepthStencilBuffer(m_depthStencilBuffer);
            if (m_multiViewDepthStencilBuffer)
                rtDesc.setDepthTexture(m_multiViewDepthStencilBuffer);

            m_textureRenderTarget = rhi->newTextureRenderTarget(rtDesc);
            m_textureRenderTarget->setName(QByteArrayLiteral("View3D"));
            m_textureRenderPassDescriptor = m_textureRenderTarget->newCompatibleRenderPassDescriptor();
            m_textureRenderTarget->setRenderPassDescriptor(m_textureRenderPassDescriptor);
            m_textureRenderTarget->create();
        }

        if (!m_ssaaTextureToTextureRenderTarget && m_layer->antialiasingMode == QSSGRenderLayer::AAMode::SSAA) {
            QRhiColorAttachment att(m_texture);
            att.setMultiViewCount(m_layer->viewCount);
            m_ssaaTextureToTextureRenderTarget = rhi->newTextureRenderTarget(QRhiTextureRenderTargetDescription({ att }));
            m_ssaaTextureToTextureRenderTarget->setName(QByteArrayLiteral("SSAA texture"));
            m_ssaaTextureToTextureRenderPassDescriptor = m_ssaaTextureToTextureRenderTarget->newCompatibleRenderPassDescriptor();
            m_ssaaTextureToTextureRenderTarget->setRenderPassDescriptor(m_ssaaTextureToTextureRenderPassDescriptor);
            m_ssaaTextureToTextureRenderTarget->create();
        }

        if (m_layer->firstEffect) {
            if (!m_effectSystem)
                m_effectSystem = new QSSGRhiEffectSystem(m_sgContext);
            m_effectSystem->setup(renderSize);
        } else if (m_effectSystem) {
            delete m_effectSystem;
            m_effectSystem = nullptr;
        }

        if (m_timeBasedAA && !m_temporalAARenderTarget) {
            m_temporalAARenderTarget = rhi->newTextureRenderTarget({ m_temporalAATexture });
            m_temporalAARenderTarget->setName(QByteArrayLiteral("Temporal AA texture"));
            m_temporalAARenderPassDescriptor = m_temporalAARenderTarget->newCompatibleRenderPassDescriptor();
            m_temporalAARenderTarget->setRenderPassDescriptor(m_temporalAARenderPassDescriptor);
            m_temporalAARenderTarget->create();
        }

        m_textureNeedsFlip = rhi->isYUpInFramebuffer();
        m_aaIsDirty = false;
    }

    if (m_renderStats)
        m_renderStats->endSync(dumpRenderTimes());

    Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DSynchronizeFrame, quint64(m_surfaceSize.width()) | quint64(m_surfaceSize.height()) << 32, profilingId);
}

void QQuick3DSceneRenderer::invalidateFramebufferObject()
{
    if (fboNode)
        fboNode->invalidatePending = true;
}

void QQuick3DSceneRenderer::releaseCachedResources()
{
    if (m_layer && m_layer->renderData) {
        if (const auto &mgr = m_layer->renderData->getShadowMapManager())
            mgr->releaseCachedResources();
        if (const auto &mgr = m_layer->renderData->getReflectionMapManager())
            mgr->releaseCachedResources();
    }
}

std::optional<QSSGRenderRay> QQuick3DSceneRenderer::getRayFromViewportPos(const QPointF &pos)
{
    if (!m_layer || !m_layer->renderData)
        return std::nullopt;

    QMutexLocker locker(&m_layer->renderedCamerasMutex);

    if (m_layer->renderedCameras.isEmpty())
        return std::nullopt;

    QMatrix4x4 globalTransform = m_layer->renderData->getGlobalTransform(*m_layer->renderedCameras[0]);

    const QVector2D viewportSize(m_surfaceSize.width(), m_surfaceSize.height());
    const QVector2D position(float(pos.x()), float(pos.y()));
    const QRectF viewportRect(QPointF{}, QSizeF(m_surfaceSize));

    // First invert the y so we are dealing with numbers in a normal coordinate space.
    // Second, move into our layer's coordinate space
    QVector2D correctCoords(position.x(), viewportSize.y() - position.y());
    QVector2D theLocalMouse = QSSGUtils::rect::toRectRelative(viewportRect, correctCoords);
    if ((theLocalMouse.x() < 0.0f || theLocalMouse.x() >= viewportSize.x() || theLocalMouse.y() < 0.0f
         || theLocalMouse.y() >= viewportSize.y()))
        return std::nullopt;

    return m_layer->renderedCameras[0]->unproject(globalTransform, theLocalMouse, viewportRect);
}

std::optional<QSSGRenderPickResult> QQuick3DSceneRenderer::syncPickClosestPoint(const QVector3D &center, float radiusSquared, QSSGRenderNode *node)
{
    if (!m_layer)
        return std::nullopt;

    return QSSGRendererPrivate::syncPickClosestPoint(*m_sgContext,
                                                     *m_layer,
                                                     center, radiusSquared,
                                                     node);
}

QQuick3DSceneRenderer::PickResultList QQuick3DSceneRenderer::syncPick(const QSSGRenderRay &ray)
{
    if (!m_layer)
        return QQuick3DSceneRenderer::PickResultList();

    return QSSGRendererPrivate::syncPick(*m_sgContext,
                                         *m_layer,
                                         ray);
}

QQuick3DSceneRenderer::PickResultList QQuick3DSceneRenderer::syncPickOne(const QSSGRenderRay &ray, QSSGRenderNode *node)
{
    if (!m_layer)
        return QQuick3DSceneRenderer::PickResultList();

    return QSSGRendererPrivate::syncPick(*m_sgContext,
                                         *m_layer,
                                         ray,
                                         node);
}

QQuick3DSceneRenderer::PickResultList QQuick3DSceneRenderer::syncPickSubset(const QSSGRenderRay &ray,
                                                                            QVarLengthArray<QSSGRenderNode *> subset)
{
    if (!m_layer)
        return QQuick3DSceneRenderer::PickResultList();

    return QSSGRendererPrivate::syncPickSubset(*m_layer,
                                               *m_sgContext->bufferManager(),
                                               ray,
                                               subset);
}

QQuick3DSceneRenderer::PickResultList QQuick3DSceneRenderer::syncPickAll(const QSSGRenderRay &ray)
{
    if (!m_layer)
        return QQuick3DSceneRenderer::PickResultList();

    return QSSGRendererPrivate::syncPickAll(*m_sgContext,
                                            *m_layer,
                                            ray);
}

QList<const QSSGRenderNode *> QQuick3DSceneRenderer::syncPickInFrustum(const QSSGFrustum &frustum)
{
    if (!m_layer)
        return {};

    return QSSGRendererPrivate::syncPickInFrustum(*m_sgContext, *m_layer, frustum);
}

void QQuick3DSceneRenderer::setGlobalPickingEnabled(bool isEnabled)
{
    QSSGRendererPrivate::setGlobalPickingEnabled(*m_sgContext->renderer(), isEnabled);
}

QQuick3DRenderStats *QQuick3DSceneRenderer::renderStats()
{
    return m_renderStats;
}

void QQuick3DRenderLayerHelpers::updateLayerNodeHelper(const QQuick3DViewport &view3D,
                                                       const std::shared_ptr<QSSGRenderContextInterface>& rci,
                                                       QSSGRenderLayer &layerNode,
                                                       bool &aaIsDirty,
                                                       bool &temporalIsDirty)
{
    QList<QSSGRenderGraphObject *> resourceLoaders; // empty list

    QQuick3DSceneRenderer dummyRenderer(rci);

    // Update the layer node properties
    dummyRenderer.updateLayerNode(layerNode, view3D, resourceLoaders);

    aaIsDirty = dummyRenderer.m_aaIsDirty;
    temporalIsDirty = dummyRenderer.m_temporalIsDirty;
}

void QQuick3DSceneRenderer::updateLayerNode(QSSGRenderLayer &layerNode,
                                            const QQuick3DViewport &view3D,
                                            const QList<QSSGRenderGraphObject *> &resourceLoaders)
{
    QQuick3DSceneEnvironment *environment = view3D.environment();
    const auto &effects = environment->effectList();

    QSSGRenderLayer::AAMode aaMode = QSSGRenderLayer::AAMode(environment->antialiasingMode());
    if (aaMode != layerNode.antialiasingMode) {
        layerNode.antialiasingMode = aaMode;
        layerNode.progAAPassIndex = 0;
        m_aaIsDirty = true;
    }
    QSSGRenderLayer::AAQuality aaQuality = QSSGRenderLayer::AAQuality(environment->antialiasingQuality());
    if (aaQuality != layerNode.antialiasingQuality) {
        layerNode.antialiasingQuality = aaQuality;
        layerNode.ssaaMultiplier = QSSGRenderLayer::ssaaMultiplierForQuality(aaQuality);
        m_aaIsDirty = true;
    }

    // NOTE: Temporal AA is disabled when MSAA is enabled.
    const bool temporalAARequested = environment->temporalAAEnabled();
    const bool wasTaaEnabled = layerNode.isTemporalAAEnabled();
    layerNode.temporalAAMode = temporalAARequested ? QSSGRenderLayer::TAAMode(environment->m_temporalAAMode + 1) // map the environment mode to the layer mode
                                                   : QSSGRenderLayer::TAAMode::Off;

    // If the state changed we need to reset the temporal AA pass index etc.
    if (wasTaaEnabled != layerNode.isTemporalAAEnabled()) {
        layerNode.tempAAPassIndex = 0;
        m_aaIsDirty = true;
        m_temporalIsDirty = true;
    }

    layerNode.temporalAAStrength = environment->temporalAAStrength();

    layerNode.specularAAEnabled = environment->specularAAEnabled();

    layerNode.background = QSSGRenderLayer::Background(environment->backgroundMode());
    layerNode.clearColor = QVector3D(float(environment->clearColor().redF()),
                                      float(environment->clearColor().greenF()),
                                      float(environment->clearColor().blueF()));

    layerNode.gridEnabled = environment->gridEnabled();
    layerNode.gridScale = environment->gridScale();
    layerNode.gridFlags = environment->gridFlags();

    layerNode.aoStrength = environment->aoStrength();
    layerNode.aoDistance = environment->aoDistance();
    layerNode.aoSoftness = environment->aoSoftness();
    layerNode.aoEnabled = environment->aoEnabled();
    layerNode.aoBias = environment->aoBias();
    layerNode.aoSamplerate = environment->aoSampleRate();
    layerNode.aoDither = environment->aoDither();

    // ### These images will not be registered anywhere
    if (environment->lightProbe())
        layerNode.lightProbe = environment->lightProbe()->getRenderImage();
    else
        layerNode.lightProbe = nullptr;
    if (view3D.environment()->skyBoxCubeMap())
        layerNode.skyBoxCubeMap = view3D.environment()->skyBoxCubeMap()->getRenderImage();
    else
        layerNode.skyBoxCubeMap = nullptr;

    layerNode.lightProbeSettings.probeExposure = environment->probeExposure();
    // Remap the probeHorizon to the expected Range
    layerNode.lightProbeSettings.probeHorizon = qMin(environment->probeHorizon() - 1.0f, -0.001f);
    layerNode.setProbeOrientation(environment->probeOrientation());

    QQuick3DViewport::updateCameraForLayer(view3D, layerNode);

    layerNode.layerFlags.setFlag(QSSGRenderLayer::LayerFlag::EnableDepthTest, environment->depthTestEnabled());
    layerNode.layerFlags.setFlag(QSSGRenderLayer::LayerFlag::EnableDepthPrePass, environment->depthPrePassEnabled());

    layerNode.tonemapMode = QQuick3DSceneRenderer::getTonemapMode(*environment);
    layerNode.skyboxBlurAmount = environment->skyboxBlurAmount();
    if (auto debugSettings = view3D.environment()->debugSettings()) {
        layerNode.debugMode = QSSGRenderLayer::MaterialDebugMode(debugSettings->materialOverride());
        layerNode.wireframeMode = debugSettings->wireframeEnabled();
        layerNode.drawDirectionalLightShadowBoxes = debugSettings->drawDirectionalLightShadowBoxes();
        layerNode.drawPointLightShadowBoxes = debugSettings->drawPointLightShadowBoxes();
        layerNode.drawShadowCastingBounds = debugSettings->drawShadowCastingBounds();
        layerNode.drawShadowReceivingBounds = debugSettings->drawShadowReceivingBounds();
        layerNode.drawCascades = debugSettings->drawCascades();
        layerNode.drawSceneCascadeIntersection = debugSettings->drawSceneCascadeIntersection();
        layerNode.disableShadowCameraUpdate = debugSettings->disableShadowCameraUpdate();
        layerNode.drawCulledObjects = debugSettings->drawCulledObjects();
    } else {
        layerNode.debugMode = QSSGRenderLayer::MaterialDebugMode::None;
        layerNode.wireframeMode = false;
    }

    if (environment->fog() && environment->fog()->isEnabled()) {
        layerNode.fog.enabled = true;
        const QQuick3DFog *fog = environment->fog();
        layerNode.fog.color = QSSGUtils::color::sRGBToLinear(fog->color()).toVector3D();
        layerNode.fog.density = fog->density();
        layerNode.fog.depthEnabled = fog->isDepthEnabled();
        layerNode.fog.depthBegin = fog->depthNear();
        layerNode.fog.depthEnd = fog->depthFar();
        layerNode.fog.depthCurve = fog->depthCurve();
        layerNode.fog.heightEnabled = fog->isHeightEnabled();
        layerNode.fog.heightMin = fog->leastIntenseY();
        layerNode.fog.heightMax = fog->mostIntenseY();
        layerNode.fog.heightCurve = fog->heightCurve();
        layerNode.fog.transmitEnabled = fog->isTransmitEnabled();
        layerNode.fog.transmitCurve = fog->transmitCurve();
    } else {
        layerNode.fog.enabled = false;
    }
    const auto method = static_cast<QSSGRenderLayer::OITMethod>(environment->oitMethod());
    layerNode.oitMethodDirty = method != layerNode.oitMethod;
    layerNode.oitMethod = method;

    // Effects need to be rendered in reverse order as described in the file.
    // NOTE: We only build up the list here, don't do anything that depends
    //       on the collected layer state yet. See sync() for that.
    layerNode.firstEffect = nullptr; // We reset the linked list
    auto rit = effects.crbegin();
    const auto rend = effects.crend();
    for (; rit != rend; ++rit) {
        QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(*rit);
        QSSGRenderEffect *effectNode = static_cast<QSSGRenderEffect *>(p->spatialNode);
        if (effectNode) {
            if (layerNode.hasEffect(effectNode)) {
                qWarning() << "Duplicate effect found, skipping!";
            } else {
                effectNode->className = (*rit)->metaObject()->className(); //### persistent, but still icky to store a const char* returned from a function
                layerNode.addEffect(*effectNode);
            }
        }
    }

    const bool hasEffects = (layerNode.firstEffect != nullptr);

    const auto renderMode = view3D.renderMode();

    const bool progressiveAA = layerNode.isProgressiveAAEnabled();
    const bool temporalAA = layerNode.isTemporalAAEnabled();
    const bool superSamplingAA = layerNode.isSsaaEnabled();
    m_timeBasedAA = progressiveAA || temporalAA;
    m_postProcessingStack = hasEffects || m_timeBasedAA  || superSamplingAA;
    m_useFBO = renderMode == QQuick3DViewport::RenderMode::Offscreen ||
            ((renderMode == QQuick3DViewport::RenderMode::Underlay || renderMode == QQuick3DViewport::RenderMode::Overlay)
             && m_postProcessingStack);

    // Update the view count

    // NOTE: If we're rendering to an FBO, the view count is more than 1, and the View3D is not an XR view instance,
    // we need to force the view count to 1 (The only time this should be the case is when embedding View3D(s)
    // in XR with multiview enabled).
    // Also, note that embedding View3D(s) in XR with multiview enabled only works if those View3D(s) are
    // being rendered through a FBO.
    if (m_useFBO && (layerNode.viewCount > 1) && !view3D.isXrViewInstance())
        layerNode.viewCount = 1;

    // ResourceLoaders
    layerNode.resourceLoaders.clear();
    layerNode.resourceLoaders = resourceLoaders;

    layerNode.renderOverrides = QSSGRenderLayer::RenderOverridesT(view3D.renderOverrides().toInt());
}

void QQuick3DSceneRenderer::removeNodeFromLayer(QSSGRenderNode *node)
{
    if (!m_layer)
        return;

    m_layer->removeChild(*node);
}

void QQuick3DSceneRenderer::maybeSetupLightmapBaking(QQuick3DViewport *view3D)
{
    if (m_layer->renderData && m_layer->renderData->lightmapBaker)
        return;

    // Check if we have interactive bake requested or if we are coming in here the second
    // time from cmd line request (needs to wait a frame before starting to bake).
    bool bakeRequested = false;
    bool denoiseRequested = false;
    bool fromCmd = false;
    QQuick3DLightmapBaker *lightmapBaker = view3D->maybeLightmapBaker();
    if (lightmapBaker && (lightmapBaker->m_bakingRequested || lightmapBaker->m_denoisingRequested)) {
        bakeRequested = std::exchange(lightmapBaker->m_bakingRequested, false);
        denoiseRequested = std::exchange(lightmapBaker->m_denoisingRequested, false);
    } else {
        bakeRequested = m_lightmapBakingFromCmdRequested;
        denoiseRequested = m_lightmapDenoisingFromCmdRequested;
        fromCmd = bakeRequested;
    }

    // Start the bake (we should have a valid layer render data at this point).
    if (bakeRequested || denoiseRequested) {
        QSSGLightmapBaker::Context ctx;
        ctx.settings.bakeRequested = bakeRequested;
        ctx.settings.denoiseRequested = denoiseRequested;
        ctx.settings.quitWhenFinished = fromCmd;

        // We want the frontend callback in the case that a QQuick3DLightmapBaker is present
        if (lightmapBaker) {
            QQuick3DLightmapBaker::Callback qq3dCallback = lightmapBaker->m_callback;
            QQuick3DLightmapBaker::BakingControl *qq3dBakingControl = lightmapBaker->m_bakingControl;
            QSSGLightmapper::Callback callback =
                [qq3dCallback,
                 qq3dBakingControl](const QVariantMap &payload,
                                    QSSGLightmapper::BakingControl *qssgBakingControl) {
                    qq3dCallback(payload, qq3dBakingControl);

                    if (qq3dBakingControl->isCancelled() && !qssgBakingControl->cancelled)
                        qssgBakingControl->cancelled = true;
                };
            ctx.callbacks.lightmapBakingOutput = callback;
        }

        // Both the QQuick3DLightmapBaker and cmd / env variant needs this
        ctx.callbacks.triggerNewFrame = [view3D](bool releaseResources) {
            if (releaseResources) {
                QMetaObject::invokeMethod(view3D->window(),
                                            &QQuickWindow::releaseResources,
                                            Qt::QueuedConnection);
            }
            QMetaObject::invokeMethod(view3D, &QQuick3DViewport::update, Qt::QueuedConnection);
        };
        ctx.callbacks.setCurrentlyBaking = [this](bool value) {
            m_sgContext->bufferManager()->setCurrentlyLightmapBaking(value);
        };

        ctx.env.rhiCtx = m_sgContext->rhiContext().get();
        ctx.env.renderer = m_sgContext->renderer().get();
        ctx.env.lmOptions = lmOptions;
        m_layer->renderData->initializeLightmapBaking(ctx);

    } else {
        // Check cmd line and env flags for request
        static bool flagsChecked = false;
        if (flagsChecked)
            return;
        flagsChecked = true;

        auto isLightmapFlagSet = [](const QString &flag, const char *envVar) {
            return QCoreApplication::arguments().contains(flag)
            || qEnvironmentVariableIntValue(envVar);
        };

        m_lightmapBakingFromCmdRequested = isLightmapFlagSet(QStringLiteral("--bake-lightmaps"), "QT_QUICK3D_BAKE_LIGHTMAPS");
        m_lightmapDenoisingFromCmdRequested = isLightmapFlagSet(QStringLiteral("--denoise-lightmaps"), "QT_QUICK3D_DENOISE_LIGHTMAPS");

        if (m_lightmapBakingFromCmdRequested || m_lightmapDenoisingFromCmdRequested) {
            // Delay one frame so the render data is initialized
            QMetaObject::invokeMethod(view3D, &QQuick3DViewport::update, Qt::QueuedConnection);
        }
    }
}

void QQuick3DSceneRenderer::addNodeToLayer(QSSGRenderNode *node)
{
    if (!m_layer)
        return;

    m_layer->addChild(*node);
}

QSGRenderNode::StateFlags QQuick3DSGRenderNode::changedStates() const
{
    return BlendState | StencilState | DepthState | ScissorState | ColorState | CullState | ViewportState | RenderTargetState;
}

namespace {
inline QRect convertQtRectToGLViewport(const QRectF &rect, const QSize surfaceSize)
{
    const int x = int(rect.x());
    const int y = surfaceSize.height() - (int(rect.y()) + int(rect.height()));
    const int width = int(rect.width());
    const int height = int(rect.height());
    return QRect(x, y, width, height);
}

inline void queryMainRenderPassDescriptorAndCommandBuffer(QQuickWindow *window, QSSGRhiContext *rhiCtx)
{
    if (rhiCtx->isValid()) {
        QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
        // Query from the rif because that is available in the sync
        // phase (updatePaintNode) already.  QSGDefaultRenderContext's
        // copies of the rp and cb are not there until the render
        // phase of the scenegraph.
        int sampleCount = 1;
        int viewCount = 1;
        QRhiSwapChain *swapchain = window->swapChain();
        if (swapchain) {
            rhiCtxD->setMainRenderPassDescriptor(swapchain->renderPassDescriptor());
            rhiCtxD->setCommandBuffer(swapchain->currentFrameCommandBuffer());
            rhiCtxD->setRenderTarget(swapchain->currentFrameRenderTarget());
            sampleCount = swapchain->sampleCount();
        } else {
            QSGRendererInterface *rif = window->rendererInterface();
            // no swapchain when using a QQuickRenderControl (redirecting to a texture etc.)
            QRhiCommandBuffer *cb = static_cast<QRhiCommandBuffer *>(
                rif->getResource(window, QSGRendererInterface::RhiRedirectCommandBuffer));
            QRhiTextureRenderTarget *rt = static_cast<QRhiTextureRenderTarget *>(
                rif->getResource(window, QSGRendererInterface::RhiRedirectRenderTarget));
            if (cb && rt) {
                rhiCtxD->setMainRenderPassDescriptor(rt->renderPassDescriptor());
                rhiCtxD->setCommandBuffer(cb);
                rhiCtxD->setRenderTarget(rt);
                const auto descr = rt->description();
                const QRhiColorAttachment *color0 = descr.cbeginColorAttachments();
                if (color0 && color0->texture()) {
                    sampleCount = color0->texture()->sampleCount();
                    if (rt->resourceType() == QRhiResource::TextureRenderTarget) {
                        const QRhiTextureRenderTargetDescription desc = static_cast<QRhiTextureRenderTarget *>(rt)->description();
                        for (auto it = desc.cbeginColorAttachments(), end = desc.cendColorAttachments(); it != end; ++it) {
                            if (it->multiViewCount() >= 2) {
                                viewCount = it->multiViewCount();
                                break;
                            }
                        }
                    }
                }
            } else {
                qWarning("Neither swapchain nor redirected command buffer and render target are available.");
            }
        }

        // MSAA is out of our control on this path: it is up to the
        // QQuickWindow and the scenegraph to set up the swapchain based on the
        // QSurfaceFormat's samples(). The only thing we need to do here is to
        // pass the sample count to the renderer because it is needed when
        // creating graphics pipelines.
        rhiCtxD->setMainPassSampleCount(sampleCount);

        // The "direct renderer", i.e. the Underlay and Overlay modes are the
        // only ones that support multiview rendering. This becomes active when
        // the QQuickWindow is redirected into a texture array, typically with
        // an array size of 2 (2 views, for the left and right eye). Otherwise,
        // when targeting a window or redirected to a 2D texture, this is not
        // applicable and the view count is 1.
        rhiCtxD->setMainPassViewCount(viewCount);
    }
}

// The alternative to queryMainRenderPassDescriptorAndCommandBuffer()
// specifically for the Inline render mode when there is a QSGRenderNode.
inline void queryInlineRenderPassDescriptorAndCommandBuffer(QSGRenderNode *node, QSSGRhiContext *rhiCtx)
{
    QSGRenderNodePrivate *d = QSGRenderNodePrivate::get(node);
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    rhiCtxD->setMainRenderPassDescriptor(d->m_rt.rpDesc);
    rhiCtxD->setCommandBuffer(d->m_rt.cb);
    rhiCtxD->setRenderTarget(d->m_rt.rt);
    rhiCtxD->setMainPassSampleCount(d->m_rt.rt->sampleCount());
    rhiCtxD->setMainPassViewCount(1);
}

} // namespace

QQuick3DSGRenderNode::~QQuick3DSGRenderNode()
{
    delete renderer;
}

void QQuick3DSGRenderNode::prepare()
{
    // this is outside the main renderpass

    if (!renderer->m_sgContext->rhiContext()->isValid())
        return;
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DPrepareFrame);
    if (renderer->renderStats())
        renderer->renderStats()->startRenderPrepare();

    queryInlineRenderPassDescriptorAndCommandBuffer(this, renderer->m_sgContext->rhiContext().get());

    qreal dpr = window->effectiveDevicePixelRatio();
    const QSizeF itemSize = renderer->surfaceSize() / dpr;
    QRectF viewport = matrix()->mapRect(QRectF(QPoint(0, 0), itemSize));
    viewport = QRectF(viewport.topLeft() * dpr, viewport.size() * dpr);
    const QRect vp = convertQtRectToGLViewport(viewport, window->size() * dpr);

    Q_TRACE_SCOPE(QSSG_prepareFrame, vp.width(), vp.height());

    renderer->beginFrame();
    renderer->rhiPrepare(vp, dpr);
    Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DPrepareFrame, quint64(vp.width()) | quint64(vp.height()) << 32, renderer->profilingId);
    if (renderer->renderStats())
        renderer->renderStats()->endRenderPrepare();
}

void QQuick3DSGRenderNode::render(const QSGRenderNode::RenderState *state)
{
    Q_UNUSED(state);

    const auto &rhiContext = renderer->m_sgContext->rhiContext();

    if (rhiContext->isValid()) {
        if (renderer->renderStats())
            renderer->renderStats()->startRender();
        Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderFrame);
        Q_TRACE_SCOPE(QSSG_renderFrame, 0, 0);

        queryInlineRenderPassDescriptorAndCommandBuffer(this, renderer->m_sgContext->rhiContext().get());

        renderer->rhiRender();
        Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DRenderFrame,
                                      STAT_PAYLOAD(QSSGRhiContextStats::get(*rhiContext)), renderer->profilingId);
        renderer->endFrame();
        if (renderer->renderStats())
            renderer->renderStats()->endRender(dumpRenderTimes());
    }
}

void QQuick3DSGRenderNode::releaseResources()
{
}

QSGRenderNode::RenderingFlags QQuick3DSGRenderNode::flags() const
{
    // don't want begin/endExternal() to be called by Quick
    return NoExternalRendering;
}

QQuick3DSGDirectRenderer::QQuick3DSGDirectRenderer(QQuick3DSceneRenderer *renderer, QQuickWindow *window, QQuick3DSGDirectRenderer::QQuick3DSGDirectRendererMode mode)
    : m_renderer(renderer)
    , m_window(window)
{
    if (QSGRendererInterface::isApiRhiBased(window->rendererInterface()->graphicsApi())) {
        connect(window, &QQuickWindow::beforeRendering, this, &QQuick3DSGDirectRenderer::prepare, Qt::DirectConnection);
        if (mode == Underlay)
            connect(window, &QQuickWindow::beforeRenderPassRecording, this, &QQuick3DSGDirectRenderer::render, Qt::DirectConnection);
        else
            connect(window, &QQuickWindow::afterRenderPassRecording, this, &QQuick3DSGDirectRenderer::render, Qt::DirectConnection);
    }
}

QQuick3DSGDirectRenderer::~QQuick3DSGDirectRenderer()
{
    delete m_renderer;
}

void QQuick3DSGDirectRenderer::setViewport(const QRectF &viewport)
{
    m_viewport = viewport;
}

void QQuick3DSGDirectRenderer::setVisibility(bool visible)
{
    if (m_isVisible == visible)
        return;
    m_isVisible = visible;
    m_window->update();
}

void QQuick3DSGDirectRenderer::requestRender()
{
    renderPending = true;
    requestFullUpdate(m_window);
}

void QQuick3DSGDirectRenderer::preSynchronize()
{
    // This is called from the QQuick3DViewport's updatePaintNode(), before
    // QQuick3DSceneRenderer::synchronize(). It is essential to query things
    // such as the view count already here, so that synchronize() can rely on
    // mainPassViewCount() for instance. prepare() is too late as that is only
    // called on beforeRendering (so after the scenegraph sync phase).
    if (m_renderer->m_sgContext->rhiContext()->isValid())
        queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().get());
}

void QQuick3DSGDirectRenderer::prepare()
{
    if (!m_isVisible || !m_renderer)
        return;

    if (m_renderer->m_sgContext->rhiContext()->isValid()) {
        // this is outside the main renderpass
        if (m_renderer->m_postProcessingStack) {
            if (renderPending) {
                renderPending = false;
                m_rhiTexture = m_renderer->renderToRhiTexture(m_window);
                // Set up the main render target again, e.g. the postprocessing
                // stack could have clobbered some settings such as the sample count.
                queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().get());
                const auto &quadRenderer = m_renderer->m_sgContext->renderer()->rhiQuadRenderer();
                quadRenderer->prepareQuad(m_renderer->m_sgContext->rhiContext().get(), nullptr);
                if (m_renderer->m_requestedFramesCount > 0) {
                    requestRender();
                    m_renderer->m_requestedFramesCount--;
                }
            }
        }
        else
        {
            QQuick3DRenderStats *renderStats = m_renderer->renderStats();
            if (renderStats)
                renderStats->startRenderPrepare();

            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DPrepareFrame);
            queryMainRenderPassDescriptorAndCommandBuffer(m_window, m_renderer->m_sgContext->rhiContext().get());
            const QRect vp = convertQtRectToGLViewport(m_viewport, m_window->size() * m_window->effectiveDevicePixelRatio());

            Q_TRACE_SCOPE(QSSG_prepareFrame, vp.width(), vp.height());
            m_renderer->beginFrame();
            m_renderer->rhiPrepare(vp, m_window->effectiveDevicePixelRatio());
            Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DPrepareFrame, quint64(vp.width()) | quint64(vp.height()) << 32, m_renderer->profilingId);

            if (renderStats)
                renderStats->endRenderPrepare();
        }
    }
}

void QQuick3DSGDirectRenderer::render()
{
    if (!m_isVisible || !m_renderer)
        return;

    const auto &rhiContext = m_renderer->m_sgContext->rhiContext();

    if (rhiContext->isValid()) {
        // the command buffer is recording the main renderpass at this point

        // No m_window->beginExternalCommands() must be done here. When the
        // renderer is using the same
        // QRhi/QRhiCommandBuffer/QRhiRenderPassDescriptor as the Qt Quick
        // scenegraph, there is no difference from the RHI's perspective. There are
        // no external (native) commands here.

        // Requery the command buffer and co. since Offscreen mode View3Ds may
        // have altered these on the context.
        if (m_renderer->m_postProcessingStack) {
            if (m_rhiTexture) {
                queryMainRenderPassDescriptorAndCommandBuffer(m_window, rhiContext.get());
                auto rhiCtx = m_renderer->m_sgContext->rhiContext().get();
                const auto &renderer = m_renderer->m_sgContext->renderer();
                QRhiCommandBuffer *cb = rhiContext->commandBuffer();
                cb->debugMarkBegin(QByteArrayLiteral("Post-processing result to main rt"));

                // Instead of passing in a flip flag we choose to rely on qsb's
                // per-target compilation mode in the fragment shader. (it does UV
                // flipping based on QSHADER_ macros) This is just better for
                // performance and the shaders are very simple so introducing a
                // uniform block and branching dynamically would be an overkill.
                QRect vp = convertQtRectToGLViewport(m_viewport, m_window->size() * m_window->effectiveDevicePixelRatio());

                const auto &shaderCache = m_renderer->m_sgContext->shaderCache();
                const auto &shaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiSimpleQuadShader(m_renderer->m_layer->viewCount);

                QRhiSampler *sampler = rhiCtx->sampler({ QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge });
                QSSGRhiShaderResourceBindingList bindings;
                bindings.addTexture(0, QRhiShaderResourceBinding::FragmentStage, m_rhiTexture, sampler);
                QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiContext.get());
                QRhiShaderResourceBindings *srb = rhiCtxD->srb(bindings);

                QSSGRhiGraphicsPipelineState ps;
                ps.viewport = QRhiViewport(float(vp.x()), float(vp.y()), float(vp.width()), float(vp.height()));
                ps.samples = rhiCtx->mainPassSampleCount();
                ps.viewCount = m_renderer->m_layer->viewCount;
                QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(ps, shaderPipeline.get());
                renderer->rhiQuadRenderer()->recordRenderQuad(rhiCtx, &ps, srb, rhiCtx->mainRenderPassDescriptor(), QSSGRhiQuadRenderer::UvCoords | QSSGRhiQuadRenderer::PremulBlend);
                cb->debugMarkEnd();
            }
        }
        else
        {
            Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderFrame);
            Q_TRACE_SCOPE(QSSG_renderFrame, 0, 0);
            if (m_renderer->renderStats())
                m_renderer->renderStats()->startRender();

            queryMainRenderPassDescriptorAndCommandBuffer(m_window, rhiContext.get());

            m_renderer->rhiRender();

            Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DRenderFrame,
                                          STAT_PAYLOAD(QSSGRhiContextStats::get(*rhiContext)),
                                          m_renderer->profilingId);
            m_renderer->endFrame();

            if (m_renderer->renderStats())
                m_renderer->renderStats()->endRender(dumpRenderTimes());
        }
    }
}

QT_END_NAMESPACE
