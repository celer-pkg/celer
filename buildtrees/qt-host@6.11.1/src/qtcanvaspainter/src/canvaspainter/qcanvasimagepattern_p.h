// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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

#ifndef QCANVASIMAGEPATTERN_P_H
#define QCANVASIMAGEPATTERN_P_H

#include "engine/qcpainterengine_p.h"
#include "qcanvasbrush_p.h"
#include "qcanvasimage.h"
#include "qcanvasimagepattern.h"
#include <QtCore/qshareddata.h>
#include <QImage>

QT_BEGIN_NAMESPACE

class QCanvasImagePatternPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasImagePatternPrivate(const QCanvasImagePatternPrivate &) = default;

    QCanvasImagePatternPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::ImagePattern) {}
    QCanvasBrushPrivate *clone() override;

    QCPaint createPaint(QCanvasPainter *painter) const override;

    static QCanvasImagePatternPrivate *get(QCanvasImagePattern *brush)
    { return static_cast<QCanvasImagePatternPrivate*>(brush->baseData.get()); }
    static const QCanvasImagePatternPrivate *get(const QCanvasImagePattern *brush)
    { return static_cast<QCanvasImagePatternPrivate*>(brush->baseData.get()); }

    QCanvasImage image;
    QCPaint paint;
    QColor tintColor = QColorConstants::White;
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 100.0f;
    float angle = 0.0f;
    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCANVASIMAGEPATTERN_P_H
