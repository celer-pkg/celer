// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "demoqcpainteritem.h"
#include "demoqcpainterrenderer.h"
#include "qcanvaslineargradient.h"
#include "qcanvasradialgradient.h"
#ifdef USE_STATIC_TEXTS
#include "qctext.h"
#endif
#include <algorithm>
#include <math.h>
#include <QVarLengthArray>
#include <QtMath>
#include <QFontDatabase>
#include <QImage>

DemoQCPainterRenderer::DemoQCPainterRenderer()
{
    // Setup colors
    m_colorWhite = QColor(255, 255, 255, 255);
    m_colorGray = QColor(180, 180, 180, 255);
    m_colorBlack = QColor(0, 0, 0, 255);
    m_color1 = QColor(180, 190, 40, 20);
    m_color2 = QColor(255, 255, 255, 150);
    m_color3 = QColor(255, 255, 255, 80);

    // Load custom font
    QStringList s = QFontDatabase::applicationFontFamilies(g_customFontId);
    if (!s.isEmpty())
        m_testFont = QFont(s.first());
    m_testFont.setPixelSize(30);
}

void DemoQCPainterRenderer::initializeResources(QCanvasPainter *painter)
{
    m_circleImage = painter->addImage(QImage(":/qml/images/circle.png"));
}

void DemoQCPainterRenderer::synchronizeData(QCanvasPainterItem *item)
{
    // Setting values here synchronized
    DemoQCPainterItem *realItem = static_cast<DemoQCPainterItem *>(item);
    if (realItem) {
        m_animationTime = realItem->animationTime();
        m_enabledTests = realItem->enabledTests();
        int newTestCount = realItem->testCount();
#ifdef USE_STATIC_TEXTS
        if (newTestCount != m_testCount) {
            // Initialize texts
            m_texts.resize(newTestCount * 50);
            for (int i = 0; i < m_texts.size(); i++) {
                auto &t = m_texts[i];
                t.setText(QString::number((i % 50) + 1));
            }
        }
#endif
        m_testCount = newTestCount;
        if (auto *p = painter()) {
            p->setRenderHint(QCanvasPainter::RenderHint::HighQualityStroking,
                             realItem->highQualityRendering());
        }

        if (!qFuzzyCompare(m_width, realItem->width()) ||
            !qFuzzyCompare(m_height, realItem->height())) {
            // Do here any initializations needed when the item size changes.
#ifdef USE_STATIC_FLOWER_PATH
            // Clear the flower path, so it will be recreated.
            m_flowerPath.clear();
#endif

            m_width = realItem->width();
            m_height = realItem->height();
        }
    }
}

void DemoQCPainterRenderer::paint(QCanvasPainter *painter)
{
    m_painter = painter;

    float w = width();
    float h = height();
    float s = std::min(w, h);
    float t = m_animationTime;

    // These painting commands are identical with both renderers
    for (int i = 0; i < m_testCount; i++) {
        //Paint ruler
        if (m_enabledTests & 1) {
            drawRuler(0, h * 0.02f, w, h * 0.05f, t, i);
        }
        // Paint circles
        if (m_enabledTests & 2) {
            float bigCircle = 50.0f + s * 0.5f;
            float smallCircle = 20.0f + s * 0.2f;
            drawGraphCircles(w / 2 - bigCircle / 2, h * 0.1f, bigCircle, bigCircle, 8, t * 2);
            drawGraphCircles(w * 0.05f, h * 0.55f, smallCircle, smallCircle, 6, t * 3);
            drawGraphCircles(w - smallCircle - w * 0.05f, h * 0.55f, smallCircle, smallCircle, 3, t);
        }
        // Paint lines
        if (m_enabledTests & 4) {
            drawGraphLine(0, h, w, -h, 4, t);
            drawGraphLine(0, h, w, -h * 0.8f, 6, t + 10);
            drawGraphLine(0, h, w, -h * 0.6f, 12, t / 2);
        }
        // Paint bars
        if (m_enabledTests & 8) {
            drawGraphBars(0, h, w, -h * 0.8f, 6, t * 3);
            drawGraphBars(0, h, w, -h * 0.4f, 10, t + 2);
            drawGraphBars(0, h, w, -h * 0.3f, 20, t * 2 + 2);
            drawGraphBars(0, h, w, -h * 0.2f, 40, t * 3 + 2);
        }
        // Paint icons
        int icons = 20;
        if (m_enabledTests & 16) {
            drawIcons(0, h * 0.2f, w, h * 0.2f, icons, t, i);
        }

        // Paint flower
        if (m_enabledTests & 32) {
            float flowerSize = 80.0f + s * 0.6f;
            drawFlower(w / 2 - flowerSize / 2, h - flowerSize, flowerSize, flowerSize, t);
        }

        // Increase animation time when m_testCount > 1
        t += 0.3f;
    }
}

