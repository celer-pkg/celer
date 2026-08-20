// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrhiquadrenderer_p.h>
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>

QT_BEGIN_NAMESPACE

static const QVector3D g_fullScreenRectFaces[] = {
    QVector3D(-1, -1, 0),
    QVector3D(-1, 1, 0),
    QVector3D(1, 1, 0),
    QVector3D(1, -1, 0),

    QVector3D(-1, -1, 1),
    QVector3D(-1, 1, 1),
    QVector3D(1, 1, 1),
    QVector3D(1, -1, 1),

    QVector3D(-1, -1, -1),
    QVector3D(-1, 1, -1),
    QVector3D(1, 1, -1),
    QVector3D(1, -1, -1),
};

static const QVector2D g_fullScreenRectUVs[] = {
    QVector2D(0, 0),
    QVector2D(0, 1),
    QVector2D(1, 1),
    QVector2D(1, 0)
};

static const quint16 g_rectIndex[] = {
    0, 1, 2, 0, 2, 3, // front face - 0, 1, 2, 3
    0, 4, 5, 0, 5, 1, // left face - 0, 4, 5, 1
    1, 5, 6, 1, 6, 2, // top face - 1, 5, 6, 2
    3, 2, 6, 3, 6, 7, // right face - 3, 2, 6, 7
    0, 3, 7, 0, 7, 4, // bottom face - 0, 3, 7, 4
    7, 6, 5, 7, 5, 4  // back face - 7, 6, 5, 4
};

void QSSGRhiQuadRenderer::ensureBuffers(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *rub)
{
    if (!m_vbuf) {
        constexpr int vertexCount = 8;
        m_vbuf = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                 QRhiBuffer::Immutable,
                                                 QRhiBuffer::VertexBuffer,
                                                 quint32(5 * sizeof(float)),
                                                 5 * vertexCount * sizeof(float));
        m_vbuf->buffer()->setName(QByteArrayLiteral("quad vertex buffer"));
        float buf[5 * vertexCount];
        float *p = buf;
        for (int i = 0; i < vertexCount; ++i) {
            *p++ = g_fullScreenRectFaces[i].x();
            *p++ = g_fullScreenRectFaces[i].y();
            *p++ = g_fullScreenRectFaces[i].z();
            *p++ = g_fullScreenRectUVs[i % 4].x();
            *p++ = g_fullScreenRectUVs[i % 4].y();
        }
        rub->uploadStaticBuffer(m_vbuf->buffer(), buf);
    }
    if (!m_ibuf) {
        m_ibuf = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                 QRhiBuffer::Immutable,
                                                 QRhiBuffer::IndexBuffer,
                                                 0,
                                                 6 * sizeof(quint16),
                                                 QRhiCommandBuffer::IndexUInt16);
        m_ibuf->buffer()->setName(QByteArrayLiteral("quad index buffer"));
        const quint16 buf[] = { 0, 1, 2, 0, 2, 3 };
        rub->uploadStaticBuffer(m_ibuf->buffer(), buf);
    }
}

void QSSGRhiQuadRenderer::prepareQuad(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *maybeRub)
{
    QRhiResourceUpdateBatch *rub = maybeRub ? maybeRub : rhiCtx->rhi()->nextResourceUpdateBatch();
    ensureBuffers(rhiCtx, rub);
    rhiCtx->commandBuffer()->resourceUpdate(rub);
}

