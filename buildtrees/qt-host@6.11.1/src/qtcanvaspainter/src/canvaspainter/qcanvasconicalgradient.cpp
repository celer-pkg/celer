// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasconicalgradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasConicalGradient
    \since 6.11
    \brief QCanvasConicalGradient is a brush for conical gradient painting.
    \inmodule QtCanvasPainter

    Conical gradient interpolate colors between start and end points
    around a point with given coordinates. The angle starts from a line
    going horizontally right from the center, and proceeds clockwise.

    \image qcconicalgradient-gallery.webp

    Here is a simple example:
    \table
    \row
    \li \inlineimage qcconicalgradient-example.webp
    \li
    \code
    QRectF rect(20, 20, 160, 160);
    QCanvasConicalGradient cg(rect.center(), 1.75 * M_PI);
    cg.setColorAt(0.0, "#fdbb2d");
    cg.setColorAt(0.5, "#1a2a6c");
    cg.setColorAt(1.0, "#fdbb2d");
    p->setFillStyle(cg);
    p->beginPath();
    p->ellipse(rect);
    p->fill();
    \endcode
    \endtable
*/

#define G_D() auto *d = QCanvasGradientPrivate::get(this)
#define DECONST(d) const_cast<QCanvasConicalGradientPrivate *>(d)

class QCanvasConicalGradientPrivate : public QCanvasGradientPrivate
{
public:
    QCanvasConicalGradientPrivate() : QCanvasGradientPrivate(QCanvasBrush::BrushType::ConicalGradient) {}
    QCanvasConicalGradientPrivate(const QCanvasConicalGradientPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createConicalGradient(const QColor &iColor, const QColor &oColor,
                               int imageId) const;
    QCanvasBrushPrivate *clone() override
    {
        return new QCanvasConicalGradientPrivate(*this);
    }
};


/*!
    Constructs a default conical gradient.
    Gradient center position is (0, 0).
    Gradient angle is 0.0.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasConicalGradient::QCanvasConicalGradient()
    : QCanvasGradient(new QCanvasConicalGradientPrivate)
{
    G_D();
    d->data.conical.cx = 0.0f;
    d->data.conical.cy = 0.0f;
    d->data.conical.angle = 0.0f;
}

/*!
    Constructs a conical gradient.
    Gradient center position is ( \a centerX, \a centerY).
    Gradient start angle is \a startAngle.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasConicalGradient::QCanvasConicalGradient(float centerX, float centerY, float startAngle)
    : QCanvasGradient(new QCanvasConicalGradientPrivate)
{
    G_D();
    d->data.conical.cx = centerX;
    d->data.conical.cy = centerY;
    d->data.conical.angle = startAngle;
}

/*!
    Constructs a conical gradient.
    Gradient center position is \a center.
    Gradient start angle is \a startAngle.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasConicalGradient::QCanvasConicalGradient(QPointF center, float startAngle)
    : QCanvasGradient(new QCanvasConicalGradientPrivate)
{
    G_D();
    d->data.conical.cx = float(center.x());
    d->data.conical.cy = float(center.y());
    d->data.conical.angle = startAngle;
}

QCanvasConicalGradient::~QCanvasConicalGradient()
{
}

/*!
    Returns the center point of conical gradient.
    \sa setCenterPosition()
*/

QPointF QCanvasConicalGradient::centerPosition() const
{
    G_D();
    return QPointF(d->data.conical.cx,
                   d->data.conical.cy);
}

/*!
    Sets the center point of conical gradient to ( \a x, \a y).
*/
void QCanvasConicalGradient::setCenterPosition(float x, float y)
{
    G_D();
    detach();
    d->data.conical.cx = x;
    d->data.conical.cy = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasConicalGradient::setCenterPosition(QPointF center)
    \overload

    Sets the center point of conical gradient to \a center.
*/

/*!
    Returns the start angle of conical gradient in radians.
*/

float QCanvasConicalGradient::angle() const
{
    G_D();
    return d->data.conical.angle;
}

/*!
    Sets the start angle of conical gradient to \a angle in radians.
    The angle starts from a line going horizontally right from the
    center, and proceeds clockwise.
*/

void QCanvasConicalGradient::setAngle(float angle)
{
    G_D();
    detach();
    d->data.conical.angle = angle;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

// ***** Private *****

/*!
   \internal
*/

QCPaint QCanvasConicalGradientPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createConicalGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createConicalGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createConicalGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createConicalGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasConicalGradientPrivate::createConicalGradient(const QColor &iColor, const QColor &oColor,
                                              int imageId) const
{
    auto *d = this;
    const auto dd = d->data.conical;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushConicalGradient;
    p.transform = QTransform::fromTranslate(dd.cx, dd.cy);

    // Note: p.extent and p.radius not used

    // Rotating clockwise, starting from east
    p.transform.rotateRadians(dd.angle + M_PI_2);

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
