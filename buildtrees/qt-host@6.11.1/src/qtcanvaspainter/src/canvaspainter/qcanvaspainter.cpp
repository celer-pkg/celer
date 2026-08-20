// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvaspainter.h"
#include "qcanvaspainter_p.h"

#include "engine/qcpainterengine_p.h"
#include "engine/qcpainterrhirenderer_p.h"
#include <rhi/qrhi.h>
#include "qcanvasbrush.h"
#include "qcanvascustombrush.h"

#include "qcanvasimage_p.h"
#include "qcanvasboxshadow.h"

#include <QtGui/QScreen>
#include <QtGui/qguiapplication.h> // Used by mmToPx & ptToPx
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QC_INFO, "qt.qcpainter.general")

/*!
    \class QCanvasPainter
    \since 6.11
    \brief The QCanvasPainter class performs hardware-accelerated painting on QRhi.
    \inmodule QtCanvasPainter

    Qt Canvas Painter (QCanvasPainter) provides painting API optimized for
    harware-accelerated (GPU) painting. The API follows closely HTML Canvas 2D
    Context specification, ported to Qt C++. It is also influenced by QPainter,
    but with a more compact API.

    Here is a simple example of using QCanvasPainter to create a round button.
    \table
    \row
    \li \inlineimage qcpainter-buttonexample.webp
    \li
    \code
    QRectF rect(40, 70, 120, 60);
    QRectF shadowRect = rect.translated(2, 4);
    // Paint shadow
    QCanvasBoxShadow shadow(shadowRect, 30, 15, "#60373F26");
    p->drawBoxShadow(shadow);
    // Paint rounded rect
    p->beginPath();
    p->roundRect(rect, 30);
    p->setFillStyle("#DBEB00");
    p->fill();
    // Paint text
    p->setTextAlign(QCanvasPainter::TextAlign::Center);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    QFont font("Titillium Web", 18);
    p->setFont(font);
    p->setFillStyle("#373F26");
    p->fillText("CLICK!", rect);
    \endcode
    \endtable

    Here is another example of painting a simple graph.
    \table
    \row
    \li \inlineimage qcpainter-graphexample.webp
    \li
    \code
    // Paint grid
    QCanvasGridPattern grid(0, 0, 10, 10, "#404040", "#202020");
    p->setFillStyle(grid);
    p->fillRect(0, 0, width(), height());
    // Paint axis
    p->setFillStyle(QColorConstants::White);
    p->fillRect(0, 0.5 * height() - 1, width(), 2);
    p->fillRect(0.5 * width() - 1, 0, 2, height());
    // Paint shadowed graph
    p->beginPath();
    p->moveTo(20, height() * 0.8);
    p->bezierCurveTo(width() * 0.2, height() * 0.4,
                     width() * 0.5, height() * 0.8,
                     width() - 20, height() * 0.2);
    p->setAntialias(10);
    p->setLineWidth(12);
    p->setStrokeStyle("#D0000000");
    p->stroke();
    p->setAntialias(1);
    p->setLineWidth(6);
    QCanvasLinearGradient lg(0, 0, 0, height());
    lg.setStartColor(QColorConstants::Red);
    lg.setEndColor(QColorConstants::Green);
    p->setStrokeStyle(lg);
    p->stroke();
    \endcode
    \endtable

    \section1 Features

    For the most parts and from the naming perspective, QCanvasPainter follow closely the
    HTML Canvas 2D Context (https://html.spec.whatwg.org/multipage/canvas.html#2dcontext).
    This makes the API familiar to use for many developers, and with the ability
    to easily reuse existing canvas code. But the aim is NOT to be 100% compatible with
    the HTML canvas. QCanvasPainter misses some of the features to make it simpler,
    more performant on QRhi hardware accelerated graphics API, and to better target
    modern UI needs. Due to these reasons, QCanvasPainter also has additional features
    compared to HTML Canvas 2D Context.

    These are some of the functionality we are at least currently missing compared
    to HTML canvas:
    \list
    \li Clipping: All clipping is (transformed) rectangle and clipping to path shapes
    are not supported.
    \li Fill mode: Only the default Non-zero fillrule is supported, no support for
    Even-odd fillrule.
    \li Dashes: Strokes are always solid lines, dashed/dotted stroke patterns are
    not supported.
    \li Path testing: There are no isPointInPath() or isPointInStroke() methods.
    \li Text stroking: No support for outline stroking of text.
    \li Filter: Canvas SVG filter effects are not supported.
    \li CompositeModes: The amount of composite modes is limited to 3, which can
    be supported without rendering into extra buffers.
    \li Shadows: Built-in shadow methods are not supported.
    \endlist

    On the other hand, some of the additional features QCanvasPainter offers compared
    to HTML canvas include:
    \list
    \li Path groups: QCanvasPainter allows painting to static paths and caching these
    paths as groups for optimal GPU usage.
    \li Adjustable antialiasing: Due to path vertex antialiasing and SDF text
    rendering, the pixel amount of antialiasing can be freely adjusted for
    smoother painting.
    \li Box Gradient: In addition to linear, radial and conical gradients,
    QCanvasPainter supports also rounded rectangle box gradient.
    \li Box Shadow: QCanvasPainter supports also CSS box-shadow type of brush. The
    rendering uses SDF approach similar to Qt Quick RectangularShadow, making
    it very performant.
    \li Grid patterns: QCanvasPainter supports QCanvasGridPattern for dynamic grid and bar
    pattern styles.
    \li Custom brushes: QCanvasPainter also allows filling & stroking with custom
    vertex and fragment shaders (QCanvasCustomBrush). These custom brushes can also
    be used for text.
    \li Text wrapping: QCanvasPainter supports automatic wrapping of text into
    multiple lines, with different wrapping modes.
    \li Color effects: With addition to globalAlpha, QCanvasPainter supports also
    global brightness, contrast and saturation.
    \li Tinted images: QCanvasPainter adds tint color support for painted images
    and image patterns.
    \endlist

    QCanvasPainter is architecture agnostic, and usable for both Qt Quick and
    Qt Widgets applications. Actually, it is usable even without either of
    those, with just QWindow and QRhi. To utilize QCanvasPainter, use it from
    one of these classes, dending on the architecture of your application:
    \list
    \li Qt Quick: Use \l QCanvasPainterItem and \l QCanvasPainterItemRenderer.
    \li Qt Widgets: Use \l QCanvasPainterWidget.
    \li QRhi-based QWindow, or offscreen QRhi buffers: Use \l QCanvasPainterFactory and \l QCanvasRhiPaintDriver.
    \endlist

    \section1 Winding rules
    QCanvasPainter uses \c nonzero (\c{WindingFill}) fillrule. To select the filling
    based on the path points direction, disable the winding forcing by setting
    \c DisableWindingEnforce rendering hint with \l setRenderHint().

    \table
    \row
    \li \inlineimage qcpainter-pathwinding2.webp
    \li
    \code
    p->setRenderHint(QCanvasPainter::RenderHint::DisableWindingEnforce);
    p->beginPath();
    // Outer shape, counterclockwise
    p->moveTo(20, 20);
    p->lineTo(100, 180);
    p->lineTo(180, 20);
    p->closePath();
    // Inner shape, clockwise
    p->moveTo(100, 40);
    p->lineTo(125, 90);
    p->lineTo(75, 90);
    p->closePath();
    p->fill();
    p->stroke();
    \endcode
    \endtable

    However, a more common case is relying on winding enforcing and setting
    the preferred winding using \l setPathWinding() or \l beginHoleSubPath()
    and \l beginSolidSubPath() helpers.

    \table
    \row
    \li \inlineimage qcpainter-pathwinding3.webp
    \li
    \code
    p->beginPath();
    p->roundRect(20, 20, 160, 160, 30);
    // Start painting holes
    p->beginHoleSubPath();
    p->roundRect(40, 40, 120, 120, 10);
    // Start painting solid
    p->beginSolidSubPath();
    p->rect(60, 60, 80, 20);
    p->circle(100, 120, 20);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

/*!
    \enum QCanvasPainter::PathWinding

    PathWinding is used to specify the direction of path drawing.
    This direction is used to determine if a subpath is solid or hole in the path.

    \value CounterClockWise (default) Counter-clockwise winding for solid shapes.

    \value ClockWise Clockwise winding for holes.

    \sa setPathWinding()
*/

/*!
    \enum QCanvasPainter::PathConnection

    With some drawing methods PathConnection is used to specify if the new
    path should be connected to the last point of the previous path.

    \value NotConnected There is no line drawn from the last point of
    the previous path to the first point of the current path.

    \value Connected The last point of the previous path will be connected to
    the first point of the current path.

    \sa arc()
*/

/*!
    \enum QCanvasPainter::LineCap

    LineCap is used to define how the end of the line (cap) is drawn.

    \value Butt (default) Square line ending that does not cover the end point of the line.

    \value Round Round line ending.

    \value Square Square line ending that covers the end point and extends beyond it by half the line width.

    \sa setLineCap()
*/

/*!
    \enum QCanvasPainter::LineJoin

    LineJoin is used to define how the joins between two connected lines are drawn.

    \value Round Circular arc between the two lines is filled.

    \value Bevel The triangular notch between the two lines is filled.

    \value Miter (default) The outer edges of the lines are extended to meet at an angle, and this area is filled.

    \sa setLineJoin(), setMiterLimit()
*/

/*!
    \enum QCanvasPainter::TextAlign

    TextAlign is used to define how the text is aligned horizontally.

    \value Left Align the left side of the text horizontally to the specified position.

    \value Right Align the right side of the text horizontally to the specified position.

    \value Center Align the center of the text horizontally to the specified position.

    \value Start (default) The text is aligned at the normal start of the line (left-aligned for left-to-right locales, right-aligned for right-to-left locales).

    \value End The text is aligned at the normal end of the line (right-aligned for left-to-right locales, left-aligned for right-to-left locales).

    \sa setTextAlign(), setTextDirection(), fillText()
*/

/*!
    \enum QCanvasPainter::TextBaseline

    TextBaseline is used to define how the text is aligned (baselined) vertically.

    \value Top Align the top of the text vertically to the specified position.

    \value Hanging Align the hanging baseline of the text vertically to the specified position.

    \value Middle Align the middle of the text vertically to the specified position.

    \value Alphabetic (default) Align the baseline of the text vertically to the specified position.

    \value Bottom Align the bottom of the text vertically to the specified position.

    \sa setTextBaseline(), fillText()
*/

/*!
    \enum QCanvasPainter::TextDirection

    TextDirection is used to define how the text is aligned horizontally.

    \value LeftToRight The text direction is left-to-right.

    \value RightToLeft The text direction is right-to-left.

    \value Inherit (default) The text direction is inherited from QGuiApplication layoutDirection. See https://doc.qt.io/qt-6/qguiapplication.html#layoutDirection-prop.

    \value Auto The text direction is detected automatically based from the text string. See \l QString::isRightToLeft().
    \note As this requires analyzing the text, it is potentially slower that other options.

    \sa setTextDirection(), setTextAlign(), fillText()
*/

