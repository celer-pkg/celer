// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasradialgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasRadialGradient
    \since 6.11
    \brief QCanvasRadialGradient is a brush for radial gradient painting.
    \inmodule QtCanvasPainter

    Radial gradient interpolate colors between start and end points, creating
    a circle shaped gradient. The gradient is controlled with \l centerPosition(),
    \l outerRadius() and \l innerRadius().

    \image qcradialgradient-gallery.webp

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcradialgradient-example.webp
    \li
    \code
    QRectF rect1(0, 0, 200, 200);
    QCanvasRadialGradient rg(rect1.center(), 100, 0);
    rg.setColorAt(0.0, "#fdbb2d");
    rg.setColorAt(0.6, "#b21f1f");
    rg.setColorAt(1.0, "#1a2a6c");
    p->setFillStyle(rg);
    p->fillRect(rect1);
    \endcode
    \endtable
*/

#define G_D() auto *d = QCanvasGradientPrivate::get(this)
#define DECONST(d) const_cast<QCanvasRadialGradientPrivate *>(d)

class QCanvasRadialGradientPrivate : public QCanvasGradientPrivate
{
public:
    QCanvasRadialGradientPrivate() : QCanvasGradientPrivate(QCanvasBrush::BrushType::RadialGradient) {}
    QCanvasRadialGradientPrivate(const QCanvasRadialGradientPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createRadialGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;
    QCanvasBrushPrivate *clone() override
    {
        return new QCanvasRadialGradientPrivate(*this);
    }
};

/*!
    Constructs a default radial gradient.
    Gradient center position is (0, 0).
    Gradient outer radius is 100.0 and inner radius is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient()
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.cx = 0.0f;
    d->data.radial.cy = 0.0f;
    d->data.radial.oRadius = 100.0f;
    d->data.radial.iRadius = 0.0f;
}

/*!
    Constructs a radial gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient(float centerX, float centerY, float outerRadius, float innerRadius)
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.cx = centerX;
    d->data.radial.cy = centerY;
    d->data.radial.oRadius = outerRadius;
    d->data.radial.iRadius = innerRadius;
}

/*!
    Constructs a radial gradient.
    Gradient center position is \a center.
    Gradient outer radius is \a outerRadius and inner radius is \a innerRadius.
    Gradient start color is white (255,255,255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasRadialGradient::QCanvasRadialGradient(QPointF center, float outerRadius, float innerRadius)
    : QCanvasGradient(new QCanvasRadialGradientPrivate)
{
    G_D();
    d->data.radial.cx = float(center.x());
    d->data.radial.cy = float(center.y());
    d->data.radial.oRadius = outerRadius;
    d->data.radial.iRadius = innerRadius;
}

QCanvasRadialGradient::~QCanvasRadialGradient()
{
}

/*!
    Returns the center point of radial gradient.
*/

QPointF QCanvasRadialGradient::centerPosition() const
{
    G_D();
    return QPointF(d->data.radial.cx,
                   d->data.radial.cy);
}

/*!
    Sets the center point of radial gradient to ( \a x, \a y).
*/

void QCanvasRadialGradient::setCenterPosition(float x, float y)
{
    G_D();
    detach();
    d->data.radial.cx = x;
    d->data.radial.cy = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasRadialGradient::setCenterPosition(QPointF center)
    \overload

    Sets the center point of radial gradient to \a center.
*/

/*!
    Returns the outer radius of radial gradient.
    \sa setOuterRadius()
*/

float QCanvasRadialGradient::outerRadius() const
{
    G_D();
    return d->data.radial.oRadius;
}

/*!
    Sets the outer radius of radial gradient to \a radius.
    End color will be drawn at this radius from center position.
*/

void QCanvasRadialGradient::setOuterRadius(float radius)
{
    G_D();
    detach();
    d->data.radial.oRadius = radius;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    Returns the inner radius of radial gradient.
    \sa setInnerRadius()
*/

float QCanvasRadialGradient::innerRadius() const
{
    G_D();
    return d->data.radial.iRadius;
}

/*!
    Sets the inner radius of radial gradient to \a radius.
    Start color will be drawn at this radius from center position.
    The default inner radius is \c 0.0 meaning that gradient starts
    directly from center position.
*/

void QCanvasRadialGradient::setInnerRadius(float radius)
{
    G_D();
    detach();
    d->data.radial.iRadius = radius;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasRadialGradientPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createRadialGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createRadialGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createRadialGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createRadialGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasRadialGradientPrivate::createRadialGradient(const QColor &iColor, const QColor &oColor,
                                               int imageId) const
{
    auto *d = this;
    const auto dd = d->data.radial;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushRadialGradient;
    const float r = (dd.iRadius + dd.oRadius) * 0.5f;
    const float f = (dd.oRadius - dd.iRadius);
    p.transform = QTransform::fromTranslate(dd.cx, dd.cy);

    // Note: extent not used.

    constexpr float small = 0.0001f;
    p.radius = r;
    p.feather = qMax(small, f);

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
