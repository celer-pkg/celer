// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "painterwindow.h"
#include "qcanvasrhipaintdriver.h"
#include <QPlatformSurfaceEvent>
#include <QtCanvasPainter/QCanvasPainter>
#include <QtCanvasPainter/QCanvasPainterFactory>

#if QT_CONFIG(vulkan)
static QVulkanInstance g_vulkanInstance;
#endif

PainterWindow::PainterWindow(QRhi::Implementation api)
{
    m_graphicsApi = api;
    updateSurfaceType();
}

PainterWindow::~PainterWindow()
{
    delete m_factory;
}

QColor PainterWindow::fillColor() const
{
    return m_fillColor;
}

void PainterWindow::setFillColor(const QColor &color)
{
    if (m_fillColor == color)
        return;
    m_fillColor = color;

    requestUpdate();
}

void PainterWindow::cleanup()
{
}

void PainterWindow::updateSurfaceType()
{
    switch (m_graphicsApi) {
    case QRhi::OpenGLES2:
        setSurfaceType(QSurface::OpenGLSurface);
        break;
    case QRhi::Vulkan:
        setSurfaceType(QSurface::VulkanSurface);
        break;
    case QRhi::D3D11:
    case QRhi::D3D12:
        setSurfaceType(QSurface::Direct3DSurface);
        break;
    case QRhi::Metal:
        setSurfaceType(QSurface::MetalSurface);
        break;
    default:
        setSurfaceType(QSurface::RasterSurface);
        break;
    }
}

void PainterWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed() && !m_running) {
        m_running = true;
        init();
        resizeSwapChain();
    }

    const QSize surfaceSize = m_hasSwapChain ? m_sc->surfacePixelSize() : QSize();

    if ((!isExposed() || (m_hasSwapChain && surfaceSize.isEmpty())) && m_running && !m_notExposed)
        m_notExposed = true;

    if (isExposed() && m_running && m_notExposed && !surfaceSize.isEmpty()) {
        m_notExposed = false;
        m_newlyExposed = true;
    }

    if (isExposed() && !surfaceSize.isEmpty())
        render();
}

bool PainterWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::UpdateRequest:
        render();
        break;

    case QEvent::PlatformSurface:
        if (static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
            cleanup();
            releaseSwapChain();
        }
        break;

    default:
        break;
    }

    return QWindow::event(e);
}

void PainterWindow::init()
{
    QRhi::Flags rhiFlags = QRhi::EnableDebugMarkers | QRhi::EnableTimestamps;

    // Create QRhi instance for the graphics API.
    if (m_graphicsApi == QRhi::Null) {
        QRhiNullInitParams params;
        m_rhi.reset(QRhi::create(QRhi::Null, &params, rhiFlags));
    }

#if QT_CONFIG(vulkan)
    if (m_graphicsApi == QRhi::Vulkan) {
        if (!g_vulkanInstance.isValid()) {
            g_vulkanInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
            if (!g_vulkanInstance.create())
                qWarning("Failed to create Vulkan instance");
        }
        if (g_vulkanInstance.isValid()) {
            setVulkanInstance(&g_vulkanInstance);
            QRhiVulkanInitParams params;
            params.inst = &g_vulkanInstance;
            params.window = this;
            m_rhi.reset(QRhi::create(QRhi::Vulkan, &params, rhiFlags));
        }
    }
#endif

#ifdef Q_OS_WIN
    if (m_graphicsApi == QRhi::D3D11) {
        QRhiD3D11InitParams params;
        m_rhi.reset(QRhi::create(QRhi::D3D11, &params, rhiFlags));
    } else if (m_graphicsApi == QRhi::D3D12) {
        QRhiD3D12InitParams params;
        m_rhi.reset(QRhi::create(QRhi::D3D12, &params, rhiFlags));
    }
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    if (m_graphicsApi == QRhi::Metal) {
        QRhiMetalInitParams params;
        m_rhi.reset(QRhi::create(QRhi::Metal, &params, rhiFlags));
    }
#endif

    if (!m_rhi) {
#if QT_CONFIG(opengl)
        m_fallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams params;
        params.fallbackSurface = m_fallbackSurface.get();
        params.window = this;
        m_rhi.reset(QRhi::create(QRhi::OpenGLES2, &params, rhiFlags));
        if (m_rhi)
            m_graphicsApi = QRhi::OpenGLES2;
#endif
    }

    if (!m_rhi)
        qFatal("Failed to create RHI backend");
//![painterfactory]
    if (!m_factory) {
        m_factory = new QCanvasPainterFactory;
        m_factory->create(m_rhi.get());
    }
//![painterfactory]
    m_sc.reset(m_rhi->newSwapChain());
    m_ds.reset(m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
                                      QSize(),
                                      1,
                                      QRhiRenderBuffer::UsedWithSwapChainOnly));
    m_sc->setWindow(this);
    m_sc->setDepthStencil(m_ds.get());
    m_rp.reset(m_sc->newCompatibleRenderPassDescriptor());
    m_sc->setRenderPassDescriptor(m_rp.get());
}

void PainterWindow::resizeSwapChain()
{
    m_hasSwapChain = m_sc->createOrResize();
}

void PainterWindow::releaseSwapChain()
{
    if (m_hasSwapChain) {
        m_hasSwapChain = false;
        m_sc->destroy();
    }
}

//![renderloop]
void PainterWindow::render()
{
    if (!m_factory || !m_factory->isValid() || !m_rhi || !m_sc)
        return;

    if (!m_hasSwapChain || m_notExposed)
        return;

    if (m_sc->currentPixelSize() != m_sc->surfacePixelSize() || m_newlyExposed) {
        resizeSwapChain();
        if (!m_hasSwapChain)
            return;
        m_newlyExposed = false;
    }

    QRhi::FrameOpResult r = m_rhi->beginFrame(m_sc.get());
    if (r == QRhi::FrameOpSwapChainOutOfDate) {
        resizeSwapChain();
        if (!m_hasSwapChain)
            return;
        r = m_rhi->beginFrame(m_sc.get());
    }
    if (r != QRhi::FrameOpSuccess) {
        qDebug("beginFrame failed with %d, retry", r);
        requestUpdate();
        return;
    }

    QRhiCommandBuffer *cb = m_sc->currentFrameCommandBuffer();
    QRhiRenderTarget *rt = m_sc->currentFrameRenderTarget();

    QCanvasRhiPaintDriver *pd = m_factory->paintDriver();
    QCanvasPainter *painter = m_factory->painter();

    pd->resetForNewFrame();
    pd->beginPaint(cb, rt, m_fillColor, size(), float(devicePixelRatio()));
    paint(painter);
    pd->endPaint();

    m_rhi->endFrame(m_sc.get());
}
//![renderloop]

void PainterWindow::paint(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}
