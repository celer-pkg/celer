// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvaspainterwidget_p.h"
#include "qcanvaspainter_p.h"
#include "qcanvaspainterfactory_p.h"
#include "engine/qcpainterengine_p.h"
#include "engine/qcpainterrhirenderer_p.h"
#include "qcanvaspainterfactory.h"
#include "qcanvasrhipaintdriver.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

// Does not really need to be atomic since widgets operate on gui thread, but for consistency with the Quick item.
QAtomicInt QCanvasPainterWidgetPrivate::m_rendered;


/*!
    \class QCanvasPainterWidget
    \since 6.11
    \brief QCanvasPainterWidget is a widget for rendering using QCanvasPainter.
    \inmodule QtCanvasPainter

    Implement the \l paint virtual function in a subclass to perform rendering
    using a QCanvasPainter.

    The below code snippet shows the typical structure of a QCanvasPainterWidget
    subclass:

    \snippet widget-ex-1.cpp 0
*/

/*!
   Constructs a QCanvasPainterWidget with the given \a parent.
 */

QCanvasPainterWidget::QCanvasPainterWidget(QWidget *parent)
    : QRhiWidget(*new QCanvasPainterWidgetPrivate, parent)
{
}

/*!
   Destroys the QCanvasPainterWidget.
 */
QCanvasPainterWidget::~QCanvasPainterWidget()
{
    Q_D(QCanvasPainterWidget);
    if (!d->m_sharedPainter) {
        // Must be destroyed here, not in the Private dtor, see ~QRhiWidget().
        delete d->m_factory;
    }
}

/*!
   \reimp
*/
void QCanvasPainterWidget::releaseResources()
{
    Q_D(QCanvasPainterWidget);

    // Happens f.ex. when the widget is moved to a new top-level window. The
    // QRhi is then different, and there will be an initialize() following this.
    // We'll need a new renderer then.

    graphicsResourcesInvalidated();

    if (!d->m_sharedPainter)
        d->m_factory->destroy();
    else
        d->m_factory = nullptr;
}

/*!
    \fn QColor QCanvasPainterWidget::fillColor() const

    Returns the current fill color.

    \sa setFillColor()
*/

QColor QCanvasPainterWidget::fillColor() const
{
    Q_D(const QCanvasPainterWidget);
    return d->m_fillColor;
}

/*!
    \fn void QCanvasPainterWidget::setFillColor (const QColor &color)

    Set the fill color to \a color. This color will be used to draw
    the background of the item. The default color is black.

    \sa fillColor()
*/

void QCanvasPainterWidget::setFillColor(const QColor &color)
{
    Q_D(QCanvasPainterWidget);
    if (d->m_fillColor == color)
        return;
    d->m_fillColor = color;

    update();
}

/*!
    Returns \c true if this widget uses a shared painter.

    \sa setSharedPainter
 */
bool QCanvasPainterWidget::hasSharedPainter() const
{
    Q_D(const QCanvasPainterWidget);
    return d->m_sharedPainter;
}

/*!
    Disable painter sharing if \a enable is \c false.

    If painter sharing is enabled, all QCanvasPainterWidget instances inside the same QWindow will use
    the same QCanvasPainter. This function must be early, e.g. from the derived class' constructor,
    and must not be changed afterwards.

    Painter sharing is enabled by default.

    If two widgets use dedicated, non-shared painters, each other's graphics
    resources, such as the ones backing QCanvasImage or QOffscreenCanvas, will not be
    visible to them. Whereas if the widgets are in the same window, and sharing
    is enabled, they can use images or canvases created by the other widget,
    because they both use the same QCanvasPainter.

    \note Even when \a enable is true, painters are not shared between
    widgets belonging to different windows (top-level widgets).

    \sa hasSharedPainter
 */
void QCanvasPainterWidget::setSharedPainter(bool enable)
{
    Q_D(QCanvasPainterWidget);
    d->m_sharedPainter = enable;
}

