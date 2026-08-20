// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "outlinerenderextension.h"

#include <rhi/qrhi.h>

#include <QtQuick3D/qquick3dobject.h>
#include <ssg/qquick3dextensionhelpers.h>

#include <ssg/qssgrenderhelpers.h>
#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrendercontextcore.h>

//! [extension back]
class OutlineRenderer : public QSSGRenderExtension
{
public:
    OutlineRenderer() = default;

    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return RenderMode::Main; }
    RenderStage stage() const override { return RenderStage::PostColor; };

    QSSGPrepContextId stencilPrepContext { QSSGPrepContextId::Invalid };
    QSSGPrepContextId outlinePrepContext { QSSGPrepContextId::Invalid };
    QSSGPrepResultId stencilPrepResult { QSSGPrepResultId::Invalid };
    QSSGPrepResultId outlinePrepResult { QSSGPrepResultId::Invalid };
    QPointer<QQuick3DObject> model;
    QSSGNodeId modelId { QSSGNodeId::Invalid };
    QPointer<QQuick3DObject> material;
    QSSGResourceId outlineMaterialId {};
    float outlineScale = 1.05f;

    QSSGRenderablesId stencilRenderables;
    QSSGRenderablesId outlineRenderables;
};
//! [extension back]

//! [extension back_prepareData]
bool OutlineRenderer::prepareData(QSSGFrameData &data)
{
    // Make sure we have a model and a material.
    if (!model || !material)
        return false;

    modelId = QQuick3DExtensionHelpers::getNodeId(*model);
    if (modelId == QSSGNodeId::Invalid)
        return false;

    outlineMaterialId = QQuick3DExtensionHelpers::getResourceId(*material);
    if (outlineMaterialId == QSSGResourceId::Invalid)
        return false;

    // This is the active camera for the scene (the camera used to render the QtQuick3D scene)
    QSSGCameraId camera = data.activeCamera();
    if (camera == QSSGCameraId::Invalid)
        return false;

    // We are going to render the same renderable(s) twice so we need to create two contexts.
    stencilPrepContext = QSSGRenderHelpers::prepareForRender(data, *this, camera, 0);
    outlinePrepContext = QSSGRenderHelpers::prepareForRender(data, *this, camera, 1);
    // Create the renderables for the target model. One for the original with stencil write, and one for the outline model.
    // Note that we 'Steal' the model here, that tells QtQuick3D that we'll take over the rendering of the model.
    stencilRenderables = QSSGRenderHelpers::createRenderables(data, stencilPrepContext, { modelId }, QSSGRenderHelpers::CreateFlag::Steal);
    outlineRenderables = QSSGRenderHelpers::createRenderables(data, outlinePrepContext, { modelId });

    // Now we can start setting the data for our models.
    // Here we set a material and a scale for the outline
    QSSGModelHelpers::setModelMaterials(data, outlineRenderables, modelId, { outlineMaterialId });
    QMatrix4x4 globalTransform = QSSGModelHelpers::getGlobalTransform(data, modelId);
    globalTransform.scale(outlineScale);
    QSSGModelHelpers::setGlobalTransform(data, outlineRenderables, modelId, globalTransform);

    // When all changes are done, we need to commit the changes.
    stencilPrepResult = QSSGRenderHelpers::commit(data, stencilPrepContext, stencilRenderables);
    outlinePrepResult = QSSGRenderHelpers::commit(data, outlinePrepContext, outlineRenderables);

    // If there's something to be rendered we return true.
    const bool dataReady = (stencilPrepResult != QSSGPrepResultId::Invalid && outlinePrepResult != QSSGPrepResultId::Invalid);

    return dataReady;
}
//! [extension back_prepareData]

