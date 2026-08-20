// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include <QRandomGenerator>
#include <QTouchEvent>

MainWindow::MainWindow(QRhi::Implementation api)
    : PainterWindow(api)
{
    setTitle(QStringLiteral("Qt Compact Health"));
    m_ecgGraph.m_mainWindow = this;
    setFillColor(m_theme.background1());

    m_views[W1].title = QStringLiteral("Pulse (BPM)");
    m_views[W2].title = QStringLiteral("SpO2 (%)");
    m_views[W3].title = QStringLiteral("Resp awRR (BRPM)");
    m_views[W4].title = QStringLiteral("Temperature (°C)");
    m_views[Center].fillBackground = false;
    m_views[B3].fillBackground = false;

    initializeTempData();
    initializeRespData();
    m_ecgGraph.initialize();

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::animateData);
    m_timer.start(500);
    m_elapsedTimer.start();

    //m_texts.resize(m_texts.capacity());
}

MainWindow::~MainWindow()
{
}

void MainWindow::initializeTempData()
{
    auto *r = QRandomGenerator::global();
    m_tempData.resize(m_tempDataCount);
    const float average = 37.0f;
    const float deltaUp = 5.0f;
    const float deltaDown = 2.0f;
    const float range = m_temperatureMax - m_temperatureMin;
    const float varying = 0.04f;
    float d = 0;
    float v = (average + r->generateDouble() * deltaUp - r->generateDouble() * deltaDown - m_temperatureMin) / range;
    for (int i = 0; i < m_tempDataCount; i++) {
        d = 0.5f * varying - varying * r->generateDouble();
        v += d;
        v = qBound(0.0f, v, 1.0f);
        m_tempData[i] = v;
    }
    // Set the current temperature to be the last measuremet in graph
    m_temperature = m_temperatureMin + m_tempData.last() * range;
    m_dirty.setFlag(Dirty::TemperatureBars);
}
void MainWindow::initializeRespData()
{
    m_respData.fill(-1.0f, m_respDataCount);
}

void MainWindow::updateRespData()
{
    // Currently this graph works so that a single point is
    // added at every update. This means that when the screen
    // update frequency is lower, the area graph "spikes" are
    // shorter and when higher they are longer. But timing is
    // correct, so "breath" takes same time. If output should
    // look similar with all frequencys then [n] amount of points
    // need to be added depending on elapsed time.

    // Update on every other frame
    static int updateTick = 0;
    updateTick++;
    if (updateTick % 2 != 0)
        return;
    // Update m_awrr based on current activity level
    const float awrrAS = 0.002f;
    m_awrr = (1.0f - awrrAS) * m_awrr + awrrAS * (12 + m_activity * 20);

    // Generate data that looks like resp CO2.
    static quint64 prevTickMs = 0;
    quint64 currentMs = m_elapsedTimer.elapsed();
    quint64 elapsedMs = currentMs - prevTickMs;
    prevTickMs = currentMs;
    float elapsedMin = float(elapsedMs) / 60000;
    static float freq = -0.2f;
    freq += m_awrr * elapsedMin;
    auto *r = QRandomGenerator::global();
    static float prevPhase = 0;
    float phase = std::fmod(freq + 0.2f, 1.0f);
    static float fade = 0.2f;
    static float boxy = 6.0f;
    if (phase < prevPhase) {
        // Starting a new phase, randomize
        fade = 0.3f * r->generateDouble();
        boxy = 1.5f + 6.0f * r->generateDouble();
    }
    prevPhase = phase;
    float resp;
    resp = std::sin(float(freq) * M_PI * 2);
    resp = std::tanh(boxy * resp);
    resp += fade * std::sin(float(freq - 0.2f) * M_PI * 2);
    resp = resp * 0.8f - 0.2f;
    resp = qBound(-1.0f, resp, 1.0f);
    m_respData.removeFirst();
    m_respData.append(resp);
}

