// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef NODESMODEL_H
#define NODESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>
#include "uniformmodel.h"

class NodesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    enum NodeType {
        SourceNode = 0,
        DestinationNode,
        CustomNode
    };
    struct Node {
        int type = CustomNode;
        // Id of the model, must be unique!
        int nodeId = -1;
        float x = 0;
        float y = 0;
        float width = 0;
        float height = 0;
        QString name;
        bool selected = false;
        bool disabled = false;
        // These are helper positions when moving starts
        float startX = 0;
        float startY = 0;
        // This points to next connected node. -1 when not connected.
        int nextNodeId = -1;

        // Unforms from the node JSON
        // Note: These are not updated when the uniforms/properties change,
        // so don't use this to other things than checking JSON uniforms.
        QList<UniformModel::Uniform> jsonUniforms = {};
        QString fragmentCode = {};
        QString vertexCode = {};
        QString qmlCode = {};
        QString description = {};
        bool operator==(const Node& rhs) const noexcept
        {
           return this->nodeId == rhs.nodeId;
        }
        bool operator!=(const Node& rhs) const noexcept
        {
           return !operator==(rhs);
        }
    };

    enum NodesModelRoles {
        Type = Qt::UserRole + 1,
        Name,
        X,
        Y,
        Width,
        Height,
        Selected,
        NodeId,
        Description,
        Disabled
    };

    explicit NodesModel(QObject *parent = nullptr);

    // Override from QAbstractItemModel
    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    bool setData(const QModelIndex &index, const QVariant &value, int role) final;
    QHash<int, QByteArray> roleNames() const final;

    Node *getNodeWithId(int id);
    void setSelectedNode(Node *node);

signals:
    void selectedNodeChanged();
    void rowCountChanged();

private:
    friend class NodeView;
    friend class EffectManager;

    QList<Node> m_nodesList;
    Node m_emptyNode;
    Node *m_selectedNode = nullptr;
};

#endif // NODESMODEL_H
