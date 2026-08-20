// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "arrowsmodel.h"

ArrowsModel::ArrowsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::rowsInserted, this, &ArrowsModel::rowCountChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &ArrowsModel::rowCountChanged);
    connect(this, &QAbstractListModel::modelReset, this, &ArrowsModel::rowCountChanged);
}

int ArrowsModel::rowCount(const QModelIndex &) const
{
    return m_arrowsList.size();
}

QHash<int, QByteArray> ArrowsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[StartX] = "startX";
    roles[StartY] = "startY";
    roles[EndX] = "endX";
    roles[EndY] = "endY";
    roles[StartNodeId] = "startNodeId";
    roles[EndNodeId] = "endNodeId";
    return roles;
}

QVariant ArrowsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_arrowsList.size())
        return false;

    const auto &node = (m_arrowsList)[index.row()];

    if (role == StartX)
        return QVariant::fromValue(node.startX);
    else if (role == StartY)
        return QVariant::fromValue(node.startY);
    else if (role == EndX)
        return QVariant::fromValue(node.endX);
    else if (role == EndY)
        return QVariant::fromValue(node.endY);
    else if (role == StartNodeId)
        return QVariant::fromValue(node.startNodeId);
    else if (role == EndNodeId)
        return QVariant::fromValue(node.endNodeId);

    return QVariant();
}
