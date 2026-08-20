// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvasgridpattern.h"
#include "qcanvasgridpattern_p.h"
#include "qcanvaspainter_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasGridPattern
    \since 6.11
    \brief QCanvasGridPattern is a brush for painting grid patterns.
    \inmodule QtCanvasPainter

    QCanvasGridPattern is a brush for painting grid and bar patterns.

    Here is a simple example:
    \code
    // Rotated grid into background
    QRectF rect(20, 20, 180, 180);
    QCanvasGridPattern gp1(rect, "#DBEB00", "#373F26");
    gp1.setCellSize(16, 16);
    gp1.setStartPosition(rect.topLeft());
    gp1.setLineWidth(2.0f);
    gp1.setRotation(M_PI / 4);
    gp1.setFeather(5.0f);
    painter.setFillStyle(gp1);
    painter.fillRect(rect);
    // Rounded rectangle, stroked with
    // grid pattern for dashes.
    float strokeW = 10;
    QRectF rect2(40, 40, 140, 140);
    QCanvasGridPattern gp2;
    gp2.setLineColor(Qt::transparent);
    gp2.setBackgroundColor(Qt::white);
    gp2.setStartPosition(rect2.x() - strokeW,
                         rect2.y() - strokeW);
    gp2.setCellSize(40, 40);
    gp2.setLineWidth(strokeW);
    painter.setLineWidth(8);
    painter.setStrokeStyle(gp2);
    painter.beginPath();
    painter.roundRect(rect2, 10);
    painter.stroke();
    \endcode

    \image gridpattern_example_1.png

*/

/*!
    Constructs a default grid pattern.
    Pattern start position position is (0, 0) and pattern size (100, 100).
    Pattern angle is 0.0, grid line color white and background color black.
*/

QCanvasGridPattern::QCanvasGridPattern()
    : QCanvasBrush(new QCanvasGridPatternPrivate)
{
}

#define G_D() auto *d = QCanvasGridPatternPrivate::get(this)

/*!
    Constructs an grid pattern.
    Pattern start position position and size is defined with \a rect.
    Grid line color is \a lineColor and background color is \a backgroundColor.
    Grid line width is \a lineWidth, feather (antialiasing) is \a feather and
    angle is \a angle.
*/

QCanvasGridPattern::QCanvasGridPattern(const QRectF &rect,
                             const QColor &lineColor,
                             const QColor &backgroundColor,
                             float lineWidth, float feather, float angle)
    : QCanvasBrush(new QCanvasGridPatternPrivate)
{
    G_D();
    d->x = float(rect.x());
    d->y = float(rect.y());
    d->width = float(rect.width());
    d->height = float(rect.height());
    d->lineColor = lineColor;
    d->backgroundColor = backgroundColor;
    d->lineWidth = lineWidth;
    d->feather = feather;
    d->angle = angle;
}

/*!
    Constructs an grid pattern.
    Pattern start position is ( \a x, \a y) and pattern size ( \a width, \a height).
    Grid line color is \a lineColor and background color is \a backgroundColor.
    Pattern angle is \a angle.
*/

QCanvasGridPattern::QCanvasGridPattern(float x, float y, float width, float height,
                             const QColor &lineColor,
                             const QColor &backgroundColor,
                             float lineWidth, float feather, float angle)
    : QCanvasBrush(new QCanvasGridPatternPrivate)
{
    G_D();
    d->x = x;
    d->y = y;
    d->width = width;
    d->height = height;
    d->lineColor = lineColor;
    d->backgroundColor = backgroundColor;
    d->lineWidth = lineWidth;
    d->feather = feather;
    d->angle = angle;
}

/*!
    Destroys the grid pattern.
*/

QCanvasGridPattern::~QCanvasGridPattern() = default;

/*!
   Returns the grid pattern as a QVariant.
*/

