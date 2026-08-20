// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "data3dsimulator.h"

#include "simulateddatamodel.h"

Data3DSimulator::Data3DSimulator(QObject *parent) : DataSimulator(parent)
{ }

int Data3DSimulator::numberOfRows() const
{
    return m_numberOfRows;
}

void Data3DSimulator::setNumberOfRows(int numberOfRows)
{
    if (m_numberOfRows == numberOfRows)
        return;

    m_numberOfRows = numberOfRows;
    emit numberOfRowsChanged(numberOfRows);
}

int Data3DSimulator::numberOfColumns() const
{
    return m_numberOfColumns;
}

void Data3DSimulator::setNumberOfColumns(int numberOfColumns)
{
    if (m_numberOfColumns == numberOfColumns)
        return;

    m_numberOfColumns = numberOfColumns;
    emit numberOfColumnsChanged(numberOfColumns);
}

SimulatedDataModel *Data3DSimulator::dataModel() const
{
    return m_dataModel;
}

void Data3DSimulator::setDataModel(SimulatedDataModel *dataModel)
{
    if (m_dataModel == dataModel)
        return;

    if (m_dataModel)
        disconnect(this, &DataSimulator::simulateDataCompleted, m_dataModel, &SimulatedDataModel::generateModelData);

    m_dataModel = dataModel;
    m_dataModel->setNumberOfColumns(m_numberOfColumns);
    m_dataModel->setNumberOfRows(m_numberOfRows);
    connect(this, &DataSimulator::simulateDataCompleted, m_dataModel, &SimulatedDataModel::generateModelData);
    emit dataModelChanged(dataModel);
}

void Data3DSimulator::generateData()
{
    DataSimulator::generateData(numberOfData(), m_numberOfColumns, m_numberOfRows);
}