/*!
    \enum QCanvasPainter::CompositeOperation

    Qt Canvas Painter supports 3 composite operations:

    \value SourceOver The default value. Draws new shapes on top of the existing content.
    \value SourceAtop The new shape is only drawn where it overlaps the existing content.
    \value DestinationOut The existing content is kept where it doesn't overlap with the new shape.

    \sa setGlobalCompositeOperation()
*/

/*!
    \enum QCanvasPainter::WrapMode

    WrapMode is used to define how the text is wrapped to multiple lines.

    \value NoWrap (default) No wrapping will be performed. If the text contains insufficient newlines, then contentWidth will exceed a set width.

    \value Wrap If possible, wrapping occurs at a word boundary; otherwise it will occur at the appropriate point on the line, even in the middle of a word.

    \value WordWrap Wrapping is done on word boundaries only. If a word is too long, content width will exceed a set width.

    \value WrapAnywhere Wrapping is done at any point on a line, even if it occurs in the middle of a word.

    \sa setTextWrapMode(), fillText()
*/

/*!
    \enum QCanvasPainter::ImageFlag

    This enum specifies flags related to images. Use with
    \l addImage() to set the flags.

    \value GenerateMipmaps Set this to generate mipmaps for the image.
    Mipmaps should be used when smoother output is preferred for images
    which are scaled to smaller than the original size.
    \value RepeatX Use with image pattern to repeate image in X-coordinate.
    \value RepeatY Use with image pattern to repeate image in Y-coordinate.
    \value Repeat Use with image pattern to repeate image in both coordinates.
    \value FlipY Flips (inverses) image in Y direction when rendered.
    \value Premultiplied Image data has premultiplied alpha.
    \value Nearest Image interpolation is Nearest instead Linear
    \value NativeTexture Signifies this is a texture outside of QCanvasPainter.
*/

/*!
    \enum QCanvasPainter::RenderHint

    This enum specifies flags to QCanvasPainter related to rendering. Use
    \l setRenderHint() to set the flags.

    \value Antialiasing Setting this to false disables antialiasing.
    Enabling it results into higher rendering cost.
    The default value is true.

    \value HighQualityStroking Setting this to true gives a more correct
    rendering in some less common cases where stroking overlaps and
    doesn't have full opacity. Enabling it results into higher rendering cost.
    The default value is false.

    \value DisableWindingEnforce Setting this to true disables enforcing
    of path winding to match what has been set into setPathWinding().
    Disabling allows e.g. creating holes into paths by adding the points
    in clock wise order. Disabling can also increase the performance.
*/

/*!
    Constructs a painter.
*/

QCanvasPainter::QCanvasPainter()
 : d_ptr(new QCanvasPainterPrivate)
{
}

/*!
    Destroys the painter.
*/

QCanvasPainter::~QCanvasPainter()
    = default;

// *** State Handling ***

/*!
    Pushes and saves the current render state into a state stack.
    A matching restore() must be used to restore the state.
    \table
    \row
    \li \inlineimage qcpainter-save.webp
    \li
    \code
    p->strokeRect(20, 20, 160, 40);
    // Save and adjust the paint state
    p->save();
    p->setStrokeStyle(QColorConstants::Black);
    p->setLineWidth(3);
    p->rotate(0.1);
    p->strokeRect(20, 80, 180, 20);
    // Restore the saved paint state
    p->restore();
    p->strokeRect(20, 140, 160, 40);
    \endcode
    \endtable

    \sa restore()
*/

void QCanvasPainter::save()
{
    Q_D(QCanvasPainter);
    d->m_e->save();
}

/*!
    Pops and restores current render state.
    So previously saved state will be restored.
    If save() has not been called and the state stack
    is empty, calling this does nothing.

    \sa save()
*/

void QCanvasPainter::restore()
{
    Q_D(QCanvasPainter);
    d->m_e->restore();
}

/*!
    Resets the current painter state to default values.
    \note This method differs from the HTML canvas 2D context reset() method
    so that it doesn't visually clear the canvas buffers.
    \table
    \row
    \li \inlineimage qcpainter-reset.webp
    \li
    \code
    // Adjust the paint state
    p->setStrokeStyle("#00414A");
    p->setFillStyle("#2CDE85");
    p->setLineWidth(10);
    QRectF rect(20, 40, 160, 50);
    p->translate(rect.center());
    p->rotate(qDegreesToRadians(-25));
    p->translate(-rect.center());
    p->beginPath();
    p->roundRect(rect, 20);
    p->fill();
    p->stroke();
    // Reset to default paint state
    p->reset();
    p->fillRect(20, 140, 60, 40);
    p->strokeRect(120, 140, 60, 40);
    \endcode
    \endtable

    \sa save(), restore()
*/

void QCanvasPainter::reset()
{
    Q_D(QCanvasPainter);
    d->m_e->reset();
}

// *** Render styles ***

/*!
    Sets the stroke style to a solid \a color.
    The default stroke style is solid black color (0, 0, 0, 1).
    \table
    \row
    \li \inlineimage qcpainter-strokestyle.webp
    \li
    \code
    p->setStrokeStyle(QColorConstants::Black);
    p->strokeRect(20, 20, 160, 160);
    p->setStrokeStyle(QColor(0, 65, 74));
    p->strokeRect(40, 40, 120, 120);
    p->setStrokeStyle("#2CDE85");
    p->strokeRect(60, 60, 80, 80);
    \endcode
    \endtable
*/

void QCanvasPainter::setStrokeStyle(const QColor &color)
{
    Q_D(QCanvasPainter);
    d->m_e->setStrokeColor(color);
}

/*!
    \overload

    Sets the stroke style to \a brush.
    The default stroke style is solid black color (0, 0, 0, 1).
    \table
    \row
    \li \inlineimage qcpainter-strokestyle2.webp
    \li
    \code
    QCanvasLinearGradient g1(180, 20, 20, 180);
    g1.setStartColor(QColor(44, 222, 133));
    g1.setEndColor(Qt::black);
    p->setStrokeStyle(g1);
    p->strokeRect(20, 20, 160, 160);
    g1.setEndColor(Qt::yellow);
    p->setStrokeStyle(g1);
    p->strokeRect(40, 40, 120, 120);
    \endcode
    \endtable
*/

void QCanvasPainter::setStrokeStyle(const QCanvasBrush &brush)
{
    Q_D(QCanvasPainter);
    if (brush.type() == QCanvasBrush::BrushType::Custom) {
        auto b = static_cast<QCanvasCustomBrush *>(const_cast<QCanvasBrush *>(&brush));
        d->m_e->setCustomStrokeBrush(b);
    } else {
        d->m_e->setStrokePaint(brush.createPaint(this));
    }
}

/*!
    Sets the fill style to a solid \a color.
    The default fill style is solid black color (0, 0, 0, 1).
    \table
    \row
    \li \inlineimage qcpainter-fillstyle.webp
    \li
    \code
    p->setFillStyle(QColorConstants::Black);
    p->fillRect(20, 20, 160, 160);
    p->setFillStyle(QColor(0, 65, 74));
    p->fillRect(40, 40, 120, 120);
    p->setFillStyle("#2CDE85");
    p->fillRect(60, 60, 80, 80);
    \endcode
    \endtable
*/

void QCanvasPainter::setFillStyle(const QColor &color)
{
    Q_D(QCanvasPainter);
    d->m_e->setFillColor(color);
}

/*!
    \overload

    Sets the fill style to \a brush.
    The default fill style is solid black color (0, 0, 0, 1).
    \table
    \row
    \li \inlineimage qcpainter-fillstyle2.webp
    \li
    \code
    QCanvasRadialGradient g2(140, 40, 300);
    g2.setStartColor(QColor(44, 222, 133));
    g2.setEndColor(QColor(0, 65, 74));
    p->setFillStyle(g2);
    p->fillRect(20, 20, 160, 160);
    g2.setCenterPosition(100, 100);
    p->setFillStyle(g2);
    p->fillRect(40, 40, 120, 120);
    \endcode
    \endtable
*/

void QCanvasPainter::setFillStyle(const QCanvasBrush &brush)
{
    Q_D(QCanvasPainter);
    if (brush.type() == QCanvasBrush::BrushType::Custom) {
        auto b = static_cast<QCanvasCustomBrush *>(const_cast<QCanvasBrush *>(&brush));
        d->m_e->setCustomFillBrush(b);
    } else {
        d->m_e->setFillPaint(brush.createPaint(this));
    }
}

/*!
    Sets the miter limit to \a limit. Miter limit controls when a sharp corner
    is beveled. When the corner length would become longer than this limit,
    QCanvasPainter::LineJoin::Bevel will be applied between the lines instead.
    This has only effect with the QCanvasPainter::LineJoin::Miter line join.
    The default limit is \c 10.0.

    \sa setLineJoin()
*/

void QCanvasPainter::setMiterLimit(float limit)
{
    Q_D(QCanvasPainter);
    d->m_e->setMiterLimit(limit);
}

/*!
    Sets the line width of stroke to \a width in pixels.
    The default line width is \c 1.0. When the antialiasing is enabled,
    the line widths under a single pixel automatically fade the
    opacity, creating a smooth output.
    \table
    \row
    \li \inlineimage qcpainter-linewidth.webp
    \li
    \code
    for (int i = 1; i < 10 ; i++) {
        int y = i * 20;
        p->setLineWidth(0.5 * i);
        p->beginPath();
        p->moveTo(20, y);
        p->bezierCurveTo(80, y + 20, 120,
                         y - 20, 180, y);
        p->stroke();
    }
    \endcode
    \endtable

    \sa stroke()
*/

void QCanvasPainter::setLineWidth(float width)
{
    Q_D(QCanvasPainter);
    d->m_e->setLineWidth(width);
}

/*!
    Sets the end of the line of stoke to \a cap.
    The default line cap is \c QCanvasPainter::LineCap::Butt.
    \table
    \row
    \li \inlineimage qcpainter-linecap.webp
    \li
    \code
    QCanvasPath path;
    path.moveTo(40, 60);
    path.lineTo(160, 60);
    p->setLineCap(QCanvasPainter::LineCap::Butt);
    p->stroke(path, -1);
    p->setLineCap(QCanvasPainter::LineCap::Square);
    p->translate(0, 40);
    p->stroke(path, -1);
    p->setLineCap(QCanvasPainter::LineCap::Round);
    p->translate(0, 40);
    p->stroke(path, -1);
    \endcode
    \endtable
*/

void QCanvasPainter::setLineCap(LineCap cap)
{
    Q_D(QCanvasPainter);
    d->m_e->setLineCap(cap);
}

/*!
    Sets the line join of stroke to \a join.
    The default line join is \c QCanvasPainter::LineJoin::Miter.
    \table
    \row
    \li \inlineimage qcpainter-linejoin.webp
    \li
    \code
    QCanvasPath path;
    path.moveTo(40, 20);
    path.lineTo(100, 80);
    path.lineTo(160, 40);
    path.lineTo(160, 70);
    p->setLineJoin(QCanvasPainter::LineJoin::Miter);
    p->stroke(path, -1);
    p->setLineJoin(QCanvasPainter::LineJoin::Bevel);
    p->translate(0, 50);
    p->stroke(path, -1);
    p->setLineJoin(QCanvasPainter::LineJoin::Round);
    p->translate(0, 50);
    p->stroke(path, -1);
    \endcode
    \endtable

    \sa setMiterLimit()
*/

