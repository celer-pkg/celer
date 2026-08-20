// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testmodel.h"

TestModel::TestModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_columnCount(1)
    , m_rowCount(1)
{
    m_data.insert({0,0}, {{Qt::DisplayRole, "0,0"},});
}

void TestModel::appendColumn()
{
    beginInsertColumns(QModelIndex{}, m_columnCount, m_columnCount);
    ++m_columnCount;
    endInsertColumns();
}

void TestModel::appendRow()
{
    beginInsertRows(QModelIndex{}, m_rowCount, m_rowCount);
    ++m_rowCount;
    endInsertRows();
}

bool TestModel::setData(int row, int column, const QVariant &value, int role)
{
    return setData(index(row, column), value, role);
}

QVariant TestModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant{};

    auto it_table = m_data.find({index.row(), index.column()});
    if (it_table == m_data.end())
        return QVariant{};

    const CellData &cellData = it_table.value();
    auto it_cell = cellData.find(role);
    return (it_cell == cellData.end()) ? QVariant{} : it_cell.value();
}

bool TestModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    const Position pos{index.row(), index.column()};
    auto it_table = m_data.find(pos);
    if (it_table != m_data.end())
        it_table.value().insert(role, value);
    else
        m_data.insert(pos, {{role, value},});

    emit dataChanged(index, index);

    return true;
}