//! [extension back_prepareRender]
void OutlineRenderer::prepareRender(QSSGFrameData &data)
{
    Q_ASSERT(modelId != QSSGNodeId::Invalid);
    Q_ASSERT(stencilPrepResult != QSSGPrepResultId::Invalid && outlinePrepResult != QSSGPrepResultId::Invalid);

    const auto &ctx = data.contextInterface();

    if (const auto &rhiCtx = ctx->rhiContext()) {
        const QSSGRhiGraphicsPipelineState basePs = data.getPipelineState();
        QRhiRenderPassDescriptor *rpDesc = rhiCtx->mainRenderPassDescriptor();
        const int samples = rhiCtx->mainPassSampleCount();


        { // Original model - Write to the stencil buffer.
            QSSGRhiGraphicsPipelineState ps = basePs;
            ps.flags |= { QSSGRhiGraphicsPipelineState::Flag::BlendEnabled,
                          QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled,
                          QSSGRhiGraphicsPipelineState::Flag::UsesStencilRef,
                          QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled };
            ps.stencilWriteMask = 0xff;
            ps.stencilRef = 1;
            ps.samples = samples;
            ps.cullMode = QRhiGraphicsPipeline::Back;

            ps.stencilOpFrontState = { QRhiGraphicsPipeline::Keep,
                                       QRhiGraphicsPipeline::Keep,
                                       QRhiGraphicsPipeline::Replace,
                                       QRhiGraphicsPipeline::Always };

            QSSGRenderHelpers::prepareRenderables(data, stencilPrepResult, rpDesc, ps);
        }

        { // Scaled version - Only draw outside the original.
            QSSGRhiGraphicsPipelineState ps = basePs;
            ps.flags |= { QSSGRhiGraphicsPipelineState::Flag::BlendEnabled,
                          QSSGRhiGraphicsPipelineState::Flag::UsesStencilRef,
                          QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled };
            ps.flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, false);
            ps.stencilWriteMask = 0;
            ps.stencilRef = 1;
            ps.cullMode = QRhiGraphicsPipeline::Back;

            ps.stencilOpFrontState = { QRhiGraphicsPipeline::Keep,
                                       QRhiGraphicsPipeline::Keep,
                                       QRhiGraphicsPipeline::Replace,
                                       QRhiGraphicsPipeline::NotEqual };

            QSSGRenderHelpers::prepareRenderables(data, outlinePrepResult, rpDesc, ps);
        }
    }
}
//! [extension back_prepareRender]

//! [extension back_render]
void OutlineRenderer::render(QSSGFrameData &data)
{
    Q_ASSERT(stencilPrepResult != QSSGPrepResultId::Invalid);

    const auto &ctx = data.contextInterface();
    if (const auto &rhiCtx = ctx->rhiContext()) {
        QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
        cb->debugMarkBegin(QByteArrayLiteral("Stencil outline pass"));
        QSSGRenderHelpers::renderRenderables(data, stencilPrepResult);
        QSSGRenderHelpers::renderRenderables(data, outlinePrepResult);
        cb->debugMarkEnd();
    }
}
//! [extension back_render]

void OutlineRenderer::resetForFrame()
{
    stencilPrepContext = { QSSGPrepContextId::Invalid };
    stencilPrepResult = { QSSGPrepResultId::Invalid };
}

OutlineRenderExtension::~OutlineRenderExtension() {}

float OutlineRenderExtension::outlineScale() const
{
    return m_outlineScale;
}

void OutlineRenderExtension::setOutlineScale(float newOutlineScale)
{
    if (qFuzzyCompare(m_outlineScale, newOutlineScale))
        return;
    m_outlineScale = newOutlineScale;

    markDirty(Dirty::OutlineScale);

    emit outlineScaleChanged();
}

QQuick3DObject *OutlineRenderExtension::target() const
{
    return m_target;
}

void OutlineRenderExtension::setTarget(QQuick3DObject *newTarget)
{
    if (m_target == newTarget)
        return;
    m_target = newTarget;

    markDirty(Dirty::Target);

    emit targetChanged();
}

QSSGRenderGraphObject *OutlineRenderExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new OutlineRenderer;

    OutlineRenderer *renderer = static_cast<OutlineRenderer *>(node);
    renderer->outlineScale = m_outlineScale;
    renderer->model = m_target;
    renderer->material = m_outlineMaterial;

    m_dirtyFlag = {};

    return node;
}

void OutlineRenderExtension::markDirty(Dirty v)
{
    m_dirtyFlag |= v;
    update();
}

QQuick3DObject *OutlineRenderExtension::outlineMaterial() const
{
    return m_outlineMaterial;
}

void OutlineRenderExtension::setOutlineMaterial(QQuick3DObject *newOutlineMaterial)
{
    if (m_outlineMaterial == newOutlineMaterial)
        return;

    m_outlineMaterial = newOutlineMaterial;

    markDirty(Dirty::OutlineMaterial);
    emit outlineMaterialChanged();
}
