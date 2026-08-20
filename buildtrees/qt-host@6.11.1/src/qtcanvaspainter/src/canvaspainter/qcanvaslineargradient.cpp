// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvaslineargradient.h"
#include "qcanvasgradient_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasLinearGradient
    \since 6.11
    \brief QCanvasLinearGradient is a brush for linear gradient painting.
    \inmodule QtCanvasPainter

    Liner gradient interpolate colors between start and end points
    along a linear pattern, so horizontally, vertically or diagonally.
    The gradient starts from \a startPosition() and ends to \a endPosition().
    Outside this area, the colors are the starting and the ending color.

    \image qclineargradient-gallery.webp

    Here is a simple example:
    \table
    \row
    \li \inlineimage qclineargradient-example.webp
    \li
    \code
    QRectF rect1(0, 0, 200, 200);
    QCanvasLinearGradient lg(rect1.topLeft(),
                        rect1.bottomRight());
    lg.setColorAt(0.0, "#1a2a6c");
    lg.setColorAt(0.5, "#b21f1f");
    lg.setColorAt(1.0, "#fdbb2d");
    p->setFillStyle(lg);
    p->fillRect(rect1);
    \endcode
    \endtable
*/

#define G_D() auto *d = QCanvasGradientPrivate::get(this)
#define DECONST(d) const_cast<QCanvasLinearGradientPrivate *>(d)

class QCanvasLinearGradientPrivate : public QCanvasGradientPrivate
{
public:
    QCanvasLinearGradientPrivate() : QCanvasGradientPrivate(QCanvasBrush::BrushType::LinearGradient) {}
    QCanvasLinearGradientPrivate(const QCanvasLinearGradientPrivate &) = default;
    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createLinearGradient(const QColor &iColor, const QColor &oColor,
                              int imageId) const;

    QCanvasBrushPrivate *clone() override
    {
        return new QCanvasLinearGradientPrivate(*this);
    }
};


/*!
    Constructs a default linear gradient.
    Gradient start color position is (0, 0) and end color position (0, 100).
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasLinearGradient::QCanvasLinearGradient()
    : QCanvasGradient(new QCanvasLinearGradientPrivate)
{
    G_D();
    d->data.linear.sx = 0.0f;
    d->data.linear.sy = 0.0f;
    d->data.linear.ex = 0.0f;
    d->data.linear.ey = 100.0f;
}

/*!
    Constructs a linear gradient.
    Gradient start color position is (\a startX, \a startY) and
    end color position (\a endX, \a endY).
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasLinearGradient::QCanvasLinearGradient(float startX, float startY, float endX, float endY)
    : QCanvasGradient(new QCanvasLinearGradientPrivate)
{
    G_D();
    d->data.linear.sx = startX;
    d->data.linear.sy = startY;
    d->data.linear.ex = endX;
    d->data.linear.ey = endY;
}

/*!
    Constructs a linear gradient.
    Gradient start color position is \a start and end color position \a end.
    Gradient start color is white (255, 255, 255) and end color
    transparent black (0, 0, 0, 0).
*/

QCanvasLinearGradient::QCanvasLinearGradient(QPointF start, QPointF end)
    : QCanvasGradient(new QCanvasLinearGradientPrivate)
{
    G_D();
    d->data.linear.sx = float(start.x());
    d->data.linear.sy = float(start.y());
    d->data.linear.ex = float(end.x());
    d->data.linear.ey = float(end.y());
}

QCanvasLinearGradient::~QCanvasLinearGradient()
{
}

/*!
    Returns the start point of linear gradient.
*/

QPointF QCanvasLinearGradient::startPosition() const
{
    G_D();
    return QPointF(d->data.linear.sx,
                   d->data.linear.sy);
}

/*!
    Sets the start point of linear gradient to ( \a x, \a y).
*/

void QCanvasLinearGradient::setStartPosition(float x, float y)
{
    G_D();
    detach();
    d->data.linear.sx = x;
    d->data.linear.sy = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasLinearGradient::setStartPosition(QPointF start)
    \overload

    Sets the start point of linear gradient to \a start.
*/

/*!
    Returns the end point of linear gradient.
*/

QPointF QCanvasLinearGradient::endPosition() const
{
    G_D();
    return QPointF(d->data.linear.ex,
                   d->data.linear.ey);
}

/*!
    Sets the end point of linear gradient to ( \a x, \a y).
*/

void QCanvasLinearGradient::setEndPosition(float x, float y)
{
    G_D();
    detach();
    d->data.linear.ex = x;
    d->data.linear.ey = y;
    d->dirty |= QCanvasGradientPrivate::DirtyFlag::Values;
}

/*!
    \fn void QCanvasLinearGradient::setEndPosition(QPointF end)
    \overload

    Sets the end point of linear gradient to \a end.
*/

// ***** Private *****

/*!
   \internal
*/


QCPaint QCanvasLinearGradientPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    if (d->dirty) {
        if (d->gradientStops.size() == 0) {
            QColor icol = { 255, 255, 255, 255 };
            QColor ocol = { 0, 0, 0, 0 };
            createLinearGradient(icol, ocol, 0);
        } else if (d->gradientStops.size() == 1) {
            QColor c = d->gradientStops.first().color;
            createLinearGradient(c, c, 0);
        } else if (d->gradientStops.size() == 2) {
            QColor ic = d->gradientStops.first().color;
            QColor oc = d->gradientStops.last().color;
            createLinearGradient(ic, oc, 0);
        } else {
            DECONST(d)->updateGradientTexture(painter);
            QColor col = { 255, 255, 255, 255 };
            createLinearGradient(col, col, d->imageId);
        }
        DECONST(d)->dirty = {};
    }
    if (d->gradientStops.size() > 2) {
        auto *painterPriv = QCanvasPainterPrivate::get(painter);
        painterPriv->markTextureIdUsed(d->imageId);
    }
    return d->paint;
}

void QCanvasLinearGradientPrivate::createLinearGradient(const QColor &iColor, const QColor &oColor,
                                            int imageId) const
{
    auto *d = this;

    const auto dd = d->data.linear;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushLinearGradient;

    float dx = dd.ex - dd.sx;
    float dy = dd.ey - dd.sy;
    float dist = std::sqrt(dx*dx + dy*dy);
    constexpr float small = 0.0001f;
    if (dist > small) {
        dx /= dist;
        dy /= dist;
    } else {
        dx = 0;
        dy = 1;
    }
    p.transform.setMatrix(dy, -dx, 0,
                          dx, dy, 0,
                          dd.sx, dd.sy, 1);
    p.feather = qMax(small, dist);

    // Note: extent and radius not used.

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
