// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rhirenderingextension.h"
#include "cube.h"
#include <rhi/qrhi.h>
#include <QGuiApplication>
#include <ssg/qssgrendercontextcore.h>
#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrenderhelpers.h>

class Renderer : public QSSGRenderExtension
{
public:
    Renderer();
    ~Renderer();

    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return RenderMode::Main; }
    RenderStage stage() const override { return renderStage; }

    bool canRender = false;
    QMatrix4x4 viewProjection;
    QRhiViewport mainViewport;

    std::unique_ptr<QRhiBuffer> vbuf;
    std::unique_ptr<QRhiBuffer> ubuf;
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

    RenderStage renderStage = RenderStage::PostColor;
};

Renderer::Renderer()
{
    qDebug() << "Renderer ctor; current thread:" << QThread::currentThread() << "main thread:" << qGuiApp->thread();
}

Renderer::~Renderer()
{
    qDebug() << "Renderer dtor; current thread:" << QThread::currentThread() << "main thread:" << qGuiApp->thread();
}

bool Renderer::prepareData(QSSGFrameData &data)
{
    auto camera = data.activeCamera();
    if (camera == QSSGCameraId::Invalid) {
        canRender = false;
        return false;
    }

    // ### would be nice to know that we do not have to query (recalculate) every time, if the camera has not changed
    viewProjection = QSSGCameraHelpers::getViewProjectionMatrix(camera);

    canRender = true;

    return true;
}

static QShader getShader(const QString &name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

void Renderer::prepareRender(QSSGFrameData &data)
{
    if (!canRender)
        return;

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

        srb.reset(rhi->newShaderResourceBindings());
        srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, ubuf.get()),
        });
        srb->create();

        ps.reset(rhi->newGraphicsPipeline());
        // depth testing is important since we test against the objects already rendered by the View3D. (the mode is Overlay)
        ps->setDepthTest(true);
        ps->setDepthWrite(true);
        ps->setCullMode(QRhiGraphicsPipeline::Back);
        ps->setShaderStages({
            { QRhiShaderStage::Vertex, getShader(QLatin1String(":/shaders/solidcolor.vert.qsb")) },
            { QRhiShaderStage::Fragment, getShader(QLatin1String(":/shaders/solidcolor.frag.qsb")) }
        });
        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({
            { 3 * sizeof(float) }
        });
        inputLayout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float3, 0 }
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

void Renderer::render(QSSGFrameData &data)
{
    if (!canRender)
        return;

    const std::unique_ptr<QSSGRhiContext> &rhiCtx = data.contextInterface()->rhiContext();
    if (!rhiCtx)
        return;

    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    // this is within a renderpass since the extension type is Main (not Standalone)

    cb->debugMarkBegin(QByteArrayLiteral("Custom rhi rendering"));

    cb->setGraphicsPipeline(ps.get());
    cb->setViewport(mainViewport);
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput vbufBindings[] = {
        { vbuf.get(), 0 }
    };
    cb->setVertexInput(0, 1, vbufBindings);
    cb->draw(36);

    cb->debugMarkEnd();
}

void Renderer::resetForFrame()
{
}

QSSGRenderGraphObject *RhiRenderingExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new Renderer;

    Renderer *renderer = static_cast<Renderer *>(node);

    if (m_dirtyFlag & PositionDirty)
        renderer->updateModelMatrix(QVector3D(m_x, m_y, m_z));

    if (m_dirtyFlag & ModeDirty) {
        // the difference is obvious when the sphere is semi-transparent and the cube overlaps with it
        switch (m_mode) {
        case Overlay:
            renderer->renderStage = QSSGRenderExtension::RenderStage::PostColor;
            break;
        case Underlay:
            renderer->renderStage = QSSGRenderExtension::RenderStage::PreColor;
            break;
        }
    }

    m_dirtyFlag = {};

    return node;
}

void RhiRenderingExtension::markDirty(Dirty v)
{
    m_dirtyFlag |= v;
    update();
}
