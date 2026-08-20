// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASBOXSHADOW_P_H
#define QCANVASBOXSHADOW_P_H

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

#include "engine/qcpainterengine_p.h"
#include "qcanvasboxshadow.h"
#include "qcanvasbrush_p.h"
#include "qcanvasimage.h"
#include <QtCore/qshareddata.h>
#include <QImage>

QT_BEGIN_NAMESPACE

class QCanvasBoxShadowPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasBoxShadowPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::BoxShadow) {}
    QCanvasBoxShadowPrivate(const QCanvasBoxShadowPrivate &) = default;
    QCanvasBrushPrivate *clone() override;

    static QCanvasBoxShadowPrivate *get(QCanvasBoxShadow *brush)
    { return static_cast<QCanvasBoxShadowPrivate*>(brush->baseData.get()); }
    static const QCanvasBoxShadowPrivate *get(const QCanvasBoxShadow *brush)
    { return static_cast<QCanvasBoxShadowPrivate*>(brush->baseData.get()); }

    QCPaint createPaint(QCanvasPainter *painter) const override;
    void createBoxShadow(float x, float y, float width, float height,
                         const QVector4D &radius,
                         float blur, const QColor &color) const;

    float clampedRadius(float radius, float width, float height) const;
    QColor clampedColor() const;

    QCPaint paint;
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 100.0f;
    float radius = 0.0f;
    float spread = 0.0f;
    float topLeftRadius = -1.0f;
    float topRightRadius = -1.0f;
    float bottomLeftRadius = -1.0f;
    float bottomRightRadius = -1.0f;
    float blur = 0.0f;
    QColor color = QColorConstants::Black;

    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCBOXSHADOW_P_H
