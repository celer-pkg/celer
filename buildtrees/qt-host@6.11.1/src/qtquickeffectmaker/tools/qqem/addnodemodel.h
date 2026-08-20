// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ADDNODEMODEL_H
#define ADDNODEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariant>

class EffectManager;

struct NodeDataProperty {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(QString type MEMBER m_type)
public:
    QString m_name;
    QString m_type;
};

class AddNodeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    struct NodeData {
        QString name;
        QString description;
        QString file;
        QString group;
        QStringList requiredNodes;
        // Properties as variant list to get access from QML
        QVariantList properties;
        // False when node would overlap with existing node in view
        bool canBeAdded = true;
        bool show = false;
    };

    enum AddNodeModelRoles {
        Name = Qt::UserRole + 1,
        Description,
        File,
        Group,
        Properties,
        CanBeAdded,
        Show,
        RequiredNodes
    };

    explicit AddNodeModel(QObject *effectManager);

    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    void updateCanBeAdded(const QStringList &propertyNames);
    void updateShowHide(const QString &groupName, bool show);
    void updateNodesList();

signals:
    void rowCountChanged();

private:
    void loadNodesFromPath(const QString &path);
    QList<NodeData> m_modelList;
    EffectManager *m_effectManager = nullptr;

};

bool operator==(const AddNodeModel::NodeData &a, const AddNodeModel::NodeData &b) noexcept;

Q_DECLARE_METATYPE(NodeDataProperty);

#endif // ADDNODEMODEL_H
