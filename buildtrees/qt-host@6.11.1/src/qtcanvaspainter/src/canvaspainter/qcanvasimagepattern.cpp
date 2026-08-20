// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasimagepattern.h"
#include "qcanvasimagepattern_p.h"
#include "qcanvaspainter_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasImagePattern
    \since 6.11
    \brief QCanvasImagePattern is a brush for painting image patterns.
    \inmodule QtCanvasPainter

    QCanvasImagePattern is a brush for painting image patterns.
    To use QCanvasImage as a pattern, it should have some repeat
    flags turned on like \l QCanvasPainter::ImageFlag::Repeat.

    Here is a simple example:
    \code
    // Setup 2 image patterns
    static QImage image1(":/pattern2.png");
    static QImage image2(":/pattern3.png");
    auto flags = QCanvasPainter::ImageFlag::Repeat |
                 QCanvasPainter::ImageFlag::GenerateMipmaps;
    QCanvasImage bg1 = painter.addImage(image1, flags);
    QCanvasImage bg2 = painter.addImage(image2, flags);
    QCanvasImagePattern ip1(bg1, 0, 0, 64, 64);
    QCanvasImagePattern ip2(bg2, 0, 0, 32, 32);
    // Fill and stroke round rectangle with
    // these image pattern brushes.
    painter.beginPath();
    painter.roundRect(50, 50, 180, 180, 40);
    painter.setFillStyle(ip1);
    painter.setStrokeStyle(ip2);
    painter.fill();
    painter.setLineWidth(20);
    painter.stroke();
    \endcode

    \image imagepattern_example_1.png

    \note When using image patterns, images are often scaled to smaller
    and it can be useful to set \l QCanvasPainter::ImageFlag::GenerateMipmaps
    for the used QCanvasImage to have smooth patterns.
*/

/*!
    Constructs a default image pattern.
    Image will not be set, please use setImage() after this constructor.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0 and tint color white (no tinting).

    \sa setImage()
*/

QCanvasImagePattern::QCanvasImagePattern()
    : QCanvasBrush(new QCanvasImagePatternPrivate)
{
}

/*!
    Constructs an image pattern.
    Pattern will use \a image. Please note that \a image should usually
    have at least \l QCanvasPainter::ImageFlag::Repeat flag set.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0 and tint color white (no tinting).
*/

QCanvasImagePattern::QCanvasImagePattern(const QCanvasImage &image)
    : QCanvasBrush(new QCanvasImagePatternPrivate)
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    d->image = image;
    d->width = d->image.width();
    d->height = d->image.height();
}

/*!
    Constructs an image pattern.
    Pattern will use \a image. Please note that \a image should usually
    have at least \l QCanvasPainter::ImageFlag::Repeat flag set.
    Pattern image position position and size is defined with \a rect.
    Pattern angle is \a angle and tint color is \a tintColor.
*/

QCanvasImagePattern::QCanvasImagePattern(const QCanvasImage &image, const QRectF &rect, float angle, const QColor &tintColor)
    : QCanvasBrush(new QCanvasImagePatternPrivate)
{
    auto *d = QCanvasImagePatternPrivate::get(this);

    d->image = image;
    d->x = float(rect.x());
    d->y = float(rect.y());
    d->width = float(rect.width());
    d->height = float(rect.height());
    d->angle = angle;
    d->tintColor = tintColor;
}

/*!
    Constructs an image pattern.
    Pattern will use \a image. Please note that \a image should usually
    have at least \l QCanvasPainter::ImageFlag::Repeat flag set.
    Pattern image start position position is ( \a x, \a y) and pattern size ( \a width, \a height).
    Pattern angle is \a angle and tint color is \a tintColor.
*/

QCanvasImagePattern::QCanvasImagePattern(const QCanvasImage &image, float x, float y, float width, float height, float angle, const QColor &tintColor)
    : QCanvasBrush(new QCanvasImagePatternPrivate)
{
    auto *d = QCanvasImagePatternPrivate::get(this);

    d->image = image;
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->angle = angle;
    d->tintColor = tintColor;
}

/*!
    Destroys the image pattern.
*/

QCanvasImagePattern::~QCanvasImagePattern() = default;

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanvasImagePatternPrivate)

/*!
   Returns the image pattern as a QVariant.
*/

