// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCANVASRHIPAINTPAINTDRIVER_P_H
#define QCANVASRHIPAINTPAINTDRIVER_P_H

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

#include "qcanvasrhipaintdriver.h"
#include "engine/qcpainterrhirenderer_p.h"
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

class QCanvasRhiPaintDriverPrivate
{
public:
    static const QCanvasRhiPaintDriverPrivate *get(const QCanvasRhiPaintDriver *obj) { return obj->d; }
    static QCanvasRhiPaintDriverPrivate *get(QCanvasRhiPaintDriver *obj) { return obj->d; }

    QCanvasPainter *painter = nullptr;
    QCPainterRhiRenderer *renderer = nullptr;
    QRhiCommandBuffer *currentCb = nullptr;
    QRhiRenderTarget *currentRt = nullptr;
    float mainLogicalWidth = 0.0f;
    float mainLogicalHeight = 0.0f;
    float mainDpr = 1.0f;
    QColor mainFillColor;
    QCanvasOffscreenCanvas currentCanvas;
};

QT_END_NAMESPACE

#endif // QCANVASRHIPAINTPAINTDRIVER_P_H
