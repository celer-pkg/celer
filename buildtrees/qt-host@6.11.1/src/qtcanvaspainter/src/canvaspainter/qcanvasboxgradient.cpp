// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasboxgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasBoxGradient
    \since 6.11
    \brief QCanvasBoxGradient is a brush for box gradient painting.
    \inmodule QtCanvasPainter

    Box gradient interpolate colors between start and end points, creating
    a rounded rectangle shaped gradient. With the \l setRadius() and
    \l setFeather() the roundness and softness of the gradient can be
    controlled.

    \image qcboxgradient-gallery.webp

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcboxgradient-example.webp
    \li
    \code
    QRectF rect(20, 20, 160, 160);
    QCanvasBoxGradient bg(rect, 20, 50);
    bg.setColorAt(0.0, "#1a2a6c");
    bg.setColorAt(0.2, "#fdbb2d");
    bg.setColorAt(1.0, QColorConstants::Transparent);
    p->setFillStyle(bg);
    p->fillRect(rect);
    \endcode
    \endtable

    \note When QCanvasBoxGradient is used to paint round rectangle, it is usually
    better to use \l QCanvasPainter::rect() than \l QCanvasPainter::roundRect() as the
    rounding comes from the brush rather than the path. This avoids creating
    extra vertices and thus performs better.
*/

#define G_D() auto *d = QCanvasGradientPrivate::get(this)
#define DECONST(d) const_cast<QCanvasBoxGradientPrivate *>(d)

class QCanvasBoxGradientPrivate : public QCanvasGradientPrivate
{
public:
    QCanvasBoxGradientPrivate() : QCanvasGradientPrivate(QCanvasBrush::BrushType::BoxGradient) {}
    QCanvasBoxGradientPrivate(const QCanvasBoxGradientPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createBoxGradient(const QColor &iColor, const QColor &oColor,
                           int imageId) const;
    QCanvasBrushPrivate *clone() override
    {
        return new QCanvasBoxGradientPrivate(*this);
    }
};


/*!
    Constructs a default box gradient.
    Position of gradient is (0, 0) and size (100, 100)
    Gradient feather is 10.0.
    Gradient radius is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasBoxGradient::QCanvasBoxGradient()
    : QCanvasGradient(new QCanvasBoxGradientPrivate)
{
    G_D();
    d->data.box.x = 0.0f;
    d->data.box.y = 0.0f;
    d->data.box.width = 100.0f;
    d->data.box.height = 100.0f;
    d->data.box.feather = 10.0f;
    d->data.box.radius = 0.0f;
}

/*!
    Constructs a box gradient.
    Position of gradient is ( \a x, \a y) and size ( \a width, \a height)
    Gradient feather is \a feather.
    Gradient radius is \a radius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasBoxGradient::QCanvasBoxGradient(float x, float y, float width, float height, float feather, float radius)
    : QCanvasGradient(new QCanvasBoxGradientPrivate)
{
    G_D();
    d->data.box.x = x;
    d->data.box.y = y;
    d->data.box.width = width;
    d->data.box.height = height;
    d->data.box.feather = feather;
    d->data.box.radius = radius;
}

/*!
    Constructs a box gradient.
    Position and size of gradient is \a rect.
    Gradient feather is \a feather.
    Gradient radius is \a radius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasBoxGradient::QCanvasBoxGradient(const QRectF &rect, float feather, float radius)
    : QCanvasGradient(new QCanvasBoxGradientPrivate)
{
    G_D();
    d->data.box.x = float(rect.x());
    d->data.box.y = float(rect.y());
    d->data.box.width = float(rect.width());
    d->data.box.height = float(rect.height());
    d->data.box.feather = feather;
    d->data.box.radius = radius;
}

QCanvasBoxGradient::~QCanvasBoxGradient()
{
}

/*!
    Returns the rectangle area of the box gradient.
    \sa setRect()
*/

QRectF QCanvasBoxGradient::rect() const
{
    G_D();
    return QRectF(d->data.box.x,
                  d->data.box.y,
                  d->data.box.width,
                  d->data.box.height);
}
/*!
    Sets the rectangle of box gradient to position ( \a x, \a y) and size ( \a width, \a height).
*/

void QCanvasBoxGradient::setRect(float x, float y, float width, float height)
{
    G_D();
    detach();
    d->data.box.x = x;
    d->data.box.y = y;
    d->data.box.width = width;
    d->data.box.height = height;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasBoxGradient::setRect(const QRectF &rect)
    \overload

    Sets the rectangle of box gradient to \a rect.
*/

/*!
    Returns the feather of the box gradient.
    \sa setFeather()
*/

float QCanvasBoxGradient::feather() const
{
    G_D();
    return d->data.box.feather;
}

/*!
    Sets the feather of box gradient to \a feather.
*/

void QCanvasBoxGradient::setFeather(float feather)
{
    G_D();
    detach();
    d->data.box.feather = feather;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    Returns the radius of the box gradient.
    \sa setRadius()
*/

float QCanvasBoxGradient::radius() const
{
    G_D();
    return d->data.box.radius;
}

/*!
    Sets the radius of box gradient to \a radius.
    The maximum radius is half of width or height of rect(),
    depending on which one is smaller.
*/

void QCanvasBoxGradient::setRadius(float radius)
{
    G_D();
    detach();
    d->data.box.radius = radius;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasBoxGradientPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createBoxGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createBoxGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createBoxGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createBoxGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasBoxGradientPrivate::createBoxGradient(const QColor &iColor, const QColor &oColor,
                                      int imageId) const
{
    auto *d = this;
    const auto dd = d->data.box;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushBoxGradient;
    p.transform = QTransform::fromTranslate(dd.x + (dd.width * 0.5f), dd.y + (dd.height * 0.5f));

    p.extent[0] = dd.width * 0.5f;
    p.extent[1] = dd.height * 0.5f;

    p.radius = qMin(dd.radius, qMin(dd.width, dd.height) * 0.5f);
    constexpr float small = 0.0001f;
    p.feather = qMax(small, dd.feather);

    if (imageId != 0) {
        // Multistop gradient
        p.imageId = imageId;
    } else {
        // 2 stops gradient
        p.innerColor = { iColor.redF(), iColor.greenF(), iColor.blueF(), iColor.alphaF() };
        p.outerColor = { oColor.redF(), oColor.greenF(), oColor.blueF(), oColor.alphaF() };
        p.imageId = 0;
    }
}

#undef G_D
#undef DECONST

QT_END_NAMESPACE
