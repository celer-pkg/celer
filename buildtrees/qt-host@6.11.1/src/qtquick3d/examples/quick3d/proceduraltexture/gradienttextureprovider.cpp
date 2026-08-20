// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "gradienttextureprovider.h"

#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrenderhelpers.h>
#include <ssg/qssgrendercontextcore.h>
#include <ssg/qssgrhicontext.h>
#include <ssg/qquick3dextensionhelpers.h>

#include <QFile>
#include <QColorSpace>

static constexpr float g_vertexData[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
};

static constexpr uint16_t g_indexData[] = {
    0, 1, 2,
    0, 2, 3
};

static QShader getShader(const QString &name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

//! [GradientTextureProviderNode]
class GradientTextureProviderNode : public QSSGRenderTextureProviderExtension
{
public:
    explicit GradientTextureProviderNode(GradientTextureProvider *ext);
    ~GradientTextureProviderNode() override;
    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;

    bool m_isDirty = false;

    // state
    int m_width = 256;
    int m_height = 256;
    QColor m_startColor = QColor(Qt::red);
    QColor m_endColor = QColor(Qt::blue);

private:

    QPointer<GradientTextureProvider> m_ext;
    QSSGExtensionId extensionId {};

    std::unique_ptr<QRhiBuffer> quadGeometryVertexBuffer;
    std::unique_ptr<QRhiBuffer> quadGeometryIndexBuffer;

    //
    std::unique_ptr<QRhiTexture> outputTexture; // the final output texture
    std::unique_ptr<QRhiTextureRenderTarget> outputTextureRenderTarget;
    std::unique_ptr<QRhiRenderPassDescriptor> ouputTextureRenderPassDescriptor;

    std::unique_ptr<QRhiBuffer> gradientTextureUniformBuffer;
    std::unique_ptr<QRhiShaderResourceBindings> gradientTextureShaderResouceBindings;
    std::unique_ptr<QRhiGraphicsPipeline> gradientTexture2dPipeline;
};
//! [GradientTextureProviderNode]

GradientTextureProviderNode::GradientTextureProviderNode(GradientTextureProvider *ext)
    : m_ext(ext)
{
}

GradientTextureProviderNode::~GradientTextureProviderNode()
{

}

//! [prepareData]
bool GradientTextureProviderNode::prepareData(QSSGFrameData &data)
{
    if (!m_isDirty)
        return false;

    const auto &ctxIfx = data.contextInterface();
    const auto &rhiCtx = ctxIfx->rhiContext();
    QRhi *rhi = rhiCtx->rhi();

    // If there is no available rhi context, then we can't create the texture
    if (!rhiCtx)
        return false;

    extensionId = m_ext ? QQuick3DExtensionHelpers::getExtensionId(*m_ext) : QSSGExtensionId{};
    if (QQuick3DExtensionHelpers::isNull(extensionId))
        return false;

    // Make sure that the output texture is created and registered as the texture provider
    if (!outputTexture ||
        outputTexture->pixelSize().width() != m_width ||
        outputTexture->pixelSize().height() != m_height) {
        outputTexture.reset(rhi->newTexture(QRhiTexture::Format::RGBA8, QSize(m_width, m_height), 1, QRhiTexture::RenderTarget | QRhiTexture::sRGB));
        outputTexture->create();

        outputTextureRenderTarget.reset(rhi->newTextureRenderTarget({ outputTexture.get() }));
        ouputTextureRenderPassDescriptor.reset(outputTextureRenderTarget->newCompatibleRenderPassDescriptor());
        outputTextureRenderTarget->setRenderPassDescriptor(ouputTextureRenderPassDescriptor.get());
        outputTextureRenderTarget->create();

        // Register the output as the texture provider
        QSSGRenderExtensionHelpers::registerRenderResult(data, extensionId, outputTexture.get());

        gradientTexture2dPipeline.reset();
    }

    // If m_isDirty is true than prepareRender and render will actually get called.
    return m_isDirty;
}
//! [prepareData]


//! [prepareRender]
void GradientTextureProviderNode::prepareRender(QSSGFrameData &data)
{
    const auto &ctxIfx = data.contextInterface();
    const auto &rhiCtx = ctxIfx->rhiContext();
    if (!rhiCtx)
        return;

    QRhi *rhi = rhiCtx->rhi();
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();

    // Create the pipeline if necessary
    if (!gradientTexture2dPipeline) {
        // 1 quad (2 trianges), pos + uv.  4 vertices, 5 values each (x, y, z, u, v)
        quadGeometryVertexBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, 5 * 4 * sizeof(float)));
        quadGeometryVertexBuffer->create();

        // 6 indexes (2 triangles)
        quadGeometryIndexBuffer.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, 6 * sizeof(uint16_t)));
        quadGeometryIndexBuffer->create();

        // Uniform buffer: packed into 2 * vec4 (2 RGBA colors)
        const size_t uBufSize = (sizeof(float) * 4) * 2;
        gradientTextureUniformBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, uBufSize));
        gradientTextureUniformBuffer->create();

        // Uniform buffer is only bound/used in Fragment Shader
        gradientTextureShaderResouceBindings.reset(rhi->newShaderResourceBindings());
        gradientTextureShaderResouceBindings->setBindings({
                QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, gradientTextureUniformBuffer.get()),
        });
        gradientTextureShaderResouceBindings->create();

        gradientTexture2dPipeline.reset(rhi->newGraphicsPipeline());
        gradientTexture2dPipeline->setShaderStages({
                { QRhiShaderStage::Vertex, getShader(QLatin1String(":/shaders/gradient.vert.qsb")) },
                { QRhiShaderStage::Fragment, getShader(QLatin1String(":/shaders/gradient.frag.qsb")) }
        });
        // 2 Attributes, Position (vec3) + UV (vec2)
        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({
                { 5 * sizeof(float) }
        });
        inputLayout.setAttributes({
                { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
                { 0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float) }
        });
        gradientTexture2dPipeline->setVertexInputLayout(inputLayout);
        gradientTexture2dPipeline->setShaderResourceBindings(gradientTextureShaderResouceBindings.get());
        gradientTexture2dPipeline->setRenderPassDescriptor(ouputTextureRenderPassDescriptor.get());
        gradientTexture2dPipeline->create();

        // Upload the static quad geometry part
        resourceUpdates->uploadStaticBuffer(quadGeometryVertexBuffer.get(), g_vertexData);
        resourceUpdates->uploadStaticBuffer(quadGeometryIndexBuffer.get(), g_indexData);
    }

    // Upload the uniform buffer data
    const float colorData[8] = {
        m_startColor.redF(), m_startColor.greenF(), m_startColor.blueF(), m_startColor.alphaF(),
        m_endColor.redF(), m_endColor.greenF(), m_endColor.blueF(), m_endColor.alphaF()
    };
    resourceUpdates->updateDynamicBuffer(gradientTextureUniformBuffer.get(), 0, sizeof(colorData), colorData);

    cb->resourceUpdate(resourceUpdates);
    m_isDirty = false;
}
//! [prepareRender]

