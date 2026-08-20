// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "linesimulator.h"

LineSimulator::LineSimulator(QObject *parent) : DataSimulator(parent)
{
    connect(this, &DataSimulator::simulateDataCompleted, this, &LineSimulator::generatePoints);
}

void LineSimulator::addSeries(QSplineSeries *series)
{
    m_seriesList.append(series);
    if (hasData())
        generatePoints();
}

void LineSimulator::generateData()
{
    DataSimulator::generateData(1, 1);
}

void LineSimulator::generatePoints()
{
    if (m_seriesList.isEmpty())
        return;

    for (QSplineSeries *series : std::as_const(m_seriesList)) {
        series->clear();
        for (int i = 0; i < data().size(); ++i)
            series->append(i + 1, qAbs(data().at(i).toReal()));
    }
}