void QSSGRhiQuadRenderer::recordRenderQuad(QSSGRhiContext *rhiCtx,
                                           QSSGRhiGraphicsPipelineState *ps, QRhiShaderResourceBindings *srb,
                                           QRhiRenderPassDescriptor *rpDesc, Flags flags)
{
    // ps must have viewport and shaderPipeline set already
    auto &ia = QSSGRhiInputAssemblerStatePrivate::get(*ps);
    if (flags.testFlag(UvCoords)) {
        ia.inputLayout.setAttributes({
                                            { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
                                            { 0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float) }
                                        });
        ia.inputs << QSSGRhiInputAssemblerState::PositionSemantic << QSSGRhiInputAssemblerState::TexCoord0Semantic;
    } else {
        ia.inputLayout.setAttributes({ { 0, 0, QRhiVertexInputAttribute::Float3, 0 } });
        ia.inputs << QSSGRhiInputAssemblerState::PositionSemantic;
    }
    ia.inputLayout.setBindings({ 5 * sizeof(float) });
    ia.topology = QRhiGraphicsPipeline::Triangles;

    ps->flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled, flags.testFlag(DepthTest));
    ps->flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, flags.testFlag(DepthWrite));
    ps->cullMode = QRhiGraphicsPipeline::None;
    if (flags.testFlag(PremulBlend)) {
        ps->flags |= QSSGRhiGraphicsPipelineState::Flag::BlendEnabled;
        ps->targetBlend[0].srcColor = QRhiGraphicsPipeline::One;
        ps->targetBlend[0].dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        ps->targetBlend[0].srcAlpha = QRhiGraphicsPipeline::One;
        ps->targetBlend[0].dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    } else { // set to default, since we may not have had a renderable previously
        ps->targetBlend[0].srcColor = QRhiGraphicsPipeline::SrcAlpha;
        ps->targetBlend[0].dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        ps->targetBlend[0].srcAlpha = QRhiGraphicsPipeline::One;
        ps->targetBlend[0].dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    }

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    QRhiGraphicsPipeline *pipeline = rhiCtxD->pipeline(*ps, rpDesc, srb);
    // Make sure that we were able to create the pipeline before trying to use it
    // When GraphicsPipeline creation fails it should return nullptr and print a warning
    if (!pipeline)
        return;

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->setGraphicsPipeline(pipeline);
    cb->setShaderResources(srb);
    cb->setViewport(ps->viewport);

    quint32 vertexOffset = flags.testAnyFlags(RenderBehind) ? 5 * 4 * sizeof(float) : 0;

    QRhiCommandBuffer::VertexInput vb(m_vbuf->buffer(), vertexOffset);
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);
    cb->setVertexInput(0, 1, &vb, m_ibuf->buffer(), m_ibuf->indexFormat());
    cb->drawIndexed(6);
    QSSGRHICTX_STAT(rhiCtx, drawIndexed(6, 1));
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderCall, 36llu | (1llu << 32), QByteArrayLiteral("render_quad"));
}

