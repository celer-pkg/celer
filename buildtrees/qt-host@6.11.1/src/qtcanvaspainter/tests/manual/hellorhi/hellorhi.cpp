// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QImage>
#include <QFile>
#include <QCommandLineParser>
#include <QOffscreenSurface>
#include <rhi/qrhi.h>

#include "qcanvaspainter.h"
#include "qcanvasradialgradient.h"
#include "qcanvaslineargradient.h"
#include "qcanvasimagepattern.h"
#include "qcanvasoffscreencanvas.h"
#include "qcanvaspainterfactory.h"
#include "qcanvasrhipaintdriver.h"

constexpr int CANVAS_WIDTH = 320;
constexpr int CANVAS_HEIGHT = 240;

static void paintOffscreenCanvas(QCanvasPainter *p, const QCanvasImage &qtLogoImage)
{
    QRectF rect1(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    QCanvasLinearGradient g1(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    g1.setStartColor(Qt::green);
    g1.setEndColor(Qt::black);
    p->setFillStyle(g1);
    p->fillRect(rect1);
    p->drawImage(qtLogoImage, 32, 0);
}

static void paintContent(QCanvasPainter *p, float width, float height,
                         const QCanvasImage &heartImage, float patternSizeFactor,
                         const char *rhiBackendName)
{
    float size = std::min(width, height);
    QPointF center(width/2, height/2);

    // Paint the background circle
    QCanvasRadialGradient gradient1(center.x(), center.y() - size*0.1, size*0.6);
    gradient1.setStartColor("#909090");
    gradient1.setEndColor("#404040");
    p->beginPath();
    p->circle(center, size*0.46);
    p->setFillStyle(gradient1);
    p->fill();
    p->setStrokeStyle("#202020");
    p->setLineWidth(size*0.02);
    p->stroke();

    // Text line 1
    p->setTextAlign(QCanvasPainter::TextAlign::Center);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(size*0.07);
    p->setFont(font1);
    p->setFillStyle("#B0D040");
    p->fillText(QString::asprintf("Hello on %s from", rhiBackendName), center.x(), center.y() - size*0.18);

    // Text line 2
    QFont font2;
    font2.setWeight(QFont::Weight::Thin);
    font2.setPixelSize(size*0.07);
    p->setFont(font2);
    p->fillText(QLatin1String("headless Canvas Painter"), center.x(), center.y() - size*0.08);

    // Paint heart
    float patternSize = size * patternSizeFactor;
    QCanvasImagePattern pattern(heartImage, center.x(), center.y(), patternSize, patternSize);
    p->setFillStyle(pattern);
    p->setLineCap(QCanvasPainter::LineCap::Round);
    p->setStrokeStyle("#B0D040");
    p->beginPath();
    p->moveTo(center.x(), center.y() + size*0.3);
    p->bezierCurveTo(center.x() - size*0.25, center.y() + size*0.1,
                     center.x() - size*0.05, center.y() + size*0.05,
                     center.x(), center.y() + size*0.15);
    p->bezierCurveTo(center.x() + size*0.05, center.y() + size*0.05,
                     center.x() + size*0.25, center.y() + size*0.1,
                     center.x(), center.y() + size*0.3);
    p->stroke();
    p->fill();
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QRhi::Implementation graphicsApi;
#if defined(Q_OS_WIN)
    graphicsApi = QRhi::D3D11;
#elif QT_CONFIG(metal)
    graphicsApi = QRhi::Metal;
#elif QT_CONFIG(vulkan)
    graphicsApi = QRhi::Vulkan;
#else
    graphicsApi = QRhi::OpenGLES2;
#endif

    QCommandLineParser cmdLineParser;
    cmdLineParser.addHelpOption();
    QCommandLineOption nullOption({ "n", "null" }, QLatin1String("Null"));
    cmdLineParser.addOption(nullOption);
    QCommandLineOption glOption({ "g", "opengl" }, QLatin1String("OpenGL"));
    cmdLineParser.addOption(glOption);
    QCommandLineOption vkOption({ "v", "vulkan" }, QLatin1String("Vulkan"));
    cmdLineParser.addOption(vkOption);
    QCommandLineOption d3d11Option({ "d", "d3d11" }, QLatin1String("Direct3D 11"));
    cmdLineParser.addOption(d3d11Option);
    QCommandLineOption d3d12Option({ "D", "d3d12" }, QLatin1String("Direct3D 12"));
    cmdLineParser.addOption(d3d12Option);
    QCommandLineOption mtlOption({ "m", "metal" }, QLatin1String("Metal"));
    cmdLineParser.addOption(mtlOption);

    cmdLineParser.process(app);
    if (cmdLineParser.isSet(nullOption))
        graphicsApi = QRhi::Null;
    if (cmdLineParser.isSet(glOption))
        graphicsApi = QRhi::OpenGLES2;
    if (cmdLineParser.isSet(vkOption))
        graphicsApi = QRhi::Vulkan;
    if (cmdLineParser.isSet(d3d11Option))
        graphicsApi = QRhi::D3D11;
    if (cmdLineParser.isSet(d3d12Option))
        graphicsApi = QRhi::D3D12;
    if (cmdLineParser.isSet(mtlOption))
        graphicsApi = QRhi::Metal;

#if QT_CONFIG(vulkan)
    QVulkanInstance inst;
#endif
    std::unique_ptr<QRhi> rhi;
    std::unique_ptr<QOffscreenSurface> fallbackSurface;
#if defined(Q_OS_WIN)
    if (graphicsApi == QRhi::D3D11) {
        QRhiD3D11InitParams params;
        rhi.reset(QRhi::create(QRhi::D3D11, &params));
    } else if (graphicsApi == QRhi::D3D12) {
        QRhiD3D12InitParams params;
        rhi.reset(QRhi::create(QRhi::D3D12, &params));
    }
#elif defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    if (graphicsApi == QRhi::Metal) {
        QRhiMetalInitParams params;
        rhi.reset(QRhi::create(QRhi::Metal, &params));
    }
#endif
#if QT_CONFIG(vulkan)
    if (graphicsApi == QRhi::Vulkan) {
        inst.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
        if (inst.create()) {
            QRhiVulkanInitParams params;
            params.inst = &inst;
            rhi.reset(QRhi::create(QRhi::Vulkan, &params));
        }
    }
#endif
#if QT_CONFIG(opengl)
    if (graphicsApi == QRhi::OpenGLES2) {
        fallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams params;
        params.fallbackSurface = fallbackSurface.get();
        rhi.reset(QRhi::create(QRhi::OpenGLES2, &params));
    }
#endif
    if (graphicsApi == QRhi::Null) {
        QRhiNullInitParams params;
        rhi.reset(QRhi::create(QRhi::Null, &params));
    }

    if (rhi)
        qDebug() << rhi->backendName() << rhi->driverInfo();
    else
        qFatal("Failed to initialize RHI");

    const int width = 1280;
    const int height = 720;
    std::unique_ptr<QRhiTexture> tex(rhi->newTexture(QRhiTexture::RGBA8,
                                                     QSize(width, height),
                                                     1,
                                                     QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    tex->create();
    std::unique_ptr<QRhiRenderBuffer> ds(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, QSize(width, height)));
    ds->create();
    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ tex.get() });
    rtDesc.setDepthStencilBuffer(ds.get());
    std::unique_ptr<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    std::unique_ptr<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.get());
    rt->create();

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    const QCanvasImage qtLogoImage = painter->addImage(QImage("qt.png"), QCanvasPainter::ImageFlag::Repeat);

    QRhiCommandBuffer *cb;
    QRhiReadbackResult readbackResult;

    rhi->beginOffscreenFrame(&cb);
    {
        pd->resetForNewFrame();

        QCanvasOffscreenCanvas canvas = painter->createCanvas(QSize(CANVAS_WIDTH, CANVAS_HEIGHT));
        canvas.setFillColor(Qt::black);
        pd->beginPaint(canvas, cb);
        paintOffscreenCanvas(painter, qtLogoImage);
        pd->endPaint();

        QCanvasImage canvasImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat);

        pd->beginPaint(cb, rt.get());
        paintContent(painter, width, height, canvasImage, 0.05f, rhi->backendName());
        pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);

        cb->beginPass(rt.get(), Qt::white, { 1.0f, 0 });
        pd->renderPaint();
        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        u->readBackTexture({ tex.get() }, &readbackResult);
        cb->endPass(u);
    }
    rhi->endOffscreenFrame();

    QImage image(reinterpret_cast<const uchar *>(readbackResult.data.constData()),
                    readbackResult.pixelSize.width(),
                    readbackResult.pixelSize.height(),
                    QImage::Format_RGBA8888);
    if (rhi->isYUpInFramebuffer())
        image.flip();
    const QString fn = QLatin1String("result.png");
    image.save(fn);
    qDebug() << "Saved to" << fn;

    return 0;
}