//![paint-1]
void MainWindow::paint(QCanvasPainter *p)
{
    if (!m_initialized) {
        auto flags = QCanvasPainter::ImageFlag::GenerateMipmaps;
        m_b1ImageLight = p->addImage(QImage(":/images/icon_random_light.png"), flags);
        m_b1ImageDark = p->addImage(QImage(":/images/icon_random_dark.png"), flags);
        m_b2ImageLight = p->addImage(QImage(":/images/icon_theme_light.png"), flags);
        m_b2ImageDark = p->addImage(QImage(":/images/icon_theme_dark.png"), flags);
        m_b3ImageLight = p->addImage(QImage(":/images/icon_settings_light.png"), flags);
        m_b3ImageDark = p->addImage(QImage(":/images/icon_settings_dark.png"), flags);
        m_sImageLight = p->addImage(QImage(":/images/icon_run_light.png"), flags);
        m_sImageDark = p->addImage(QImage(":/images/icon_run_dark.png"), flags);
        m_initialized = true;
    }
//![paint-1]
    int textIndex = 0;

    m_painter = p;
    // Scalable px
    m_px = float(width()) / 960;
    const float viewRadius = 10 * m_px;
    // Paint view backgrounds
    static QCanvasPath viewBackgroundsPath;
    if (m_dirty.testFlag(Dirty::ViewBackgrounds)) {
        viewBackgroundsPath.clear();
        for (int i = 0; i < ViewsEnd; i++) {
            auto r = m_views[i].rect;
            if (!r.isEmpty() && m_views[i].fillBackground) {
                viewBackgroundsPath.roundRect(r.x(), r.y(),
                                              r.width(), r.height(),
                                              viewRadius);
            }
        }
    }
    p->setFillStyle(m_theme.background2());
    p->fill(viewBackgroundsPath, StaticPaths);

    if (!m_warningViews.isEmpty()) {
        p->beginPath();
        for (const auto &view : std::as_const(m_warningViews)) {
            auto r = m_views[view].rect;
            p->roundRect(r.x(), r.y(),
                         r.width(), r.height(),
                         viewRadius);
        }
        p->setStrokeStyle(m_theme.warning());
        p->setLineWidth(3.0f * m_px);
        p->stroke();
    }
    // Paint title backgrounds
    static QCanvasPath viewTitlesPath;
    static QVarLengthArray<QRectF> titleRects;
    if (m_dirty.testFlag(Dirty::ViewBackgrounds)) {
        viewTitlesPath.clear();
        titleRects.clear();
        const float titleHeight = m_margin * 4;
        const float titleWidth = m_views[W1].rect.width() - 4 * m_margin;
        const float titleRadius = 0.5 * titleHeight;
        for (int i = W1; i <= W4; i++) {
            auto r = m_views[i].rect;
            if (!r.isEmpty()) {
                QRectF rect(r.x() + 2 * m_margin, r.y() - 0.5 * titleHeight,
                            titleWidth, titleHeight);
                viewTitlesPath.roundRect(rect, titleRadius);
                titleRects << rect;
            }
        }
    }
    p->setFillStyle(m_theme.background1());
    p->setStrokeStyle(m_theme.foreground3());
    p->setLineWidth(3.0f * m_px);
    p->fill(viewTitlesPath, StaticPaths);
    p->stroke(viewTitlesPath, StaticPaths);

    // Widget titles
    p->setFont(m_titleFont);
    p->setFillStyle(m_theme.foreground2());
    p->setTextAlign(QCanvasPainter::TextAlign::Center);
    p->setTextBaseline(QCanvasPainter::TextBaseline::Middle);
    for (int i = 0; i < titleRects.size(); i++) {
        const auto r = titleRects.at(i);
        p->fillText(m_views[i].title, r);
    }

    // Widget texts
    p->setFont(m_bigFont);
    p->setFillStyle(m_theme.foreground1());
    auto r1 = m_views[W1].rect;
    p->fillText(QString::number(int(m_hr)), r1);

    auto r2 = m_views[W2].rect;
    p->fillText(QString::number(m_spo2, 'f', 1), r2);

    auto r3 = m_views[W3].rect;
    p->fillText(QString::number(m_awrr, 'f', 1), r3);

    auto r4 = m_views[W4].rect;
    p->fillText(QString::number(m_temperature, 'f', 1), r4);

    // Paint graphs grid
    const auto gridRect = m_views[Center].rect;
    paintGrid(gridRect.x(), gridRect.y(), gridRect.width(), gridRect.height());

    // Paint graphs
    const auto cRect = m_views[Center].rect;
    const float graphW = cRect.width() - 2 * m_margin;
    const float graphH = (cRect.height() + m_margin) * 0.25f - m_margin;
    const float graphX = cRect.x() + m_margin;
    float graphY = cRect.y() + m_margin;

    // Paint ECG & SpO2 graphs
    m_ecgGraph.updateData(m_hr);
    m_ecgGraph.paintWaves(p, graphX, graphY, graphW, graphH * 2);
    graphY += 2 * graphH + m_margin;

    // Paint resp area graph
    paintRespGraph(graphX, graphY, graphW, graphH);
    graphY += graphH + m_margin;

    // Paint temperature bars graph
    updateRespData();
    paintTempBars(graphX, graphY, graphW, graphH);

    // Settings icon
    QPointF b3C = m_views[B3].rect.center();
    QRectF b3Rect(b3C.x() - 0.5 * m_iconSize, b3C.y() - 0.5 * m_iconSize, m_iconSize, m_iconSize);
    auto &b3Icon = m_theme.isDark() ? m_b3ImageLight : m_b3ImageDark;
    p->setGlobalAlpha(0.3f);
    p->drawImage(b3Icon, b3Rect);
    p->setGlobalAlpha(1.0f);

    // Paint settings view
    if (m_showSettings) {
        const QRectF &sr = m_sliderRect;
        paintSlider(sr.x(), sr.y(), sr.width(), sr.height());
        // Scale images while keeping the aspect ratio.
        QPointF b1C = m_views[B1].rect.center();
        QRectF b1Rect(b1C.x() - 0.5 * m_iconSize, b1C.y() - 0.5 * m_iconSize, m_iconSize, m_iconSize);
        auto &b1Icon = m_theme.isDark() ? m_b1ImageLight : m_b1ImageDark;
        p->drawImage(b1Icon, b1Rect);
        QPointF b2C = m_views[B2].rect.center();
        QRectF b2Rect(b2C.x() - 0.5 * m_iconSize, b2C.y() - 0.5 * m_iconSize, m_iconSize, m_iconSize);
        auto &b2Icon = m_theme.isDark() ? m_b2ImageLight : m_b2ImageDark;
        p->drawImage(b2Icon, b2Rect);
    }
//![paint-2]
    // Highlight pressed button
    if (m_selectedButton) {
        p->beginPath();
        p->roundRect(m_views[m_selectedButton].rect, viewRadius);
        p->setLineWidth(2.0f * m_px);
        p->setStrokeStyle(m_theme.highlight());
        p->stroke();
    }
//![paint-2]

    // At this point nothing in backgrounds is dirty
    m_dirty.setFlag(Dirty::ViewBackgrounds, false);

    // Update in VSync
    requestUpdate();
}