void QCanvasPainter::setLineJoin(LineJoin join)
{
    Q_D(QCanvasPainter);
    d->m_e->setLineJoin(join);
}

/*!
    Sets the global alpha (transparency) value to \a alpha. This alpha value is
    applied to all rendered shapes. Already transparent paths will get
    proportionally more transparent as well.
    Alpha should be between 0.0 (fully transparent) and 1.0 (fully opaque).
    By default alpha is \c 1.0.
    \table
    \row
    \li \inlineimage qcpainter-globalalpha.webp
    \li
    \code
    static QImage logo(":/qt_logo2.png");
    QCanvasImage image = p->addImage(logo);
    p->setFillStyle("#d9f720");
    for (int i = 0; i < 4; i++) {
        float x = 100 * (i % 2);
        float y = 100 * (i / 2);
        QRectF rect(x, y, 100, 100);
        p->setGlobalAlpha(1.0 - i * 0.3);
        p->fillRect(rect);
        p->drawImage(image, rect);
    }
    \endcode
    \endtable
*/

void QCanvasPainter::setGlobalAlpha(float alpha)
{
    Q_D(QCanvasPainter);
    d->m_e->setGlobalAlpha(alpha);
}

/*!
    Sets the global composite operation mode to \a operation. This mode is
    applied to all painting operations. The default mode is
    \c QCanvasPainter::CompositeOperation::SourceOver.
*/

void QCanvasPainter::setGlobalCompositeOperation(CompositeOperation operation)
{
    Q_D(QCanvasPainter);
    d->m_e->setGlobalCompositeOperation(operation);
}

/*!
    Sets the global brightness to \a value. This brightess is
    applied to all rendered shapes. A value of 0 will cause painting
    to be completely black. Value can also be bigger than 1.0, to
    increase the brightness.
    By default, brightness is \c 1.0.
    \table
    \row
    \li \inlineimage qcpainter-globalbrightness.webp
    \li
    \code
    static QImage logo(":/qt_logo2.png");
    QCanvasImage image = p->addImage(logo);
    p->setFillStyle("#d9f720");
    for (int i = 0; i < 4; i++) {
        float x = 100 * (i % 2);
        float y = 100 * (i / 2);
        QRectF rect(x, y, 100, 100);
        p->setGlobalBrightness(1.5 - i * 0.45);
        p->fillRect(rect);
        p->drawImage(image, rect);
    }
    \endcode
    \endtable
*/

void QCanvasPainter::setGlobalBrightness(float value)
{
    Q_D(QCanvasPainter);
    d->m_e->setGlobalBrightness(value);
}

/*!
    Sets the global contrast to \a value. This contrast is
    applied to all rendered shapes. A value of 0 will cause painting
    to be completely gray (0.5, 0.5, 0.5). Value can also be bigger
    than 1.0, to increase the contrast.
    By default, contrast is \c 1.0.
    \table
    \row
    \li \inlineimage qcpainter-globalcontrast.webp
    \li
    \code
    static QImage logo(":/qt_logo2.png");
    QCanvasImage image = p->addImage(logo);
    p->setFillStyle("#d9f720");
    for (int i = 0; i < 4; i++) {
        float x = 100 * (i % 2);
        float y = 100 * (i / 2);
        QRectF rect(x, y, 100, 100);
        p->setGlobalContrast(1.5 - i * 0.45);
        p->fillRect(rect);
        p->drawImage(image, rect);
    }
    \endcode
    \endtable
*/

void QCanvasPainter::setGlobalContrast(float value)
{
    Q_D(QCanvasPainter);
    d->m_e->setGlobalContrast(value);
}

/*!
    Sets the global saturations to \a value. This saturations is
    applied to all rendered shapes. A value of 0 will disable saturation
    and cause painting to be completely grayscale. Value can also be bigger
    than 1.0, to increase the saturation.
    By default, saturation is \c 1.0.
    \table
    \row
    \li \inlineimage qcpainter-globalsaturate.webp
    \li
    \code
    static QImage logo(":/qt_logo2.png");
    QCanvasImage image = p->addImage(logo);
    p->setFillStyle("#d9f720");
    for (int i = 0; i < 4; i++) {
        float x = 100 * (i % 2);
        float y = 100 * (i / 2);
        QRectF rect(x, y, 100, 100);
        p->setGlobalSaturate(1.5 - i * 0.5);
        p->fillRect(rect);
        p->drawImage(image, rect);
    }
    \endcode
    \endtable
*/

void QCanvasPainter::setGlobalSaturate(float value)
{
    Q_D(QCanvasPainter);
    d->m_e->setGlobalSaturate(value);
}

// *** Transforms ***

/*!
    Resets current transform to a identity matrix.
*/

void QCanvasPainter::resetTransform()
{
    Q_D(QCanvasPainter);
    d->m_e->resetTransform();
}

/*!
    Resets the current transform and uses \a transform instead.
    \table
    \row
    \li \inlineimage qcpainter-transform.webp
    \li
    \code
    p->beginPath();
    p->roundRect(80, 20, 40, 40, 10);
    p->fill();
    p->stroke();
    QTransform t;
    t.translate(100, 20);
    t.rotate(45);
    t.scale(2.0, 2.0);
    p->setTransform(t);
    p->beginPath();
    p->roundRect(20, 20, 40, 40, 10);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::setTransform(const QTransform &transform)
{
    Q_D(QCanvasPainter);
    d->m_e->setTransform(transform);
}

/*!
    Multiplies the current coordinate system by specified \a transform.
    \table
    \row
    \li \inlineimage qcpainter-transform2.webp
    \li
    \code
    QTransform t;
    t.translate(100, 100);
    t.rotate(36);
    t.translate(-100, -100);
    for (int i = 0; i < 10; i++) {
        p->transform(t);
        p->beginPath();
        p->roundRect(80, 15, 40, 20, 10);
        p->fill();
        p->stroke();
    }
    \endcode
    \endtable
*/

void QCanvasPainter::transform(const QTransform &transform)
{
    Q_D(QCanvasPainter);
    d->m_e->transform(transform);
}

/*!
    Translates current coordinate system by \a x and \a y.
    \table
    \row
    \li \inlineimage qcpainter-translate.webp
    \li
    \code
    auto paintRect = [p]() {
        p->beginPath();
        p->roundRect(20, 20, 160, 60, 10);
        p->fill();
        p->stroke();
    };
    paintRect();
    p->translate(0, 100);
    paintRect();
    \endcode
    \endtable
*/

void QCanvasPainter::translate(float x, float y)
{
    Q_D(QCanvasPainter);
    d->m_e->translate(x, y);
}

/*!
    \fn void QCanvasPainter::translate(QPointF point)
    \overload

    Translates current coordinate system by \a point.
*/

/*!
    Rotates current coordinate system clockwise by \a angle.

    The angle is specified in radians. Use qDegreesToRadians() to convert from
    degrees to radians.
    \table
    \row
    \li \inlineimage qcpainter-rotate.webp
    \li
    \code
    QRectF rect(20, 70, 160, 60);
    p->translate(rect.center());
    p->rotate(-M_PI / 4);
    p->translate(-rect.center());
    p->beginPath();
    p->roundRect(rect, 10);
    p->fill();
    p->stroke();
    p->setFillStyle(QColorConstants::Black);
    p->fillText("Cute!", rect);
    \endcode
    \endtable
*/

void QCanvasPainter::rotate(float angle)
{
    Q_D(QCanvasPainter);
    d->m_e->rotate(angle);
}

/*!
    Skews (shears) the current coordinate system along X axis by \a angleX
    and along Y axis by \a angleY. Angles are specifid in radians.
    \table
    \row
    \li \inlineimage qcpainter-skew.webp
    \li
    \code
    QRectF rect(40, 70, 120, 60);
    p->translate(rect.center());
    p->skew(-0.6);
    p->translate(-rect.center());
    p->beginPath();
    p->roundRect(rect, 10);
    p->fill();
    p->stroke();
    p->setFillStyle(QColorConstants::Black);
    p->fillText("Cute!", rect);
    \endcode
    \endtable
*/

void QCanvasPainter::skew(float angleX, float angleY)
{
    Q_D(QCanvasPainter);
    d->m_e->skew(angleX, angleY);
}

/*!
    Scales the current coordinat system by \a scale. Both x and y coordinates
    are scaled evenly.
    \table
    \row
    \li \inlineimage qcpainter-scale.webp
    \li
    \code
    QRectF rect(20, 20, 160, 160);
    for (int i = 0; i < 20; i++) {
        p->beginPath();
        p->roundRect(rect, 10);
        p->stroke();
        p->translate(rect.center());
        p->scale(0.8);
        p->translate(-rect.center());
    }
    \endcode
    \endtable
*/

void QCanvasPainter::scale(float scale)
{
    Q_D(QCanvasPainter);
    d->m_e->scale(scale, scale);
}

/*!
    \overload

    Scales the current coordinat system by \a scaleX and \a scaleY.
*/

void QCanvasPainter::scale(float scaleX, float scaleY)
{
    Q_D(QCanvasPainter);
    d->m_e->scale(scaleX, scaleY);
}

/*!
    Returns the current transform.
*/

QTransform QCanvasPainter::getTransform() const
{
    Q_D(const QCanvasPainter);
    return d->m_e->currentTransform();
}

/*!
   \internal
    Sets the current brush transform to \a transform. This transform is
    applied to both stroke and fill brushes.

    \note Setting the brush transform will change the internal state.
    Make sure to set it back to the identity transform before doing other
    paint operations.
*/

void QCanvasPainter::setBrushTransform(const QTransform &transform)
{
    Q_D(QCanvasPainter);
    d->m_e->setBrushTransform(transform);
}

// *** Clipping ***

/*!
    Sets the current scissor rectangle to (\a x, \a y, \a width, \a height).
    The scissor rectangle is transformed by the current transform.
    \note Clipping has some performance cost and it should only be used
    when needed.
    \table
    \row
    \li \inlineimage qcpainter-cliprect.webp
    \li
    \code
    QRectF viewArea(20, 20, 160, 160);
    p->setClipRect(viewArea);
    p->beginPath();
    p->circle(40, 40, 110);
    p->fill();
    p->setFillStyle(Qt::black);
    p->fillText("Clip me...", 40, 100);
    p->strokeRect(viewArea);
    \endcode
    \endtable

    \sa resetClipping()
*/

void QCanvasPainter::setClipRect(float x, float y, float width, float height)
{
    Q_D(QCanvasPainter);
    d->m_e->setClipRect(QRectF(x, y, width, height));
}

/*!
    \fn void QCanvasPainter::setClipRect(const QRectF &rect)
    \overload

    Sets the current scissor rectangle to \a rect.
    The scissor rectangle is transformed by the current transform.
    \note Clipping has some performance cost and it should only be used
    when needed.
    \sa resetClipping()
*/

/*!
    Resets and disables clipping.
    \sa setClipRect()
*/

