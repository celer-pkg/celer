// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTMODEL_H
#define TESTMODEL_H

#include <QtCore/qabstractitemmodel.h>

class TestModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TestModel(QObject *parent = nullptr);

    void appendColumn();
    void appendRow();

    bool setData(int row, int column, const QVariant &value, int role);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex & = QModelIndex{}) const override { return m_rowCount; }
    int columnCount(const QModelIndex & = QModelIndex{}) const override { return m_columnCount; }
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
    // use appendRow() instead
    using QAbstractTableModel::insertRow;
    using QAbstractTableModel::insertRows;
    // use appendColumn() instead
    using QAbstractTableModel::insertColumn;
    using QAbstractTableModel::insertColumns;

private:
    using CellData = QHash<int, QVariant>;
    using Position = std::pair<int, int>;
    QHash<Position, CellData> m_data;
    int m_columnCount = 0;
    int m_rowCount = 0;
};

#endif // TESTMODEL_H