void MainWindow::touchEvent(QTouchEvent *ev)
{
    const auto type = ev->type();
    QPointF pos;
    QPointF pressPos;
    if (ev->pointCount() > 0) {
        const auto point = ev->point(0);
        pos = point.position();
        pressPos = point.pressPosition();
    }

    if ((type == QTouchEvent::TouchBegin || type == QTouchEvent::TouchUpdate) && m_sliderRect.contains(pressPos)) {
        // Controlling slider
        m_activity = ((m_sliderRect.y() + m_sliderRect.height() - m_sliderMarginBottom - pos.y()) / (m_sliderRect.height() - m_sliderMarginTop - m_sliderMarginBottom));
        m_activity = qBound(0.0f, m_activity, 1.0f);
        m_dirty.setFlag(Dirty::ActivitySlider);
    } else if (type == QTouchEvent::TouchBegin) {
        // Pressing button
        for (int i = B1; i <= B3; i++) {
            if (m_views[i].rect.contains(pressPos))
                m_selectedButton = i;
        }
    } else if (type == QTouchEvent::TouchEnd) {
        // Releasing, see if button is being pressed
        if (m_selectedButton)
            buttonClicked(m_selectedButton);
        m_selectedButton = 0;
    } else if (type == QTouchEvent::TouchUpdate) {
        // Moving away from above button
        if (m_selectedButton && !m_views[m_selectedButton].rect.contains(pos))
            m_selectedButton = 0;
    } else if (type == QTouchEvent::TouchCancel) {
        m_selectedButton = 0;
    }

    ev->accept();
}

void MainWindow::buttonClicked(int buttonID) {
    if (buttonID == B1) {
        initializeTempData();
    } else if (buttonID == B2) {
        m_theme.switchTheme();
        m_ecgGraph.m_theme = &m_theme;
        setFillColor(m_theme.background1());
    } else if (buttonID == B3) {
        m_showSettings = !m_showSettings;
        updateViewSizes();
    }
}


void MainWindow::resizeEvent(QResizeEvent *)
{
    updateViewSizes();
}

