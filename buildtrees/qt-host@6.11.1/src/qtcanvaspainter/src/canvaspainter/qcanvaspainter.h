// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPAINTER_H
#define QCANVASPAINTER_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtGui/qcolor.h>
#include <QtCore/qrect.h>
#include <QtGui/qtransform.h>
#include <QtGui/qfont.h>

#include <QtCanvasPainter/qcanvasimage.h>
#include <QtCanvasPainter/qcanvasoffscreencanvas.h>

QT_BEGIN_NAMESPACE

class QCanvasBrush;
class QCanvasImage;
class QCanvasPath;
class QCanvasBoxShadow;
class QRhiTexture;
class QCanvasPainterPrivate;

class QCanvasPainter
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasPainter();
    Q_CANVASPAINTER_EXPORT ~QCanvasPainter();

    enum class PathWinding : quint8 { CounterClockWise, ClockWise };
    enum class PathConnection : quint8 { NotConnected, Connected };
    enum class LineCap : quint8 { Butt, Round, Square };
    enum class LineJoin : quint8 { Round, Bevel, Miter };
    enum class TextAlign : quint8 { Left, Right, Center, Start, End };
    enum class TextBaseline : quint8 { Top, Hanging, Middle, Alphabetic, Bottom };
    enum class TextDirection : quint8 { LeftToRight, RightToLeft, Inherit, Auto };

    enum class CompositeOperation : quint8 {
        SourceOver,
        SourceAtop,
        DestinationOut,
    };

    enum class WrapMode : quint8 {
        NoWrap,
        Wrap,
        WordWrap,
        WrapAnywhere,
    };

    // TODO: NativeTexture is used internally, so remove from this public API?
    enum class ImageFlag {
        GenerateMipmaps = 1 << 0,
        RepeatX = 1 << 1,
        RepeatY = 1 << 2,
        Repeat = RepeatX | RepeatY,
        FlipY = 1 << 3,
        Premultiplied = 1 << 4,
        Nearest = 1 << 5,
        NativeTexture = 1 << 6,
    };
    Q_DECLARE_FLAGS(ImageFlags, ImageFlag)

    enum class RenderHint {
        Antialiasing = 1 << 0,
        HighQualityStroking = 1 << 1,
        DisableWindingEnforce = 1 << 2,
    };
    Q_DECLARE_FLAGS(RenderHints, RenderHint)

    // *** State Handling ***

    Q_CANVASPAINTER_EXPORT void save();
    Q_CANVASPAINTER_EXPORT void restore();
    Q_CANVASPAINTER_EXPORT void reset();

    // *** Render styles ***

    Q_CANVASPAINTER_EXPORT void setStrokeStyle(const QColor &color);
    Q_CANVASPAINTER_EXPORT void setStrokeStyle(const QCanvasBrush &brush);
    Q_CANVASPAINTER_EXPORT void setFillStyle(const QColor &color);
    Q_CANVASPAINTER_EXPORT void setFillStyle(const QCanvasBrush &brush);
    Q_CANVASPAINTER_EXPORT void setMiterLimit(float limit);
    Q_CANVASPAINTER_EXPORT void setLineWidth(float width);
    Q_CANVASPAINTER_EXPORT void setLineCap(LineCap cap);
    Q_CANVASPAINTER_EXPORT void setLineJoin(LineJoin join);
    Q_CANVASPAINTER_EXPORT void setGlobalAlpha(float alpha);
    Q_CANVASPAINTER_EXPORT void setGlobalCompositeOperation(CompositeOperation operation);
    Q_CANVASPAINTER_EXPORT void setGlobalBrightness(float value);
    Q_CANVASPAINTER_EXPORT void setGlobalContrast(float value);
    Q_CANVASPAINTER_EXPORT void setGlobalSaturate(float value);

    // *** Transforms ***

    Q_CANVASPAINTER_EXPORT void resetTransform();
    Q_CANVASPAINTER_EXPORT void setTransform(const QTransform &transform);
    Q_CANVASPAINTER_EXPORT void transform(const QTransform &transform);
    Q_CANVASPAINTER_EXPORT void translate(float x, float y);
    inline void translate(QPointF point);
    Q_CANVASPAINTER_EXPORT void rotate(float angle);
    Q_CANVASPAINTER_EXPORT void skew(float angleX, float angleY = 0.0f);
    Q_CANVASPAINTER_EXPORT void scale(float scale);
    Q_CANVASPAINTER_EXPORT void scale(float scaleX, float scaleY);
    Q_CANVASPAINTER_EXPORT QTransform getTransform() const;

    Q_CANVASPAINTER_EXPORT void setBrushTransform(const QTransform &transform);

    // *** Clipping ***

    Q_CANVASPAINTER_EXPORT void setClipRect(float x, float y, float width, float height);
    inline void setClipRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void resetClipping();

    //  *** Paths ***

    Q_CANVASPAINTER_EXPORT void beginPath();
    // These should match to path methods of QCanvasPath for consistency.
    Q_CANVASPAINTER_EXPORT void closePath();
    Q_CANVASPAINTER_EXPORT void moveTo(float x, float y);
    inline void moveTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void lineTo(float x, float y);
    inline void lineTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void bezierCurveTo(float cp1X, float cp1Y, float cp2X, float cp2Y, float x, float y);
    inline void bezierCurveTo(
        QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void quadraticCurveTo(float cpX, float cpY, float x, float y);
    inline void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void arcTo(float x1, float y1, float x2, float y2, float radius);
    inline void arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius);

    Q_CANVASPAINTER_EXPORT void arc(
        float centerX,
        float centerY,
        float radius,
        float a0,
        float a1,
        PathWinding direction = PathWinding::ClockWise,
        PathConnection connection = PathConnection::Connected);
    inline void arc(
        QPointF centerPoint,
        float radius,
        float a0,
        float a1,
        PathWinding direction = PathWinding::ClockWise,
        PathConnection connection = PathConnection::Connected);
    Q_CANVASPAINTER_EXPORT void rect(float x, float y, float width, float height);
    inline void rect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void roundRect(float x, float y, float width, float height, float radius);
    inline void roundRect(const QRectF &rect, float radius);
    Q_CANVASPAINTER_EXPORT void roundRect(
        float x,
        float y,
        float width,
        float height,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    inline void roundRect(
        const QRectF &rect,
        float radiusTopLeft,
        float radiusTopRight,
        float radiusBottomRight,
        float radiusBottomLeft);
    Q_CANVASPAINTER_EXPORT void ellipse(float centerX, float centerY, float radiusX, float radiusY);
    inline void ellipse(QPointF centerPoint, float radiusX, float radiusY);
    inline void ellipse(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void circle(float centerX, float centerY, float radius);
    inline void circle(QPointF centerPoint, float radius);

    // TODO: Consider if we want to have this as functionality
    // doesn't fully cover QPainterPath (like fillRule).
    Q_CANVASPAINTER_EXPORT void addPath(const QPainterPath &path);
    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path,
                                        const QTransform &transform = QTransform());
    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path,
                                        qsizetype start, qsizetype count,
                                        const QTransform &transform = QTransform());

    Q_CANVASPAINTER_EXPORT void setPathWinding(PathWinding winding);
    Q_CANVASPAINTER_EXPORT void beginSolidSubPath();
    Q_CANVASPAINTER_EXPORT void beginHoleSubPath();

    Q_CANVASPAINTER_EXPORT void fill();
    Q_CANVASPAINTER_EXPORT void stroke();

    Q_CANVASPAINTER_EXPORT void fill(const QCanvasPath &path, int pathGroup = -1);
    Q_CANVASPAINTER_EXPORT void stroke(const QCanvasPath &path, int pathGroup = -1);

    // *** Direct drawing ***

    Q_CANVASPAINTER_EXPORT void fillRect(float x, float y, float width, float height);
    inline void fillRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void clearRect(float x, float y, float width, float height);
    inline void clearRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void strokeRect(float x, float y, float width, float height);
    inline void strokeRect(const QRectF &rect);

    // *** Shadows ***

    Q_CANVASPAINTER_EXPORT void drawBoxShadow(const QCanvasBoxShadow &shadow);

    // *** Images ***

    Q_CANVASPAINTER_EXPORT void drawImage(const QCanvasImage &image, float x, float y);
    Q_CANVASPAINTER_EXPORT void drawImage(const QCanvasImage &image, float x, float y, float width, float height);
    inline void drawImage(const QCanvasImage &image, const QRectF &destinationRect);
    Q_CANVASPAINTER_EXPORT void drawImage(const QCanvasImage &image, const QRectF &sourceRect, const QRectF &destinationRect);

    // *** Text ***

    Q_CANVASPAINTER_EXPORT void setFont(const QFont &font);
    Q_CANVASPAINTER_EXPORT void setTextAlign(QCanvasPainter::TextAlign align);
    Q_CANVASPAINTER_EXPORT void setTextBaseline(QCanvasPainter::TextBaseline baseline);
    Q_CANVASPAINTER_EXPORT void setTextDirection(QCanvasPainter::TextDirection direction);
    Q_CANVASPAINTER_EXPORT void setTextWrapMode(QCanvasPainter::WrapMode wrapMode);
    Q_CANVASPAINTER_EXPORT void setTextLineHeight(float height);
    Q_CANVASPAINTER_EXPORT void setTextAntialias(float antialias);

    Q_CANVASPAINTER_EXPORT void fillText(const QString &text, float x, float y, float maxWidth = -1);
    inline void fillText(const QString &text, QPointF point, float maxWidth = -1);
    Q_CANVASPAINTER_EXPORT void fillText(const QString &text, const QRectF &rect);

    Q_CANVASPAINTER_EXPORT QRectF textBoundingBox(const QString &text, float x, float y, float maxWidth = -1);
    inline QRectF textBoundingBox(const QString &text, QPointF point, float maxWidth = -1);
    Q_CANVASPAINTER_EXPORT QRectF textBoundingBox(const QString &text, const QRectF &rect);

    // *** Other ***

    Q_CANVASPAINTER_EXPORT void setAntialias(float antialias);
    Q_CANVASPAINTER_EXPORT float devicePixelRatio() const;
    Q_CANVASPAINTER_EXPORT void setRenderHint(RenderHint hint, bool on = true);
    Q_CANVASPAINTER_EXPORT void setRenderHints(RenderHints hints, bool on = true);
    Q_CANVASPAINTER_EXPORT RenderHints renderHints() const;

    // *** Static methods ***

    Q_CANVASPAINTER_EXPORT static float mmToPx(float mm);
    Q_CANVASPAINTER_EXPORT static float ptToPx(float pt);

    // *** Image & path cache handling ***

    Q_CANVASPAINTER_EXPORT QCanvasImage addImage(const QImage &image, QCanvasPainter::ImageFlags flags = {});
    Q_CANVASPAINTER_EXPORT QCanvasImage addImage(QRhiTexture *texture, QCanvasPainter::ImageFlags flags = {});
    Q_CANVASPAINTER_EXPORT QCanvasImage addImage(const QCanvasOffscreenCanvas &canvas, QCanvasPainter::ImageFlags flags = {});
    Q_CANVASPAINTER_EXPORT void removeImage(const QCanvasImage &image);
    Q_CANVASPAINTER_EXPORT void cleanupResources();
    Q_CANVASPAINTER_EXPORT qsizetype activeImageMemoryUsage() const;
    Q_CANVASPAINTER_EXPORT qsizetype activeImageCount() const;
    Q_CANVASPAINTER_EXPORT void removePathGroup(int pathGroup);

    Q_CANVASPAINTER_EXPORT QCanvasOffscreenCanvas createCanvas(QSize pixelSize, int sampleCount = 1, QCanvasOffscreenCanvas::Flags flags = {});
    Q_CANVASPAINTER_EXPORT void destroyCanvas(QCanvasOffscreenCanvas &canvas);
    Q_CANVASPAINTER_EXPORT void grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback);

