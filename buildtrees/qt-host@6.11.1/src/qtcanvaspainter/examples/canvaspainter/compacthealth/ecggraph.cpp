// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "ecggraph.h"
#include "mainwindow.h"
#include <QColor>
#include <QRandomGenerator>
#include <QTimer>
#include <QtCanvasPainter/QCanvasPath>

ECGGraph::ECGGraph()
{
    // Empty space size in percentages of data
    m_clearAmount = m_dataLength * 0.04f;
}

void ECGGraph::initialize()
{
    m_waveData.fill(0, m_dataLength);
    m_spo2Data.fill(0, m_dataLength);
}

// Paint all waves
void ECGGraph::paintWaves(QCanvasPainter *painter, float x, float y, float w, float h)
{
    m_theme = &m_mainWindow->m_theme;
    m_painter = painter;
    m_lineWidth = m_mainWindow->m_px * 2.5f;
    // Update heartDataIndex and clearDataIndex
    m_heartDataIndex++;
    if (m_heartDataIndex >= m_dataLength)
        m_heartDataIndex = 0;
    m_clearDataIndex = (m_heartDataIndex + m_clearAmount) % m_dataLength;

    // Paint the graph lines
    const float graphHeight = h * 0.5f;
    const float graphWidth = w;
    float posX = x;
    float posY = y;
    paintWave(posX, posY, graphWidth, graphHeight);
    posY += graphHeight;
    paintSpo2Wave(posX, posY, graphWidth, graphHeight);

    if (m_beatDataIndex != -1) {
        // Beat is progressing
        m_beatDataIndex++;
        if (m_beatDataIndex > 7)
            m_beatDataIndex = -1;
    }

    m_currentFrame++;
}

void ECGGraph::paintWave(float x, float y, float w, float h)
{
    auto p = m_painter;
    p->setStrokeStyle(m_theme->graph1());
    p->setLineWidth(m_lineWidth);
    const float xMove = float(w) / m_dataLength;
    const float hC = h * 0.5f;
    const auto &data = m_waveData;
    p->setLineJoin(QCanvasPainter::LineJoin::Bevel);
    p->beginPath();
    if (m_clearDataIndex > m_heartDataIndex) {
        // Stroke with 2 lines
        // Before the space
        const float firstY = y + hC - hC*data.constFirst();
        p->moveTo(x, firstY);
        for (int i = 1; i < m_heartDataIndex; i++) {
            const float d = data.at(i);
            p->lineTo(x + i * xMove, y + hC - hC * d);
        }

        // After the space
        const float firstY2 = y + hC - hC * data.at(m_clearDataIndex);
        p->moveTo(x + m_clearDataIndex * xMove, firstY2);
        for (int i = m_clearDataIndex + 1; i < m_dataLength; i++) {
            const float d = data.at(i);
            p->lineTo(x + i * xMove, y + hC - hC * d);
        }
    } else {
        // Stroke with 1 line
        const float firstY = y + hC + hC*data.at(m_clearDataIndex);
        p->moveTo(x + m_clearDataIndex * xMove, firstY);
        for (int i = m_clearDataIndex + 1; i < m_heartDataIndex; i++) {
            const float d = data.at(i);
            p->lineTo(x + i * xMove, y+hC - hC * d);
        }
    }
    p->stroke();
}

void ECGGraph::paintSpo2Wave(float x, float y, float w, float h)
{
    auto p = m_painter;
    p->setStrokeStyle(m_theme->graph2());
    p->setLineWidth(m_lineWidth);
    const float xMove = float(w) / m_dataLength;
    const float hC = h * 0.5f;
    const auto &data = m_spo2Data;
    p->setLineJoin(QCanvasPainter::LineJoin::Bevel);
    p->beginPath();
    if (m_clearDataIndex > m_heartDataIndex) {
        // Stroke with 2 lines
        // Before the space
        const float firstY = y + hC - hC*data.constFirst();
        p->moveTo(x, firstY);
        for (int i = 1; i < m_heartDataIndex; i++) {
            const float d = data.at(i);
            p->lineTo(x + i * xMove, y + hC - hC * d);
        }

        // After the space
        const float firstY2 = y + hC - hC * data.at(m_clearDataIndex);
        p->moveTo(x + m_clearDataIndex * xMove, firstY2);
        for (int i = m_clearDataIndex + 1; i < m_dataLength; i++) {
            const float d = data.at(i);
            p->lineTo(x + i * xMove, y + hC - hC * d);
        }
    } else {
        // Stroke with 1 line
        const float firstY = y + hC - hC*data.at(m_clearDataIndex);
        p->moveTo(x + m_clearDataIndex * xMove, firstY);
        for (int i = m_clearDataIndex + 1; i < m_heartDataIndex; i++) {
            const float d = data.at(i);
            p->lineTo(x + i * xMove, y+hC - hC*d);
        }
    }
    p->stroke();
}

// Update new data at index
void ECGGraph::updateData(float hr)
{
    //auto *rand = QRandomGenerator::global();
    m_pulseRate = 60 * (60.0f / hr);
    // Start a beat?
    if (m_currentFrame > m_beatFrame + m_pulseRate) {
        m_beatFrame = m_currentFrame;
        m_beatDataIndex = 0;
    }

    if (!m_attached) {
        fillEmptyData();
    } else if (m_beatDataIndex >= 0) {
        fillBeatData();
    } else {
        fillRandomData();
    }

    // Update SpO2
    // Beat state 0...1
    const float beatState = float(m_currentFrame - m_beatFrame) / (m_pulseRate + 1);
    // Generate something that looks like SpO2, synchronized with beats.
    float sp = std::sin(-1.5f + beatState * M_PI * 2);
    const float spPhaseVarying = 2.8f + 0.3f * (std::sin(0.123 * m_currentFrame) + std::cos(0.141 * m_currentFrame));
    sp += 0.6 * std::sin(spPhaseVarying + beatState * M_PI * 4);
    const float spSizeVarying = 0.05f * (std::sin(0.14 * m_currentFrame) + std::cos(0.24 * m_currentFrame));
    sp *= 0.4 + spSizeVarying;
    // Add random interference.
    //const float spRand = 0.02f;
    //sp += 0.5f * spRand - spRand * rand->generateDouble();
    m_spo2Data[m_heartDataIndex] = sp;
}

void ECGGraph::fillEmptyData() {
    m_waveData[m_heartDataIndex] = 0;
}

void ECGGraph::fillRandomData() {
    auto *rand = QRandomGenerator::global();
    if (m_defibrillator)
        m_waveData[m_heartDataIndex] = rand->generateDouble() * 1.2f - 0.6f;
    else
        m_waveData[m_heartDataIndex] = rand->generateDouble() * 0.05f - 0.025f;
}

// Create random data which looks like heart beat
void ECGGraph::fillBeatData() {
    auto *rand = QRandomGenerator::global();
    const int i = m_beatDataIndex;
    float value = 0;
    float r = float(rand->generateDouble());
    if (i == 0) value = r * 0.1f + 0.1f;
    else if (i == 1) value = r * 0.2f - 0.1f;
    else if (i == 2) value = r * 0.4f + 0.6f;
    else if (i == 3) value = r * 0.1f - 0.05f;
    else if (i == 4) value = r * 0.4f - 0.8f;
    else if (i == 5) value = r * 0.2f - 0.05f;
    else if (i == 6) value = r * 0.1f - 0.05f;
    else if (i == 7) value = r * 0.1f + 0.15f;
    m_waveData[m_heartDataIndex] = value;
}
