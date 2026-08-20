// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "piemodelmappermodel.h"
#include <QtCore/qlist.h>
#include <QtCore/qrect.h>
#include <QtGui/qcolor.h>

PieModelMapperModel::PieModelMapperModel()
{
    m_rowCount = 6;
    m_columnCount = 2;
    m_data.resize(m_rowCount);
    m_data[0] = QList<QString>{"Tesla", "10.0"};
    m_data[1] = QList<QString>{"Volvo", "7.5"};
    m_data[2] = QList<QString>{"Hyundai", "8.5"};
    m_data[3] = QList<QString>{"Lada", "70.0"};
    m_data[4] = QList<QString>{"Citroen", "2.0"};
    m_data[5] = QList<QString>{"Toyota", "2.0"};
}

PieModelMapperModel::~PieModelMapperModel() {}

int PieModelMapperModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rowCount;
}

int PieModelMapperModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_columnCount;
}

QVariant PieModelMapperModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_data[index.row()].at(index.column());
    } else if (role == Qt::EditRole) {
        return m_data[index.row()].at(index.column());
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

QVariant PieModelMapperModel::headerData(int section, Qt::Orientation orientation, int role) const

{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Col%").arg(section);
    else if (orientation == Qt::Vertical)
        return QString("Row%").arg(section);
    return QVariant();
}

bool PieModelMapperModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_data[index.row()].replace(index.column(), value.toString());
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags PieModelMapperModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> PieModelMapperModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[MyRoles::background] = "background";
    return roles;
}

void PieModelMapperModel::addMapping(QColor color, int colStart, int rowStart, int colEnd, int rowEnd)
{
    const QString colorAsString = "#" + QString::number(color.rgb(), 16).right(6).toUpper();
    m_mapping.insert(colorAsString, QRect(colStart, rowStart, colEnd, rowEnd));
}
