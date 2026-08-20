// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "piesimulator.h"
#include <QPieSlice>

PieSimulator::PieSimulator(QObject *parent) : DataSimulator(parent)
{
    connect(this, &DataSimulator::simulateDataCompleted, this, &PieSimulator::generatePieSlices);
}

void PieSimulator::addSeries(QPieSeries *series)
{
    m_seriesList.append(series);
    if (hasData())
        generatePieSlices();
}

void PieSimulator::generatePieSlices()
{
    if (m_seriesList.isEmpty())
        return;

    for (QPieSeries *series : std::as_const(m_seriesList)) {
        series->clear();
        for (auto data : data()) {
            QPieSlice *pieSlice = new QPieSlice();
            pieSlice->setValue(data.toDouble());
            series->append(pieSlice);
        }
    }
}
