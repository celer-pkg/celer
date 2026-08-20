// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterseries.h"

ScatterSeries::ScatterSeries(QScatterSeries *series)
    : m_cpuUpdater(this)
{
    connect(&m_timer, &QTimer::timeout, this, &ScatterSeries::frameUpdate);
    m_timer.start(100);

    for (int i = 0; i < 5; ++i)
        append(QPointF(double(i), double(i)));
}

void ScatterSeries::frameUpdate()
{
    auto reading = m_cpuUpdater.update();

    replace(m_counter, m_counter, reading);
    m_counter++;

    emit update();

    if (m_counter == 5)
        m_counter = 0;
}