private:
    Q_DISABLE_COPY(QCanvasPainter)
    Q_DECLARE_PRIVATE(QCanvasPainter)
    std::unique_ptr<QCanvasPainterPrivate> d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasPainter::ImageFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCanvasPainter::RenderHints)

inline void QCanvasPainter::translate(QPointF point)
{
    translate(float(point.x()), float(point.y()));
}

inline void QCanvasPainter::setClipRect(const QRectF &rect)
{
    setClipRect(float(rect.x()),
                float(rect.y()),
                float(rect.width()),
                float(rect.height()));
}

inline void QCanvasPainter::moveTo(QPointF point)
{
    moveTo(float(point.x()), float(point.y()));
}

inline void QCanvasPainter::lineTo(QPointF point)
{
    lineTo(float(point.x()), float(point.y()));
}

inline void QCanvasPainter::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(float(controlPoint1.x()),
                  float(controlPoint1.y()),
                  float(controlPoint2.x()),
                  float(controlPoint2.y()),
                  float(endPoint.x()),
                  float(endPoint.y()));
}

inline void QCanvasPainter::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(float(controlPoint.x()),
                     float(controlPoint.y()),
                     float(endPoint.x()),
                     float(endPoint.y()));
}

inline void QCanvasPainter::arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius)
{
    arcTo(float(controlPoint1.x()),
          float(controlPoint1.y()),
          float(controlPoint2.x()),
          float(controlPoint2.y()),
          radius);
}

