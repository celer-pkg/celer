// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasimage.h"
#include "qcanvasimage_p.h"
#include "qvariant.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasImage
    \since 6.11
    \brief QCanvasImage is the image class for QCanvasPainter.
    \inmodule QtCanvasPainter

    QCanvasImage is the image class used by QCanvasPainter.
    To be able to paint images, they fist need to be made available
    with e.g. \l QCanvasPainter::addImage(). Then images can be painted
    as-is using \l QCanvasPainter::drawImage() or used with
    \l QCanvasImagePattern brush to fill / stroke.

    Here is a simple example:
    \code
    static QImage logoImage(":/qtlogo.png");
    // Paint an image pattern.
    QCanvasImage bg = painter.addImage(logoImage,
                 QCanvasPainter::ImageFlag::Repeat |
                 QCanvasPainter::ImageFlag::GenerateMipmaps);
    QCanvasImagePattern ip(bg, 0, 0, 44, 32);
    painter.setFillStyle(ip);
    painter.fillRect(50, 50, 320, 230);
    // Paint a single image, with tint color.
    QCanvasImage logo = painter.addImage(logoImage);
    logo.setTintColor("#2cde85");
    painter.drawImage(logo, 100, 80);
    \endcode

    \image image_example_1.png

    In the above example the QImage is static and addImage() is called
    on every repaint. This is not a problem as when the image and flags
    remain the same, addImage() fetches the image from cache instead of
    uploads it again as a texture. But a more common approac is having
    QCanvasImage variables as class members and calling \l QCanvasPainter::addImage()
    e.g. in \l QCanvasPainterItemRenderer::initializeResources().

    Similarly to QCanvasBrush and QCanvasOffscreenCanvas, QCanvasImage is explicitly shared.
    See \l{Implicit Data Sharing} and \l QSharedDataPointer for details.

    \note A QCanvasImage object contains only a handle to a graphics resource, such
    as a texture. Even when a detach occurs, the actual resource, i.e. the
    underlying texture and the image data in it, is never actually copied or
    duplicated. The actual owner of the real graphics resource (e.g., a
    QRhiTexture) is the QCanvasPainter that handed out the QCanvasImage via
    \l{QCanvasPainter::}{addImage()}.

    A QCanvasImage always belongs to the QCanvasPainter that created it. Manually
    removing images is done by calling \l{QCanvasPainter::}{removeImage()}. In
    most cases this will not be necessary, however, since the painter will
    automatically destroy any images during its own destruction.
*/

/*!
    Constructs a default image.
*/

QCanvasImage::QCanvasImage()
    : d(new QCanvasImagePrivate())
{
}

/*!
    Constructs an image that is a copy of the given \a image.
*/

QCanvasImage::QCanvasImage(const QCanvasImage &image)
    : d(image.d)
{
}

/*!
    Destroys the image.
*/

QCanvasImage::~QCanvasImage() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanvasImagePrivate)

/*!
    Assigns the given \a image to this image and returns a reference to
    this image.
*/

QCanvasImage &QCanvasImage::operator=(const QCanvasImage &image)
{
    QCanvasImage(image).swap(*this);
    return *this;
}

/*!
    \fn QCanvasImage::QCanvasImage(QCanvasImage &&other) noexcept

    Move-constructs a new QCanvasImage from \a other.
*/

/*!
    \fn QCanvasImage &QCanvasImage::operator=(QCanvasImage &&other)

    Move-assigns \a other to this QCanvasImage instance.
*/

/*!
    \fn void QCanvasImage::swap(QCanvasImage &other)
    \memberswap{image}
*/

/*!
   Returns the image as a QVariant.
*/

QCanvasImage::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasImage::operator!=(const QCanvasImage &lhs, const QCanvasImage &rhs)

    \return \c true if the image handle \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasImage::operator==(const QCanvasImage &lhs, const QCanvasImage &rhs)

    \return \c true if the image handle \a lhs is equal to \a rhs; \c false otherwise.

    \note Equality means that the two image objects reference the same graphics
    resources and the tint colors are the same. The contents (pixel data) is not
    compared.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasImage &lhs, const QCanvasImage &rhs) noexcept
{
    auto *d = QCanvasImagePrivate::get(&lhs);
    auto *pd = QCanvasImagePrivate::get(&rhs);
    if (pd == d)
        return true;

    if (pd->id != d->id ||
        pd->width != d->width ||
        pd->height != d->height ||
        pd->sizeInBytes != d->sizeInBytes ||
        pd->type != d->type ||
        pd->tintColor != d->tintColor)
        return false;
    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasImage &i)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCanvasImage(" << i.id() << ": " << i.width() << " X " << i.height() << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*!
    Returns the texture id for this image.
    When the image has not been assigned yet, this returns 0.
    \sa isNull()
*/

int QCanvasImage::id() const
{
    return d->id;
}

/*!
    Returns the width of this image.
*/

int QCanvasImage::width() const
{
    return d->width;
}

/*!
    Returns the height of this image.
*/

int QCanvasImage::height() const
{
    return d->height;
}

/*!
    \fn QSize QCanvasImage::size() const

    \return the size (width and height) of this image.
 */

/*!
    Returns the size of this image in bytes.

    QCanvasPainter does not keep copies of the CPU-side QImage data once
    \l{QCanvasPainter::}{addImage()} has returned. If the source is a
    \l{QCanvasOffscreenCanvas}{offscreen canvas}, then there is no CPU-side
    image data in the first place. Therefore, the result of this function is an
    approximation of the GPU memory that is used for the underlying texture.

    \note The value is only an estimate based on the image format and
    dimensions. Qt has no knowledge of how the data for textures is stored and
    laid out in memory on the GPU side.

    \note This function does not take mipmap or multisample data into
    consideration.
*/

qsizetype QCanvasImage::sizeInBytes() const
{
    return d->sizeInBytes;
}

/*!
    Returns true if the image has not been assigned yet.
    See \l QCanvasPainter::addImage() for details.
*/

bool QCanvasImage::isNull() const
{
    return d->id == 0;
}

/*!
    Returns the tint color of the image.
    The default value is Qt::white.
*/

QColor QCanvasImage::tintColor() const
{
    return d->tintColor;
}

/*!
    Set the tint \a color of the image.
    This color will be multiplied with the image color to
    colorize the image. This can be used for example to highlight
    icons or to adjust images becase on the theme.
*/

void QCanvasImage::setTintColor(const QColor &color)
{
    detach();
    d->tintColor = color;
}

/*!
    \internal
*/

void QCanvasImage::detach()
{
    if (d)
        d.detach();
    else
        d = new QCanvasImagePrivate();
}

// ***** Private *****

QCanvasImagePrivate::QCanvasImagePrivate()
    : id(0)
    , width(0)
    , height(0)
    , type(DataType::Unknown)
    , sizeInBytes(0)
    , tintColor(QColorConstants::White)
{
}

QT_END_NAMESPACE
