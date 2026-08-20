// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Frame capture needs qtbase built with the graphicsframecapture feature
// enabled. The feature flag is private, so cannot test for it in an
// application. Just enable it manually, when this is wanted.
// May cause warnings and some test cases failing, that is as expected.
//#define FRAME_CAPTURE

#include <QTest>
#include <memory>
#include <rhi/qrhi.h>

#ifdef FRAME_CAPTURE
#include <QtGui/private/qgraphicsframecapture_p.h>
#endif

#include "qcanvaspainter.h"
#include "qcanvaspainterfactory.h"
#include "qcanvasrhipaintdriver.h"
#include "qcanvasimagepattern.h"

#if QT_CONFIG(opengl)
#include <QOffscreenSurface>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>
#define TST_GL
#endif

#if QT_CONFIG(vulkan)
#include <QVulkanInstance>
#define TST_VK
#endif

#ifdef Q_OS_WIN
#define TST_D3D11
#define TST_D3D12
#endif

#if QT_CONFIG(metal)
#define TST_MTL
#endif

Q_DECLARE_METATYPE(QRhi::Implementation)
Q_DECLARE_METATYPE(QRhiInitParams *)

class tst_CanvasRhiRendering : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void rhiTestData();
    void create_data();
    void create();
    void createShared_data();
    void createShared();
    void render_data();
    void render();
    void renderWithDepthTest_data();
    void renderWithDepthTest();
    void canvasRender_data();
    void canvasRender();
    void canvasRenderMipMap_data();
    void canvasRenderMipMap();
    void canvasRenderHqStroking_data();
    void canvasRenderHqStroking();

private:
    void setWindowType(QWindow *window, QRhi::Implementation impl);

    struct RenderTarget
    {
        QRhiTexture *tex = nullptr;
        QRhiRenderBuffer *ds = nullptr;
        QRhiTextureRenderTarget *rt = nullptr;
        QRhiRenderPassDescriptor *rp = nullptr;
        static void deleter(RenderTarget *rt) {
            delete rt->tex;
            delete rt->ds;
            delete rt->rt;
            delete rt->rp;
        }
    };
    using RenderTargetPtr = std::unique_ptr<RenderTarget, decltype(&RenderTarget::deleter)>;
    RenderTargetPtr createRenderTarget(QRhi *rhi);

    struct {
        QRhiNullInitParams null;
#ifdef TST_GL
        QRhiGles2InitParams gl;
#endif
#ifdef TST_VK
        QRhiVulkanInitParams vk;
#endif
#ifdef TST_D3D11
        QRhiD3D11InitParams d3d11;
#endif
#ifdef TST_D3D12
        QRhiD3D12InitParams d3d12;
#endif
#ifdef TST_MTL
        QRhiMetalInitParams mtl;
#endif
    } initParams;

#ifdef TST_VK
    QVulkanInstance vulkanInstance;
#endif
#ifdef TST_GL
    QOffscreenSurface *fallbackSurface = nullptr;
#endif
#ifdef FRAME_CAPTURE
    std::unique_ptr<QGraphicsFrameCapture> m_cap;
#endif

    QRhi::Flags rhiCreateFlags;
};

#ifdef FRAME_CAPTURE
// must be done very early, before any QRhi is ever created (RenderDoc hooking limitations etc.)
static std::unique_ptr<QGraphicsFrameCapture> createFrameCapture()
{
    std::unique_ptr<QGraphicsFrameCapture> cap(new QGraphicsFrameCapture);
    return cap;
}

static void configureFrameCapture(QGraphicsFrameCapture *cap, QRhi *rhi)
{
    if (rhi->backend() == QRhi::Null)
        return;

    cap->setRhi(rhi);
}

static void startFrameCapture(QGraphicsFrameCapture *cap, QRhi *rhi, const char *filePrefix)
{
    if (!cap->isLoaded())
        return;

    const QString capPrefix = QString::asprintf("%s_%s", filePrefix, rhi->backendName());
    cap->setCapturePrefix(capPrefix);
    cap->setCapturePath(QLatin1String("."));
    cap->startCaptureFrame();
}

static void endFrameCapture(QGraphicsFrameCapture *cap)
{
    if (cap->isLoaded() && cap->isCapturing()) {
        cap->endCaptureFrame();
        qDebug() << "Frame capture saved to" << cap->capturedFileName();
    }
}
#endif

