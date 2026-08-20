// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASPATH_H
#define QCANVASPATH_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtCanvasPainter/qcanvaspainter.h>

QT_BEGIN_NAMESPACE

class QCanvasPathPrivate;
class QTransform;

class QCanvasPath
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasPath();
    Q_CANVASPAINTER_EXPORT explicit QCanvasPath(qsizetype commandsSize, qsizetype commandsDataSize = -1);
    Q_CANVASPAINTER_EXPORT QCanvasPath(const QCanvasPath &path);
    Q_CANVASPAINTER_EXPORT ~QCanvasPath();

    Q_CANVASPAINTER_EXPORT QCanvasPath &operator=(const QCanvasPath &path);
    QCanvasPath(QCanvasPath &&other) : d_ptr(std::exchange(other.d_ptr, nullptr)) { }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QCanvasPath)
    void swap(QCanvasPath &other) noexcept { qt_ptr_swap(d_ptr, other.d_ptr); }

    Q_CANVASPAINTER_EXPORT operator QVariant() const;

    // Path commands
    // These should match to path methods of QCanvasPainter for consistency.
    Q_CANVASPAINTER_EXPORT void closePath();
    Q_CANVASPAINTER_EXPORT void moveTo(float x, float y);
    inline void moveTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void lineTo(float x, float y);
    inline void lineTo(QPointF point);
    Q_CANVASPAINTER_EXPORT void bezierCurveTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    inline void bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void quadraticCurveTo(float cx, float cy, float x, float y);
    inline void quadraticCurveTo(QPointF controlPoint, QPointF endPoint);
    Q_CANVASPAINTER_EXPORT void arcTo(float c1x, float c1y, float c2x, float c2y, float radius);
    inline void arcTo(QPointF controlPoint1, QPointF controlPoint2, float radius);
    Q_CANVASPAINTER_EXPORT void arc(
        float centerX,
        float centerY,
        float radius,
        float a0,
        float a1,
        QCanvasPainter::PathWinding direction = QCanvasPainter::PathWinding::ClockWise,
        QCanvasPainter::PathConnection connection = QCanvasPainter::PathConnection::Connected);
    inline void arc(
        QPointF centerPoint,
        float radius,
        float a0,
        float a1,
        QCanvasPainter::PathWinding direction = QCanvasPainter::PathWinding::ClockWise,
        QCanvasPainter::PathConnection connection = QCanvasPainter::PathConnection::Connected);
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
    Q_CANVASPAINTER_EXPORT void ellipse(float x, float y, float radiusX, float radiusY);
    inline void ellipse(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT void circle(float x, float y, float radius);
    inline void circle(QPointF centerPoint, float radius);

    Q_CANVASPAINTER_EXPORT void setPathWinding(QCanvasPainter::PathWinding winding);
    Q_CANVASPAINTER_EXPORT void beginSolidSubPath();
    Q_CANVASPAINTER_EXPORT void beginHoleSubPath();

    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path, const QTransform &transform = QTransform());
    Q_CANVASPAINTER_EXPORT void addPath(const QCanvasPath &path, qsizetype start, qsizetype count, const QTransform &transform = QTransform());

    // Memory and size management
    Q_CANVASPAINTER_EXPORT bool isEmpty() const;
    Q_CANVASPAINTER_EXPORT void clear();
    Q_CANVASPAINTER_EXPORT void squeeze();
    Q_CANVASPAINTER_EXPORT qsizetype commandsSize() const;
    Q_CANVASPAINTER_EXPORT qsizetype commandsDataSize() const;
    Q_CANVASPAINTER_EXPORT qsizetype commandsCapacity() const;
    Q_CANVASPAINTER_EXPORT qsizetype commandsDataCapacity() const;
    Q_CANVASPAINTER_EXPORT void reserve(qsizetype commandsSize);
    Q_CANVASPAINTER_EXPORT void reserve(qsizetype commandsSize, qsizetype commandsDataSize);

    // Other
    Q_CANVASPAINTER_EXPORT QPointF currentPosition() const;
    Q_CANVASPAINTER_EXPORT QPointF positionAt(qsizetype index) const;
    [[nodiscard]] Q_CANVASPAINTER_EXPORT QCanvasPath sliced(qsizetype start, qsizetype count, const QTransform &transform = QTransform()) const;

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasPath &lhs, const QCanvasPath &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasPath)

    friend class QCPainterEngine;
    friend class QCPainterRhiRenderer;

    Q_DECLARE_PRIVATE(QCanvasPath)
    QCanvasPathPrivate *d_ptr;
};

inline void QCanvasPath::moveTo(QPointF point)
{
    moveTo(float(point.x()), float(point.y()));
}

inline void QCanvasPath::lineTo(QPointF point)
{
    lineTo(float(point.x()), float(point.y()));
}

inline void QCanvasPath::bezierCurveTo(QPointF controlPoint1, QPointF controlPoint2, QPointF endPoint)
{
    bezierCurveTo(float(controlPoint1.x()), float(controlPoint1.y()),
                  float(controlPoint2.x()), float(controlPoint2.y()),
                  float(endPoint.x()), float(endPoint.y()));
}

inline void QCanvasPath::quadraticCurveTo(QPointF controlPoint, QPointF endPoint)
{
    quadraticCurveTo(float(controlPoint.x()), float(controlPoint.y()),
                     float(endPoint.x()), float(endPoint.y()));
}

inline void QCanvasPath::arcTo(QPointF point1, QPointF point2, float radius)
{
    arcTo(float(point1.x()), float(point1.y()),
          float(point2.x()), float(point2.y()),
          radius);
}

inline void QCanvasPath::arc(
    QPointF centerPoint,
    float radius,
    float a0,
    float a1,
    QCanvasPainter::PathWinding direction,
    QCanvasPainter::PathConnection connection)
{
    arc(float(centerPoint.x()), float(centerPoint.y()),
        radius, a0, a1, direction, connection);
}

inline void QCanvasPath::rect(const QRectF &rect)
{
    this->rect(float(rect.x()),
               float(rect.y()),
               float(rect.width()),
               float(rect.height()));
}

inline void QCanvasPath::roundRect(const QRectF &rect, float radius)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radius);
}

inline void QCanvasPath::roundRect(
    const QRectF &rect,
    float radiusTopLeft,
    float radiusTopRight,
    float radiusBottomRight,
    float radiusBottomLeft)
{
    roundRect(float(rect.x()),
              float(rect.y()),
              float(rect.width()),
              float(rect.height()),
              radiusTopLeft, radiusTopRight,
              radiusBottomRight, radiusBottomLeft);
}

inline void QCanvasPath::ellipse(const QRectF &rect)
{
    ellipse(float(rect.x() + rect.width() * 0.5),
            float(rect.y() + rect.height() * 0.5),
            float(rect.width() * 0.5),
            float(rect.height() * 0.5));
}

inline void QCanvasPath::circle(QPointF centerPoint, float radius)
{
    circle(float(centerPoint.x()), float(centerPoint.y()), radius);
}

QT_END_NAMESPACE

#endif // QCANVASPATH_H