/*!
   \reimp
*/
void QCanvasPainterWidget::initialize(QRhiCommandBuffer *)
{
    Q_D(QCanvasPainterWidget);
    bool callInitRes = false;
    if (!d->m_factory) {
        callInitRes = true;
        if (!d->m_sharedPainter) {
            d->m_factory = new QCanvasPainterFactory;
        } else {
            // m_factory here is not owned by us, and is valid as long as the QRhi lives
            d->m_factory = QCanvasPainterFactory::sharedInstance(rhi());
            connect(this, &QRhiWidget::frameSubmitted, this, [this] {
                Q_D(QCanvasPainterWidget);
                if (QCanvasPainterWidgetPrivate::m_rendered.testAndSetAcquire(1, 0))
                    d->m_factory->paintDriver()->resetForNewFrame();
            });
        }
    }

    if (!d->m_factory->isValid()) {
        d->m_factory->create(rhi());
        callInitRes = true;
    }

    if (callInitRes && d->m_factory->isValid())
        initializeResources(d->m_factory->painter());
}

/*!
   \reimp
*/
void QCanvasPainterWidget::render(QRhiCommandBuffer *cb)
{
    Q_D(QCanvasPainterWidget);
    if (!d->m_factory || !d->m_factory->isValid())
        return;

    d->m_currentCb = cb;

    static bool renderDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_RENDER");
    QCPainterEngine *engine = QCanvasPainterFactoryPrivate::get(d->m_factory)->renderer.engine();
    if (renderDebug)
        d->m_debugVis.start();

    QCanvasRhiPaintDriver *pd = d->m_factory->paintDriver();
    QCanvasPainter *painter = d->m_factory->painter();

    if (!d->m_sharedPainter) {
        pd->resetForNewFrame();
    } else {
        // Draw statistics should only reflect this widget's drawing, not all
        // widgets that use the same painter.
        engine->resetDebugCounters();
    }

    prePaint(painter);

    pd->beginPaint(cb, renderTarget(), d->m_fillColor, size(), devicePixelRatio());

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

    if (renderDebug) {
        engine->syncDebugCounters();
        d->m_debugCounters = engine->debugCounters();
    }

    d->m_currentCb = nullptr;

    QCanvasPainterWidgetPrivate::m_rendered.storeRelease(1);
}

/*!
    Reimplement this method to initialize resources using \a painter. Generally,
    this will be called once before the first prePaint() and paint(). An
    exception is when graphics resources are lost, see
    graphicsResourcesInvalidated(). In that case, this method will get invoked
    again afterwards.

    The default implementation is empty.
*/
void QCanvasPainterWidget::initializeResources(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Reimplement this function to perform drawing into one or more offscreen
    canvases using \a painter.

    The default implementation is empty.

    \sa beginCanvasPainting(), endCanvasPainting()
 */
void QCanvasPainterWidget::prePaint(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Reimplement this method to paint using \a painter.

    The widget is first filled with fillColor().

    The default implementation is empty.
 */
void QCanvasPainterWidget::paint(QCanvasPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Called when underlying graphics resources, such as textures, are lost.

    This indicates that QCanvasImage objects returned from addImage() are no longer
    valid, and addImage() needs to be called again. If the paint()
    implementation is such that this does not matter, for example because images
    are not used, or addImage() is always called, then no action is necessary.
    Otherwise, it is recommended to toggle a flag, or similar, and act
    accordingly in the next invocation of paint().

    The same applies to QCanvasOffscreenCanvas objects returned from
    QCanvasPainter::createCanvas(). When this function is called, the next invocation
    of paint() should create new canvases and redraw their contents.

    Graphics resources can be lost, for example, when the widget is moved to a
    new top-level window, because that implies being associated with a new QRhi
    instance.

    \sa QRhiWidget::releaseResources()
 */
void QCanvasPainterWidget::graphicsResourcesInvalidated()
{
}

/*!
    Starts recording QCanvasPainter draw commands targeting \a canvas.

    \note This function should only be called from prePaint().

    beginCanvasPainting() must always be followed by corresponding
    endCanvasPainting() before returning from prePaint().

    The following snippet from a QCanvasPainterWidget subclass shows how an offscreen
    canvas could be rendered into and then used as an image or image pattern
    when drawing the contents for the widget:

    \snippet widget-canvas-ex-1.cpp 0
 */
void QCanvasPainterWidget::beginCanvasPainting(QCanvasOffscreenCanvas &canvas)
{
    Q_D(QCanvasPainterWidget);
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
void QCanvasPainterWidget::endCanvasPainting()
{
    Q_D(QCanvasPainterWidget);
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
void QCanvasPainterWidget::grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback)
{
    Q_D(QCanvasPainterWidget);
    d->m_factory->paintDriver()->grabCanvas(canvas, callback);
}

QT_END_NAMESPACE