void MainWindow::updateViewSizes()
{
    const float w = width();
    const float h = height();
    m_margin = w * 0.01f;
    const float leftColumnWidth = w * 0.22f - m_margin;
    const float buttonSize = w * 0.06f;
    const float rightColumnWidth = m_showSettings ? buttonSize + 2 * m_margin : m_margin;
    m_iconSize = buttonSize * 0.45f;
    const float centerColumnWidth = w - leftColumnWidth - rightColumnWidth - 2 * m_margin;
    const float wMargin = 3.25f * m_margin;
    const float wMargin2 = 3 * m_margin;
    const float wHeight = h * 0.25f - wMargin;
    m_sliderRect.setRect(w * 0.8, m_margin, w * 0.2, h * 0.5 - m_margin);
    m_sliderMarginTop = m_iconSize * 2.0;
    m_sliderMarginBottom = m_iconSize * 0.5;
    m_views[W1].rect = { m_margin, 3 * m_margin, leftColumnWidth, wHeight };
    m_views[W2].rect = { m_margin, m_views[W1].rect.bottom() + wMargin2, leftColumnWidth, wHeight };
    m_views[W3].rect = { m_margin, m_views[W2].rect.bottom() + wMargin2, leftColumnWidth, wHeight };
    m_views[W4].rect = { m_margin, m_views[W3].rect.bottom() + wMargin2, leftColumnWidth, wHeight };
    m_views[Center].rect = { m_views[W1].rect.right() + m_margin, m_margin, centerColumnWidth, h - 2 * m_margin };
    m_views[Slider].rect = { m_views[Center].rect.right() + m_margin, m_margin, buttonSize, h - 2 * buttonSize - 4 * m_margin };
    m_views[B1].rect = { m_views[Center].rect.right() + m_margin, m_views[Slider].rect.bottom() + m_margin, buttonSize, buttonSize };
    m_views[B2].rect = { m_views[Center].rect.right() + m_margin, m_views[B1].rect.bottom() + m_margin, buttonSize, buttonSize };
    m_views[B3].rect = { m_views[Center].rect.right() - buttonSize, m_margin, buttonSize, buttonSize};
    const float m2 = m_margin * 2;
    m_sliderRect = m_views[Slider].rect.adjusted(0, m2, 0, -m2);

    m_titleFont.setPixelSize(wMargin2 * 0.55f);
    m_bigFont.setPixelSize(w * 0.05f);
    // After resize, consider every path to be dirty
    m_dirty.setFlag(Dirty::All);
}

void MainWindow::paintTempBars(float x, float y, float w, float h)
{
    if (m_dirty.testFlag(Dirty::TemperatureBars)) {
        m_tempBarsPath.clear();
        const int rects = m_tempDataCount;
        float margin = (w / rects) * 0.2f;
        float barWidth = w / rects - margin;
        for (int i = 0; i < rects; i++) {
            float posX = x + i * (barWidth + margin);
            float posY = y + h;
            float dh = -h * m_tempData.at(i);
            m_tempBarsPath.rect(posX, posY, barWidth, dh);
        };
        m_dirty.setFlag(Dirty::TemperatureBars, false);
    }
    auto g1 = m_theme.gradient1();
    g1.setStartPosition(0, y);
    g1.setEndPosition(0, y+h);
    m_painter->setFillStyle(g1);
    m_painter->fill(m_tempBarsPath, GraphPaths);
}

void MainWindow::paintRespGraph(float x, float y, float w, float h)
{
    m_painter->beginPath();
    float firstPosY = y + 0.5 * h - m_respData.constFirst() * 0.5 * h;
    m_painter->moveTo(x, firstPosY);
    for (int i = 1; i < m_respDataCount; i++) {
        const float fi = float(i);
        const float resp = m_respData.at(i);
        float posX = x + (fi / m_respDataCount) * w;
        float posY = y + 0.5 * h - resp * 0.5 * h;
        m_painter->lineTo(posX, posY);
    }
    m_painter->lineTo(x + w, y + h);
    m_painter->lineTo(x, y + h);
    m_painter->closePath();
    auto g2 = m_theme.gradient2();
    g2.setStartPosition(0, y);
    g2.setEndPosition(0, y+h);
    m_painter->setFillStyle(g2);
    m_painter->fill();
}

