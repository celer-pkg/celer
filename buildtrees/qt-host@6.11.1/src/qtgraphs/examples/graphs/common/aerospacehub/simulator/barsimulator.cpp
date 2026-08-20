// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "barsimulator.h"
#include <QBarSet>

BarSimulator::BarSimulator(QObject *parent) : DataSimulator(parent)
{
    connect(this, &DataSimulator::simulateDataCompleted, this, &BarSimulator::generateBarSet);
}

int BarSimulator::numberOfCategory() const
{
    return m_numberOfCategory;
}

void BarSimulator::setNumberOfCategory(int numberOfCategory)
{
    if (m_numberOfCategory == numberOfCategory)
        return;

    m_numberOfCategory = numberOfCategory;
    emit numberOfCategoryChanged(numberOfCategory);
}

void BarSimulator::addSeries(QBarSeries *series)
{
    m_seriesList.append(series);
    if (hasData())
        generateBarSet();
}

void BarSimulator::generateData()
{
    DataSimulator::generateData(1, m_numberOfCategory);
}

void BarSimulator::generateBarSet()
{
    if (m_seriesList.isEmpty())
        return;

    for (QBarSeries *series : std::as_const(m_seriesList)) {
        series->clear();
        for (int i = 0; i < data().size() - m_numberOfCategory + 1; i += m_numberOfCategory) {
            QBarSet *barSet = new QBarSet();
            barSet->setValues(data().sliced(i, m_numberOfCategory));
            series->append(barSet);
        }
    }
}