void QCanvasPainter::resetClipping()
{
    Q_D(QCanvasPainter);
    d->m_e->resetClipRect();
}

//  *** Paths ***

/*!
    Begins drawing a new path while clearing the current path.
*/

void QCanvasPainter::beginPath()
{
    Q_D(QCanvasPainter);
    d->m_e->beginPath();
}

/*!
    Closes the current sub-path with a line segment.
    This is equivalent to lineTo([starting point]) as the last path element.
*/

void QCanvasPainter::closePath()
{
    Q_D(QCanvasPainter);
    d->m_e->closePath();
}

/*!
    Starts new sub-path with ( \a x, \a y) as first point.
*/

void QCanvasPainter::moveTo(float x, float y)
{
    Q_D(QCanvasPainter);
    d->m_e->moveTo(x, y);
}

/*!
    \fn void QCanvasPainter::moveTo(QPointF point)
    \overload

    Starts new sub-path with \a point as first point.
*/

/*!
    Adds line segment from the last point in the path to the ( \a x, \a y) point.
    \table
    \row
    \li \inlineimage qcpainter-line.webp
    \li
    \code
    p->beginPath();
    p->moveTo(20, 20);
    p->lineTo(140, 180);
    p->lineTo(180, 120);
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::lineTo(float x, float y)
{
    Q_D(QCanvasPainter);
    d->m_e->lineTo(x, y);
}

/*!
    \fn void QCanvasPainter::lineTo(QPointF point)
    \overload

    Adds line segment from the last point in the path to the \a point.
*/

/*!
    Adds cubic bezier segment from last point in the path via two
    control points (\a cp1X, \a cp1Y and \a cp2X, \a cp2Y) to the specified point (\a x, \a y).
    \table
    \row
    \li \inlineimage qcpainter-beziercurve.webp
    \li
    \code
    p->beginPath();
    p->moveTo(20, 20);
    p->bezierCurveTo(150, 50, 50, 250, 180, 120);
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::bezierCurveTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y)
{
    Q_D(QCanvasPainter);
    d->m_e->bezierTo(cp1X, cp1Y, cp2X, cp2Y, x, y);
}

/*!
    \fn void QCanvasPainter::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
    \overload

    Adds cubic bezier segment from last point in the path via two
    control points (\a controlPoint1 and \a controlPoint2) to the specified point \a endPoint.
*/

/*!
    Adds quadratic bezier segment from last point in the path via
    a control point (\a cpX, \a cpY) to the specified point (\a x, \a y).
    \table
    \row
    \li \inlineimage qcpainter-quadraticcurve.webp
    \li
    \code
    p->beginPath();
    p->moveTo(20, 20);
    p->quadraticCurveTo(150, 50, 180, 180);
    p->quadraticCurveTo(20, 220, 20, 20);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::quadraticCurveTo(float cpX, float cpY, float x, float y)
{
    Q_D(QCanvasPainter);
    d->m_e->quadTo(cpX, cpY, x, y);
}

/*!
    \fn void QCanvasPainter::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
    \overload

    Adds quadratic bezier segment from last point in the path via
    a \a controlPoint to the specified \a endPoint.
*/

/*!
    Adds an arc segment at the corner defined by the last path point,
    and two specified points (\a x1, \a y1 and \a x2, \a y2) with \a radius.
    The arc is automatically connected to the path's latest point with
    a straight line if necessary.
    \table
    \row
    \li \inlineimage qcpainter-arcto.webp
    \li
    \code
    p->beginPath();
    p->moveTo(20, 20);
    p->arcTo(240, 20, 20, 220, 50);
    p->arcTo(20, 220, 20, 20, 30);
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::arcTo(float x1, float y1, float x2, float y2, float radius)
{
    Q_D(QCanvasPainter);
    d->m_e->arcTo(x1, y1, x2, y2, radius);
}

/*!
    \fn void QCanvasPainter::arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius)
    \overload

    Adds an arc segment at the corner defined by the last path point,
    and two specified points (\a controlPoint1 and \a controlPoint2) with \a radius.
    The arc is automatically connected to the path's latest point with
    a straight line if necessary.
*/

/*!
    Creates new circle arc shaped sub-path. The arc center is at \a centerX, \a centerY,
    with \a radius, and the arc is drawn from angle \a a0 to \a a1,
    and swept in \a direction (ClockWise or CounterClockWise).
    When \a connection is \l{PathConnection::}{NotConnected}, arc does not add a line from the previous
    path position to the start of the arc.
    Angles are specified in radians.
    \table
    \row
    \li \inlineimage qcpainter-arc.webp
    \li
    \code
    p->beginPath();
    p->moveTo(100, 100);
    p->arc(100, 100, 80, 0, 1.5 * M_PI);
    p->closePath();
    p->fill();
    p->stroke();
    \endcode
    \endtable

    \note While HTML canvas 2D context uses arc() for painting circles, with
    QCanvasPainter it is recommended to use \l circle() or \l ellipse() for those.
*/

void QCanvasPainter::arc(float centerX, float centerY, float radius, float a0, float a1, PathWinding direction, PathConnection connection)
{
    Q_D(QCanvasPainter);
    const bool isConnected = connection == PathConnection::Connected;
    d->m_e->addArc(centerX, centerY, radius, a0, a1, direction, isConnected);
}

/*!
    \fn void QCanvasPainter::arc(QPointF centerPoint, float radius, float a0, float a1, PathWinding direction, PathConnection connection)
    \overload

    Creates new circle arc shaped sub-path. The arc center is at \a centerPoint,
    with \a radius, and the arc is drawn from angle \a a0 to \a a1,
    and swept in \a direction (ClockWise or CounterClockWise).
    When \a connection is \l{PathConnection::}{NotConnected}, arc does not add a line from the previous
    path position to the start of the arc.
    Angles are specified in radians.

    \note While HTML canvas 2D context uses arc() for painting circles, with
    QCanvasPainter it is recommended to use \l circle() or \l ellipse() for those.
*/

/*!
    Creates new rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height.
    \table
    \row
    \li \inlineimage qcpainter-rect.webp
    \li
    \code
    p->beginPath();
    p->rect(20, 20, 160, 160);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::rect(float x, float y, float width, float height)
{
    Q_D(QCanvasPainter);
    d->m_e->addRect(x, y, width, height);
}

/*!
    \fn void QCanvasPainter::rect(const QRectF &rect)
    \overload

    Creates new rectangle shaped sub-path at \a rect.
    This is an overloaded method using QRectF.
*/

/*!
    Creates new rounded rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height. Corners rounding will be \a radius.
    \table
    \row
    \li \inlineimage qcpainter-roundrect.webp
    \li
    \code
    p->beginPath();
    p->roundRect(20, 20, 160, 160, 30);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::roundRect(float x, float y, float width, float height, float radius)
{
    Q_D(QCanvasPainter);
    d->m_e->addRoundRect(x, y, width, height, radius);
}

/*!
    \fn void QCanvasPainter::roundRect(const QRectF &rect, float radius)
    \overload

    Creates new rounded rectangle shaped sub-path at \a rect with \a radius corners.
    This is an overloaded method using QRectF.
*/

/*!
    \overload

    Creates new rounded rectangle shaped sub-path in position \a x, \a y with
    size \a width, \a height. Corners rounding can be varying per-corner, with
    \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight, \a radiusBottomLeft.
    \table
    \row
    \li \inlineimage qcpainter-roundrect2.webp
    \li
    \code
    p->beginPath();
    p->roundRect(20, 20, 160, 160,
                 0, 40, 20, 80);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::roundRect(float x, float y, float width, float height, float radiusTopLeft, float radiusTopRight, float radiusBottomRight, float radiusBottomLeft)
{
    Q_D(QCanvasPainter);
    d->m_e->addRoundRect(x, y, width, height, radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

/*!
    \fn void QCanvasPainter::roundRect(const QRectF &rect, float radiusTopLeft, float radiusTopRight, float radiusBottomRight, float radiusBottomLeft)
    \overload

    Creates new rounded rectangle shaped sub-path at \a rect. Corners rounding can be
    varying per-corner, with \a radiusTopLeft, \a radiusTopRight, \a radiusBottomRight,
    \a radiusBottomLeft.
*/

/*!
    Creates new ellipse shaped sub-path into ( \a centerX, \a centerY) with \a radiusX and \a radiusY.
    \table
    \row
    \li \inlineimage qcpainter-ellipse.webp
    \li
    \code
    p->beginPath();
    p->ellipse(100, 100, 80, 60);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::ellipse(float centerX, float centerY, float radiusX, float radiusY)
{
    Q_D(QCanvasPainter);
    d->m_e->addEllipse(centerX, centerY, radiusX, radiusY);
}

/*!
    \fn void QCanvasPainter::ellipse(QPointF centerPoint, float radiusX, float radiusY)
    \overload

    Creates new ellipse shaped sub-path into \a centerPoint with \a radiusX and \a radiusY.
*/

/*!
    \fn void QCanvasPainter::ellipse(const QRectF &rect)
    \overload

    Creates new ellipse shaped sub-path into \a rect.
    This ellipse will cover the \a rect area.
    \table
    \row
    \li \inlineimage qcpainter-ellipse2.webp
    \li
    \code
    QRectF rect(40, 20, 120, 160);
    p->fillRect(rect);
    p->beginPath();
    p->ellipse(rect);
    p->stroke();
    \endcode
    \endtable
*/

/*!
    Creates new circle shaped sub-path into ( \a centerX, \a centerY) with \a radius.
    \table
    \row
    \li \inlineimage qcpainter-circle.webp
    \li
    \code
    p->beginPath();
    p->circle(100, 100, 80);
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::circle(float centerX, float centerY, float radius)
{
    Q_D(QCanvasPainter);
    d->m_e->addCircle(centerX, centerY, radius);
}

/*!
    \fn void QCanvasPainter::circle(QPointF centerPoint, float radius)
    \overload

    Creates new circle shaped sub-path into \a centerPoint with \a radius.
*/

/*!
    Adds \a path into the current path.

    \note QCanvasPainter uses WindingFill (nonzero) fillrule,
    which means that all QPainterPaths don't render correctly. This is notable
    for example when path contains text characters with holes in them.

    \note This method is available mostly for the compatibility with QPainter
    and QPainterPath. It does not increase performance compared to painting
    the path directly with QCanvasPainter methods.
*/

void QCanvasPainter::addPath(const QPainterPath &path)
{
    Q_D(QCanvasPainter);
    d->m_e->addPath(path);
}