void tst_CanvasRhiRendering::initTestCase()
{
#ifdef FRAME_CAPTURE
    m_cap = createFrameCapture();
#endif

    // Have QRhi's own resource leak checking active in release builds too.
    qputenv("QT_RHI_LEAK_CHECK", "1");

#ifdef TST_GL
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
#ifdef Q_OS_MACOS
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
    QSurfaceFormat::setDefaultFormat(fmt);

    initParams.gl.format = QSurfaceFormat::defaultFormat();
    fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
    initParams.gl.fallbackSurface = fallbackSurface;
#endif

#ifdef TST_VK
    const QVersionNumber supportedVersion = vulkanInstance.supportedApiVersion();
    if (supportedVersion >= QVersionNumber(1, 2))
        vulkanInstance.setApiVersion(QVersionNumber(1, 2));
    else if (supportedVersion >= QVersionNumber(1, 1))
        vulkanInstance.setApiVersion(QVersionNumber(1, 1));
    vulkanInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
    vulkanInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
    vulkanInstance.create();
    initParams.vk.inst = &vulkanInstance;
#endif

#ifdef TST_D3D11
    initParams.d3d11.enableDebugLayer = true;
#endif
#ifdef TST_D3D12
    initParams.d3d12.enableDebugLayer = true;
#endif
}

void tst_CanvasRhiRendering::cleanupTestCase()
{
#ifdef TST_VK
    vulkanInstance.destroy();
#endif

#ifdef TST_GL
    delete fallbackSurface;
#endif
}

void tst_CanvasRhiRendering::rhiTestData()
{
    QTest::addColumn<QRhi::Implementation>("impl");
    QTest::addColumn<QRhiInitParams *>("initParams");

#ifndef Q_OS_WEBOS
    QTest::newRow("Null") << QRhi::Null << static_cast<QRhiInitParams *>(&initParams.null);
#endif
#ifdef TST_GL
    if (QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::OpenGL))
        QTest::newRow("OpenGL") << QRhi::OpenGLES2 << static_cast<QRhiInitParams *>(&initParams.gl);
#endif
#ifdef TST_VK
    if (vulkanInstance.isValid())
        QTest::newRow("Vulkan") << QRhi::Vulkan << static_cast<QRhiInitParams *>(&initParams.vk);
#endif
#ifdef TST_D3D11
    QTest::newRow("Direct3D 11") << QRhi::D3D11 << static_cast<QRhiInitParams *>(&initParams.d3d11);
#endif
#ifdef TST_D3D12
    QTest::newRow("Direct3D 12") << QRhi::D3D12 << static_cast<QRhiInitParams *>(&initParams.d3d12);
#endif
#ifdef TST_MTL
    QTest::newRow("Metal") << QRhi::Metal << static_cast<QRhiInitParams *>(&initParams.mtl);
#endif

#ifdef FRAME_CAPTURE
    rhiCreateFlags |= QRhi::EnableDebugMarkers;
#endif
}

static constexpr int RT_WIDTH = 1280;
static constexpr int RT_HEIGHT = 720;

tst_CanvasRhiRendering::RenderTargetPtr tst_CanvasRhiRendering::createRenderTarget(QRhi *rhi)
{
    RenderTargetPtr result(nullptr, &RenderTarget::deleter);
    std::unique_ptr<QRhiTexture> tex(rhi->newTexture(QRhiTexture::RGBA8,
                                                     QSize(RT_WIDTH, RT_HEIGHT),
                                                     1,
                                                     QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!tex->create())
        return result;

    std::unique_ptr<QRhiRenderBuffer> ds(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, tex->pixelSize()));
    if (!ds->create())
        return result;

    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ tex.get() });
    rtDesc.setDepthStencilBuffer(ds.get());
    std::unique_ptr<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    std::unique_ptr<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.get());
    if (!rt->create())
        return result;

    result.reset(new RenderTarget);
    *result = { tex.release(), ds.release(), rt.release(), rp.release() };
    return result;
}

void tst_CanvasRhiRendering::create_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::create()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCOMPARE(painter, factory->painter());
    QVERIFY(factory->isValid());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

    QCanvasPainter *samePainter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *samePaintDriver = factory->paintDriver();
    QCOMPARE(samePainter, painter);
    QCOMPARE(samePaintDriver, pd);
    QVERIFY(factory->isValid());

    factory->destroy();
    QVERIFY(!factory->isValid());
    painter = factory->create(rhi.get());
    QVERIFY(painter);
}