void MainWindow::paintSlider(float x, float y, float w, float h)
{
    static QCanvasPath sliderBackground;
    static QCanvasPath sliderFill;
    static QCanvasPath sliderKnob;
    QRectF sRect(x + 0.5 * w - 0.5 * m_iconSize, y, m_iconSize, m_iconSize);
    auto &sIcon = m_theme.isDark() ? m_sImageLight : m_sImageDark;
    m_painter->drawImage(sIcon, sRect);
    if (m_dirty.testFlag(Dirty::ActivitySlider)) {
        sliderBackground.clear();
        const float barW = 4.0f;
        const float knobSize = 16.0f;
        const float radius = barW * 0.5f;
        const float cW = x + w * 0.5;
        const float margins = m_sliderMarginTop + m_sliderMarginBottom;
        const float barY = y + m_sliderMarginTop + (h - margins) * (1.0f - m_activity);
        sliderBackground.roundRect(cW - barW * 0.5,
                                   y + m_sliderMarginTop,
                                   barW,
                                   h - margins,
                                   radius);
        sliderFill.clear();
        sliderFill.roundRect(cW - barW * 0.5,
                             barY,
                             barW,
                             (h - margins) * m_activity,
                             radius);
        sliderKnob.clear();
        sliderKnob.circle(cW, barY, knobSize);
        m_dirty.setFlag(Dirty::ActivitySlider, false);
    }
    m_painter->setFillStyle(m_theme.foreground3());
    m_painter->fill(sliderBackground, ActivitySliderPaths);
    m_painter->setFillStyle(m_theme.highlight());
    m_painter->fill(sliderFill, ActivitySliderPaths);
    m_painter->setFillStyle(m_theme.foreground3());
    m_painter->setStrokeStyle(m_theme.foreground2());
    m_painter->setLineWidth(2.0f);
    m_painter->fill(sliderKnob, ActivitySliderPaths);
    m_painter->stroke(sliderKnob, ActivitySliderPaths);
}

void MainWindow::paintGrid(float x, float y, float w, float h)
{
    const int hLines = 40;
    const int vLines = 20;
    auto *p = m_painter;
    static QCanvasPath path;
    if (m_dirty.testFlag(Dirty::GraphGrid)) {
        path.clear();
        // Hozontal minor lines
        for (int i = 0; i<=hLines; i++) {
            float yPos = y + float(i) / hLines * h;
            path.moveTo(x, yPos);
            path.lineTo(x + w, yPos);
        }

        // Vertical minor lines
        p->setLineWidth(1.0);
        for (int i = 0; i<=vLines; i++) {
            float xPos = x + float(i)/vLines * w;
            path.moveTo(xPos, y);
            path.lineTo(xPos, y + h);
        }
        m_dirty.setFlag(Dirty::GraphGrid, false);
    }
    p->setStrokeStyle(m_theme.foreground3());
    p->setLineWidth(1.0f);
    p->stroke(path, StaticPaths);
}

void MainWindow::animateData()
{
    auto *r = QRandomGenerator::global();
    quint64 elapsed = m_elapsedTimer.elapsed();

    // HR adjustment speed
    const float hrAS = 0.02f;
    m_hr = (1.0f - hrAS) * m_hr + hrAS * (40 + 170 * m_activity);
    const float hrRand = 0.005f;
    m_hr *= (1.0f + hrRand - 2 * hrRand * r->generateDouble());

    const float spRand = 0.0005f;
    m_spo2 *= (1.0f + spRand - 2 * spRand * r->generateDouble());
    m_spo2 += spRand;
    m_spo2 = qBound(0.0f, m_spo2, 100.0f);

    const float tempAS = 0.02f;
    m_temperature = (1.0f - tempAS) * m_temperature + tempAS * (35.5f + 1.8f * (m_hr * 0.01f));
    const float tempRand = 0.0005f;
    m_temperature *= (1.0f + tempRand - 2 * tempRand * r->generateDouble());
    static quint64 tempTickMs = 0;
    if (elapsed > tempTickMs + m_temperatureLogMs) {
        // Add temperature into graph
        m_tempData.removeFirst();
        float t = (m_temperature - m_temperatureMin) / (m_temperatureMax - m_temperatureMin);
        t = qBound(0.0f, t, 1.0f);
        m_tempData.append(t);
        tempTickMs = elapsed;
        m_dirty.setFlag(Dirty::TemperatureBars);
    }
    updateWarnings();
}

void MainWindow::updateWarnings()
{
    if (m_temperature >= 38 || m_temperature < 36)
        m_warningViews << W4;
    else
        m_warningViews.remove(W4);

    if (m_hr > 180 || m_hr < 45)
        m_warningViews << W1;
    else
        m_warningViews.remove(W1);

    if (m_spo2 < 96.0f)
        m_warningViews << W2;
    else
        m_warningViews.remove(W2);
};
