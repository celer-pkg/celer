// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASRADIALGRADIENT_H
#define QCANVASRADIALGRADIENT_H

#include <QtCanvasPainter/qtcanvaspainterglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCanvasRadialGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient();
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(float centerX, float centerY, float outerRadius, float innerRadius = 0.0f);
    Q_CANVASPAINTER_EXPORT QCanvasRadialGradient(QPointF center, float outerRadius, float innerRadius = 0.0f);
    Q_CANVASPAINTER_EXPORT ~QCanvasRadialGradient();

    Q_CANVASPAINTER_EXPORT QPointF centerPosition() const;
    Q_CANVASPAINTER_EXPORT void setCenterPosition(float x, float y);
    inline void setCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT float outerRadius() const;
    Q_CANVASPAINTER_EXPORT void setOuterRadius(float radius);
    Q_CANVASPAINTER_EXPORT float innerRadius() const;
    Q_CANVASPAINTER_EXPORT void setInnerRadius(float radius);
};

inline void QCanvasRadialGradient::setCenterPosition(QPointF center)
{
    setCenterPosition(float(center.x()), float(center.y()));
}

QT_END_NAMESPACE

#endif // QCANVASRADIALGRADIENT_H