void tst_CanvasRhiRendering::createShared_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::createShared()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    QCanvasPainterFactory *factory1 = QCanvasPainterFactory::sharedInstance(rhi.get());
    QVERIFY(factory1);
    QVERIFY(factory1->isValid());
    QCanvasPainter *painter1 = factory1->painter();
    QVERIFY(painter1);
    QCanvasRhiPaintDriver *pd1 = factory1->paintDriver();
    QVERIFY(pd1);

    QCanvasPainterFactory *factory2 = QCanvasPainterFactory::sharedInstance(rhi.get());
    QVERIFY(factory2);
    QVERIFY(factory2->isValid());
    QCOMPARE(factory1, factory2);
    QCanvasPainter *painter2 = factory2->painter();
    QCOMPARE(painter1, painter2);
    QCanvasRhiPaintDriver *pd2 = factory2->paintDriver();
    QCOMPARE(pd1, pd2);

    // sharedInstance with another QRhi should give a different factory
    std::unique_ptr<QRhi> anotherRhi(QRhi::create(impl, initParams));
    QVERIFY(anotherRhi);
    QCanvasPainterFactory *factory3 = QCanvasPainterFactory::sharedInstance(anotherRhi.get());
    QVERIFY(factory3);
    QVERIFY(factory3->isValid());
    QCOMPARE_NE(factory1, factory3);
    QCanvasPainter *painter3 = factory3->painter();
    QCOMPARE_NE(painter1, painter3);
    QCanvasRhiPaintDriver *pd3 = factory3->paintDriver();
    QCOMPARE_NE(pd1, pd3);
}

static void drawCircleInCenter(QCanvasPainter *painter)
{
    painter->beginPath();
    painter->circle(RT_WIDTH / 2, RT_HEIGHT / 2, std::min(RT_WIDTH, RT_HEIGHT) / 2);
    painter->setStrokeStyle(Qt::green);
    painter->setLineWidth(4);
    painter->stroke();
    painter->setFillStyle("#ff0000");
    painter->fill();
}

static void drawCircleInCenter(QCanvasPainter *painter, const QCanvasImage &imageForPattern)
{
    painter->beginPath();
    painter->circle(RT_WIDTH / 2, RT_HEIGHT / 2, std::min(RT_WIDTH, RT_HEIGHT) / 2);
    painter->setStrokeStyle(Qt::blue);
    painter->setLineWidth(4);
    painter->stroke();
    QCanvasImagePattern pattern(imageForPattern, 0, 0, 100, 100);
    painter->setFillStyle(pattern);
    painter->fill();
}

static void drawCircleAndTextInCenter(QCanvasPainter *painter)
{
    const QPointF center(RT_WIDTH / 2, RT_HEIGHT / 2);
    painter->beginPath();
    painter->circle(center.x(), center.y(), std::min(RT_WIDTH, RT_HEIGHT) / 2);
    painter->setStrokeStyle(Qt::green);
    painter->setLineWidth(4);
    painter->stroke();
    painter->setFillStyle("#ff0000");
    painter->fill();
    painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font1;
    font1.setWeight(QFont::Weight::Bold);
    font1.setItalic(true);
    font1.setPixelSize(24);
    painter->setFont(font1);
    painter->setFillStyle(Qt::blue);
    painter->fillText("Hello", center.x(), center.y());
}

static bool testColor(const QImage &image, int x, int y, const QColor &expected, bool warnOnMismatch = true)
{
    const int maxFuzz = 1;
    QRgb c1 = image.pixel(x, y);
    QRgb c2 = expected.rgba();
    bool result = qAbs(qRed(c1) - qRed(c2)) <= maxFuzz
                  && qAbs(qGreen(c1) - qGreen(c2)) <= maxFuzz
                  && qAbs(qBlue(c1) - qBlue(c2)) <= maxFuzz
                  && qAbs(qAlpha(c1) - qAlpha(c2)) <= maxFuzz;
    if (!result && warnOnMismatch)
        qWarning() << "Color mismatch at" << x << "," << y << ": got" << QColor(c1) << "expected" << QColor(c2);
    return result;
}

static QImage imageFromReadback(QRhi *rhi, const QRhiReadbackResult &readbackResult)
{
    QImage image(reinterpret_cast<const uchar *>(readbackResult.data.constData()),
                 readbackResult.pixelSize.width(),
                 readbackResult.pixelSize.height(),
                 QImage::Format_RGBA8888);

    if (rhi->isYUpInFramebuffer())
        return image.flipped();

    return image.copy();
}

static QImage imageFromReadback(QRhi *rhi, QRhiTexture *texture)
{
    QRhiReadbackResult readbackResult;
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture({ texture }, &readbackResult);
    cb->resourceUpdate(u);
    rhi->endOffscreenFrame();
    return imageFromReadback(rhi, readbackResult);
}

