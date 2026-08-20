// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasoffscreencanvas_p.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasOffscreenCanvas
    \since 6.11
    \brief QCanvasOffscreenCanvas represents an offscreen canvas that QCanvasPainter can target.
    \inmodule QtCanvasPainter

    An offscreen canvas is essentially a color buffer (a QRhiTexture), and
    possibly other associated graphics resources. After content has been
    rendered into the canvas's color buffer, the canvas can be
    \l{QCanvasPainter::addImage()}{registered} to the painter to retrieve a QCanvasImage
    that can then be used in \l{QCanvasPainter::drawImage()}{drawImage()} calls or as
    an \l{QCanvasImagePattern}{image pattern}.

    To create an offscreen canvas, call \l QCanvasPainter::createCanvas(). To target
    an offscreen canvas with with draw commands, call the appropriate \l
    QCanvasRhiPaintDriver::beginPaint() overload when working with the lower level
    API, or \l QCanvasPainterWidget::beginCanvasPainting() or \l
    QCanvasPainterItemRenderer::beginCanvasPainting() when using the convenience
    widget or Qt Quick item classes.

    Similarly to QCanvasImage and QCanvasBrush, QCanvasOffscreenCanvas is explicitly shared.
    See \l{Implicit Data Sharing} and \l QSharedDataPointer for details.

    \note As with QCanvasImage, a QCanvasOffscreenCanvas can be seen as an object merely
    containing handles. Even when a detach occurs, the actual resources, e.g.
    the underlying texture and the image data in it, are never actually copied
    or duplicated. The actual owner of any real graphics resources (e.g., a
    QRhiTexture) is the QCanvasPainter that handed out the QCanvasOffscreenCanvas via
    \l{QCanvasPainter::}{createCanvas()}.

    A canvas always belongs to the QCanvasPainter that created it. Manually
    destroying canvases is done by calling \l{QCanvasPainter::}{destroyCanvas()}. In
    most cases this will not be necessary, however, since the painter will
    automatically destroy any canvases during its own destruction.
 */

 /*!
    \enum QCanvasOffscreenCanvas::Flag

    Specifies the flags for the canvas.

    \value PreserveContents Indicates that the contents of the canvas is
    preserved when painting to it. This can have a negative effect on
    performance, depending on the GPU architecture. See
    \l{QRhiTextureRenderTarget::PreserveColorContents} for details. Setting
    this flag is not supported when the sample count is greater than 1.

    \value MipMaps Indicates that the canvas' backing texture should have
    mipmapping enabled. Setting this flag is not supported when the sample count
    is greater than 1. Note that the mipmap sequence is generated when calling
    \l{QCanvasPainter::}{addImage()} with the
    \l{QCanvasPainter::ImageFlag::}{GenerateMipmaps} flag set. This implies that
    addImage() must be called every time after the canvas' content changes, so
    that the mipmap chain is regenerated. Frequent mipmap generation can have a
    negative effect on performance.
 */

/*!
    \internal
*/
QCanvasOffscreenCanvas::QCanvasOffscreenCanvas()
    : d(new QCanvasOffscreenCanvasPrivate)
{
}

/*!
    Copy constructor.
*/
QCanvasOffscreenCanvas::QCanvasOffscreenCanvas(const QCanvasOffscreenCanvas &canvas)
    : d(canvas.d)
{
}

/*!
    Destructor.
*/
QCanvasOffscreenCanvas::~QCanvasOffscreenCanvas() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanvasOffscreenCanvasPrivate)

/*!
    Assigns a shallow copy of the given \a canvas to this canvas and returns a
    reference to this canvas. For more information about shallow copies, see the
    \l{Implicit Data Sharing}{implicit data sharing} documentation.

    \note The underlying resources, the graphics resources such as the
    QRhiTexture, are never copied or duplicated.
*/
QCanvasOffscreenCanvas &QCanvasOffscreenCanvas::operator=(const QCanvasOffscreenCanvas &canvas)
{
    QCanvasOffscreenCanvas(canvas).swap(*this);
    return *this;
}

/*!
    \fn bool QCanvasOffscreenCanvas::operator!=(const QCanvasOffscreenCanvas &lhs, const QCanvasOffscreenCanvas &rhs)

    \return \c true if the canvas handle \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasOffscreenCanvas::operator==(const QCanvasOffscreenCanvas &lhs, const QCanvasOffscreenCanvas &rhs)

    \return \c true if the canvas handle \a lhs is equal to \a rhs; \c false otherwise.

    \note Equality means that the two canvas objects' fill colors are the same and they
    reference the same graphics resources. The contents (pixel data) is not
    compared.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasOffscreenCanvas &lhs, const QCanvasOffscreenCanvas &rhs) noexcept
{
    auto *d = QCanvasOffscreenCanvasPrivate::get(&lhs);
    auto *pd = QCanvasOffscreenCanvasPrivate::get(&rhs);

    if (pd == d)
        return true;

    if (pd->rhiCanvas != d->rhiCanvas
        || pd->fillColor != d->fillColor)
    {
        return false;
    }

    return true;
}

/*!
    If multiple canvases share common data, this image makes a copy of the data
    and detaches itself from the sharing mechanism, making sure that this image
    is the only one referring to the data.

    Nothing is done if there is just a single reference.

    \note The canvas only contains handles to the underlying graphics resources,
    and data such as the fill color. The graphics resources themselves, such as
    the QRhiTexture, are never copied or duplicated.
 */
void QCanvasOffscreenCanvas::detach()
{
    if (d)
        d.detach();
    else
        d = new QCanvasOffscreenCanvasPrivate;
}

/*!
    \return true if this canvas has valid data.

    Normally this will always be true.

    \sa texture()
 */
bool QCanvasOffscreenCanvas::isNull() const
{
    return d->rhiCanvas.isNull();
}

/*!
    \return the flags with which the canvas was created.

    The flags are immutable and cannot be changed once the canvas has been
    created.
 */
QCanvasOffscreenCanvas::Flags QCanvasOffscreenCanvas::flags() const
{
    return d->rhiCanvas.flags;
}

/*!
    \return the current fill color.
 */
QColor QCanvasOffscreenCanvas::fillColor() const
{
    return d->fillColor;
}

/*!
    Sets the fill (clear) color to \a color.

    By default this is set to the value passed to
    \l{QCanvasPainter::}{createCanvas()}.
 */
void QCanvasOffscreenCanvas::setFillColor(const QColor &color)
{
    if (d->fillColor == color)
        return;

    detach();
    d->fillColor = color;
}

/*!
    \return the texture backing this offscreen canvas.

    \sa isNull()
 */
QRhiTexture *QCanvasOffscreenCanvas::texture() const
{
    return d->rhiCanvas.tex;
}

QT_END_NAMESPACE