void DemoQCPainterRenderer::drawGraphLine(float x, float y, float w, float h, int items, float t)
{
    QVarLengthArray<float, 1024> samples(items);
    QVarLengthArray<float, 1024> sx(items);
    QVarLengthArray<float, 1024> sy(items);
    float dx = w / (items - 1);
    float dotSize = 4.0f + w * 0.005;
    int i;

    // Generate positions
    for (i = 0; i < items; i++) {
        samples[i] = 0.5 + sinf((i + 1) * t * 0.2) * 0.1;
    }
    for (i = 0; i < items; i++) {
        sx[i] = x + i * dx;
        sy[i] = y + h * samples[i] * 0.8f;
    }

    // Draw graph background area
    QCanvasLinearGradient bg(x, y, x, y + h);
    bg.setStartColor(m_color1);
    bg.setEndColor(m_color2);
    m_painter->beginPath();
    m_painter->moveTo(sx[0], sy[0]);
    for (i = 1; i < items; i++)
        m_painter->bezierCurveTo(sx[i - 1] + dx * 0.5f, sy[i - 1], sx[i] - dx * 0.5f, sy[i], sx[i], sy[i]);
    m_painter->lineTo(x + w, y);
    m_painter->lineTo(x, y);
    m_painter->setFillStyle(bg);
    m_painter->fill();

    // Draw graph line
    m_painter->beginPath();
    m_painter->moveTo(sx[0], sy[0]);
    for (i = 1; i < items; i++)
        m_painter->bezierCurveTo(sx[i - 1] + dx * 0.5f, sy[i - 1], sx[i] - dx * 0.5f, sy[i], sx[i], sy[i]);
    m_painter->setStrokeStyle(m_colorGray);
    m_painter->setLineWidth(1.0f + dotSize * 0.2f);
    m_painter->stroke();

    // Draw dots
    m_painter->beginPath();
    for (i = 0; i < items; i++)
        m_painter->circle(sx[i], sy[i], dotSize * 0.8);
    m_painter->setLineWidth(dotSize * 0.2);
    m_painter->setStrokeStyle(m_colorBlack);
    m_painter->setFillStyle(m_colorWhite);
    m_painter->fill();
    m_painter->stroke();
}

void DemoQCPainterRenderer::drawGraphBars(float x, float y, float w, float h, int items, float t)
{
    QVarLengthArray<float, 1024> samples(items);
    QVarLengthArray<float, 1024> sx(items);
    QVarLengthArray<float, 1024> sy(items);
    float dx = w / items;
    float barWidth = dx * 0.8f;
    float margin = dx - barWidth;
    int i;

    // Generate positions
    for (i = 0; i < items; i++) {
        samples[i] = 0.5f + sinf(i * 0.1f + t) * 0.5f;
    }
    for (i = 0; i < items; i++) {
        sx[i] = x + i * dx + margin / 2;
        sy[i] = h * samples[i];
    }

    // Draw graph bars
    m_painter->beginPath();
    for (i = 0; i < items; i++) {
        m_painter->rect((int) sx[i] + 0.5f, (int) y + 1.5f, (int) barWidth, (int) sy[i]);
    }
    int lineWidth = 1.0f;
    m_painter->setLineWidth(lineWidth);
    m_painter->setLineJoin(QCanvasPainter::LineJoin::Miter);
    m_painter->setFillStyle(m_color3);
    m_painter->setStrokeStyle(m_colorBlack);
    m_painter->fill();
    m_painter->stroke();
}

void DemoQCPainterRenderer::drawGraphCircles(float x, float y, float w, float h, int items, float t)
{
    float barWidth = 0.3f * w / items;
    float lineMargin = 0.2f * barWidth;
    float showAnimationProgress = 0.1f + 0.4f * sinf(t * 0.8f) + 0.5f;
    float lineWidth = barWidth * showAnimationProgress;

    float cx = x + w / 2;
    float cy = y + h / 2;
    float radius1 = w / 2 - lineWidth;

    m_painter->setLineWidth(lineWidth);
    m_painter->setLineJoin(QCanvasPainter::LineJoin::Round);
    m_painter->setLineCap(QCanvasPainter::LineCap::Round);

    // Draw cicle backgrounds
    float r = radius1;
    QColor c_background(215, 215, 215, 50);
    m_painter->setStrokeStyle(c_background);
    for (int i = 0; i < items; i++) {
        m_painter->beginPath();
        m_painter->circle(cx, cy, r);
        m_painter->stroke();
        r -= (lineWidth + lineMargin);
    }

    // Draw circle bars
    r = radius1;
    const float a1 = -(float) M_PI / 2;
    for (int i = 0; i < items; i++) {
        float a0 = -M_PI / 2 + 2 * M_PI * (((float) items - i) / items) * showAnimationProgress;
        m_painter->beginPath();
        m_painter->arc(cx, cy, r, a0, a1, QCanvasPainter::PathWinding::CounterClockWise);
        float s = (float) i / items;
        QColor c(200 - 150 * s, 200 - 50 * s, 100 + 50 * s, 255 * showAnimationProgress);
        m_painter->setStrokeStyle(c);
        m_painter->stroke();
        r -= (lineWidth + lineMargin);
    }
}