void tst_CanvasRhiRendering::render_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::render()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "render");
#endif

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

    QRhiReadbackResult readbackResult;
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    pd->resetForNewFrame();

    pd->beginPaint(cb, rt->rt);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);

    cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
    pd->renderPaint();
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture({ rt->tex }, &readbackResult);
    cb->endPass(u);

    rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    // cannot check rendering results with Null, because there is no rendering there
    if (impl == QRhi::Null)
        return;

    QImage image = imageFromReadback(rhi.get(), readbackResult);

    QVERIFY(testColor(image, 1, 1, Qt::black));
    QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
}

void tst_CanvasRhiRendering::renderWithDepthTest_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::renderWithDepthTest()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    const int centerX = RT_WIDTH / 2;
    const int centerY = RT_HEIGHT / 2;

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt, Qt::black, QSize(), 0.0f, QCanvasRhiPaintDriver::BeginPaintFlag::DepthTest);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
    // Depth buffer is cleared to the usual 1.0, so the the circle should show up normally.
    cb->beginPass(rt->rt, Qt::black, { 1.0f, 0 });
    pd->renderPaint();
    cb->endPass();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, centerX, centerY, Qt::red));
    }

    // Now clear the depth buffer to 0, so that the default Less comparison
    // fails. First verify however that rendering without the DepthTest flag
    // still does what it should (ignoring the depth buffer contents).
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
    cb->beginPass(rt->rt, Qt::black, { 0.0f, 0 }); // note that depth is cleared to 0
    pd->renderPaint();
    cb->endPass();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, centerX, centerY, Qt::red));
    }

    // Now repeat with the DepthTest flag
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt, Qt::black, QSize(), 0.0f, QCanvasRhiPaintDriver::BeginPaintFlag::DepthTest);
    drawCircleInCenter(painter);
    pd->endPaint(QCanvasRhiPaintDriver::EndPaintFlag::DoNotRecordRenderPass);
    cb->beginPass(rt->rt, Qt::black, { 0.0f, 0 }); // note that depth is cleared to 0
    pd->renderPaint();
    cb->endPass();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        const QImage image = imageFromReadback(rhi.get(), rt->tex);
        QVERIFY(testColor(image, 1, 1, Qt::black));
        // the red circle should not be there
        QVERIFY(!testColor(image, centerX, centerY, Qt::red, false));
    }
}

void tst_CanvasRhiRendering::canvasRender_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRender()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRender");
#endif

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

    QCanvasOffscreenCanvas canvas;
    QVERIFY(canvas.isNull());
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    QVERIFY(canvas.texture());
    QCOMPARE(canvas.texture()->pixelSize().width(), RT_WIDTH);
    QCOMPARE(canvas.texture()->pixelSize().height(), RT_HEIGHT);
    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
    }

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    QCanvasImage canvasImage;
    QVERIFY(canvasImage.isNull());
    canvasImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat);
    QVERIFY(!canvasImage.isNull());
    QCanvasImage secondRegistrationImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat);
    QCOMPARE(canvasImage, secondRegistrationImage);
    drawCircleInCenter(painter, canvasImage);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), rt->tex);
        int x = RT_WIDTH / 2;
        int y = RT_HEIGHT / 2;
        // the distorted circle in the middle
        QVERIFY(testColor(image, x, y, Qt::red));
        // black between columns and rows
        QVERIFY(testColor(image, x - 50, y, Qt::black));
        // the distorted circle to the left
        QVERIFY(testColor(image, x - 100, y, Qt::red));
    }

    // Now render some text.
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    drawCircleAndTextInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), rt->tex);
        // the image only has red, green, blue, and black
        int redCount = 0, greenCount = 0, blueCount = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qRed(image.pixel(x, y)) > 240)
                    ++redCount;
                else if (qGreen(image.pixel(x, y)) > 240)
                    ++greenCount;
                else if (qBlue(image.pixel(x, y)) > 240)
                    ++blueCount;
            }
        }
        // the fill
        QCOMPARE_GT(redCount, 400000);
        // the border
        QCOMPARE_GT(greenCount, 2000);
        // the text, use a low threshold since the font may differ between
        // platforms, and so it may mean fewer or more blue pixels.
        QCOMPARE_GT(blueCount, 200);
    }

    QCanvasOffscreenCanvas canvas2 = canvas;
    QCOMPARE(canvas, canvas2);
    canvas2.setFillColor(Qt::red);
    QCOMPARE_NE(canvas, canvas2);

    QCanvasOffscreenCanvas canvas3 = canvas;
    painter->destroyCanvas(canvas);
    QVERIFY(canvas.isNull());
    QVERIFY(canvas3.isNull());
    QCOMPARE(canvas, canvas3);
    // This is evil, since destroyCanvas was used explicitly and canvas2 may still seem valid, but it is not.
    // Should not happen in practice, though.
    QVERIFY(!canvas2.isNull());
    QCOMPARE_NE(canvas, canvas2);

    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    QVERIFY(canvas.texture());

    // let the painter destroy 'canvas' automatically

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif
}

