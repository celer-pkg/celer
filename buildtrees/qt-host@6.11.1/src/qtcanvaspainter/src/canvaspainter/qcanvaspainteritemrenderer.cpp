// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvaspainteritemrenderer_p.h"
#include "qcanvaspainter_p.h"
#include "qcanvaspainterfactory_p.h"
#include "qcanvasrhipaintdriver.h"
#include "qcanvaspainteritem_p.h"
#include "engine/qcpainterengine_p.h"

#include <QDebug>
#include <QQuickWindow>
#include <QtMath>

QT_BEGIN_NAMESPACE

// As in, any item has updated its content since this flag was reset, including
// all QQuickWindows, and so all render threads.
QAtomicInt QCanvasPainterItemRendererPrivate::m_rendered;

/*!
    \class QCanvasPainterItemRenderer
    \since 6.11
    \brief The QCanvasPainterItemRenderer handles all painting for a QCanvasPainterItem.
    \inmodule QtCanvasPainter

    Implement the paint() method to perform the rendering.

    To expose data from the item to the renderer in a thread-safe manner,
    implement synchronizeData().

    The renderer object lives and operates on the \l {Scene Graph and
    Rendering}{Qt Quick scene graph render thread}, if there is one, since that
    is the thread on which \l{QCanvasPainterItem::}{createItemRenderer()} and
    all the functions in this class are invoked.

    If the QCanvasPainterItem is moved to a different \l{QQuickWindow}{window},
    it will get associated with a new \l QRhi. Therefore, the renderer object is
    automatically destroyed, and a new one is created by invoking
    \l{QCanvasPainterItem::}{createItemRenderer()} again. This allows simple
    management of QCanvasImage and QCanvasOffscreenCanvas objects, because they can be
    member variables in renderer, set up either in initializeResources() or in
    paint(), without having to reset them when the graphics resources are lost
    due to the window and QRhi change, since it all happens implicitly by the
    destruction of the whole renderer object.

    The below code snippet shows the typical structure of a
    QCanvasPainterItemRenderer subclass. See QCanvasPainterItem for an example of
    the \c MyItem class.

    \snippet renderer-ex-1.cpp 0

    \sa QCanvasPainterItem
*/


/*!
    Constructs a QCanvasPainterItemRenderer.
*/

QCanvasPainterItemRenderer::QCanvasPainterItemRenderer()
    : d_ptr(new QCanvasPainterItemRendererPrivate(this))
{
}

/*!
    Destroys the QCanvasPainterItemRenderer.
*/

QCanvasPainterItemRenderer::~QCanvasPainterItemRenderer()
{
    Q_D(QCanvasPainterItemRenderer);
    if (d->m_factory && !d->m_sharedPainter)
        delete d->m_factory;

    QObject::disconnect(d->m_sharedPainterNewFrameConn);

    delete d_ptr;
}


/*!
    \fn void QCanvasPainterItemRenderer::initializeResources(QCanvasPainter *painter)

    Reimplement this method to initialize resources using \a painter. This will
    be called once \b before the first synchronizeData().

    \note This function is not called when the size of the QCanvasPainterItem changes.

    \sa QCanvasPainter::addImage, QCanvasPainter::createCanvas
*/

