// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rhirenderingextensions.h"
#include "cube.h"
#include <QGuiApplication>
#include <QFile>

#include <ssg/qquick3dextensionhelpers.h>

#include <ssg/qssgrendercontextcore.h>
#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrenderhelpers.h>

// The extension exposed to QML is the consumer, backed by an extension node
// with Main mode that renders a textured cube within the main render pass. It
// automatically creates a child extension, acting as the producer. This
// backend node for this has the Standalone mode, as it creates and renders
// into a QRhiTexture (doing its own render pass with a custom render target).
//
// The order is like this because extension backend nodes are invoked
// (prepareData, prepareRender, render) in a bottom-up order based on the
// frontend object relationships, i.e. the backend node of a child gets its
// functions called before the backend node of its parent. For siblings the
// order is undefined.

class ProducerRenderer : public QSSGRenderExtension
{
public:
    ProducerRenderer();
    ~ProducerRenderer();

    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return RenderMode::Standalone; }
    RenderStage stage() const override { return RenderStage::PostColor; }

    std::unique_ptr<QRhiTexture> texture;
    std::unique_ptr<QRhiTextureRenderTarget> rt;
    std::unique_ptr<QRhiRenderPassDescriptor> rpDesc;

    std::unique_ptr<QRhiBuffer> vbuf;
    std::unique_ptr<QRhiBuffer> ubuf;
    std::unique_ptr<QRhiShaderResourceBindings> srb;
    std::unique_ptr<QRhiGraphicsPipeline> ps;

    float rotation = 0.0f;
};

ProducerRenderer::ProducerRenderer()
{
    qDebug() << "ProducerRenderer" << this << "ctor; current thread:" << QThread::currentThread() << "main thread:" << qGuiApp->thread();
}

ProducerRenderer::~ProducerRenderer()
{
    qDebug() << "ProducerRenderer" << this << "dtor; current thread:" << QThread::currentThread() << "main thread:" << qGuiApp->thread();
}

bool ProducerRenderer::prepareData(QSSGFrameData &)
{
    return true;
}

static QShader getShader(const QString &name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

static float triangle[] = {
    0.0f,   0.5f,   1.0f, 0.0f, 0.0f,
    -0.5f,  -0.5f,   0.0f, 1.0f, 0.0f,
    0.5f,  -0.5f,   0.0f, 0.0f, 1.0f,
};

void ProducerRenderer::prepareRender(QSSGFrameData &data)
{
    const std::unique_ptr<QSSGRhiContext> &rhiCtx = data.contextInterface()->rhiContext();
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

void ProducerRenderer::render(QSSGFrameData &data)
{
    const std::unique_ptr<QSSGRhiContext> &rhiCtx = data.contextInterface()->rhiContext();
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

void ProducerRenderer::resetForFrame()
{
}

class ProducerExtension : public QQuick3DRenderExtension
{
public:
    ProducerExtension(QQuick3DRenderExtension *parent = nullptr)
        : QQuick3DRenderExtension(parent)
    {
    }

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

    ProducerRenderer *renderer = nullptr;
    float rotation = 0.0f;
};

QSSGRenderGraphObject *ProducerExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!renderer)
        renderer = new ProducerRenderer;

    if (node != renderer) {
        delete node;
        node = renderer;
    }

    renderer->rotation = rotation;

    return node;
}

class ConsumerRenderer : public QSSGRenderExtension
{
public:
    ConsumerRenderer(ProducerExtension *producer);
    ~ConsumerRenderer();

    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return RenderMode::Main; }
    RenderStage stage() const override { return RenderStage::PostColor; }

    ProducerExtension *producer;

    bool canRender = false;
    QMatrix4x4 viewProjection;
    QRhiViewport mainViewport;

    std::unique_ptr<QRhiBuffer> vbuf;
    std::unique_ptr<QRhiBuffer> ubuf;
    std::unique_ptr<QRhiSampler> sampler;
    std::unique_ptr<QRhiShaderResourceBindings> srb;
    std::unique_ptr<QRhiGraphicsPipeline> ps;
    QVector<quint32> rpFormat;

    QMatrix4x4 modelMatrix;
    void updateModelMatrix(const QVector3D &pos)
    {
        modelMatrix.setToIdentity();
        modelMatrix.translate(pos);
        modelMatrix.scale(10, 10, 10);
    }
};

ConsumerRenderer::ConsumerRenderer(ProducerExtension *producer)
    : producer(producer)
{
    qDebug() << "ConsumerRenderer" << this << "ctor; current thread:" << QThread::currentThread() << "main thread:" << qGuiApp->thread();
}

ConsumerRenderer::~ConsumerRenderer()
{
    qDebug() << "ConsumerRenderer" << this << "dtor; current thread:" << QThread::currentThread() << "main thread:" << qGuiApp->thread();
}

bool ConsumerRenderer::prepareData(QSSGFrameData &data)
{
    auto camera = data.activeCamera();
    if (camera == QSSGCameraId::Invalid) {
        canRender = false;
        return false;
    }

    viewProjection = QSSGCameraHelpers::getViewProjectionMatrix(camera);

    canRender = true;

    return true;
}

