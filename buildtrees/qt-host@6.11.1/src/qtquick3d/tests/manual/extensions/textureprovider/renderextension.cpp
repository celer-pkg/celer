// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "renderextension.h"

#include <QtCore/qpointer.h>
#include <QtCore/qfile.h>

#include <rhi/qshader.h>
#include <rhi/qrhi.h>

#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrenderhelpers.h>
#include <ssg/qssgrendercontextcore.h>
#include <ssg/qssgrhicontext.h>
#include <ssg/qquick3dextensionhelpers.h>

static constexpr float triangle[] = {
    0.0f,  0.5f, 1.0f, 0.0f, 0.0f,
   -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
};

class TextureProvider : public QSSGRenderExtension
{
public:
    explicit TextureProvider(RenderExtension *ext);
    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return QSSGRenderExtension::RenderMode::Standalone; }
    RenderStage stage() const override { return QSSGRenderExtension::RenderStage::PreColor; }

private:
    static QShader getShader(const QString &name)
    {
        QFile f(name);
        return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
    }

    QPointer<RenderExtension> m_ext;
    QSSGExtensionId m_extensionId {};

    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiTextureRenderTarget> rt;
    std::unique_ptr<QRhiRenderPassDescriptor> rpDesc;

    std::unique_ptr<QRhiBuffer> vbuf;
    std::unique_ptr<QRhiBuffer> ubuf;
    std::unique_ptr<QRhiShaderResourceBindings> srb;
    std::unique_ptr<QRhiGraphicsPipeline> ps;

    float rotation = 0.0f;
};

TextureProvider::TextureProvider(RenderExtension *ext)
    : m_ext(ext)
{
}

bool TextureProvider::prepareData(QSSGFrameData &data)
{
    Q_UNUSED(data);

    // Update the extension ID.
    // NOTE: This needs to be done on each call, the extension object may have changed.
    //       If the extension is destroyed (m_ext == nullptr), we cannot proceed and should return false!
    m_extensionId = m_ext ? QQuick3DExtensionHelpers::getExtensionId(*m_ext) : QSSGExtensionId{};
    if (QQuick3DExtensionHelpers::isNull(m_extensionId))
        return false;

    return true;
}

void TextureProvider::prepareRender(QSSGFrameData &data)
{
    const auto &ctxIfx = data.contextInterface();
    const auto &rhiCtx = ctxIfx->rhiContext();
    if (!rhiCtx)
        return;

    QRhi *rhi = rhiCtx->rhi();
    if (!texture) {
        texture.reset(rhi->newTexture(QRhiTexture::RGBA8, QSize(512, 512), 1, QRhiTexture::RenderTarget));
        texture->create();

        rt.reset(rhi->newTextureRenderTarget({ texture.get() }));
        rpDesc.reset(rt->newCompatibleRenderPassDescriptor());
        rt->setRenderPassDescriptor(rpDesc.get());
        rt->create();

        QSSGRenderExtensionHelpers::registerRenderResult(data, m_extensionId, texture.get());
    }

    if (!ps) {
        vbuf.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(triangle)));
        vbuf->create();

        ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
        ubuf->create();

        srb.reset(rhi->newShaderResourceBindings());
        srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, ubuf.get()),
        });
        srb->create();

        ps.reset(rhi->newGraphicsPipeline());
        ps->setShaderStages({
                { QRhiShaderStage::Vertex, getShader(QLatin1String(":/shaders/color.vert.qsb")) },
                { QRhiShaderStage::Fragment, getShader(QLatin1String(":/shaders/color.frag.qsb")) }
        });
        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({
                { 5 * sizeof(float) }
        });
        inputLayout.setAttributes({
                { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
                { 0, 1, QRhiVertexInputAttribute::Float3, 2 * sizeof(float) }
        });
        ps->setVertexInputLayout(inputLayout);
        ps->setShaderResourceBindings(srb.get());
        ps->setRenderPassDescriptor(rpDesc.get());
        ps->create();

        QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
        QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();
        resourceUpdates->uploadStaticBuffer(vbuf.get(), triangle);
        cb->resourceUpdate(resourceUpdates);
    }
}

void TextureProvider::render(QSSGFrameData &data)
{
    const auto &ctxIfx = data.contextInterface();
    const auto &rhiCtx = ctxIfx->rhiContext();
    if (!rhiCtx)
        return;

    QRhi *rhi = rhiCtx->rhi();
    QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();
    const QSize outputSize = rt->pixelSize();
    QMatrix4x4 viewProjection = rhi->clipSpaceCorrMatrix();
    viewProjection.perspective(45.0f, outputSize.width() / (float) outputSize.height(), 0.01f, 1000.0f);
    viewProjection.translate(0, 0, -4);
    QMatrix4x4 modelViewProjection = viewProjection;
    modelViewProjection.rotate(rotation, 0, 1, 0);
    resourceUpdates->updateDynamicBuffer(ubuf.get(), 0, 64, modelViewProjection.constData());

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    const QColor clearColor = QColor::fromRgbF(0.4f, 0.7f, 0.0f, 1.0f);
    cb->beginPass(rt.get(), clearColor, { 1.0f, 0}, resourceUpdates);

    cb->setGraphicsPipeline(ps.get());
    cb->setViewport(QRhiViewport(0, 0, outputSize.width(), outputSize.height()));
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput vbufBinding(vbuf.get(), 0);
    cb->setVertexInput(0, 1, &vbufBinding);
    cb->draw(3);

    cb->endPass();
}

void TextureProvider::resetForFrame()
{

}


RenderExtension::RenderExtension()
{

}

QSSGRenderGraphObject *RenderExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    TextureProvider *renderNode = static_cast<TextureProvider *>(node);
    if (!renderNode)
        renderNode = new TextureProvider(this);

    return renderNode;
}