/*!
    Adds \a path into the current path, optionally using \a transform to
    alter the path points. When \a transform is not provided (or it is
    identity matrix), this operation is very fast as it reuses the path data.
    \table
    \row
    \li \inlineimage qcpainter-addpath.webp
    \li
    \code
    // m_path is QCanvasPath
    if (m_path.isEmpty())
        m_path.circle(60, 60, 40);
    p->beginPath();
    p->addPath(m_path);
    p->addPath(m_path, QTransform::fromTranslate(80, 80));
    p->fill();
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::addPath(const QCanvasPath &path, const QTransform &transform)
{
    Q_D(QCanvasPainter);
    d->m_e->addPath(path, transform);
}

/*!
    Adds \a path into the current path, starting from the command at \a start
    and including \a count amount of commands. Optionally using \a transform to
    alter the path points.
    The range of \a start and \a count is checked, so that commands are not
    accessed more than \l QCanvasPath::commandsSize().
    In case the path shouldn't continue from the current path position, call
    first \l moveTo() e.g. with \c{path.positionAt(start - 1)}.
    \table
    \row
    \li \inlineimage qcpainter-addpath2.webp
    \li
    \code
    // m_path is QCanvasPath
    if (m_path.isEmpty()) {
        m_path.moveTo(20, 60);
        for (int i = 1; i < 160; i++) {
            m_path.lineTo(20 + i,
                          60 + 20 * sin(0.1 * i));
        }
    }
    p->stroke(m_path);
    p->beginPath();
    p->addPath(m_path, 20, 100,
               QTransform::fromTranslate(0, 80));
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::addPath(const QCanvasPath &path,
                        qsizetype start, qsizetype count,
                        const QTransform &transform)
{
    Q_D(QCanvasPainter);
    d->m_e->addPath(path, start, count, transform);
}


/*!
    Sets the current sub-path \a winding to either CounterClockWise (default) or ClockWise.
    CounterClockWise draws solid subpaths while ClockWise draws holes.
    \table
    \row
    \li \inlineimage qcpainter-pathwinding.webp
    \li
    \code
    p->beginPath();
    p->roundRect(20, 20, 160, 160, 40);
    p->setPathWinding(QCanvasPainter::PathWinding::ClockWise);
    p->circle(140, 60, 20);
    p->rect(60, 120, 80, 30);
    p->fill();
    p->stroke();
    \endcode
    \endtable

    \sa beginHoleSubPath(), beginSolidSubPath()
*/

void QCanvasPainter::setPathWinding(PathWinding winding)
{
    Q_D(QCanvasPainter);
    d->m_e->setPathWinding(winding);
}

/*!
    Start a solid subpath. This is equivalent to
    \c setPathWinding(QCanvasPainter::PathWinding::CounterClockWise))
    \sa beginHoleSubPath()
*/

void QCanvasPainter::beginSolidSubPath()
{
    Q_D(QCanvasPainter);
    d->m_e->setPathWinding(PathWinding::CounterClockWise);
}

/*!
    Start a hole subpath. This is equivalent to
    \c setPathWinding(QCanvasPainter::PathWinding::ClockWise))
    \table
    \row
    \li \inlineimage qcpainter-beginhole.webp
    \li
    \code
    p->beginPath();
    p->circle(100, 100, 80);
    p->beginHoleSubPath();
    p->rect(60, 60, 80, 80);
    p->beginSolidSubPath();
    p->circle(100, 100, 20);
    p->fill();
    p->stroke();
    \endcode
    \endtable

    \sa beginSolidSubPath()
*/

void QCanvasPainter::beginHoleSubPath()
{
    Q_D(QCanvasPainter);
    d->m_e->setPathWinding(PathWinding::ClockWise);
}

/*!
    Fills the current path with current fill style.
    \sa setFillStyle()
    \table
    \row
    \li \inlineimage qcpainter-fill.webp
    \li
    \code
    p->beginPath();
    p->rect(20, 20, 40, 160);
    p->rect(140, 20, 40, 160);
    p->circle(100, 100, 60);
    p->fill();
    \endcode
    \endtable
*/

void QCanvasPainter::fill()
{
    Q_D(QCanvasPainter);
    d->m_e->fill();
}

/*!
    Strokes the current path with current stroke style.
    \sa setStrokeStyle()
    \table
    \row
    \li \inlineimage qcpainter-stroke.webp
    \li
    \code
    p->beginPath();
    p->rect(20, 20, 40, 160);
    p->rect(140, 20, 40, 160);
    p->circle(100, 100, 60);
    p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::stroke()
{
    Q_D(QCanvasPainter);
    d->m_e->stroke();
}

/*!
    \overload

    Fills the \a path with current fill style and belonging into \a
    pathGroup. Painting through QCanvasPath is optimal when the path
    contains more commands is mostly static.

    When \a pathGroup is \c -1, the path will not be cached on GPU side.
    This is the default. To request the caching of path data, pass a
    value equal or greater to \c 0. More information about using path
    cache groups in \l{QCanvasPath} documentation.

    Calling beginPath() before this method is not required.

    \table
    \row
    \li \inlineimage qcpainter-fill2.webp
    \li
    \code
    // m_path is QCanvasPath
    if (m_path.isEmpty()) {
        for (int i = 0; i < 16; i++) {
            float w = 100 + 60 * sin(i);
            m_path.rect(100 - w * 0.5,
                        22 + i * 10,
                        w, 6);
        }
    }
    p->fill(m_path);
    \endcode
    \endtable

    \sa setFillStyle(), removePathGroup()
*/

void QCanvasPainter::fill(const QCanvasPath &path, int pathGroup)
{
    Q_D(QCanvasPainter);
    d->m_e->fill(path, pathGroup);
}

/*!
    \overload

    Strokes the \a path with current stroke style and belonging into \a
    pathGroup. Painting through QCanvasPath is optimal when the path
    contains more commands is mostly static.

    When \a pathGroup is \c -1, the path's rendering-related data will not be
    cached. This is the default. To request the caching of path data, pass a
    value equal or greater to \c 0. More information about using path cache
    groups can be found in the \l{QCanvasPath} documentation.

    Calling beginPath() before this method is not required.

    \table
    \row
    \li \inlineimage qcpainter-stroke2.webp
    \li
    \code
    // m_path is QCanvasPath
    if (m_path.isEmpty()) {
        for (int i = 0; i < 16; i++) {
            int h = 100 + 60 * sin(i);
            m_path.rect(22 + i * 10,
                        180 - h,
                        6, h);
        }
    }
    p->stroke(m_path);
    \endcode
    \endtable

    \sa setStrokeStyle(), removePathGroup()
*/

void QCanvasPainter::stroke(const QCanvasPath &path, int pathGroup)
{
    Q_D(QCanvasPainter);
    d->m_e->stroke(path, pathGroup);
}

// *** Direct drawing ***

/*!
    Draws a filled rectangle into specified position ( \a x, \a y) at size \a width, \a height.
    \note This is provided for convenience. When filling more than just a single rect,
    prefer using rect().
    \table
    \row
    \li \inlineimage qcpainter-fillrect.webp
    \li
    \code
    p->fillRect(20, 20, 160, 160);
    // The above code does same as:
    //  p->beginPath();
    //  p->rect(20, 20, 160, 160);
    //  p->fill();
    \endcode
    \endtable
*/

void QCanvasPainter::fillRect(float x, float y, float width, float height)
{
    Q_D(QCanvasPainter);
    d->m_e->beginPath();
    d->m_e->addRect(x, y, width, height);
    d->m_e->fill();
}

/*!
    \fn void QCanvasPainter::fillRect(const QRectF &rect)
    \overload

    Draws a filled rectangle into \a rect.
    This is an overloaded method using QRectF.
    \note This is provided for convenience. When filling more than just a single rect,
    prefer using rect().
*/

/*!
    Erases the pixels in a rectangular area by filling the rectangle
    specified by \a x, \a y, \a width, \a height with transparent black.
    As clearing does not need blending, it can be faster than fillRect().
    \table
    \row
    \li \inlineimage qcpainter-clearrect.webp
    \li
    \code
    p->beginPath();
    p->circle(100, 100, 80);
    p->fill();
    p->stroke();
    p->clearRect(60, 0, 80, 120);
    \endcode
    \endtable
*/

void QCanvasPainter::clearRect(float x, float y, float width, float height)
{
    Q_D(QCanvasPainter);
    d->m_e->beginPath();
    d->m_e->addRect(x, y, width, height);
    d->m_e->fillForClear();
}

/*!
    \fn void QCanvasPainter::clearRect(const QRectF &rect)
    \overload

    Erases the pixels in a rectangular area by filling the rectangle
    specified by \a rect with transparent black.
    This is an overloaded method using QRectF.
*/

/*!
    Draws a stoked rectangle into specified position ( \a x, \a y) at size \a width, \a height.
    \note This is provided for convenience. When stroking more than just a single rect,
    prefer using rect().
    \table
    \row
    \li \inlineimage qcpainter-strokerect.webp
    \li
    \code
    p->strokeRect(20, 20, 160, 160);
    // The above code does same as:
    // p->beginPath();
    // p->rect(20, 20, 160, 160);
    // p->stroke();
    \endcode
    \endtable
*/

void QCanvasPainter::strokeRect(float x, float y, float width, float height)
{
    Q_D(QCanvasPainter);
    d->m_e->beginPath();
    d->m_e->addRect(x, y, width, height);
    d->m_e->stroke();
}

/*!
    \fn void QCanvasPainter::strokeRect(const QRectF &rect)
    \overload

    Draws a stoked rectangle into \a rect.
    This is an overloaded method using QRectF.
    \note This is provided for convenience. When stroking more than just a single rect,
    prefer using rect().
*/

// *** Shadows ***

/*!
    Draws a box \a shadow. The shadow will be painted with the
    position, size, color, blur etc. set in the \a shadow.
    Calling beginPath() before this method is not required.
    \note To visually see the area covered by drawBoxShadow(), set
    \c QCPAINTER_DEBUG_SHADOW_RECT environment variable.
    \table
    \row
    \li \inlineimage qcpainter-shadowbox.webp
    \li
    \code
    QRectF rect(40, 40, 120, 120);
    QRectF shadowRect = rect.translated(-2, 4);
    QCanvasBoxShadow shadow(shadowRect, 0, 30);
    p->drawBoxShadow(shadow);
    p->beginPath();
    p->roundRect(rect, 30);
    p->setFillStyle("#2CDE85");
    p->fill();
    \endcode
    \endtable

    \sa QCanvasBoxShadow
*/

void QCanvasPainter::drawBoxShadow(const QCanvasBoxShadow &shadow)
{
    Q_D(QCanvasPainter);
    d->drawBoxShadow(this, shadow);
}

// *** Images ***

/*!
    Draw \a image into \a x, \a y, at its default size.
    \table
    \row
    \li \inlineimage qcpainter-drawimage.webp
    \li
    \code
    static QImage logo(":/qt_logo.png");
    QCanvasImage image = p->addImage(logo);
    p->drawImage(image, 36, 36);
    \endcode
    \endtable

    \sa addImage()
*/

void QCanvasPainter::drawImage(const QCanvasImage &image, float x, float y)
{
    Q_D(QCanvasPainter);
    if (image.isNull())
        return;

    d->m_e->drawImageId(image.id(), x, y, image.width(), image.height(), image.tintColor());
    d->markTextureIdUsed(image.id());
}

/*!
    \overload

    Draw \a image into \a x, \a y, at given \a width and \a height.
    \table
    \row
    \li \inlineimage qcpainter-drawimage2.webp
    \li
    \code
    static QImage logo(":/qt_logo.png");
    QCanvasImage image = p->addImage(logo);
    p->drawImage(image, 50, 0, 100, 200);
    \endcode
    \endtable

    \sa addImage()
*/

