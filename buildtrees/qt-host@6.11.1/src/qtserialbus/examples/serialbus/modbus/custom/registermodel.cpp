// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "registermodel.h"

enum { NumColumn = 0, RegistersColumn = 1, ColumnCount = 2, RowCount = 10 };

RegisterModel::RegisterModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_registers(RowCount, 0u)
{
}

int RegisterModel::rowCount(const QModelIndex &/*parent*/) const
{
    return RowCount;
}

int RegisterModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ColumnCount;
}

QVariant RegisterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= RowCount || index.column() >= ColumnCount)
        return QVariant();

    Q_ASSERT(m_registers.count() == RowCount);

    if (index.column() == NumColumn && role == Qt::DisplayRole)
        return QString::number(index.row());

    if (index.column() == RegistersColumn && role == Qt::DisplayRole)
        return QString("0x%1").arg(QString::number(m_registers.at(index.row()), 16));

    return QVariant();
}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case NumColumn:
            return tr("Address");
        case RegistersColumn:
            return tr("Registers");
        default:
            break;
        }
    }
    return QVariant();
}

bool RegisterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() ||  index.row() >= RowCount || index.column() >= ColumnCount)
        return false;

    Q_ASSERT(m_registers.count() == RowCount);

    if (index.column() == RegistersColumn && role == Qt::EditRole) {
        bool result = false;
        quint16 newValue = value.toString().toUShort(&result, 16);
        if (result)
            m_registers[index.row()] = newValue;

        emit dataChanged(index, index);
        return result;
    }

    return false;
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= RowCount || index.column() >= ColumnCount)
        return QAbstractTableModel::flags(index);

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if ((index.row() < m_address) || (index.row() >= (m_address + m_number)))
        flags &= ~Qt::ItemIsEnabled;

    if (index.column() == RegistersColumn)
        return flags | Qt::ItemIsEditable;

    return flags;
}

void RegisterModel::setStartAddress(int address)
{
    m_address = address;
    emit updateViewport();
}

void RegisterModel::setNumberOfValues(const QString &number)
{
    m_number = number.toInt();
    emit updateViewport();
}