void DemoQCPainterRenderer::drawIcons(float x, float y, float w, float h, int items, float t, int index)
{
    float s = std::min(width(), height());
    float size = 16.0f + s * 0.05f;
    float fontSize = size * 0.5f;
    m_testFont.setPixelSize(fontSize);
    m_painter->setFont(m_testFont);
    m_painter->setFillStyle("#FFFFFF");
    m_painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    m_painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    for (int i = 0; i < items; i++) {
        float xp = x + (w - size) / items * i;
        float yp = y + h * 0.5f + h * sinf((i + 1) * t * 0.1) * 0.5f;
        const QRectF rect(xp, yp, size, size);
        m_painter->drawImage(m_circleImage, rect);
#ifdef USE_STATIC_TEXTS
        int ti = index * items + i;
        auto &text = m_texts[ti];
        text.setRect(rect);
        m_painter->fillText(text);
#else
        // Alternative with cacheIndex, so QCText.
        // int cacheIndex = index * items + i;
        // m_painter->fillText(QString::number(i + 1), rect, cacheIndex);
        m_painter->fillText(QString::number(i + 1), rect);
#endif
    }
}

void DemoQCPainterRenderer::drawRuler(float x, float y, float w, float h, float t, int index)
{
    float posX = x + w * 0.05f;
    double space = w * 0.03f + sinf(t) * w * 0.02f;
    m_painter->setTextAlign(QCanvasPainter::TextAlign::Center);
    m_painter->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    float fontSize = (10.0f + w * 0.01f);
    m_testFont.setPixelSize(fontSize);
    m_painter->setFont(m_testFont);
    m_painter->setStrokeStyle("#E0E0E0");
    m_painter->setFillStyle("#E0E0B0");
    m_painter->beginPath();
    int i = 0;
    while (posX < w) {
        m_painter->moveTo(posX, y);
        float height = h * 0.2;
        QPointF textPoint(posX, y + h);
        if (i % 10 == 0) {
            height = h * 0.5;
            m_painter->fillText(QString::number(i), textPoint);
        } else if (i % 5 == 0) {
            height = h * 0.3;
            if (space > w * 0.02) {
                m_painter->fillText(QString::number(i), textPoint);
            }
        }
        m_painter->lineTo(posX, y + height);
        posX += space;
        i++;
    }
    m_painter->setLineWidth(1.0f);
    m_painter->stroke();
}

float DemoQCPainterRenderer::_flowerPos(int i)
{
    const int items = 12;
    return (2 * M_PI) * (1 - (float) i / items) - M_PI / 2;
}

void DemoQCPainterRenderer::drawFlower(float x, float y, float w, float h, float t)
{
    const float cx = x + w / 2;
    const float cy = y + h / 2;
    const float leafSize = w / 2;

    QTransform transform;
    transform.translate(cx, cy);
    transform.rotate(sinf(t) * 20);
    transform.translate(-cx, -cy);

    m_painter->setStrokeStyle("#40000000");
    m_painter->setLineWidth(4);
    QCanvasRadialGradient gradient1(cx, cy, leafSize);
    QColor startColor((0.5 + sinf(t * 2) * 0.5) * 255, 0, (0.5 + sinf(t + M_PI) * 0.5) * 255);
    gradient1.setStartColor(startColor);
    gradient1.setEndColor("#ffffff");
    m_painter->setFillStyle(gradient1);

#ifdef USE_STATIC_FLOWER_PATH
    if (m_flowerPath.isEmpty()) {
        m_flowerPath.moveTo(cx, cy);
        const int items = 12;
        for (int i = 0; i < items; i += 2) {
            m_flowerPath.quadraticCurveTo(
                cx + cosf(_flowerPos(i)) * leafSize,
                cy + sinf(_flowerPos(i)) * leafSize,
                cx + cosf(_flowerPos(i + 1)) * leafSize,
                cy + sinf(_flowerPos(i + 1)) * leafSize);
            m_flowerPath.quadraticCurveTo(
                cx + cosf(_flowerPos(i + 2)) * leafSize,
                cy + sinf(_flowerPos(i + 2)) * leafSize,
                cx,
                cy);
        }
    }
    m_painter->setTransform(transform);
    m_painter->fill(m_flowerPath, 1);
    m_painter->stroke(m_flowerPath, 1);
    m_painter->resetTransform();
#else
    m_painter->setTransform(transform);
    m_painter->beginPath();
    m_painter->moveTo(cx, cy);
    const int items = 12;
    for (int i = 0; i < items; i += 2) {
        m_painter->quadraticCurveTo(
            cx + cosf(_flowerPos(i)) * leafSize,
            cy + sinf(_flowerPos(i)) * leafSize,
            cx + cosf(_flowerPos(i + 1)) * leafSize,
            cy + sinf(_flowerPos(i + 1)) * leafSize);
        m_painter->quadraticCurveTo(
            cx + cosf(_flowerPos(i + 2)) * leafSize,
            cy + sinf(_flowerPos(i + 2)) * leafSize,
            cx,
            cy);
    }
    m_painter->fill();
    m_painter->stroke();
#endif

    m_painter->setFillStyle("#ffffff");
    m_painter->beginPath();
    m_painter->circle(cx, cy, 0.1 * w);
    m_painter->fill();
}
