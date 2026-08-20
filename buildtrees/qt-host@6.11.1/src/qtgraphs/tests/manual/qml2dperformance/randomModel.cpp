// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "randomModel.h"
#include <qabstractitemmodel.h>
#include <qnamespace.h>
#include <QRandomGenerator>

RandomModel::RandomModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnCount = 2;
    m_rowCount = 0;
}

int RandomModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data[m_currentCache].count();
}

void RandomModel::setRowCount(int rowCount)
{
    if (m_rowCount == rowCount)
        return;

    m_rowCount = rowCount;
    emit rowCountChanged(rowCount);
    generateCaches();
}

int RandomModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_columnCount;
}

QVariant RandomModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation != Qt::Horizontal)
        return section == 0 ? "X" : "Y";

    return QVariant();
}

QVariant RandomModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto data = m_data[m_currentCache];
    switch (role) {
    case Qt::DisplayRole:
        return data[index.row()].at(index.column());
    case XRole:
        return data[index.row()].at(0);
    case YRole:
        return data[index.row()].at(1);
    default:
        return QVariant();
    }
}

bool RandomModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        for (auto data : m_data)
            data[index.row()].replace(index.column(), value.toReal());
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags RandomModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> RandomModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    roles[Roles::XRole] = "xData";
    roles[Roles::YRole] = "yData";

    return roles;
}

void RandomModel::nextCache()
{

    beginResetModel();
    m_currentCache = (m_currentCache + 1) % 60;
    endResetModel();
}

void RandomModel::generateCaches()
{
    beginResetModel();
    m_data.clear();
    m_data.resize(m_cacheCount);
    for (int i = 0; i < m_cacheCount; i++) {
        QList<QList<qreal>> &array = m_data[i];
        array.reserve(m_rowCount);
        for (int j = 0; j < m_rowCount; j++) {
            qreal y = float(QRandomGenerator::global()->bounded(100)) / (100.0);

            auto dataList = QList<qreal>{qreal(j), y};

            array.append(dataList);
        }
    }
    endResetModel();
}

void RandomModel::clearData()
{
    for (auto data : m_data) {
        for (auto l : data)
            l.clear();
        data.clear();
    }
    m_data.clear();
}