void QCanvasPainterItemRenderer::initializeResources(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    This function is called at the start of rendering using \a painter, before
    paint().

    There is no render target active when this function is invoked. Call
    beginCanvasPainting() to initialize drawing into an offscreen canvas.

    \sa beginCanvasPainting(), endCanvasPainting()
*/
void QCanvasPainterItemRenderer::prePaint(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    \fn void QCanvasPainterItemRenderer::paint(QCanvasPainter *painter)

    Reimplement this method to paint using \a painter.

    This will get called after the item has been filled with fillColor().

    paint() is called from renderer thread. To access item data safely, copy it
    in synchronizeData().

    \sa synchronizeData()
*/
void QCanvasPainterItemRenderer::paint(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Reimplement this method to synchronize data between \a item and
    item painter instances. This will be called before paint() each
    time item needs to be repainted.

    This method is the only place where it is safe for the painter and the
    item to read and write each others variables.

    Usually you should static_cast \a item to your real item type, and then
    exchange the data.
*/
void QCanvasPainterItemRenderer::synchronizeData(QCanvasPainterItem *item)
{
    Q_UNUSED(item);
}

/*!
    \fn QColor QCanvasPainterItemRenderer::fillColor() const

    Return the current fill color of the item. This can be set
    by the parent QCanvasPainterItem.

*/

QColor QCanvasPainterItemRenderer::fillColor() const
{
    Q_D(const QCanvasPainterItemRenderer);
    return d->m_fillColor;
}

/*!
    Returns \c true if this item renderer uses a shared painter.

    \sa setSharedPainter
 */
bool QCanvasPainterItemRenderer::hasSharedPainter() const
{
    Q_D(const QCanvasPainterItemRenderer);
    return d->m_sharedPainter;
}

/*!
    Disable painter sharing if \a enable is \c false.

    Painter sharing is enabled by default.

    When painter sharing is enabled, all painter items inside the same
    QQuickWindow will use the same QCanvasPainter.

    If disabling painter sharing is desired, this function must be called early
    enough, for example from the derived class' constructor. Changing it
    afterwards, when the item has already initialized for painting, will have no
    effect.

    If two items use dedicated, non-shared painters, each other's graphics
    resources, such as the ones backing QCanvasImage or QOffscreenCanvas, will not be
    visible to them. Whereas if the items are in the same window, and sharing is
    enabled, they can use images or canvases created by the other item, because
    they both use the same QCanvasPainter.

    \note Even when \a enable is true, painters are not shared between
    items belonging to different QQuickWindow instances, and by extension, to
    different scene graphs.

    \sa hasSharedPainter
 */
void QCanvasPainterItemRenderer::setSharedPainter(bool enable)
{
    Q_D(QCanvasPainterItemRenderer);
    d->m_sharedPainter = enable;
}

/*!
    \fn QCanvasPainter *QCanvasPainterItemRenderer::painter() const

    Returns the painter attached to this painter item.
*/
QCanvasPainter *QCanvasPainterItemRenderer::painter() const
{
    Q_D(const QCanvasPainterItemRenderer);
    return d->m_factory && d->m_factory->isValid() ? d->m_factory->painter() : nullptr;
}

/*!
    Returns the width of the painted area, in logical units, without the
    \l{QQuickWindow::effectiveDevicePixelRatio()}{scale factor (device pixel
    ratio)}. This is usually the same as the painter item's
    \l{QQuickItem::}{width}, unless \l QQuickRhiItem::fixedColorBufferWidth has
    been set.
*/
float QCanvasPainterItemRenderer::width() const
{
    Q_D(const QCanvasPainterItemRenderer);
    return float(d->m_itemData.width);
}

/*!
    Returns the height of the painted area, in logical units, without the
    \l{QQuickWindow::effectiveDevicePixelRatio()}{scale factor (device pixel
    ratio)}. This is usually the same as the painter item's
    \l{QQuickItem::}{height}, unless \l QQuickRhiItem::fixedColorBufferHeight has
    been set.
*/
float QCanvasPainterItemRenderer::height() const
{
    Q_D(const QCanvasPainterItemRenderer);
    return float(d->m_itemData.height);
}

/*!
   \reimp
 */
void QCanvasPainterItemRenderer::synchronize(QQuickRhiItem * item)
{
    Q_D(QCanvasPainterItemRenderer);
    if (!item)
        return;

    d->m_window = item->window();
    QCanvasPainterItem *realItem = static_cast<QCanvasPainterItem*>(item);
    if (!realItem)
        return;

    // Make sure initialization has been done
    if (!d->m_initialized)
        initialize(nullptr);

    d->m_fillColor = realItem->fillColor();
    bool antialiasing = realItem->antialiasing();
    if (antialiasing != d->m_antialiasing) {
        d->m_antialiasing = antialiasing;
        auto *painter = d->m_factory->painter();
        painter->setRenderHint(QCanvasPainter::RenderHint::Antialiasing, antialiasing);
    }

    static bool collectDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_COLLECT");
    static bool renderDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_RENDER");

    if (collectDebug && d->m_renderedOnce)
        realItem->d_func()->updateDebugData(d->m_debugCounters);
    if (renderDebug)
        d->m_debugVis.start();

    if (!d->m_initializeResourcesCalled) {
        d->m_initializeResourcesCalled = true;
        initializeResources(d->m_factory->painter());
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 12, 0)
    // Keep synchronize() working with Qt 6.11.x
    synchronize(realItem);
#endif
    synchronizeData(realItem);
}

QCanvasPainterItemRendererPrivate::QCanvasPainterItemRendererPrivate(QCanvasPainterItemRenderer *q)
    : q_ptr(q)
{
}

QCanvasPainterItemRendererPrivate::~QCanvasPainterItemRendererPrivate()
{
}

/*!
    \reimp
 */
void QCanvasPainterItemRenderer::initialize(QRhiCommandBuffer *cb)
{
    Q_UNUSED(cb);
    Q_D(QCanvasPainterItemRenderer);
    if (!d->m_window)
        return;

    if (!d->m_rhi || rhi() != d->m_rhi) {
        d->m_rhi = rhi();
        if (!d->m_sharedPainter) {
            if (!d->m_factory)
                d->m_factory = new QCanvasPainterFactory;
            else
                d->m_factory->destroy();
        } else {
            d->m_factory = QCanvasPainterFactory::sharedInstance(d->m_rhi);
            d->m_sharedPainterNewFrameConn = QObject::connect(d->m_window, &QQuickWindow::beforeFrameBegin,
                                                              d->m_window, [this] {
                Q_D(QCanvasPainterItemRenderer);
                if (QCanvasPainterItemRendererPrivate::m_rendered.testAndSetAcquire(1, 0))
                    d->m_factory->paintDriver()->resetForNewFrame();
            }, Qt::DirectConnection);
        }
        if (!d->m_factory->isValid())
            d->m_factory->create(d->m_rhi);
        d->m_renderedOnce = false;
    }

    if (renderTarget() != d->m_rt) {
        if (d->m_rt) {
            // in case the QRhiRenderPassDescriptor is incompatible with the new rt
            if (d->m_factory && !d->m_sharedPainter) {
                d->m_factory->destroy();
                d->m_factory->create(d->m_rhi);
                d->m_renderedOnce = false;
            } // cannot do anything for the shared painter case
        }
        d->m_rt = renderTarget();
    }

    // Set dpr and width & height from the colorTexture.
    const qreal dpr = d->m_window->effectiveDevicePixelRatio();
    d->m_itemData.devicePixelRatio = float(dpr);
    if (auto ct = colorTexture()) {
        d->m_itemData.width = int(ct->pixelSize().width() / dpr);
        d->m_itemData.height = int(ct->pixelSize().height() / dpr);
    } else if (auto msaacb = msaaColorBuffer()) {
        d->m_itemData.width = int(msaacb->pixelSize().width() / dpr);
        d->m_itemData.height = int(msaacb->pixelSize().height() / dpr);
    }

    d->m_initialized = true;
}

/*!
    \reimp
 */
void QCanvasPainterItemRenderer::render(QRhiCommandBuffer *cb)
{
    Q_D(QCanvasPainterItemRenderer);
    if (!d->m_initialized)
        return;

    d->m_currentCb = cb;

    QCanvasRhiPaintDriver *pd = d->m_factory->paintDriver();
    QCanvasPainter *painter = d->m_factory->painter();

    QCanvasPainterPrivate::get(painter)->m_devicePixelRatio = d->m_itemData.devicePixelRatio;

    static bool collectDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_COLLECT");
    static bool renderDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_RENDER");
    QCPainterEngine *engine = QCanvasPainterFactoryPrivate::get(d->m_factory)->renderer.engine();

    if (!d->m_sharedPainter) {
        pd->resetForNewFrame();
    } else {
        // resetForNewFrame is hooked up to the QQuickWindow's signal when using
        // a shared painter.
        //
        // Draw statistics should only reflect this item's drawing, not all
        // items that use the same painter.
        engine->resetDebugCounters();
    }

    if ((d->m_itemData.width > 0 && d->m_itemData.height > 0)) {
        d->m_renderedOnce = true;
#ifdef QCPAINTER_PERF_DEBUG
        QCanvasPainterFactoryPrivate::get(d->m_factory)->renderer.engine()->perfLogger()->logStart(QCPerfLogging::PAINT);
#endif

        prePaint(painter);

        const float dpr = d->m_itemData.devicePixelRatio;
        const QSize outputLogicalSize = d->m_rt->pixelSize() / dpr;
        pd->beginPaint(cb, d->m_rt, d->m_fillColor, outputLogicalSize, dpr);

        paint(painter);

        if (renderDebug) {
            // This will show the numbers from the previous frame, because
            // m_debugCounters only gets the new numbers from the above prePaint()
            // and paint() once the content is rendered in endPaint()...
            d->m_debugVis.paint(painter, width(), height(), d->m_debugCounters);
            // ...so make it render continously to remedy this.
            update();
        }

        pd->endPaint();

#ifdef QCPAINTER_PERF_DEBUG
        QCanvasPainterFactoryPrivate::get(d->m_factory)->renderer.engine()->perfLogger()->logEnd(QCPerfLogging::PAINT);
#endif

        if (collectDebug || renderDebug) {
            engine->syncDebugCounters();
            d->m_debugCounters = engine->debugCounters();
        }

        QCanvasPainterItemRendererPrivate::m_rendered.storeRelease(1);
    }

    d->m_currentCb = nullptr;
}

/*!
    Starts recording QCanvasPainter draw commands targeting \a canvas.

    \note This function should only be called from prePaint().

    beginCanvasPainting() must always be followed by corresponding
    endCanvasPainting() before returning from prePaint().
 */
void QCanvasPainterItemRenderer::beginCanvasPainting(QCanvasOffscreenCanvas &canvas)
{
    Q_D(QCanvasPainterItemRenderer);
    if (!d->m_currentCb) {
        qWarning("beginCanvasPainting() can only be called from prePaint()");
        return;
    }

    d->m_factory->paintDriver()->beginPaint(canvas, d->m_currentCb);
}

/*!
    Indicates the end of the drawing targeting the canvas specified in
    beginCanvasPainting().

    \note This function should only be called from prePaint().

    beginCanvasPainting() must always be followed by corresponding
    endCanvasPainting() before returning from prePaint().
 */
void QCanvasPainterItemRenderer::endCanvasPainting()
{
    Q_D(QCanvasPainterItemRenderer);
    if (!d->m_currentCb)
        return;

    d->m_factory->paintDriver()->endPaint();
}

/*!
    Issues a texture readback request for \a canvas.

    \a callback is invoked either before the function returns, or later,
    depending on the underlying QRhi and 3D API implementation. Reading back
    texture contents may involve a GPU->CPU copy, depending on the GPU
    architecture.
 */
void QCanvasPainterItemRenderer::grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback)
{
    Q_D(QCanvasPainterItemRenderer);
    d->m_factory->paintDriver()->grabCanvas(canvas, callback);
}

QT_END_NAMESPACE