void ConsumerRenderer::prepareRender(QSSGFrameData &data)
{
    if (!canRender)
        return;

    // Due to lifetime guarantees (that the backend node of the child frontend
    // object is destroyed before the backend node of the parent frontend
    // object), accessing producer->m_renderer should always be safe in
    // ConsumerRenderer.
    QRhiTexture *texture = producer->renderer->texture.get();
    if (!texture)
        qFatal("No texture from producer");

    const std::unique_ptr<QSSGRhiContext> &rhiCtx = data.contextInterface()->rhiContext();
    if (!rhiCtx)
        return;

    QRhi *rhi = rhiCtx->rhi();
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();

    // The SceneEnvironment changes the MSAA setting dynamically. To follow
    // this, we need this extra check so that we can recreate the graphics
    // pipeline with the correct sample count upon a change.
    if (ps && ps->sampleCount() != rhiCtx->mainPassSampleCount())
        ps.reset();

    // This is not strictly required for this application. However, if there is
    // a chance that the View3D moves to become a (texture-backed) layer, part
    // of a ShaderEffect, or gets moved to a new window, robust implementations
    // should prepare for that like this, since some render target parameters
    // may be different then, which may imply that the graphics pipeline needs
    // to be recreated.
    QVector<quint32> currentRpFormat = rhiCtx->mainRenderPassDescriptor()->serializedFormat();
    if (ps && rpFormat != currentRpFormat)
        ps.reset();

    if (!ps) {
        vbuf.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(cube)));
        vbuf->create();

        resourceUpdates->uploadStaticBuffer(vbuf.get(), cube);

        ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64 + 16));
        ubuf->create();

        sampler.reset(rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                      QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        sampler->create();

        srb.reset(rhi->newShaderResourceBindings());
        srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, ubuf.get()),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, texture, sampler.get())
        });
        srb->create();

        ps.reset(rhi->newGraphicsPipeline());
        // depth testing is important since we test against the objects already rendered by the View3D. (the mode is Overlay)
        ps->setDepthTest(true);
        ps->setDepthWrite(true);
        ps->setCullMode(QRhiGraphicsPipeline::Back);
        ps->setShaderStages({
            { QRhiShaderStage::Vertex, getShader(QLatin1String(":/shaders/texture.vert.qsb")) },
            { QRhiShaderStage::Fragment, getShader(QLatin1String(":/shaders/texture.frag.qsb")) }
        });
        QRhiVertexInputLayout inputLayout;
        // The cube is provided as non-interleaved sets of positions, UVs, normals.
        // Normals are not interesting here, only need the positions and UVs.
        inputLayout.setBindings({
            { 3 * sizeof(float) },
            { 2 * sizeof(float) }
        });
        inputLayout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
            { 1, 1, QRhiVertexInputAttribute::Float2, 0 }
        });
        ps->setVertexInputLayout(inputLayout);
        ps->setShaderResourceBindings(srb.get());

        // Must play nice with the View3D's render target.
        ps->setRenderPassDescriptor(rhiCtx->mainRenderPassDescriptor());
        rpFormat = currentRpFormat;
        ps->setSampleCount(rhiCtx->mainPassSampleCount());

        ps->create();
    }

    // clipSpaceCorrMatrix has to be applied manually
    QMatrix4x4 mvp = rhi->clipSpaceCorrMatrix() * viewProjection * modelMatrix;
    resourceUpdates->updateDynamicBuffer(ubuf.get(), 0, 64, mvp.constData());

    QVector4D color(0, 0.5f, 0, 1);
    resourceUpdates->updateDynamicBuffer(ubuf.get(), 64, 16, &color);

    cb->resourceUpdate(resourceUpdates);

    mainViewport = data.getPipelineState().viewport;
}

void ConsumerRenderer::render(QSSGFrameData &data)
{
    if (!canRender)
        return;

    const std::unique_ptr<QSSGRhiContext> &rhiCtx = data.contextInterface()->rhiContext();
    if (!rhiCtx)
        return;

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    cb->debugMarkBegin(QByteArrayLiteral("Custom rhi rendering (consumer)"));

    cb->setGraphicsPipeline(ps.get());
    cb->setViewport(mainViewport);
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput vbufBindings[] = {
        { vbuf.get(), 0 },
        { vbuf.get(), quint32(36 * 3 * sizeof(float)) }
    };
    cb->setVertexInput(0, 2, vbufBindings);
    cb->draw(36);

    cb->debugMarkEnd();
}

void ConsumerRenderer::resetForFrame()
{
}

MyExtension::MyExtension(QQuick3DObject *parent)
    : QQuick3DRenderExtension(parent)
{
    m_producer = new ProducerExtension(this);

    // Calling update() on m_producer in updateSpatialNode() would be bad: that
    // would mean the producer only saw every *other* change. Instead,
    // trigger update() via the changed signals.
    QObject::connect(this, &MyExtension::subSceneRotationChanged, m_producer, &ProducerExtension::update);
}

QSSGRenderGraphObject *MyExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new ConsumerRenderer(m_producer);

    ConsumerRenderer *renderer = static_cast<ConsumerRenderer *>(node);

    if (m_dirtyFlag & PositionDirty)
        renderer->updateModelMatrix(QVector3D(m_x, m_y, m_z));

    if (m_dirtyFlag & SubSceneRotationDirty)
        renderer->producer->rotation = m_subSceneRotation;

    m_dirtyFlag = {};

    return node;
}

void MyExtension::markDirty(Dirty v)
{
    m_dirtyFlag |= v;
    update();
}