//! [render]
void GradientTextureProviderNode::render(QSSGFrameData &data)
{
    const auto &ctxIfx = data.contextInterface();
    const auto &rhiCtx = ctxIfx->rhiContext();
    if (!rhiCtx)
        return;

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    // Render the quad with our pipeline to the outputTexture
    cb->beginPass(outputTextureRenderTarget.get(), Qt::black, { 1.0f, 0 }, nullptr, rhiCtx->commonPassFlags());
    cb->setViewport(QRhiViewport(0, 0, m_width, m_height));
    cb->setGraphicsPipeline(gradientTexture2dPipeline.get());
    cb->setShaderResources(gradientTextureShaderResouceBindings.get());
    QRhiCommandBuffer::VertexInput vb(quadGeometryVertexBuffer.get(), 0);
    cb->setVertexInput(0, 1, &vb, quadGeometryIndexBuffer.get(), QRhiCommandBuffer::IndexFormat::IndexUInt16);
    cb->drawIndexed(6);
    cb->endPass();
}
//! [render]

void GradientTextureProviderNode::resetForFrame()
{

}


GradientTextureProvider::GradientTextureProvider() { }

int GradientTextureProvider::height() const
{
    return m_height;
}

void GradientTextureProvider::setHeight(int newHeight)
{
    if (m_height == newHeight)
        return;
    m_height = newHeight;
    emit heightChanged();
    update();
}

int GradientTextureProvider::width() const
{
    return m_width;
}

void GradientTextureProvider::setWidth(int newWidth)
{
    if (m_width == newWidth)
        return;
    m_width = newWidth;
    emit widthChanged();
    update();
}

QColor GradientTextureProvider::startColor() const
{
    return m_startColor;
}

void GradientTextureProvider::setStartColor(const QColor &newStartColor)
{
    if (m_startColor == newStartColor)
        return;
    m_startColor = newStartColor;
    emit startColorChanged();
    update();
}

QColor GradientTextureProvider::endColor() const
{
    return m_endColor;
}

void GradientTextureProvider::setEndColor(const QColor &newEndColor)
{
    if (m_endColor == newEndColor)
        return;
    m_endColor = newEndColor;
    emit endColorChanged();
    update();
}

//! [updateSpatialNode]
QSSGRenderGraphObject *GradientTextureProvider::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new GradientTextureProviderNode(this);

    // Update the state of the backend node
    auto gradientNode = static_cast<GradientTextureProviderNode *>(node);
    gradientNode->m_isDirty = true;
    gradientNode->m_width = m_width;
    gradientNode->m_height = m_height;
    gradientNode->m_startColor = m_startColor;
    gradientNode->m_endColor = m_endColor;

    return node;
}
//! [updateSpatialNode]
