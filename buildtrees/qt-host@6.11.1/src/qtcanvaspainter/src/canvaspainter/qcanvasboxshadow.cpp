// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasboxshadow.h"
#include "qcanvasboxshadow_p.h"
#include "qcanvaspainter_p.h"

QT_BEGIN_NAMESPACE

static const float QCPAINTER_BOX_SHADOW_MULTIPLIER = 1.0f;

/*!
    \class QCanvasBoxShadow
    \since 6.11
    \brief QCanvasBoxShadow is a brush for painting rectangle shadows / glows.
    \inmodule QtCanvasPainter

    QCanvasBoxShadow is a rounded rectangle brush with blur applied.
    The performance of QCanvasBoxShadow is much better than a general shadow that
    creates blurred shadow/glow of any shaped item, as it uses SDF approach.

    The features of QCanvasBoxShadow are similar to CSS box-shadow, with radius,
    spread, blur and color values.
    The rendering output also matches the CSS box-shadow, with few notable
    differences to make the QCanvasBoxShadow as high-performance as possible.
    Blurring is calculated mathematically in the shader rather than using
    Gaussian blur, which CSS box-shadow implementations often use. This makes
    the shadow look slightly different, especially when the blur amount
    grows bigger than half of the shadow width / height.

    Here are screenshots to compare the rendering output of QCanvasBoxShadow and
    CSS box-shadow in the Chrome browser. These show how blur radius of
    0, 10, 20 and 40 pixels affect smaller shadows.

    CSS box-shadow:
    \image examples_css_box_shadow.png

    QCanvasBoxShadow:
    \image examples_qcpainter_box_shadow.png
*/


#define B_D() auto *d = static_cast<QCanvasBoxShadowPrivate*>(baseData.get())

/*!
    Constructs a default box shadow.
    The box shadow position is (0, 0) and size (100, 100).
    Shadow corner radius is \c 0.0 and blur \c 0.0.
    Shadow color is \c Qt::black.
*/

QCanvasBoxShadow::QCanvasBoxShadow()
    : QCanvasBrush(new QCanvasBoxShadowPrivate)
{
}

/*!
    Constructs a box shadow.
    The box shadow position and size are defined by \a rect.
    Shadow corner radius is \a radius and blur \a blur.
    Shadow color is \a color.
*/

QCanvasBoxShadow::QCanvasBoxShadow(const QRectF &rect, float radius, float blur, const QColor &color)
    : QCanvasBrush(new QCanvasBoxShadowPrivate)
{
    B_D();
    d->x = float(rect.x());
    d->y = float(rect.y());
    d->width = float(rect.width());
    d->height = float(rect.height());
    d->radius = radius;
    d->blur = blur;
    d->color = color;
}

/*!
    Constructs a box shadow.
    The box shadow position is \a x, \a y and size \a width, \a height.
    Shadow corner radius is \a radius and blur \a blur.
    Shadow color is \a color.
*/

QCanvasBoxShadow::QCanvasBoxShadow(float x, float y, float width, float height, float radius, float blur, const QColor &color)
    : QCanvasBrush(new QCanvasBoxShadowPrivate)
{
    B_D();
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->radius = radius;
    d->blur = blur;
    d->color = color;
}

QCanvasBoxShadow::~QCanvasBoxShadow() = default;

QCanvasBoxShadow::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasBoxShadow::operator!=(const QCanvasBoxShadow &lhs, const QCanvasBoxShadow &rhs)

    \return \c true if the box shadow \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasBoxShadow::operator==(const QCanvasBoxShadow &lhs, const QCanvasBoxShadow &rhs)

    \return \c true if the box shadow \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasBoxShadow &lhs, const QCanvasBoxShadow &rhs) noexcept
{
    auto *d = QCanvasBoxShadowPrivate::get(&lhs);
    auto *pd = QCanvasBoxShadowPrivate::get(&rhs);
    if (pd == d)
        return true;

    if (d->x != pd->x
        || d->y != pd->y
        || d->width != pd->width
        || d->height != pd->height
        || d->radius != pd->radius
        || d->blur != pd->blur
        || d->color != pd->color
        || d->topLeftRadius != pd->topLeftRadius
        || d->topRightRadius != pd->topRightRadius
        || d->bottomLeftRadius != pd->bottomLeftRadius
        || d->bottomRightRadius != pd->bottomRightRadius)
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasBoxShadow &p)
{
    QDebugStateSaver saver(dbg);
    const auto &rect = p.rect();
    dbg.nospace() << "QCanvasBoxShadow(" << rect << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCanvasBoxShadow stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasBoxShadow &shadow)
    \relates QCanvasBoxShadow

    Writes the given \a shadow to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCanvasBoxShadow &p)
{
    const auto &r = p.rect();
    s << r.x() << r.y() << r.width() << r.height();
    s << p.radius() << p.blur() << p.spread();
    s << p.topLeftRadius() << p.topRightRadius();
    s << p.bottomLeftRadius() << p.bottomRightRadius();
    s << p.color();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasBoxShadow &shadow)
    \relates QCanvasBoxShadow

    Reads the given \a shadow from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCanvasBoxShadow &p)
{
    float x, y, width, height;
    float radius, blur, spread;
    float tlRad, trRad, blRad, brRad;
    QColor color;
    s >> x >> y >> width >> height;
    s >> radius >> blur >> spread;
    s >> tlRad >> trRad >> blRad >> brRad;
    s >> color;
    p.setRect(x, y, width, height);
    p.setRadius(radius);
    p.setBlur(blur);
    p.setSpread(spread);
    p.setTopLeftRadius(tlRad);
    p.setTopRightRadius(trRad);
    p.setBottomLeftRadius(blRad);
    p.setBottomRightRadius(brRad);
    p.setColor(color);
    return s;
}

