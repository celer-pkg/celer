// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "custombarseries.h"
#include <QRandomGenerator>

// Data contains 3 sets
const int SET_COUNT = 3;

CustomBarSeries::CustomBarSeries(QBarSeries *parent)
    : QBarSeries{ parent }
{

    // Initialize bar sets
    for (int i = 0; i < SET_COUNT; i++) {
        m_sets.append(new QBarSet(this));
        m_sets[i]->append({0, 0});
        append(m_sets[i]);
    }

    updateData();

    // Update data with a timer
    connect(&m_updateTimer, &QTimer::timeout, this, &CustomBarSeries::updateData);
    m_updateTimer.start(2000);
}

void CustomBarSeries::updateData()
{
    const int maxValue = 30;
    for (int i = 0; i < SET_COUNT; i++) {
        m_sets[i]->replace(0, QRandomGenerator::global()->generateDouble() * maxValue);
        m_sets[i]->replace(1, QRandomGenerator::global()->generateDouble() * maxValue);
    }
}
