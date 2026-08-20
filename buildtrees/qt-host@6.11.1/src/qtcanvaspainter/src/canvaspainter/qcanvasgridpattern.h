// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASGRIDPATTERN_H
#define QCANVASGRIDPATTERN_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasbrush.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qsize.h>
#include <QtCore/qdebug.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QCanvasGridPattern : public QCanvasBrush
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern();
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern(const QRectF &rect,
                  const QColor &lineColor = QColorConstants::White,
                  const QColor &backgroundColor = QColorConstants::Black,
                  float lineWidth = 1.0f, float feather = 1.0f, float angle = 0.0f);
    Q_CANVASPAINTER_EXPORT QCanvasGridPattern(float x, float y, float width, float height,
                  const QColor &lineColor = QColorConstants::White,
                  const QColor &backgroundColor = QColorConstants::Black,
                  float lineWidth = 1.0f, float feather = 1.0f, float angle = 0.0f);
    Q_CANVASPAINTER_EXPORT ~QCanvasGridPattern();

    Q_CANVASPAINTER_EXPORT operator QVariant() const;

    Q_CANVASPAINTER_EXPORT QPointF startPosition() const;
    Q_CANVASPAINTER_EXPORT void setStartPosition(float x, float y);
    inline void setStartPosition(QPointF point);
    Q_CANVASPAINTER_EXPORT QSizeF cellSize() const;
    Q_CANVASPAINTER_EXPORT void setCellSize(float width, float height);
    inline void setCellSize(QSizeF size);
    Q_CANVASPAINTER_EXPORT float lineWidth() const;
    Q_CANVASPAINTER_EXPORT void setLineWidth(float width);
    Q_CANVASPAINTER_EXPORT float feather() const;
    Q_CANVASPAINTER_EXPORT void setFeather(float feather);
    Q_CANVASPAINTER_EXPORT float rotation() const;
    Q_CANVASPAINTER_EXPORT void setRotation(float rotation);
    Q_CANVASPAINTER_EXPORT QColor lineColor() const;
    Q_CANVASPAINTER_EXPORT void setLineColor(const QColor &color);
    Q_CANVASPAINTER_EXPORT QColor backgroundColor() const;
    Q_CANVASPAINTER_EXPORT void setBackgroundColor(const QColor &color);

private:
    friend Q_CANVASPAINTER_EXPORT bool comparesEqual(const QCanvasGridPattern &lhs, const QCanvasGridPattern &rhs) noexcept;
    Q_DECLARE_EQUALITY_COMPARABLE(QCanvasGridPattern)

    friend class QCanvasGridPatternPrivate;
#ifndef QT_NO_DATASTREAM
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGridPattern &);
    friend Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGridPattern &);
#endif
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGridPattern &);
#endif
};

#ifndef QT_NO_DATASTREAM
Q_CANVASPAINTER_EXPORT QDataStream &operator<<(QDataStream &, const QCanvasGridPattern &);
Q_CANVASPAINTER_EXPORT QDataStream &operator>>(QDataStream &, QCanvasGridPattern &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_CANVASPAINTER_EXPORT QDebug operator<<(QDebug, const QCanvasGridPattern &);
#endif

inline void QCanvasGridPattern::setStartPosition(QPointF point)
{
    setStartPosition(float(point.x()), float(point.y()));
}

inline void QCanvasGridPattern::setCellSize(QSizeF size)
{
    setCellSize(float(size.width()), float(size.height()));
}

QT_END_NAMESPACE

#endif // QCANVASGRIDPATTERN_H