QCanvasImagePattern::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasImagePattern::operator!=(const QCanvasImagePattern &lhs, const QCanvasImagePattern &rhs)

    \return \c true if the image pattern \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasImagePattern::operator==(const QCanvasImagePattern &lhs, const QCanvasImagePattern &rhs)

    \return \c true if the image pattern \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasImagePattern &lhs, const QCanvasImagePattern &rhs) noexcept
{
    auto *d = QCanvasImagePatternPrivate::get(&lhs);
    auto *pd = QCanvasImagePatternPrivate::get(&rhs);

    if (pd == d)
        return true;

    if (d->x != pd->x
        || d->y != pd->y
        || d->width != pd->width
        || d->height != pd->height
        || d->angle != pd->angle
        || d->tintColor != pd->tintColor)
        return false;

    // For images comparing the id is enough
    if (d->image.id() != pd->image.id())
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasImagePattern &p)
{
    QDebugStateSaver saver(dbg);
    const auto &sp = p.startPosition();
    const auto &size = p.imageSize();
    dbg.nospace() << "QCanvasImagePattern(" << sp << ", " << size << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCanvasImagePattern stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasImagePattern &pattern)
    \relates QCanvasImagePattern

    Writes the given \a pattern to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCanvasImagePattern &p)
{
    const auto &sp = p.startPosition();
    const auto &size = p.imageSize();
    s << sp.x() << sp.y();
    s << size.width() << size.height();
    s << p.rotation() << p.tintColor();
    // Note: Image not handled as it is mostly ID into texture
    //s << p.image();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasImagePattern &pattern)
    \relates QCanvasImagePattern

    Reads the given \a pattern from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCanvasImagePattern &p)
{
    float x, y, width, height, angle;
    QColor tintColor;
    s >> x >> y >> width >> height >> angle>> tintColor;
    p.setStartPosition(x, y);
    p.setImageSize(width, height);
    p.setRotation(angle);
    p.setTintColor(tintColor);
    // Note: Image not handled as it is mostly ID into texture
    //QImage image;
    //s >> image;
    //p.setImage(std::move(image));
    return s;
}

#endif // QT_NO_DATASTREAM


/*!
    Returns the start point of image pattern.
    \sa setStartPosition()
*/

QPointF QCanvasImagePattern::startPosition() const
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    return QPointF(d->x, d->y);
}

/*!
    Sets the start point of image pattern to (\a x, \a y).
    Start position means top-left corner of an image in pattern.
    Pattern will then be extended to all positions from here
    (if image \c Repeat flag has been set).
*/

void QCanvasImagePattern::setStartPosition(float x, float y)
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    detach();
    d->x = x;
    d->y = y;
    d->changed = true;
}

/*!
    \fn void QCanvasImagePattern::setStartPosition(QPointF point)
    \overload

    Sets the start point of image pattern to \a point.
    Start position means top-left corner of an image in pattern.
    Pattern will then be extended to all positions from here
    (if image \c Repeat flag has been set).
*/

/*!
    Returns the size of a single image in pattern.
    \sa setImageSize()
*/

QSizeF QCanvasImagePattern::imageSize() const
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    return QSizeF(d->width,
                  d->height);
}

/*!
    Sets the size of a single image in pattern to ( \a width, \a height).
*/

void QCanvasImagePattern::setImageSize(float width, float height)
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    detach();
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    \fn void QCanvasImagePattern::setImageSize(QSizeF size)
    \overload

    Sets the size of a single image in pattern to \a size.
*/

/*!
    Returns the image of the pattern.
    \sa setImage()
*/
QCanvasImage QCanvasImagePattern::image() const
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    return d->image;
}

/*!
    Sets the image of the pattern to \a image.
    Note: Image should usually have \c Repeat flag set
    when used in image pattern.
*/

void QCanvasImagePattern::setImage(const QCanvasImage &image)
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    detach();
    d->image = image;
    d->changed = true;
}

/*!
    Returns the pattern rotation in radians.
*/

float QCanvasImagePattern::rotation() const
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    return d->angle;
}

/*!
    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the image startPosition().
    The default value is \c 0.0, meaning the image is not rotated.
*/

void QCanvasImagePattern::setRotation(float rotation)
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    detach();
    d->angle = rotation;
    d->changed = true;
}

/*!
    Returns the pattern tint color.
*/

QColor QCanvasImagePattern::tintColor() const
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    return d->tintColor;
}

/*!
    Sets the pattern tint color to \a color.
    The color of the pattern image will be multiplied with
    this tint color in the shader.
    The default value is white, meaning no tinting.
    \note To set alpha globally, use QCanvasPainter::setGlobalAlpha()
*/

void QCanvasImagePattern::setTintColor(const QColor &color)
{
    auto *d = QCanvasImagePatternPrivate::get(this);
    detach();
    d->tintColor = color;
    d->changed = true;
}

// ***** Private *****

/*!
   \internal
*/
#define DECONST(d) const_cast<QCanvasImagePatternPrivate *>(d)

QCPaint QCanvasImagePatternPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    auto *painterPriv = QCanvasPainterPrivate::get(painter);
    if (d->changed) {
        auto *e = painterPriv->engine();
        if (d->image.isNull()) {
            qWarning() << "No image set for pattern, please use setImage()";
        } else {
            DECONST(d)->paint = e->createImagePattern(d->x, d->y, d->width, d->height,
                                             d->image.id(), d->angle, d->tintColor);
        }
        DECONST(d)->changed = false;
    }
    painterPriv->markTextureIdUsed(d->image.id());
    return d->paint;
}

QCanvasBrushPrivate *QCanvasImagePatternPrivate::clone()
{
    return new QCanvasImagePatternPrivate(*this);
}

#undef DECONST

QT_END_NAMESPACE
