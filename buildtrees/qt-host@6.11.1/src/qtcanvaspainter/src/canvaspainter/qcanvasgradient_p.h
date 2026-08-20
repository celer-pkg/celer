// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASGRADIENT_P_H
#define QCANVASGRADIENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qcanvasbrush_p.h"
#include "qcanvasgradient.h"
#include "engine/qcpainterengineutils_p.h"
#include <QImage>
#include <QColor>

QT_BEGIN_NAMESPACE

class QCanvasGradientPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasGradientPrivate(QCanvasBrush::BrushType type);

    static QCanvasGradientPrivate *get(QCanvasGradient *brush)
    { return static_cast<QCanvasGradientPrivate*>(brush->baseData.get()); }
    static const QCanvasGradientPrivate *get(const QCanvasGradient *brush)
    { return static_cast<QCanvasGradientPrivate*>(brush->baseData.get()); }

    qint64 generateGradientKey() const;
    void updateGradientTexture(QCanvasPainter *painter);

    enum class DirtyFlag {
        Stops = 0x01,
        Values = 0x02,
        All = 0xFF
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    // Variables specific to gradient types
    union QCanvasGradientData {
        QCanvasGradientData() {}
        struct {
            float sx, sy, ex, ey;
        } linear;
        struct {
            float cx, cy, oRadius, iRadius;
        } radial;
        struct {
            float cx, cy, angle;
        } conical;
        struct {
            float x, y, width, height, feather, radius;
        } box;
    };
    QCanvasGradientStops gradientStops;
    QCanvasGradientData data;
    QCPaint paint;
    DirtyFlags dirty;
    int imageId;
};

QT_END_NAMESPACE

#endif // QCANVASGRADIENT_P_H
