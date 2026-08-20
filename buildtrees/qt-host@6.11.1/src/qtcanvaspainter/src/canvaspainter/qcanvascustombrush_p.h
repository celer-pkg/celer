// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASCUSTOMBRUSH_P_H
#define QCANVASCUSTOMBRUSH_P_H

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
#include "qcanvascustombrush.h"
#include <QtCore/qshareddata.h>
#include <qvectornd.h>
#include <rhi/qshader.h>

QT_BEGIN_NAMESPACE

class QCanvasCustomBrushPrivate : public QCanvasBrushPrivate
{
public:
    QCanvasCustomBrushPrivate() : QCanvasBrushPrivate(QCanvasBrush::BrushType::Custom) {}
    QCanvasCustomBrushPrivate(const QCanvasCustomBrushPrivate&) = default;
    QCanvasBrushPrivate *clone() override;
    QCPaint createPaint(QCanvasPainter *painter) const override;

    static QCanvasCustomBrushPrivate *get(QCanvasCustomBrush *brush)
    { return static_cast<QCanvasCustomBrushPrivate*>(brush->baseData.get()); }
    static const QCanvasCustomBrushPrivate *get(const QCanvasCustomBrush *brush)
    { return static_cast<QCanvasCustomBrushPrivate*>(brush->baseData.get()); }

    struct CommonUniforms {
        // Total size 112 + 112 = 224 bytes.
        // Built-in variables size is 112 bytes
        float scissorMat[12]; // matrices are actually 3 vec4s
        float scissorExt[2];
        float scissorScale[2];
        float alphaMult;
        float strokeThr;
        float fontAlphaMin;
        float fontAlphaMax;
        float colorEffects[4];
        int texType;
        int type;
        float globalAlpha;
        int unusedInt;
        // Custom input size is 112 bytes.
        float iTime;
        int alphaIsRed;
        // Take into use when needed
        int unused1;
        float unused3;
        QVector4D data[4];
        // Take these into use when needed
        float unused2[8];
    };

    QShader fragmentShader;
    QShader vertexShader;
    bool timeRunning = false;
    float time = 0;
    QVector4D data[4];
};

QT_END_NAMESPACE

#endif // QCANVASCUSTOMBRUSH_P_H