void tst_CanvasRhiRendering::canvasRenderMipMap_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRenderMipMap()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QVERIFY(painter);
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd);

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderMipMap_part1");
#endif

    QCanvasOffscreenCanvas canvas;
    QVERIFY(canvas.isNull());
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT), 1, QCanvasOffscreenCanvas::Flag::MipMaps);
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    QVERIFY(canvas.texture());
    QCOMPARE(canvas.texture()->pixelSize().width(), RT_WIDTH);
    QCOMPARE(canvas.texture()->pixelSize().height(), RT_HEIGHT);
    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
    }

#ifdef FRAME_CAPTURE
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderMipMap_part2");
#endif

    RenderTargetPtr rt = createRenderTarget(rhi.get());
    QVERIFY(rt);

    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(cb, rt->rt);
    QCanvasImage canvasImage;
    QVERIFY(canvasImage.isNull());
    // request addImage to generate the mimap sequence
    canvasImage = painter->addImage(canvas, QCanvasPainter::ImageFlag::Repeat | QCanvasPainter::ImageFlag::GenerateMipmaps);
    QVERIFY(!canvasImage.isNull());
    drawCircleInCenter(painter, canvasImage);
    pd->endPaint();
    rhi->endOffscreenFrame();

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif

    // won't verify here that the linear mipmap filtering works, since the result is the same anyway
    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), rt->tex);
        int x = RT_WIDTH / 2;
        int y = RT_HEIGHT / 2;
        // the distorted circle in the middle
        QVERIFY(testColor(image, x, y, Qt::red));
        // black between columns and rows
        QVERIFY(testColor(image, x - 50, y, Qt::black));
        // the distorted circle to the left
        QVERIFY(testColor(image, x - 100, y, Qt::red));
    }
}

void tst_CanvasRhiRendering::canvasRenderHqStroking_data()
{
    rhiTestData();
}

void tst_CanvasRhiRendering::canvasRenderHqStroking()
{
    QFETCH(QRhi::Implementation, impl);
    QFETCH(QRhiInitParams *, initParams);

    std::unique_ptr<QRhi> rhi(QRhi::create(impl, initParams, rhiCreateFlags));
    if (!rhi)
        QSKIP("Failed to create QRhi, skip");

#ifdef FRAME_CAPTURE
    configureFrameCapture(m_cap.get(), rhi.get());
    startFrameCapture(m_cap.get(), rhi.get(), "canvasRenderHqStroking");
#endif

    std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
    QCanvasPainter *painter = factory->create(rhi.get());
    QCanvasRhiPaintDriver *pd = factory->paintDriver();
    QVERIFY(pd && painter);

    QCanvasOffscreenCanvas canvas;
    canvas = painter->createCanvas(QSize(RT_WIDTH, RT_HEIGHT));
    QVERIFY(!canvas.isNull());
    canvas.setFillColor(Qt::black);

    // this triggers using the stencil buffer
    painter->setRenderHint(QCanvasPainter::RenderHint::HighQualityStroking);

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);
    pd->resetForNewFrame();
    pd->beginPaint(canvas, cb);
    drawCircleInCenter(painter);
    pd->endPaint();
    rhi->endOffscreenFrame();

    if (impl != QRhi::Null) {
        QImage image = imageFromReadback(rhi.get(), canvas.texture());
        QVERIFY(testColor(image, 1, 1, Qt::black));
        QVERIFY(testColor(image, RT_WIDTH / 2, RT_HEIGHT / 2, Qt::red));
        int greenCount = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (qGreen(image.pixel(x, y)) > 250)
                    ++greenCount;
            }
        }
        // ca. 2068 green pixels, the rest is either black or red
        QCOMPARE_GT(greenCount, 2000);
    }

#ifdef FRAME_CAPTURE
    endFrameCapture(m_cap.get());
#endif
}

#include <tst_qcrhiplumbing.moc>
QTEST_MAIN(tst_CanvasRhiRendering)