inline void QCanvasPainter::arc(QPointF centerPoint, float radius, float a0, float a1, PathWinding direction, PathConnection connection)
{
    arc(float(centerPoint.x()),
        float(centerPoint.y()),
        radius, a0, a1, direction,
        connection);
}

inline void QCanvasPainter::rect(const QRectF &rect)
{
    this->rect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

inline void QCanvasPainter::roundRect(const QRectF &rect, float radius)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radius);
}

inline void QCanvasPainter::roundRect(const QRectF &rect, float radiusTopLeft, float radiusTopRight, float radiusBottomRight, float radiusBottomLeft)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
}

inline void QCanvasPainter::ellipse(QPointF centerPoint, float radiusX, float radiusY)
{
    ellipse(float(centerPoint.x()),
            float(centerPoint.y()),
            radiusX,
            radiusY);
}

inline void QCanvasPainter::ellipse(const QRectF &rect)
{
    ellipse(float(rect.x() + rect.width() * 0.5),
            float(rect.y() + rect.height() * 0.5),
            float(rect.width() * 0.5),
            float(rect.height() * 0.5));
}

inline void QCanvasPainter::circle(QPointF centerPoint, float radius)
{
    circle(float(centerPoint.x()),
           float(centerPoint.y()), radius);
}

inline void QCanvasPainter::fillRect(const QRectF &rect)
{
    fillRect(float(rect.x()),
             float(rect.y()),
             float(rect.width()),
             float(rect.height()));
}

inline void QCanvasPainter::clearRect(const QRectF &rect)
{
    clearRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()));
}

inline void QCanvasPainter::strokeRect(const QRectF &rect)
{
    strokeRect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

inline void QCanvasPainter::drawImage(const QCanvasImage &image, const QRectF &destinationRect)
{
    drawImage(image,
              float(destinationRect.x()),
              float(destinationRect.y()),
              float(destinationRect.width()),
              float(destinationRect.height()));
}

inline void QCanvasPainter::fillText(const QString &text, QPointF point, float maxWidth)
{
    fillText(text, float(point.x()), float(point.y()), maxWidth);
}

inline QRectF QCanvasPainter::textBoundingBox(const QString &text, QPointF point, float maxWidth)
{
    return textBoundingBox(text, float(point.x()), float(point.y()), maxWidth);
}

QT_END_NAMESPACE

#endif // QCANVASPAINTER_H
