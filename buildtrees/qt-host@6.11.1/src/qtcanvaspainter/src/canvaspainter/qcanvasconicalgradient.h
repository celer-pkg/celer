// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASCONICALGRADIENT_H
#define QCANVASCONICALGRADIENT_H

#include <QtCore/qglobal.h>
#include <QtCanvasPainter/qcanvasgradient.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

class QCanvasConicalGradient : public QCanvasGradient
{
public:
    Q_CANVASPAINTER_EXPORT QCanvasConicalGradient();
    Q_CANVASPAINTER_EXPORT QCanvasConicalGradient(float centerX, float centerY, float startAngle);
    Q_CANVASPAINTER_EXPORT QCanvasConicalGradient(QPointF center, float startAngle);
    Q_CANVASPAINTER_EXPORT ~QCanvasConicalGradient();

    Q_CANVASPAINTER_EXPORT QPointF centerPosition() const;
    Q_CANVASPAINTER_EXPORT void setCenterPosition(float x, float y);
    inline void setCenterPosition(QPointF center);
    Q_CANVASPAINTER_EXPORT float angle() const;
    Q_CANVASPAINTER_EXPORT void setAngle(float angle);
};

inline void QCanvasConicalGradient::setCenterPosition(QPointF center)
{
    setCenterPosition(float(center.x()), float(center.y()));
}

QT_END_NAMESPACE

#endif // QCANVASCONICALGRADIENT_H
