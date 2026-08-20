// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "nodesmodel.h"

NodesModel::NodesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::rowsInserted, this, &NodesModel::rowCountChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &NodesModel::rowCountChanged);
    connect(this, &QAbstractListModel::modelReset, this, &NodesModel::rowCountChanged);
}

int NodesModel::rowCount(const QModelIndex &) const
{
    return m_nodesList.size();
}

QHash<int, QByteArray> NodesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Type] = "type";
    roles[Name] = "name";
    roles[X] = "x";
    roles[Y] = "y";
    roles[Width] = "width";
    roles[Height] = "height";
    roles[Selected] = "selected";
    roles[NodeId] = "nodeId";
    roles[Description] = "description";
    roles[Disabled] = "disabled";
    return roles;
}

QVariant NodesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_nodesList.size())
        return false;

    const auto &node = (m_nodesList)[index.row()];

    if (role == Type)
        return QVariant::fromValue(node.type);
    else if (role == Name)
        return QVariant::fromValue(node.name);
    else if (role == X)
        return QVariant::fromValue(node.x);
    else if (role == Y)
        return QVariant::fromValue(node.y);
    else if (role == Width)
        return QVariant::fromValue(node.width);
    else if (role == Height)
        return QVariant::fromValue(node.height);
    else if (role == Selected)
        return QVariant::fromValue(node.selected);
    else if (role == NodeId)
        return QVariant::fromValue(node.nodeId);
    else if (role == Description)
        return QVariant::fromValue(node.description);
    else if (role == Disabled)
        return QVariant::fromValue(node.disabled);

    return QVariant();
}

bool NodesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.row() >= m_nodesList.size())
        return false;

    auto &uniform = (m_nodesList)[index.row()];

    if (role == Disabled) {
        uniform.disabled = value.toBool();
    }

    emit dataChanged(index, index, {role});

    return true;
}

NodesModel::Node *NodesModel::getNodeWithId(int id)
{
    for (auto &node : m_nodesList) {
        if (node.nodeId == id)
            return &node;
    }
    return nullptr;
}

void NodesModel::setSelectedNode(Node *node)
{
    if (m_selectedNode == node)
        return;

    m_selectedNode = node;
    Q_EMIT selectedNodeChanged();
}