void QCanvasPainter::drawImage(const QCanvasImage &image, float x, float y, float width, float height)
{
    Q_D(QCanvasPainter);
    if (image.isNull())
        return;

    d->m_e->drawImageId(image.id(), x, y, width, height, image.tintColor());
    d->markTextureIdUsed(image.id());
}

/*!
    \fn void QCanvasPainter::drawImage(const QCanvasImage &image, const QRectF &destinationRect)
    \overload

    Draw \a image into position and size of \a destinationRect.
    \sa addImage()
*/

/*!
    \overload

    Draw \a image into position and size of \a destinationRect, from \a sourceRect area of image.
    \table
    \row
    \li \inlineimage qcpainter-drawimage3.webp
    \li
    \code
    static QImage logo(":/qt_logo.png");
    QCanvasImage image = p->addImage(logo);
    QRectF sourceRect(20, 30, 54, 76);
    QRectF destinationRect(0, 0, 200, 200);
    p->drawImage(image, sourceRect, destinationRect);
    \endcode
    \endtable

    \sa addImage()
*/

void QCanvasPainter::drawImage(const QCanvasImage &image, const QRectF &sourceRect, const QRectF &destinationRect)
{
    Q_D(QCanvasPainter);
    if (image.isNull())
        return;

    float sx = float(sourceRect.x());
    float sy = float(sourceRect.y());
    float sw = float(sourceRect.width());
    float sh = float(sourceRect.height());
    float dx = float(destinationRect.x());
    float dy = float(destinationRect.y());
    float dw = float(destinationRect.width());
    float dh = float(destinationRect.height());
    float startX = dx - sx * (dw/sw);
    float startY = dy - sy * (dh/sh);
    float endX = dw * image.width() / sw;
    float endY = dh * image.height() / sh;
    d->m_e->drawImageIdAt(image.id(), startX, startY, endX, endY,
                          dx, dy, dw, dh, image.tintColor());
    d->markTextureIdUsed(image.id());
}

// *** Text ***

/*!
    Sets the \a font as currently active font.
*/

void QCanvasPainter::setFont(const QFont &font)
{
    Q_D(QCanvasPainter);
    d->setFont(font);
}

/*!
    Sets the horizontal alignment of text to \a align.
    The default alignment is \c QCanvasPainter::TextAlign::Start.
    \table
    \row
    \li \inlineimage qcpainter-textalign.webp
    \li
    \code
    QFont font("Titillium Web", 22);
    p->setFont(font);
    p->fillRect(100, 0, 1, 200);
    p->setTextAlign(QCanvasPainter::TextAlign::Left);
    p->fillText("Left", 100, 40);
    p->setTextAlign(QCanvasPainter::TextAlign::Center);
    p->fillText("Center", 100, 70);
    p->setTextAlign(QCanvasPainter::TextAlign::Right);
    p->fillText("Right", 100, 100);
    p->setTextAlign(QCanvasPainter::TextAlign::Start);
    p->fillText("Start", 100, 130);
    p->setTextAlign(QCanvasPainter::TextAlign::End);
    p->fillText("End", 100, 160);
    \endcode
    \endtable

    \sa setTextBaseline()
*/

void QCanvasPainter::setTextAlign(QCanvasPainter::TextAlign align)
{
    Q_D(QCanvasPainter);
    d->m_e->setTextAlignment(align);
}

/*!
    Sets the vertical alignment (baseline) of text to \a baseline.
    The default alignment is \c QCanvasPainter::TextBaseline::Alphabetic.
    \table
    \row
    \li \inlineimage qcpainter-textbaseline.webp
    \li
    \code
    QFont font("Titillium Web", 16);
    p->setFont(font);
    p->fillRect(0, 60, 200, 1);
    p->fillRect(0, 140, 200, 1);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Bottom);
    p->fillText("Bottom", 40, 60);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    p->fillText("Middle", 100, 60);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Top);
    p->fillText("Top", 160, 60);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Alphabetic);
    p->fillText("Alphabetic", 50, 140);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Hanging);
    p->fillText("Hanging", 150, 140);
    \endcode
    \endtable

    \sa setTextAlign()
*/

void QCanvasPainter::setTextBaseline(QCanvasPainter::TextBaseline baseline)
{
    Q_D(QCanvasPainter);
    d->engine()->setTextBaseline(baseline);
}

/*!
    Sets the direction of text to \a direction.
    The default direction is \c QCanvasPainter::TextDirection::Inherit.
*/

void QCanvasPainter::setTextDirection(QCanvasPainter::TextDirection direction)
{
    Q_D(QCanvasPainter);
    d->engine()->setTextDirection(direction);
}

/*!
    Sets the text wrap mode to \a wrapMode.
    The default wrap mode is \c QCanvasPainter::WrapMode::NoWrap.
    \table
    \row
    \li \inlineimage qcpainter-textwrapmode.webp
    \li
    \code
    QRectF r1(50, 5, 100, 60);
    QRectF r2(50, 70, 100, 60);
    QRectF r3(50, 135, 100, 60);
    p->strokeRect(r1);
    p->strokeRect(r2);
    p->strokeRect(r3);
    QString s("This is a long string.");
    p->setTextWrapMode(QCanvasPainter::WrapMode::NoWrap);
    p->fillText(s, r1);
    p->setTextWrapMode(QCanvasPainter::WrapMode::Wrap);
    p->fillText(s, r2);
    p->setTextWrapMode(QCanvasPainter::WrapMode::WrapAnywhere);
    p->fillText(s, r3);
    \endcode
    \endtable
*/

void QCanvasPainter::setTextWrapMode(QCanvasPainter::WrapMode wrapMode)
{
    Q_D(QCanvasPainter);
    d->engine()->setTextWrapMode(wrapMode);
}

/*!
    Sets the line height adjustment in pixels to \a height
    for wrapped text. The default line height is \c 0.
    \table
    \row
    \li \inlineimage qcpainter-textlineheight.webp
    \li
    \code
    QRectF r1(40, 5, 120, 60);
    QRectF r2(40, 70, 120, 60);
    QRectF r3(40, 135, 120, 60);
    p->strokeRect(r1);
    p->strokeRect(r2);
    p->strokeRect(r3);
    p->setTextLineHeight(-10);
    p->fillText("Text with line height: -10", r1);
    p->setTextLineHeight(0);
    p->fillText("Text with line height: 0", r2);
    p->setTextLineHeight(10);
    p->fillText("Text with line height: 10", r3);
    \endcode
    \endtable
*/

void QCanvasPainter::setTextLineHeight(float height)
{
    Q_D(QCanvasPainter);
    d->engine()->setTextLineHeight(height);
}

/*!
    Set the current text antialiasing amount. The value \a antialias
    is multiplier to normal antialiasing, meaning that \c 0.0 disables
    antialiasing and 2.0 doubles it. The default value is 1.0.

    \note Due to the used text antialiasing technique (SDF),
    the maximum antialiasing amount is quite limited and this
    affects less when the font size is small.
    \table
    \row
    \li \inlineimage qcpainter-textantialias.webp
    \li
    \code
    QFont font("Titillium Web", 20);
    p->setFont(font);
    p->setTextAntialias(1.0);
    p->fillText("Antialiasing: 1.0", 100, 25);
    p->setTextAntialias(2.0);
    p->fillText("Antialiasing: 2.0", 100, 75);
    p->setTextAntialias(3.0);
    p->fillText("Antialiasing: 3.0", 100, 125);
    p->setTextAntialias(4.0);
    p->fillText("Antialiasing: 4.0", 100, 175);
    \endcode
    \endtable
*/

void QCanvasPainter::setTextAntialias(float antialias)
{
    Q_D(QCanvasPainter);
    d->engine()->setTextAntialias(antialias);
}

/*!
    Draws \a text string at specified location ( \a x, \a y), with current textAlign and textBaseline.
    To make the text wrap into multiple lines, set optional \a maxWidth parameter to preferred
    row width in pixels. White space is stripped at the beginning of the rows,
    the text is split at word boundaries or when new-line characters are encountered.
    Words longer than the max width are split at nearest character (i.e. no hyphenation).
*/
void QCanvasPainter::fillText(const QString &text, float x, float y, float maxWidth)
{
    Q_D(QCanvasPainter);
    d->fillText(text, x, y, maxWidth);
}

/*!
    \fn void QCanvasPainter::fillText(const QString &text, QPointF point, float maxWidth)
    \overload

    Draws \a text string at specified \a point, with current textAlign and textBaseline.
    To make the text wrap into multiple lines, set optional \a maxWidth parameter to preferred
    row width in pixels. White space is stripped at the beginning of the rows,
    the text is split at word boundaries or when new-line characters are encountered.
    Words longer than the max width are split at nearest character (i.e. no hyphenation).

    This is an overloaded method using QPointF.
*/

/*!
    \overload

    Draws \a text string inside \a rect, with current textAlign and textBaseline.
    Width of the rect parameter is used as maxWidth.

    This is an overloaded method using QRectF. It is often useful to set the
    text baseline to \l TextBaseline::Top or \l TextBaseline::Middle when painting
    text with this method.
*/
void QCanvasPainter::fillText(const QString &text, const QRectF &rect)
{
    Q_D(QCanvasPainter);
    d->fillText(text, rect);
}

/*!
    Measures bounding box of a \a text string at (\a x, \a y).
    To measure multi-line text, set optional \a maxWidth parameter to preferred row width in pixels.
    Returns QRectF with values [xmin, ymin, width, height].
    Measured values are returned in local coordinate space.
    \table
    \row
    \li \inlineimage qcpainter-textboundingbox.webp
    \li
    \code
    QString s("Built with Qt");
    QPointF pos1(20, 20);
    QRectF box1 = p->textBoundingBox(s, pos1);
    p->strokeRect(box1);
    p->fillText(s, pos1);
    p->setTextWrapMode(QCanvasPainter::WrapMode::WordWrap);
    p->setTextAlign(QCanvasPainter::TextAlign::Center);
    QPointF pos2(100, 80);
    QRectF box2 = p->textBoundingBox(s, pos2, 100);
    p->strokeRect(box2);
    p->fillText(s, pos2, 100);
    \endcode
    \endtable
*/

QRectF QCanvasPainter::textBoundingBox(const QString &text, float x, float y, float maxWidth)
{
    Q_D(QCanvasPainter);
    return d->textBoundingBox(text, x, y, maxWidth);
}

/*!
    \fn QRectF QCanvasPainter::textBoundingBox(const QString &text, QPointF point, float maxWidth)
    \overload

    Measures bounding box of a \a text string at \a point.
    To measure multi-line text, set optional \a maxWidth parameter to preferred row width in pixels.
    Returns QRectF with values [xmin, ymin, width, height].
    Measured values are returned in local coordinate space.
*/

/*!
    \overload

    Measures bounding box of a \a text string at \a rect.
    Returns QRectF with values [xmin, ymin, width, height].
    Measured values are returned in local coordinate space.
*/

QRectF QCanvasPainter::textBoundingBox(const QString &text, const QRectF &rect)
{
    Q_D(QCanvasPainter);
    return d->textBoundingBox(text, rect);
}