#endif // QT_NO_DATASTREAM

/*!
    Returns the rect area of shadow box.
    \sa setRect()
*/

QRectF QCanvasBoxShadow::rect() const
{
    B_D();
    return QRectF(d->x,
                  d->y,
                  d->width,
                  d->height);
}

/*!
    \fn void QCanvasBoxShadow::setRect(const QRectF &rect)
    \overload

    Sets the rect area of shadow box to \a rect.
*/

/*!
    Sets the rect area of shadow box to (\a x, \a y, \a width, \a height).
    \sa rect()
*/

void QCanvasBoxShadow::setRect(float x, float y, float width, float height)
{
    B_D();
    detach();
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    \readonly

    Returns the area covered by the shadow. This takes into
    account the shadow \l rect(), \l blur() and \l spread().
    If you don't want to use \l QCanvasPainter::drawBoxShadow() helper
    method, this can be used to for e.g. adding correctly sized
    \l QCanvasPainter::rect() into the path.

    \sa setRect(), setBlur(), setSpread()
*/

QRectF QCanvasBoxShadow::boundingRect() const
{
    B_D();
    // Extend the rect with blur, spread and aa
    const float aa = 1.0f;
    const float extend = d->blur + d->spread + aa;
    const float x = d->x - extend;
    const float y = d->y - extend;
    const float width = d->width + 2 * extend;
    const float height = d->height + 2 * extend;
    return QRectF(x, y, width, height);
}

/*!
    Returns the radius of shadow box.
    \sa setRadius()
*/

float QCanvasBoxShadow::radius() const
{
    B_D();
    return d->radius;
}

/*!
    Sets the shadow corner radius to \a radius in pixels.
    The default value is \c 0.0 meaning no radius.
*/

void QCanvasBoxShadow::setRadius(float radius)
{
    B_D();
    detach();
    d->radius = radius;
    d->changed = true;
}

/*!
    Returns the blur of shadow box.
    \sa setBlur()
*/

float QCanvasBoxShadow::blur() const
{
    B_D();
    return d->blur;
}

/*!
    Sets the shadow blur to \a blur in pixels.
    The default value is \c 0.0 meaning no blur.
*/

void QCanvasBoxShadow::setBlur(float blur)
{
    B_D();
    detach();
    d->blur = blur;
    d->changed = true;
}

/*!
    Returns the spread of shadow box.
    \sa setSpread()
*/

float QCanvasBoxShadow::spread() const
{
    B_D();
    return d->spread;
}

/*!
    Sets the shadow spread to \a spread in pixels.
    The default value is \c 0.0 meaning no spread.
*/

void QCanvasBoxShadow::setSpread(float spread)
{
    B_D();
    detach();
    d->spread = spread;
    d->changed = true;
}

/*!
    Returns the color of shadow box.
    \sa setColor()
*/

QColor QCanvasBoxShadow::color() const
{
    B_D();
    return d->color;
}

/*!
    Sets the shadow color to \a color.
    The default value is black with full opacity.
*/

void QCanvasBoxShadow::setColor(const QColor &color)
{
    B_D();
    detach();
    d->color = color;
    d->changed = true;
}

/*!
    Returns the top-left radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvasBoxShadow::topLeftRadius() const
{
    B_D();
    return d->topLeftRadius;
}

/*!
    Sets the top-left corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvasBoxShadow::setTopLeftRadius(float radius)
{
    B_D();
    detach();
    d->topLeftRadius = radius;
    d->changed = true;
}

/*!
    Returns the top-right radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvasBoxShadow::topRightRadius() const
{
    B_D();
    return d->topRightRadius;
}

/*!
    Sets the top-right corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvasBoxShadow::setTopRightRadius(float radius)
{
    B_D();
    detach();
    d->topRightRadius = radius;
    d->changed = true;
}

/*!
    Returns the bottom-left radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvasBoxShadow::bottomLeftRadius() const
{
    B_D();
    return d->bottomLeftRadius;
}

/*!
    Sets the bottom-left corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvasBoxShadow::setBottomLeftRadius(float radius)
{
    B_D();
    detach();
    d->bottomLeftRadius = radius;
    d->changed = true;
}

/*!
    Returns the bottom-right radius of shadow box.
    When this is \c -1, painting of the shadow
    will use the value of \l radius() for this corner.
    The default value is \c -1.
*/

float QCanvasBoxShadow::bottomRightRadius() const
{
    B_D();
    return d->bottomRightRadius;
}

/*!
    Sets the bottom-right corner radius to \a radius.
    The default value is \c -1 which means that individual
    radius has not been set for this corner, and common \l radius()
    is used instead.
*/

void QCanvasBoxShadow::setBottomRightRadius(float radius)
{
    B_D();
    detach();
    d->bottomRightRadius = radius;
    d->changed = true;
}


// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasBoxShadowPrivate::createPaint(QCanvasPainter *painter) const
{
    Q_UNUSED(painter);
    auto *d = this;
    if (d->changed) {
        // TODO: Support non-antialiased shadows?
        const float aa = 1.0f;

        // Adjust blur to grow equally towards in & out.
        const float blurIn = QCPAINTER_BOX_SHADOW_MULTIPLIER * d->blur + 0.5 * aa;
        const float blurOut = 2 * QCPAINTER_BOX_SHADOW_MULTIPLIER * d->blur + aa;

        float extendX = d->spread - blurIn;
        float extendY = d->spread - blurIn;

        // Limit max extends when width & height are < 0.
        // This reduces rendering issues of thin rects with big blur
        // values being to sharp at center (smoothstep is clipped),
        // but causes some extra blurriness to those.
        if (extendX <  -0.5f * d->width) {
            float diff = extendX + 0.5f * d->width;
            extendX -= 0.25 * diff;
        }
        if (extendY <  -0.5f * d->height) {
            float diff = extendY + 0.5f * d->height;
            extendY -= 0.25 * diff;
        }

        float x = d->x - extendX;
        float y = d->y - extendY;
        float width = d->width + 2 * extendX;
        float height = d->height + 2 * extendY;

        QColor color = d->clampedColor();

        // Use adjusted width & height extended with blur & spread.
        float commonRadius = d->clampedRadius(d->radius, width, height);
        QVector4D radius { commonRadius, commonRadius, commonRadius, commonRadius };

        if (d->topLeftRadius >= 0)
            radius.setX(d->clampedRadius(d->topLeftRadius, width, height));
        if (d->topRightRadius >= 0)
            radius.setY(d->clampedRadius(d->topRightRadius, width, height));
        if (d->bottomLeftRadius >= 0)
            radius.setZ(d->clampedRadius(d->bottomLeftRadius, width, height));
        if (d->bottomRightRadius >= 0)
            radius.setW(d->clampedRadius(d->bottomRightRadius, width, height));

        createBoxShadow(x, y, width, height, radius, blurOut, color);

        const_cast<QCanvasBoxShadowPrivate *>(d)->changed = false;
    }

    return d->paint;
}

void QCanvasBoxShadowPrivate::createBoxShadow(float x, float y, float width, float height,
                                  const QVector4D &radius,
                                  float blur, const QColor &color) const
{
    QCPaint &p = const_cast<QCanvasBoxShadowPrivate *>(this)->paint;
    p.brushType = BrushBoxShadow;

    p.transform = QTransform::fromTranslate(x + (width * 0.5f), y + (height * 0.5f));

    p.extent[0] = width * 0.5f;
    p.extent[1] = height * 0.5f;

    // Unused, individual corner radius in outerColor
    //p.radius = radius;

    p.feather = blur;

    p.innerColor = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
    p.outerColor = { radius.x(), radius.y(), radius.z(), radius.w() };
    p.imageId = 0;
}

QCanvasBrushPrivate *QCanvasBoxShadowPrivate::clone()
{
    return new QCanvasBoxShadowPrivate(*this);
}

float QCanvasBoxShadowPrivate::clampedRadius(float rad, float width, float height) const
{
    float maxRadius = qMin(width, height) * 0.5f;
    float spreadRadius = rad + spread;
    if (radius < spread && !qFuzzyIsNull(spread)) {
        // CSS box-shadow has a specific math to calculate radius with spread
        // https://www.w3.org/TR/css-backgrounds-3/#shadow-shape
        // "the spread distance is first multiplied by the proportion 1 + (r-1)^3,
        // where r is the ratio of the border radius to the spread distance".
        float r = (rad / spread) - 1;
        spreadRadius = rad + spread * (1 + r * r * r);
    }
    spreadRadius = qMax(0.0f, spreadRadius);
    return qMin(spreadRadius, maxRadius);
}

// Clamp the color to fade when the blur amount is
// bigger than the rect size.
QColor QCanvasBoxShadowPrivate::clampedColor() const
{
    QColor c = color;
    const float minSize = qMin(width + 2 * spread, height + 2 * spread);
    const float blurTreshold = minSize * 0.5f * QCPAINTER_BOX_SHADOW_MULTIPLIER;
    if (blur > blurTreshold) {
        float fade = blurTreshold / blur;
        c.setAlphaF(color.alphaF() * fade);
    }
    return c;
}

QT_END_NAMESPACE