void QSSGRhiQuadRenderer::recordRenderQuadPass(QSSGRhiContext *rhiCtx,
                                               QSSGRhiGraphicsPipelineState *ps, QRhiShaderResourceBindings *srb,
                                               QRhiTextureRenderTarget *rt, Flags flags)
{
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->beginPass(rt, Qt::black, { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
    QSSGRHICTX_STAT(rhiCtx, beginRenderPass(rt));
    recordRenderQuad(rhiCtx, ps, srb, rt->renderPassDescriptor(), flags);
    cb->endPass();
    QSSGRHICTX_STAT(rhiCtx, endRenderPass());
}

void QSSGRhiCubeRenderer::prepareCube(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *maybeRub)
{
    QRhiResourceUpdateBatch *rub = maybeRub ? maybeRub : rhiCtx->rhi()->nextResourceUpdateBatch();
    ensureBuffers(rhiCtx, rub);
    rhiCtx->commandBuffer()->resourceUpdate(rub);
}

//### The flags UvCoords and RenderBehind are ignored
void QSSGRhiCubeRenderer::recordRenderCube(QSSGRhiContext *rhiCtx, QSSGRhiGraphicsPipelineState *ps, QRhiShaderResourceBindings *srb, QRhiRenderPassDescriptor *rpDesc, QSSGRhiQuadRenderer::Flags flags)
{
    auto &ia = QSSGRhiInputAssemblerStatePrivate::get(*ps);
    // ps must have viewport and shaderPipeline set already
    ia.inputLayout.setAttributes({ { 0, 0, QRhiVertexInputAttribute::Float3, 0 } });
    ia.inputs << QSSGRhiInputAssemblerState::PositionSemantic;
    ia.inputLayout.setBindings({ 3 * sizeof(float) });
    ia.topology = QRhiGraphicsPipeline::Triangles;

    ps->flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled, flags.testFlag(QSSGRhiQuadRenderer::DepthTest));
    ps->flags.setFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled, flags.testFlag(QSSGRhiQuadRenderer::DepthWrite));
    ps->cullMode = QRhiGraphicsPipeline::None;
    if (flags.testFlag(QSSGRhiQuadRenderer::PremulBlend)) {
        ps->flags |= QSSGRhiGraphicsPipelineState::Flag::BlendEnabled;
        ps->targetBlend[0].srcColor = QRhiGraphicsPipeline::One;
        ps->targetBlend[0].dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        ps->targetBlend[0].srcAlpha = QRhiGraphicsPipeline::One;
        ps->targetBlend[0].dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    } else { // set to default, since we may not have had a renderable previously
        ps->targetBlend[0].srcColor = QRhiGraphicsPipeline::SrcAlpha;
        ps->targetBlend[0].dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        ps->targetBlend[0].srcAlpha = QRhiGraphicsPipeline::One;
        ps->targetBlend[0].dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
    }

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    QRhiGraphicsPipeline *pipeline = rhiCtxD->pipeline(*ps, rpDesc, srb);
    // Make sure that we were able to create the pipeline before trying to use it
    // When GraphicsPipeline creation fails it should return nullptr and print a warning
    if (!pipeline)
        return;

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    cb->setGraphicsPipeline(pipeline);
    cb->setShaderResources(srb);
    cb->setViewport(ps->viewport);

    QRhiCommandBuffer::VertexInput vb(m_vbuf->buffer(), 0);
    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DRenderCall);
    cb->setVertexInput(0, 1, &vb, m_ibuf->buffer(), m_ibuf->indexFormat());
    cb->drawIndexed(36);
    QSSGRHICTX_STAT(rhiCtx, drawIndexed(36, 1));
    Q_QUICK3D_PROFILE_END_WITH_STRING(QQuick3DProfiler::Quick3DRenderCall, 36, QByteArrayLiteral("render_cube"));
}

void QSSGRhiCubeRenderer::ensureBuffers(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *rub)
{
    if (!m_vbuf) {
        constexpr int vertexCount = 8;
        m_vbuf = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                 QRhiBuffer::Immutable,
                                                 QRhiBuffer::VertexBuffer,
                                                 quint32(3 * sizeof(float)),
                                                 3 * vertexCount * sizeof(float));
        m_vbuf->buffer()->setName(QByteArrayLiteral("cube vertex buffer"));

        float buf[3 * vertexCount];
        float *p = buf;
        for (int i = 0; i < vertexCount; ++i) {
            *p++ = g_fullScreenRectFaces[4 + i].x();
            *p++ = g_fullScreenRectFaces[4 + i].y();
            *p++ = g_fullScreenRectFaces[4 + i].z();
        }
        rub->uploadStaticBuffer(m_vbuf->buffer(), buf);
    }
    if (!m_ibuf) {
        m_ibuf = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                 QRhiBuffer::Immutable,
                                                 QRhiBuffer::IndexBuffer,
                                                 0,
                                                 sizeof(g_rectIndex),
                                                 QRhiCommandBuffer::IndexUInt16);
        m_ibuf->buffer()->setName(QByteArrayLiteral("cube index buffer"));
        rub->uploadStaticBuffer(m_ibuf->buffer(), g_rectIndex);
    }
}

QT_END_NAMESPACE
