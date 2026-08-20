// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QRhiWidget>
#include <QPushButton>
#include <QFile>
#include <rhi/qrhi.h>
#include <QCanvasPainter>
#include <QCanvasPainterFactory>
#include <QCanvasRhiPaintDriver>

class ExampleRhiWidget : public QRhiWidget
{
public:
    ExampleRhiWidget(QWidget *parent = nullptr) : QRhiWidget(parent) { }
    void initialize(QRhiCommandBuffer *cb) override;
    void releaseResources() override;
    void render(QRhiCommandBuffer *cb) override;

private:
    QRhi *m_rhi = nullptr;
    std::unique_ptr<QRhiBuffer> m_vbuf;
    std::unique_ptr<QRhiBuffer> m_ubuf;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;
    QMatrix4x4 m_viewProjection;
    float m_rotation = 0.0f;
    std::unique_ptr<QCanvasPainterFactory> m_factory;
};

void ExampleRhiWidget::initialize(QRhiCommandBuffer *cb)
{
    static float vertexData[] = { // Y up, CCW
        0.0f,   0.5f,     1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,     0.0f, 1.0f, 0.0f,
        0.5f,  -0.5f,     0.0f, 0.0f, 1.0f,
    };

    m_rhi = rhi();
    if (!m_pipeline) {
        m_vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertexData)));
        m_vbuf->create();

        m_ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
        m_ubuf->create();

        m_srb.reset(m_rhi->newShaderResourceBindings());
        m_srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_ubuf.get()),
        });
        m_srb->create();

        m_pipeline.reset(m_rhi->newGraphicsPipeline());
        static auto getShader = [](const QString &name) {
            QFile f(name);
            return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
        };
        m_pipeline->setShaderStages({
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
        m_pipeline->setVertexInputLayout(inputLayout);
        m_pipeline->setShaderResourceBindings(m_srb.get());
        m_pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        m_pipeline->create();

        QRhiResourceUpdateBatch *resourceUpdates = m_rhi->nextResourceUpdateBatch();
        resourceUpdates->uploadStaticBuffer(m_vbuf.get(), vertexData);
        cb->resourceUpdate(resourceUpdates);
    }

    const QSize outputSize = colorTexture()->pixelSize();
    m_viewProjection = m_rhi->clipSpaceCorrMatrix();
    m_viewProjection.perspective(45.0f, outputSize.width() / (float) outputSize.height(), 0.01f, 1000.0f);
    m_viewProjection.translate(0, 0, -4);

    if (!m_factory) {
        m_factory.reset(new QCanvasPainterFactory);
        m_factory->create(m_rhi);
    }
}

void ExampleRhiWidget::releaseResources()
{
    m_factory.reset();
}

void ExampleRhiWidget::render(QRhiCommandBuffer *cb)
{
    QRhiResourceUpdateBatch *resourceUpdates = m_rhi->nextResourceUpdateBatch();
    m_rotation += 1.0f;
    QMatrix4x4 modelViewProjection = m_viewProjection;
    modelViewProjection.rotate(m_rotation, 0, 1, 0);
    resourceUpdates->updateDynamicBuffer(m_ubuf.get(), 0, 64, modelViewProjection.constData());

    const QSize outputSize = colorTexture()->pixelSize();

    QCanvasRhiPaintDriver *pd = m_factory->paintDriver();
    QCanvasPainter *painter = m_factory->painter();
    pd->resetForNewFrame();
    pd->beginPaint(cb, renderTarget());
    painter->beginPath();
    const QPointF center(outputSize.width() / 2, outputSize.height() / 2);
    painter->circle(center, 100);
    painter->setFillStyle(Qt::red);
    painter->fill();
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(24);
    painter->setFont(font1);
    painter->setFillStyle(Qt::green);
    painter->fillText("Hello", center.x(), center.y());
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);

    const QColor clearColor = QColor::fromRgbF(0.4f, 0.7f, 0.0f, 1.0f);
    cb->beginPass(renderTarget(), clearColor, { 1.0f, 0 }, resourceUpdates);

    cb->setGraphicsPipeline(m_pipeline.get());
    cb->setViewport(QRhiViewport(0, 0, outputSize.width(), outputSize.height()));
    cb->setShaderResources();
    const QRhiCommandBuffer::VertexInput vbufBinding(m_vbuf.get(), 0);
    cb->setVertexInput(0, 1, &vbufBinding);
    cb->draw(3);

    pd->renderPaint();

    cb->endPass();

    update();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ExampleRhiWidget rhiWidget;
    rhiWidget.resize(1280, 720);
    new QPushButton("This is a QPushButton", &rhiWidget);
    rhiWidget.show();
    return app.exec();
}
