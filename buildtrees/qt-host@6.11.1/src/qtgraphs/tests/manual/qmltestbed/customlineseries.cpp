// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "customlineseries.h"
#include <QRandomGenerator>

const int POINT_COUNT = 1000;
const float MAX_X = 4.0f;

CustomLineSeries::CustomLineSeries(QLineSeries *parent)
    : QLineSeries{ parent }
{
    setColor(QColorConstants::Green);

    for (int i = 0; i < POINT_COUNT; i++)
        append(QPointF(i / (POINT_COUNT / MAX_X), 0));

    // Update data with a timer
    connect(&m_updateTimer, &QTimer::timeout, this, &CustomLineSeries::updateData);
    m_updateTimer.start(16);
}

void CustomLineSeries::updateData()
{
    for (int i = 0; i < m_scanCount; i++) {
        int index = m_scanIndex + i;
        while (index >= POINT_COUNT)
            index -= POINT_COUNT;

        QPointF p(index / (POINT_COUNT / MAX_X), m_amplitude * qSin(2 * M_PI * (m_frequency / POINT_COUNT) * index + m_phase));
        replace(index, p);
    }

    m_scanIndex += m_scanCount;
    if (m_scanIndex >= POINT_COUNT)
        m_scanIndex = 0;

    update();
}

qreal CustomLineSeries::frequency() const
{
    return m_frequency;
}

void CustomLineSeries::setFrequency(qreal newFrequency)
{
    if (qFuzzyCompare(m_frequency, newFrequency))
        return;
    m_frequency = newFrequency;
    emit frequencyChanged();
}

qreal CustomLineSeries::amplitude() const
{
    return m_amplitude;
}

void CustomLineSeries::setAmplitude(qreal newAmplitude)
{
    if (qFuzzyCompare(m_amplitude, newAmplitude))
        return;
    m_amplitude = newAmplitude;
    emit amplitudeChanged();
}

qreal CustomLineSeries::phase() const
{
    return m_phase;
}

void CustomLineSeries::setPhase(qreal newPhase)
{
    if (qFuzzyCompare(m_phase, newPhase))
        return;
    m_phase = newPhase;
    emit phaseChanged();
}
