// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef IMAGETESTINGRENDERER_H
#define IMAGETESTINGRENDERER_H


#include "qcanvaspainteritemrenderer.h"
#include "qcanvasimage.h"
#include "qcanvasoffscreencanvas.h"
#include <QImage>
#include <QList>
class ImageTestingRenderer : public QCanvasPainterItemRenderer
{

public:
    explicit ImageTestingRenderer();
    void synchronizeData(QCanvasPainterItem *item) override;
    void prePaint(QCanvasPainter *painter) override;
    void paint(QCanvasPainter *painter) override;

private:
    struct ImageData {
        QCanvasImage image;
        bool visible = true;
    };
    struct CanvasData {
        QCanvasOffscreenCanvas canvas;
        QCanvasImage image;
    };
    void generateImage();

    QList<ImageData> m_images;
    QList<CanvasData> m_canvases;
    bool m_showGradient = false;
    bool m_animateGradient = false;
    int m_newCanvasPending = 0;
    bool m_reregisterPending = false;
};

#endif // IMAGETESTINGRENDERER_H
