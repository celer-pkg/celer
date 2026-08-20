// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "barmodelmappermodel.h"

#include <QtCore/QList>
#include <QtCore/QRandomGenerator>
#include <QtCore/QRect>

BarModelMapperModel::BarModelMapperModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnCount = 6;
    m_rowCount = 12;

    for (int i = 0; i < m_rowCount; i++) {
        auto dataList = new QList<qreal>(m_columnCount);
        for (int k = 0; k < dataList->size(); k++) {
            if (k % 2 == 0)
                dataList->replace(k, i + QRandomGenerator::global()->bounded(10));
            else
                dataList->replace(k, QRandomGenerator::global()->bounded(10));
        }
        m_data.append(dataList);
    }
}

BarModelMapperModel::~BarModelMapperModel()
{
    qDeleteAll(m_data);
}

int BarModelMapperModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.count();
}

int BarModelMapperModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_columnCount;
}

QVariant BarModelMapperModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("201%1").arg(section);
    else
        return QString("%1").arg(section + 1);
}

QVariant BarModelMapperModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_data[index.row()]->at(index.column());
    } else if (role == Qt::EditRole) {
        return m_data[index.row()]->at(index.column());
    } else if (role == MyRoles::background) {
        for (const QRect &rect : m_mapping) {
            if (rect.contains(index.column(), index.row())) {
                auto color = QColor(m_mapping.key(rect));
                return color;
            }
        }
        return QColor(Qt::white);
    }
    return QVariant();
}

bool BarModelMapperModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()]->replace(index.column(), value.toDouble());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags BarModelMapperModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void BarModelMapperModel::addMapping(QColor color, int colStart, int rowStart, int colEnd, int rowEnd)
{
    const QString colorAsString = "#" + QString::number(color.rgb(), 16).right(6).toUpper();
    m_mapping.insert(colorAsString, QRect(colStart, rowStart, colEnd, rowEnd));
}

QHash<int, QByteArray> BarModelMapperModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[MyRoles::background] = "background";
    return roles;
}
