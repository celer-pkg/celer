// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "barseries.h"

#include <QtGraphs/QBarSet>

BarSeries::BarSeries(QObject *parent)
    : QBarSeries(parent)
    , m_cpuUpdater(this)
    , m_counter(0)
{
    connect(&m_timer, &QTimer::timeout, this, &BarSeries::frameUpdate);
    m_timer.start(100);

    m_barList = new QBarSet(this);

    for (int i = 0; i < 5; ++i)
        m_barList->append(double(0));
}

void BarSeries::frameUpdate()
{
    auto reading = m_cpuUpdater.update();

    m_barList->replace(m_counter++, reading);

    if (m_counter == 5)
        m_counter = 0;
}
