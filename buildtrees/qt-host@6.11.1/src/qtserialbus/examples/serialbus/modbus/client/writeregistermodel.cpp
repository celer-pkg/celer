// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "writeregistermodel.h"

using namespace Qt::StringLiterals;

enum { NumColumn = 0, CoilsColumn = 1, HoldingColumn = 2, ColumnCount = 3, RowCount = 10 };

WriteRegisterModel::WriteRegisterModel(QObject *parent)
    : QAbstractTableModel(parent),
      m_coils(RowCount, false), m_holdingRegisters(RowCount, 0u)
{
}

int WriteRegisterModel::rowCount(const QModelIndex & /* parent */) const
{
    return RowCount;
}

int WriteRegisterModel::columnCount(const QModelIndex & /* parent */) const
{
    return ColumnCount;
}

QVariant WriteRegisterModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const int column = index.column();
    if (!index.isValid() || row >= RowCount || column >= ColumnCount)
        return {};

    Q_ASSERT(m_coils.count() == RowCount);
    Q_ASSERT(m_holdingRegisters.count() == RowCount);

    if (column == NumColumn && role == Qt::DisplayRole)
        return QString::number(row);

    if (column == CoilsColumn && role == Qt::CheckStateRole) // coils
        return m_coils.at(row) ? Qt::Checked : Qt::Unchecked;

    if (column == HoldingColumn && role == Qt::DisplayRole) // holding registers
        return QString("0x%1").arg(QString::number(m_holdingRegisters.at(row), 16));

    return {};

}

QVariant WriteRegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case NumColumn:
            return u"#"_s;
        case CoilsColumn:
            return u"Coils  "_s;
        case HoldingColumn:
            return u"Holding Registers"_s;
        default:
            break;
        }
    }
    return {};
}

bool WriteRegisterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int column = index.column();
    if (!index.isValid() ||  row >= RowCount || column >= ColumnCount)
        return false;

    Q_ASSERT(m_coils.count() == RowCount);
    Q_ASSERT(m_holdingRegisters.count() == RowCount);

    if (column == CoilsColumn && role == Qt::CheckStateRole) { // coils
        auto s = static_cast<Qt::CheckState>(value.toUInt());
        s == Qt::Checked ? m_coils.setBit(row) : m_coils.clearBit(row);
        emit dataChanged(index, index);
        return true;
    }

    if (column == HoldingColumn && role == Qt::EditRole) { // holding registers
        bool result = false;
        quint16 newValue = value.toString().toUShort(&result, 16);
        if (result)
            m_holdingRegisters[row] = newValue;

        emit dataChanged(index, index);
        return result;
    }

    return false;
}

Qt::ItemFlags WriteRegisterModel::flags(const QModelIndex &index) const
{
    const int row = index.row();
    const int column = index.column();
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (!index.isValid() || row >= RowCount || column >= ColumnCount)
        return flags;

    if ((row < m_address) || (row >= (m_address + m_number)))
        flags &= ~Qt::ItemIsEnabled;

    if (column == CoilsColumn) // coils
        return flags | Qt::ItemIsUserCheckable;
    if (column == HoldingColumn) // holding registers
        return flags | Qt::ItemIsEditable;

    return flags;
}

void WriteRegisterModel::setStartAddress(int address)
{
    m_address = address;
    emit updateViewport();
}

void WriteRegisterModel::setNumberOfValues(const QString &number)
{
    m_number = number.toInt();
    emit updateViewport();
}
