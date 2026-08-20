// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBOXGRADIENT_H
#define QCANVASBOXGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

class QCanvasBoxGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasBoxGradient();
    Q_CANVASPAINTER_EXPORT QCanvasBoxGradient(float x, float y, float width, float height, float feather, float radius = 0.0f);
    Q_CANVASPAINTER_EXPORT QCanvasBoxGradient(const QRectF &rect, float feather, float radius = 0.0f);
    Q_CANVASPAINTER_EXPORT ~QCanvasBoxGradient();

    Q_CANVASPAINTER_EXPORT QRectF rect() const;
    Q_CANVASPAINTER_EXPORT void setRect(float x, float y, float width, float height);
    inline void setRect(const QRectF &rect);
    Q_CANVASPAINTER_EXPORT float feather() const;
    Q_CANVASPAINTER_EXPORT void setFeather(float feather);
    Q_CANVASPAINTER_EXPORT float radius() const;
    Q_CANVASPAINTER_EXPORT void setRadius(float radius);
};

inline void QCanvasBoxGradient::setRect(const QRectF &rect)
{
    setRect(float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()));
}

QT_END_NAMESPACE

#endif // QCANVASBOXGRADIENT_H