/*!
    Set the current antialiasing amount to \a antialias in pixels.
    More antialias means smoother painting. This only affects fill and stroke painting,
    not images or texts.
    The default value is \c 1.0 and the maximum value is \c 10.0.

    Antialiasing can be modified per-path so it can be set before each stroke/fill.
    To disable antialiasing from the whole canvas painter, use
    QCanvasPainter::RenderHint::Antialiasing render hint.
    \table
    \row
    \li \inlineimage qcpainter-antialias.webp
    \li
    \code
    p->setLineWidth(6);
    for (int i = 1; i < 10 ; i++) {
        int y = i * 20;
        p->setAntialias(i);
        p->beginPath();
        p->moveTo(20, y);
        p->bezierCurveTo(80, y + 20, 120,
                         y - 20, 180, y);
        p->stroke();
    }
    \endcode
    \endtable

    \sa setRenderHints(), setTextAntialias()
*/

void QCanvasPainter::setAntialias(float antialias)
{
    Q_D(QCanvasPainter);
    d->m_e->setAntialias(antialias);
}

/*!
    Returns the ratio between physical pixels and device-independent pixels.
    The default value is \c 1.0.
*/

float QCanvasPainter::devicePixelRatio() const
{
    Q_D(const QCanvasPainter);
    return d->m_devicePixelRatio;
}

/*!
    Sets the given render \a hint on the painter if \a on is true;
    otherwise clears the render hint.

    \sa setRenderHints(), renderHints()
*/

void QCanvasPainter::setRenderHint(RenderHint hint, bool on)
{
    setRenderHints(hint, on);
}

/*!
    Sets the given render \a hints on the painter if \a on is true;
    otherwise clears the render hints.

    \sa setRenderHint(), renderHints()
*/

void QCanvasPainter::setRenderHints(RenderHints hints, bool on)
{
    Q_D(QCanvasPainter);
    d->m_e->setRenderHints(hints, on);
}

/*!
    Returns a flag that specifies the rendering hints that are set for
    this painter.

    \sa setRenderHint()
*/

QCanvasPainter::RenderHints QCanvasPainter::renderHints() const
{
    Q_D(const QCanvasPainter);
    return d->m_e->renderHints();
}

// ***** Static methods *****

/*!
    Static helper method to convert millimeters \a mm into pixels.
    This allows doing resolution independent drawing. For example to set
    the line width to 2mm use:

    painter->setLineWidth(QCanvasPainter::mmToPx(2));
*/

float QCanvasPainter::mmToPx(float mm)
{
    float ldp = 72.0f;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        ldp = float(screen->physicalDotsPerInch());
    } else {
        qWarning() << "QScreen required for mmToPx";
    }
    return ldp * mm / 25.4f;
}

/*!
    Static helper method to convert points \a pt into pixels.
*/

float QCanvasPainter::ptToPx(float pt)
{
    float ldp = 72.0f;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        ldp = float(screen->physicalDotsPerInch());
    } else {
        qWarning() << "QScreen required for ptToPx";
    }
    return pt * (ldp / 72.0f);
}

/*!
    Adds \a image with \a flags available for the painter as a texture.
    Returns QCanvasImage with the texture id and other information about the image.
    Returned QCanvasImage can then be used with \l drawImage and \l QCanvasImagePattern.
    After calling this method, \a image QImage does not need be kept in memory.

    Calling with the same \a image is a cheap operation, since a cache hit is
    expected.

    Care must be taken when optimizing to call addImage() only once. That is not
    always sufficient, depending on the application design. For example, if the
    underlying graphics resources are lost, e.g. because the painter is
    associated with a new QRhi under the hood due to moving a widget to a new
    top-level, then calling this function is essential in order to re-create the
    native graphics textures from \a image.

    \sa drawImage(), removeImage()
*/

QCanvasImage QCanvasPainter::addImage(const QImage &image, QCanvasPainter::ImageFlags flags)
{
    Q_D(QCanvasPainter);
    return d->getQCanvasImage(image, flags);
}

/*!
    \overload

    Adds \a texture with \a flags available for the painter as a texture. The
    flag NativeTexture is set implicitly. The returned QCanvasImage can be used with
    \l drawImage and \l QCanvasImagePattern.

    \note The ownership of \a texture is \b not taken.

    \sa drawImage(), removeImage()
*/

QCanvasImage QCanvasPainter::addImage(QRhiTexture *texture, QCanvasPainter::ImageFlags flags)
{
    Q_D(QCanvasPainter);
    return d->getQCanvasImage(texture, flags);
}

/*!
    \overload

    Registers \a canvas with \a flags to the painter so that it is available as an image.
    The returned QCanvasImage can be used with \l drawImage and \l QCanvasImagePattern.

    \note \a canvas continues to manage the underlying native graphics
    resources, meaning removeImage() does not render \a canvas invalid.

    \sa drawImage(), removeImage()
*/

QCanvasImage QCanvasPainter::addImage(const QCanvasOffscreenCanvas &canvas, QCanvasPainter::ImageFlags flags)
{
    Q_D(QCanvasPainter);
    return d->getQCanvasImage(canvas, flags);
}

/*!
    Releases the resources associated with \a image and removes the image from
    the painter.

    \note This does not need to be normally called, because resources such as
    textures are released in the painter destructor anyway. This function is
    useful when there is a need to reduce memory usage due to having a lot
    images, and some of them are not used anymore.

    \note Removed images can not be used in paint operations anymore.

    \note Resources such as the textures created with the underlying 3D API may
    not get released immediately. Such operations may get defered to subsequent
    frames, typically when this QCanvasPainter begins painting again after the active
    set of draw calls has been submitted.

    \sa addImage()
*/

void QCanvasPainter::removeImage(const QCanvasImage &image)
{
    Q_D(QCanvasPainter);
    if (image.isNull())
        return;

    const int id = image.id();
    switch (QCanvasImagePrivate::get(&image)->type) {
    case QCanvasImagePrivate::DataType::GradientTextureFromImage:
    case QCanvasImagePrivate::DataType::TextureFromImage:
        d->m_imageTracker.removeTextureId(id);
        break;
    case QCanvasImagePrivate::DataType::ImportedTexture:
        for (auto it = d->m_nativeTextureCache.cbegin(); it != d->m_nativeTextureCache.cend(); ) {
            if (it->id() == id)
                it = d->m_nativeTextureCache.erase(it);
            else
                ++it;
        }
        d->m_pendingNativeTextureDelete.insert(id);
        break;
    case QCanvasImagePrivate::DataType::Unknown:
        Q_UNREACHABLE();
        break;
    }
}

/*!
    Schedules dropping unused textures from the cache.

    Additionally, other caches and pools may get shrunk upon
    calling this function, in order to minimize memory usage.
    This may potentially lead to more expensive drawing calls
    afterwards.
*/

void QCanvasPainter::cleanupResources()
{
    Q_D(QCanvasPainter);
    d->m_imageTracker.removeTemporaryResources();
    d->m_e->releaseUnusedResources();
}

/*!
    \return an approximation in kilobytes of the memory used by the image
    (pixel) data for all active QCanvasImage instances for this painter that were
    created by the addImage() overload taking a QImage. It also includes the
    data from internally created images for \l{QCanvasGradient}{gradients}.

    QCanvasPainter does not keep copies of the CPU-side QImage data once
    addImage() has returned. Therefore, the result of this function is an
    approximation of the GPU memory that is used for textures.

    \note The value is only an estimate based on the image format and
    dimensions. Qt has no knowledge of how the data for textures is stored and
    laid out in memory on the GPU side.

    Offscreen canvases and externally managed textures registered via the other
    addImage() overloads are not taken into account by this function.

    For every valid QCanvasImage, the individual size in bytes can always be queried
    by calling \l{QCanvasImage::}{sizeInBytes()}. That function returns valid results also
    when the QCanvasImage was created from a QCanvasOffscreenCanvas or QRhiTexture, but it
    does not consider mipmap or multisample data.

    \sa activeImageCount(), addImage(), removeImage()
*/

qsizetype QCanvasPainter::activeImageMemoryUsage() const
{
    Q_D(const QCanvasPainter);
    return d->m_imageTracker.dataAmount() * 0.001;
}

/*!
    \return the number of active QCanvasImage objects registered with this QCanvasPainter.
    This also includes the internally created images for
    \l{QCanvasGradient}{gradients}.

    QCanvasImage objects created by registering QCanvasOffscreenCanvas instances or
    externally managed textures are not taken into account by this function.

    \sa activeImageMemoryUsage(), addImage(), removeImage()
*/

qsizetype QCanvasPainter::activeImageCount() const
{
    Q_D(const QCanvasPainter);
    return d->m_imageTracker.size();
}

/*!
    Removes \a pathGroup from the painter cache. Calling fill() or stroke()
    for \a pathGroup after this, will regenerate the path into the group cache.

    \note This does not need to be normally called as paths are removed
    in the painter destructor. Only use this to reduce memory
    usage when \a pathGroup is not needed anymore or e.g. when the path
    has a lot less commands that it has had in the past and buffer size
    should be reduced.

    \sa fill(), stroke()
*/

void QCanvasPainter::removePathGroup(int pathGroup)
{
    Q_D(QCanvasPainter);
    d->engine()->removePathGroup(pathGroup);
}

// ***** Private *****

// Marks all unused temporary textures to be removed.
void QCanvasImageTracker::removeTemporaryResources()
{
    m_doingResourcesRemoval = true;
}

// Marks a texture with imageId to be removed
void QCanvasImageTracker::removeTextureId(int imageId)
{
    for (auto i = m_data.cbegin(), end = m_data.cend(); i != end; ++i) {
        if (i.value().id() == imageId) {
            m_cleanupTextures << i.value();
            break;
        }
    }
}

// Handles texture removal from GPU and from the cache
void QCanvasImageTracker::handleRemoveTextures()
{
    Q_ASSERT(m_painterPrivate);
    if (m_cleanupTextures.isEmpty() && !m_doingResourcesRemoval) {
        // Nothing to be done, reset the used textures list.
        m_usedTextureIDs.clear();
        return;
    }
    auto it = m_data.begin();
    while (it != m_data.end()) {
        QCanvasImagePrivate *ip = QCanvasImagePrivate::get(&*it);
        bool remove = m_cleanupTextures.contains(*it);
        bool removeUnusedGradients = m_doingResourcesRemoval &&
                                     ip->type == QCanvasImagePrivate::DataType::GradientTextureFromImage &&
                                     !m_usedTextureIDs.contains(ip->id);
        if (remove || removeUnusedGradients) {
            m_painterPrivate->m_e->deleteImage(ip->id);
            m_dataAmount -= ip->sizeInBytes;
            it = m_data.erase(it);
        } else {
            it++;
        }
    }
    // Cleanups done, so reset
    m_cleanupTextures.clear();
    m_usedTextureIDs.clear();
    m_doingResourcesRemoval = false;
}

void QCanvasImageTracker::clear()
{
    m_data.clear();
    m_cleanupTextures.clear();
    m_usedTextureIDs.clear();
    m_dataAmount = 0;
    m_doingResourcesRemoval = false;
}

