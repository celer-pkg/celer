// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "piegraph.h"
#include <QRandomGenerator>

PieGraph::PieGraph(QObject *parent)
{
    Q_UNUSED(parent)
    //! [0]
    m_pieSeries = new QPieSeries;

    fillSliceInfo();

    for (int i = 1; i < 5; ++i) {
        QPieSlice *slice = new QPieSlice;
        slice->setValue(m_sliceInfo.value.at(QRandomGenerator::global()->bounded(0, 6)));
        slice->setLabel(m_sliceInfo.label.at(QRandomGenerator::global()->bounded(0, 6)));
        slice->setLabelColor(m_sliceInfo.color.at(QRandomGenerator::global()->bounded(0, 6)));
        m_pieSeries->append(slice);
    }
    m_pieSeries->setLabelsVisible(true);
    //! [0]
}

PieGraph::~PieGraph()
{
    delete m_pieSeries;
}

QPieSeries *PieGraph::pieSeries() const
{
    return m_pieSeries;
}

void PieGraph::appendSlice()
{
    //! [1]
    QPieSlice *slice = new QPieSlice;
    slice->setValue(m_sliceInfo.value.at(QRandomGenerator::global()->bounded(0, 6)));
    slice->setLabel(m_sliceInfo.label.at(QRandomGenerator::global()->bounded(0, 6)));
    slice->setLabelColor(m_sliceInfo.color.at(QRandomGenerator::global()->bounded(0, 6)));
    slice->setLabelVisible(true);
    m_pieSeries->append(slice);
    //! [1]
}

void PieGraph::removeSlice()
{
    m_pieSeries->remove(m_pieSeries->count() - 1);
}

void PieGraph::explodeSlices()
{
    for (auto slice: m_pieSeries->slices()) {
        if (slice->isExploded())
            slice->setExploded(false);
        else
            slice->setExploded(true);
    }
}

void PieGraph::clearSeries()
{
    m_pieSeries->clear();
}

void PieGraph::fillSliceInfo()
{
    m_sliceInfo.value = {
        10,
        15,
        20,
        25,
        30,
        35
    };
    m_sliceInfo.label = {
        "Strawberry",
        "Blueberry",
        "Raspberry",
        "Grape",
        "Banana",
        "Melon"
    };
    m_sliceInfo.color = {
        "white",
        "red",
        "green",
        "blue",
        "grey",
        "yellow"
    };
}

void PieGraph::onAddSlice()
{
    appendSlice();
}

void PieGraph::onRemoveSlice()
{
    removeSlice();
}

void PieGraph::onExplode()
{
    explodeSlices();
}

void PieGraph::onClearSeries()
{
    clearSeries();
}

void PieGraph::setPieSeries(QPieSeries *series)
{
    if (m_pieSeries != series) {
        m_pieSeries = series;
        emit pieSeriesChanged();
    }
}
