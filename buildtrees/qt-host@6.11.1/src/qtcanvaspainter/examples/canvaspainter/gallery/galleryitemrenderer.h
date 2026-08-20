// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GALLERYITEMRENDERER_H
#define GALLERYITEMRENDERER_H

#include <QtCanvasPainter/qcanvaspainteritemrenderer.h>
#include <QtCanvasPainter/qcanvaspainter.h>
#include <QtCanvasPainter/qcanvasradialgradient.h>
#include <QtCanvasPainter/qcanvascustombrush.h>
#include <QtCanvasPainter/qcanvasimage.h>
#include <QtCanvasPainter/qcanvaspath.h>

#include <QQuickItem>

//![0]
class GalleryItemRenderer : public QCanvasPainterItemRenderer
{
public:
    explicit GalleryItemRenderer();
    ~GalleryItemRenderer();

    void initializeResources(QCanvasPainter *painter) override;
    void synchronizeData(QCanvasPainterItem *item) override;
    void paint(QCanvasPainter *painter) override;
//![0]

private:

    // View - Rectangles
    void drawRectsWithLinearGradient();
    void drawRectsWithRadialGradient();
    void drawRectsWithBoxGradient();
    void drawRectsWithConicalGradients();
    void drawRectsWithImagePattern();
    void drawRectsWithBrushStroke();

    // View - Paths
    void drawPaths();

    // View - Painter Paths
    void drawPainterPaths();

    // View - States and transforms
    void drawTransforms();

    // View - Antialiasing
    void drawAntialiasing();

    // View - Composite
    void drawCompositeItem1(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode);
    void drawCompositeItem2(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode);
    void drawCompositeItem3(float x, float y, float w, float h, QCanvasPainter::CompositeOperation mode);
    void drawCompositeModes();

    // View - Grid patterns
    void drawGridPatterns();

    // View - Shadows
    void drawRectangularShadows();

    // View - Custom Brushes
    void drawCustomBrushes();

    // View - Color Effects
    void drawButton(float x, float y, float w, float h, const QString &label);
    void drawColorEffects();

    // View - Texts
    void drawTextsFonts();
    void drawTextsBrushes();
    void drawTextsAlignments();
    void drawTextsWrapping();

    // View - Images
    void drawImages();

    void drawRect(float x, float y, float w, float h);

    //QCanvasPainter *m_painter;
    float m_animationTime = 0.0f;
    float m_animationSine = 0.0f;
    float m_animState = 0.0f;
    float m_viewAlpha = 1.0f;
    float m_viewSaturate = 1.0f;
    int m_viewIndex = 0;
    float m_previousWidth = 0;
    float m_previousHeight = 0;
    bool m_sizeChanged = false;
    float m_topMargin = 0;
    QCanvasRadialGradient m_radGrad;
    QCanvasImage m_testImage;
    QCanvasImage m_patternImage;
    QCanvasImage m_patternImage2;
    QCanvasImage m_patternImage3;
    QCanvasImage image3Gray;
    QCanvasImage image3Plain;
    QCanvasImage image3Nearest;
    QCanvasImage image3Mips;
    QCanvasImage image3NearestMips;
    QCanvasCustomBrush m_customBrush;
    QCanvasCustomBrush m_customBrush2;
    QCanvasCustomBrush m_customBrush3;
    QCanvasCustomBrush m_customBrush4;
    QCanvasPath m_path1;
    QCanvasPath m_path2;
    QCanvasPath m_path3;
    QCanvasPath m_path4;
    QCanvasPath m_pathGraphLine;
    QCanvasPath m_pathGraphArea;
};

#endif // GALLERYITEMRENDERER_H