// Marks imageId as being currently used.
void QCanvasImageTracker::markTextureIdUsed(int imageId)
{
    if (imageId > 0 && !m_usedTextureIDs.contains(imageId))
        m_usedTextureIDs << imageId;
}


QCanvasPainterPrivate::QCanvasPainterPrivate()
{
    m_imageTracker.m_painterPrivate = this;
    m_e = new QCPainterEngine();
    const int defaultMaxTextures = 1024;
    static int maxTexturesEnv = qEnvironmentVariableIntValue("QCPAINTER_MAX_TEXTURES");
    m_maxTextures = maxTexturesEnv > 0 ? maxTexturesEnv : defaultMaxTextures;
    m_trackingDisabled = qEnvironmentVariableIsSet("QCPAINTER_DISABLE_TEXTURE_USAGE_TRACKING");
}

QCanvasPainterPrivate::~QCanvasPainterPrivate()
{
    // Cleanup possible pending textures
    handleCleanupTextures();

    delete m_e;
}

QCPainterEngine *QCanvasPainterPrivate::engine() const
{
    return m_e;
}


static QRectF textAlignedRectFromPoint(QCanvasPainter::TextAlign textAlignment, float x, float y, float maxWidth)
{
    QRectF rect;
    if (textAlignment == QCanvasPainter::TextAlign::Left) {
        rect.setRect(x, y, maxWidth, 0);
    } else if (textAlignment == QCanvasPainter::TextAlign::Center) {
        const float hWidth = maxWidth * 0.5f;
        rect.setRect(x - hWidth, y, maxWidth, 0);
    } else {
        rect.setRect(x - maxWidth, y, maxWidth, 0);
    }
    return rect;
}

// Delete the textures marked to be cleaned.
// This is also automatically called by engine to keep the amount
// of (gradient) textures in control.
void QCanvasPainterPrivate::handleCleanupTextures()
{
    if (m_renderer && m_renderer->ctx) {
        m_imageTracker.handleRemoveTextures();

        for (int id : std::as_const(m_pendingNativeTextureDelete))
            m_e->deleteImage(id); // does not destroy the actual QRhiTexture since it is not owned
        m_pendingNativeTextureDelete.clear();
    }
}

// Removes all data from m_imageTracker, does not destroy the actual textures. To
// be called when the renderer is going away (and so all rhi textures are going
// to be deleted by it).
void QCanvasPainterPrivate::clearTextureCache()
{
    m_imageTracker.clear();

    m_nativeTextureCache.clear(); // these we do not own anyway
}

// Marks that this imageId was used during the paint operation.
void QCanvasPainterPrivate::markTextureIdUsed(int imageId) {
    if (!m_trackingDisabled)
        m_imageTracker.markTextureIdUsed(imageId);
}

qint64 QCanvasPainterPrivate::generateImageKey(const QImage &image, QCanvasPainter::ImageFlags flags) const
{
    return image.cacheKey() ^ int(flags);
}

// Get the texture id for the image. Id will be unique for the image+flags, or imageKey
// in case that is provided.
QCanvasImage QCanvasPainterPrivate::getQCanvasImage(const QImage &image, QCanvasPainter::ImageFlags flags, qint64 imageKey)
{
    Q_ASSERT(m_renderer);
    QCanvasImage qcimage;
    qint64 key = imageKey;
    // the imageKey is 0 except when coming from QCanvasGradient
    QCanvasImagePrivate::DataType type = QCanvasImagePrivate::DataType::GradientTextureFromImage;
    if (imageKey == 0) {
        key = generateImageKey(image, flags);
        type = QCanvasImagePrivate::DataType::TextureFromImage;
    }
    if (m_imageTracker.contains(key)) {
        // Image is in cache
        qcimage = m_imageTracker.image(key);
    } else {
        if (image.isNull()) {
            qWarning() << "Empty image";
        } else {
            QImage convertedImage;
            if (image.format() != QImage::Format_RGBA8888 && image.format() != QImage::Format_RGBA8888_Premultiplied) {
                qCDebug(QC_INFO) << "Converting image" << key << "from" << image.format() << "to" << QImage::Format_RGBA8888_Premultiplied;
                convertedImage = image.convertedTo(QImage::Format_RGBA8888_Premultiplied);
            } else {
                convertedImage = image;
            }
            if (convertedImage.format() == QImage::Format_RGBA8888_Premultiplied)
                flags |= QCanvasPainter::ImageFlag::Premultiplied;
            else
                flags &= ~int(QCanvasPainter::ImageFlag::Premultiplied);
            QCanvasImagePrivate *ip = QCanvasImagePrivate::get(&qcimage);
            ip->id = m_e->createImage(image.width(), image.height(), flags, convertedImage.constBits());
            ip->width = convertedImage.width();
            ip->height = convertedImage.height();
            ip->sizeInBytes = convertedImage.sizeInBytes();
            ip->type = type;
            m_imageTracker.insert(key, qcimage);
            // When the amount of cache reaches the limit and tracking is enabled,
            // remove all the unused temporary textures automatically.
            if (!m_trackingDisabled && m_imageTracker.size() > m_maxTextures) {
                qCDebug(QC_INFO) << "Removing temporary gradient textures as max amount of" << m_maxTextures << "was reached.";
                m_imageTracker.removeTemporaryResources();
            }
        }
    }
    return qcimage;
}

QCanvasImage QCanvasPainterPrivate::getQCanvasImage(QRhiTexture *texture, QCanvasPainter::ImageFlags flags)
{
    Q_ASSERT(m_renderer);
    QCanvasImage qcimage;
    const quint64 key = texture->globalResourceId();
    if (m_nativeTextureCache.contains(key)) {
        qcimage = m_nativeTextureCache.value(key);
        QCanvasImagePrivate *ip = QCanvasImagePrivate::get(&qcimage);
        if (ip->width != texture->pixelSize().width() || ip->height != texture->pixelSize().height())
            qcimage = {};
    }
    if (qcimage.isNull()) {
        QCanvasImagePrivate *ip = QCanvasImagePrivate::get(&qcimage);
        ip->id = m_renderer->renderCreateNativeTexture(texture, flags)->id;
        ip->width = texture->pixelSize().width();
        ip->height = texture->pixelSize().height();
        quint32 byteSize = 0;
        QCPainterRhiRenderer::textureFormatInfo(texture->format(), texture->pixelSize(), nullptr, &byteSize, nullptr);
        ip->sizeInBytes = byteSize;
        ip->type = QCanvasImagePrivate::DataType::ImportedTexture;
        m_nativeTextureCache.insert(key, qcimage);
    }
    return qcimage;
}

QCanvasImage QCanvasPainterPrivate::getQCanvasImage(const QCanvasOffscreenCanvas &canvas, QCanvasPainter::ImageFlags flags)
{
    if (canvas.isNull())
        return {};

    if (m_renderer->isOffscreenCanvasYUp())
        flags.setFlag(QCanvasPainter::ImageFlag::FlipY, !flags.testFlag(QCanvasPainter::ImageFlag::FlipY));

    // From the QCanvasImage and painter perspective an offscreen canvas is no
    // different from a QDCImage created directly from a QRhiTexture.
    return getQCanvasImage(canvas.texture(), flags);
}

void QCanvasPainterPrivate::setFont(const QFont &font)
{
    m_e->state.font = font;
}

void QCanvasPainterPrivate::fillText(const QString &text, float x, float y, float maxWidth)
{
    // Unify point & rect APIs behavior.
    const QRectF rect = textAlignedRectFromPoint(m_e->state.textAlignment, x, y, maxWidth);
    fillText(text, rect);
}

void QCanvasPainterPrivate::fillText(const QString &text, const QRectF &rect)
{
    m_e->fillText(text, rect);
}

QRectF QCanvasPainterPrivate::textBoundingBox(const QString &text, float x, float y, float maxWidth)
{
    // Unify point & rect APIs behavior.
    const QRectF rect = textAlignedRectFromPoint(m_e->state.textAlignment, x, y, maxWidth);
    return m_e->textBoundingBox(text, rect);
}

QRectF QCanvasPainterPrivate::textBoundingBox(const QString &text, const QRectF &rect)
{
    return m_e->textBoundingBox(text, rect);
}

void QCanvasPainterPrivate::drawBoxShadow(QCanvasPainter *painter, const QCanvasBoxShadow &shadow)
{
    const auto paint = shadow.createPaint(painter);
    const auto r = shadow.boundingRect();
    m_e->fillPlainRect(paint, r.x(), r.y(), r.width(), r.height());
    static bool shadowRectDebug = qEnvironmentVariableIsSet("QCPAINTER_DEBUG_SHADOW_RECT");
    if (Q_UNLIKELY(shadowRectDebug)) {
        m_e->save();
        m_e->setStrokeColor(QColorConstants::Red);
        m_e->setLineWidth(1.0f);
        painter->strokeRect(r);
        m_e->restore();
    }
}

/*!
    \return a new offscreen canvas with the given \a pixelSize, \a sampleCount,
    and \a flags.

    The size of the canvas is specified in pixels. The \a pixelSize, \a
    sampleCount, and \a flags properties are immutable afterwards. To get a
    canvas with a different size, sample count, or flags, create a new one.

    To target an offscreen canvas with with draw commands, call the appropriate
    \l QCanvasRhiPaintDriver::beginPaint() overload when working with the lower level
    API, or \l QCanvasPainterWidget::beginCanvasPainting() or \l
    QCanvasPainterItemRenderer::beginCanvasPainting() when using the convenience
    widget or Qt Quick item classes.

    Normally the contents of the canvas is cleared when painting to it. To
    disable this, pass \l{QCanvasOffscreenCanvas::Flag::}{PreserveContents} in \a flags.

    To request multisample rendering onto the canvas (multisample antialiasing,
    MSAA), set a sample count larger than 1, such as 4 or 8. Preserving the
    canvas contents between render passes is not supported however when
    multisampling is enabled, and the \l{QCanvasOffscreenCanvas::Flag::}{PreserveContents} flag will not
    work in this case.
 */
QCanvasOffscreenCanvas QCanvasPainter::createCanvas(QSize pixelSize, int sampleCount, QCanvasOffscreenCanvas::Flags flags)
{
    Q_D(QCanvasPainter);
    if (!d->m_renderer || !d->m_renderer->ctx) {
        qWarning("createCanvas() cannot be called without renderer");
        return {};
    }
    return d->m_renderer->createCanvas(pixelSize, sampleCount, flags);
}

/*!
    Destroys the resources backing \a canvas. \a canvas becomes a
    \l{QCanvasOffscreenCanvas::isNull()}{null canvas} then.

    The painter automatically does this upon its destruction. Therefore, calling
    this function is only necessary when releasing the associated resources is
    desired right away.
*/
void QCanvasPainter::destroyCanvas(QCanvasOffscreenCanvas &canvas)
{
    Q_D(QCanvasPainter);
    if (!d->m_renderer || !d->m_renderer->ctx) {
        qWarning("destroyCanvas() cannot be called without renderer");
        return;
    }
    d->m_renderer->destroyCanvas(canvas);
}

QT_END_NAMESPACE