QCanvasGridPattern::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasGridPattern::operator!=(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs)

    \return \c true if the grid pattern \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasGridPattern::operator==(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs)

    \return \c true if the grid pattern \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs) noexcept
{
    auto *d = QCanvasGridPatternPrivate::get(&lhs);
    auto *pd = QCanvasGridPatternPrivate::get(&rhs);
    if (pd == d)
        return true;

    if (d->x != pd->x
        || d->y != pd->y
        || d->width != pd->width
        || d->height != pd->height
        || d->feather != pd->feather
        || d->angle != pd->angle
        || d->lineWidth != pd->lineWidth
        || d->lineColor != pd->lineColor
        || d->backgroundColor != pd->backgroundColor)
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasGridPattern &p)
{
    QDebugStateSaver saver(dbg);
    const auto &sp = p.startPosition();
    dbg.nospace() << "QCanvasGridPattern(" << sp << ')';
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

/*****************************************************************************
  QCanvasGridPattern stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QCanvasGridPattern &pattern)
    \relates QCanvasGridPattern

    Writes the given \a pattern to the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QCanvasGridPattern &p)
{
    const auto sp = p.startPosition();
    const auto size = p.cellSize();
    s << sp.x() << sp.y();
    s << size.width() << size.height();
    s << p.lineWidth() << p.feather() << p.rotation();
    s << p.lineColor() << p.backgroundColor();
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QCanvasGridPattern &pattern)
    \relates QCanvasGridPattern

    Reads the given \a pattern from the given \a stream and returns a
    reference to the \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QCanvasGridPattern &p)
{
    float x, y, width, height, lineWidth, feather, angle;
    QColor lineColor, backgroundColor;
    s >> x >> y >> width >> height;
    s >> lineWidth >> feather >> angle;
    s >> lineColor >> backgroundColor;
    p.setStartPosition(x, y);
    p.setCellSize(width, height);
    p.setLineWidth(lineWidth);
    p.setFeather(feather);
    p.setRotation(angle);
    p.setLineColor(lineColor);
    p.setBackgroundColor(backgroundColor);
    return s;
}

#endif // QT_NO_DATASTREAM

/*!
    Returns the start point of grid pattern.
    \sa setStartPosition()
*/

QPointF QCanvasGridPattern::startPosition() const
{
    G_D();
    return QPointF(d->x, d->y);
}

/*!
    Sets the start point of grid pattern to (\a x, \a y).
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/

void QCanvasGridPattern::setStartPosition(float x, float y)
{
    G_D();
    detach();
    d->x = x;
    d->y = y;
    d->changed = true;
}

/*!
    \fn void QCanvasGridPattern::setStartPosition(QPointF point)
    \overload

    Sets the start point of grid pattern to \a point.
    Start position means top-left corner of the grid in pattern.
    Pattern will then be extended to all positions from here.
    The default value is \c{(0.0, 0.0)}.
*/


/*!
    Returns the size of a single cell in grid pattern.
    \sa setCellSize()
*/

QSizeF QCanvasGridPattern::cellSize() const
{
    G_D();
    return QSizeF(d->width, d->height);
}

/*!
    Sets the size of a single cell in grid pattern to \a width, \a height.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

void QCanvasGridPattern::setCellSize(float width, float height)
{
    G_D();
    detach();
    d->width = width;
    d->height = height;
    d->changed = true;
}

/*!
    \fn void QCanvasGridPattern::setCellSize(QSizeF size)
    \overload

    Sets the size of a single cell in grid pattern to \a size.
    When width is \c 0, the horizontal bars are not painted.
    When height is \c 0, the vertical bars are not painted.
    The default value is \c{(10, 10)}.
*/

/*!
    Returns the width of a stroke line in grid pattern.
    \sa setLineWidth()
*/

float QCanvasGridPattern::lineWidth() const
{
    G_D();
    return d->lineWidth;
}

/*!
    Sets the width of a stroke line in grid pattern to \a width.
    The default value is \c 1.0.
*/

void QCanvasGridPattern::setLineWidth(float width)
{
    G_D();
    detach();
    d->lineWidth = width;
    d->changed = true;
}

/*!
    Returns the pattern feather in pixels.
*/

float QCanvasGridPattern::feather() const
{
    G_D();
    return d->feather;
}

/*!
    Sets the pattern feather to \a feather in pixels.
    The default value is \c 1.0, meaning a single pixel antialiasing.
*/

void QCanvasGridPattern::setFeather(float feather)
{
    G_D();
    detach();
    d->feather = feather;
    d->changed = true;
}

/*!
    Returns the pattern rotation in radians.
*/

float QCanvasGridPattern::rotation() const
{
    G_D();
    return d->angle;
}

/*!
    Sets the pattern rotation to \a rotation in radians.
    Rotation is done around the grid startPosition().
    The default value is \c 0.0, meaning the grid is not rotated.
*/

void QCanvasGridPattern::setRotation(float rotation)
{
    G_D();
    detach();
    d->angle = rotation;
    d->changed = true;
}

/*!
    Returns the pattern grid line color.
*/

QColor QCanvasGridPattern::lineColor() const
{
    G_D();
    return d->lineColor;
}

/*!
    Sets the pattern grid line color to \a color.
    The default value is white.
*/

void QCanvasGridPattern::setLineColor(const QColor &color)
{
    G_D();
    detach();
    d->lineColor = color;
    d->changed = true;
}

/*!
    Returns the pattern grid background color.
*/

QColor QCanvasGridPattern::backgroundColor() const
{
    G_D();
    return d->backgroundColor;
}

/*!
    Sets the pattern grid background color to \a color.
    The default value is black.
*/

void QCanvasGridPattern::setBackgroundColor(const QColor &color)
{
    G_D();
    detach();
    d->backgroundColor = color;
    d->changed = true;
}

// ***** Private *****

/*!
   \internal
*/
#define DECONST(d) const_cast<QCanvasGridPatternPrivate *>(d)

QCPaint QCanvasGridPatternPrivate::createPaint(QCanvasPainter *painter) const
{
    auto *d = this;
    Q_UNUSED(painter);
    if (d->changed) {
        createGridPattern();
        DECONST(d)->changed = false;
    }
    return d->paint;
}

void QCanvasGridPatternPrivate::createGridPattern() const
{
    auto *d = this;
    QCPaint &p = DECONST(d)->paint;
    p.brushType = BrushGrid;
    p.transform = QTransform::fromTranslate(d->x, d->y);
    if (!qFuzzyIsNull(d->angle))
        p.transform = p.transform.rotateRadians(d->angle);

    p.extent[0] = d->width;
    p.extent[1] = d->height;

    p.feather = d->feather;
    p.radius = d->lineWidth;

    p.innerColor = { d->lineColor.redF(),
                     d->lineColor.greenF(),
                     d->lineColor.blueF(),
                     d->lineColor.alphaF() };
    p.outerColor = { d->backgroundColor.redF(),
                     d->backgroundColor.greenF(),
                     d->backgroundColor.blueF(),
                     d->backgroundColor.alphaF() };
}

QCanvasBrushPrivate *QCanvasGridPatternPrivate::clone()
{
    return new QCanvasGridPatternPrivate(*this);
}

#undef G_D
#undef DECONST

QT_END_NAMESPACE
