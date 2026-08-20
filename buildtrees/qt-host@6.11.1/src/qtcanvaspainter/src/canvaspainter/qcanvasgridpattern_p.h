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

#ifndef QCANVASGRIDPATTERN_P_H
#define QCANVASGRIDPATTERN_P_H

#include "engine/qcpainterengine_p.h"
#include "qcanvasbrush_p.h"
#include <QtCore/qshareddata.h>
#include <QtGui/qcolor.h>
#include <qcanvasgridpattern.h>

QT_BEGIN_NAMESPACE

class QCanvasGridPatternPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasGridPatternPrivate(const QCanvasGridPatternPrivate &) = default;
    QCanvasGridPatternPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::GridPattern) {}
    QCanvasBrushPrivate *clone() override;
    QCPaint createPaint(QCanvasPainter *painter) const override;

    static QCanvasGridPatternPrivate *get(QCanvasGridPattern *brush)
    { return static_cast<QCanvasGridPatternPrivate*>(brush->baseData.get()); }
    static const QCanvasGridPatternPrivate *get(const QCanvasGridPattern *brush)
    { return static_cast<QCanvasGridPatternPrivate*>(brush->baseData.get()); }

    void createGridPattern() const;

    QCPaint paint;
    QColor lineColor = QColorConstants::White;
    QColor backgroundColor = QColorConstants::Black;
    float x = 0.0f;
    float y = 0.0f;
    float width = 10.0f;
    float height = 10.0f;
    float feather = 1.0f;
    float angle = 0.0f;
    float lineWidth = 1.0f;
    bool changed = true;
};

QT_END_NAMESPACE

#endif // QCANVASGRIDPATTERN_P_H
