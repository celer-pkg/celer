// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DEMOQCPAINTERRENDERER_H
#define DEMOQCPAINTERRENDERER_H

#include "qcanvaspainteritemrenderer.h"
#ifdef USE_STATIC_TEXTS
#include "qctext.h"
#endif
#include "qcanvasimage.h"
#include "qcanvaspath.h"
#include <QColor>

extern int g_customFontId; // set in main.cpp

class DemoQCPainterRenderer : public QCanvasPainterItemRenderer
{

public:
    explicit DemoQCPainterRenderer();

    // Reimplement
    void initializeResources(QCanvasPainter *painter) override;
    void synchronizeData(QCanvasPainterItem *item) override;
    void paint(QCanvasPainter *painter) override;

private:
    void drawGraphLine(float x, float y, float w, float h, int items, float t);
    void drawGraphBars(float x, float y, float w, float h, int items, float t);
    void drawGraphCircles(float x, float y, float w, float h, int items, float t);
    void drawIcons(float x, float y, float w, float h, int items, float t, int index);
    void drawRuler(float x, float y, float w, float h, float t, int index);
    void drawFlower(float x, float y, float w, float h, float t);
    float _flowerPos(int i);

    QCanvasPainter *m_painter = nullptr;
    float m_animationTime = 0.0f;
    int m_enabledTests = 0;
    int m_testCount = 0;

    // Colors
    QColor m_colorWhite, m_colorGray, m_colorBlack,
    m_color1, m_color2, m_color3;

    QCanvasImage m_circleImage;
    QFont m_testFont;

    qreal m_width = 0;
    qreal m_height = 0;

#ifdef USE_STATIC_TEXTS
    // Texts are declared beforehand during initialization and cache the layout.
    QList<QCText> m_texts;
#endif

#define USE_STATIC_FLOWER_PATH
#ifdef USE_STATIC_FLOWER_PATH
    QCanvasPath m_flowerPath;
#endif

};

#endif // DEMOQCPAINTERRENDERER_H
