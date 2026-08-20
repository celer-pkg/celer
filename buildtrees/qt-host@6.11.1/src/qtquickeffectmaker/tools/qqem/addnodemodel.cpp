// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "addnodemodel.h"
#include "nodesmodel.h"
#include "effectmanager.h"
#include <QDir>

bool operator==(const AddNodeModel::NodeData &a, const AddNodeModel::NodeData &b) noexcept
{
    return a.name == b.name;
}

AddNodeModel::AddNodeModel(QObject *effectManager)
    : QAbstractListModel(effectManager)
{
    m_effectManager = static_cast<EffectManager *>(effectManager);
    connect(this, &QAbstractListModel::modelReset, this, &AddNodeModel::rowCountChanged);
    updateNodesList();
}

int AddNodeModel::rowCount(const QModelIndex &) const
{
    return m_modelList.size();
}

QHash<int, QByteArray> AddNodeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Description] = "description";
    roles[File] = "file";
    roles[Group] = "group";
    roles[Properties] = "properties";
    roles[CanBeAdded] = "canBeAdded";
    roles[Show] = "show";
    roles[RequiredNodes] = "requires";
    return roles;
}

QVariant AddNodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_modelList.size())
        return false;

    const auto &node = (m_modelList)[index.row()];

    if (role == Name)
        return QVariant::fromValue(node.name);
    else if (role == Description)
        return QVariant::fromValue(node.description);
    else if (role == File)
        return QVariant::fromValue(node.file);
    else if (role == Group)
        return QVariant::fromValue(node.group);
    else if (role == Properties)
        return QVariant::fromValue(node.properties);
    else if (role == CanBeAdded)
        return QVariant::fromValue(node.canBeAdded);
    else if (role == Show)
        return QVariant::fromValue(node.show);
    else if (role == RequiredNodes)
        return QVariant::fromValue(node.requiredNodes.join(", "));

    return QVariant();
}

void AddNodeModel::loadNodesFromPath(const QString &path) {
    qInfo() << "Loading nodes from:" << path;
    QList<NodeData> nodes;

    QDir rootDirectory(path);
    QStringList dirList;
    dirList << path;
    // List subdirectories, in our preferred order
    QStringList subDirList = rootDirectory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
    for (auto &subDir : subDirList) {
        QString dirPath = (path + "/" + subDir);
        // Trick to get "common" nodes first in list
        if (subDir == "common")
            dirList.prepend(dirPath);
        else
            dirList.append(dirPath);
    }
    // Load nodes from all subdirectories
    for (const auto &dirPath : dirList) {
        QDir directory(dirPath);
        QStringList nodeList = directory.entryList(QStringList() << "*.qen" << "*.QEN", QDir::Files, QDir::Name);
        for (auto &filename : nodeList) {
            QString filePath = directory.path() + "/" + filename;
            auto node = m_effectManager->loadEffectNode(filePath);
            if (!node.name.isEmpty()) {
                NodeData data;
                data.file = filePath;
                data.name = node.name;
                if (!node.jsonUniforms.isEmpty()) {
                    for (const auto &u : node.jsonUniforms) {
                        NodeDataProperty property;
                        property.m_name = u.name;
                        if (u.type == UniformModel::Uniform::Type::Define)
                            property.m_type = QStringLiteral("define");
                        else
                            property.m_type = UniformModel::typeToProperty(u.type);
                        QVariant varProperty;
                        varProperty.setValue(property);
                        data.properties << varProperty;
                    }
                }
                data.description = node.description;
                data.group = directory.dirName();
                // Seek through code to get tags
                QStringList shaderCodeLines;
                shaderCodeLines += node.vertexCode.split('\n');
                shaderCodeLines += node.fragmentCode.split('\n');
                for (const auto &codeLine : shaderCodeLines) {
                    QString trimmedLine = codeLine.trimmed();
                    if (trimmedLine.startsWith("@requires")) {
                        // Get the required node, remove "@requires"
                        QString l = trimmedLine.sliced(9).trimmed();
                        QString nodeName = l.split(' ').first();
                        if (!nodeName.isEmpty() && !data.requiredNodes.contains(nodeName))
                            data.requiredNodes << nodeName;
                    }
                }
                if (!m_modelList.contains(data))
                    nodes << data;
            }
        }
    }

    // Add all nodes into model
    for (const auto &node : nodes)
        m_modelList << node;
}

void AddNodeModel::updateCanBeAdded(const QStringList &propertyNames)
{
    beginResetModel();
    // Check which nodes contain overlapping properties
    for (auto &nodeData : m_modelList) {
        for (const auto &variant : nodeData.properties) {
            NodeDataProperty property = variant.value<NodeDataProperty>();
            if (propertyNames.contains(property.m_name)) {
                nodeData.canBeAdded = false;
            } else {
                nodeData.canBeAdded = true;
            }
        }
    }
    endResetModel();
}

void AddNodeModel::updateShowHide(const QString &groupName, bool show)
{
    int i = 0;
    for (auto &nodeData : m_modelList) {
        if (nodeData.group == groupName) {
            nodeData.show = show;
            Q_EMIT dataChanged(QAbstractItemModel::createIndex(0, 0),
                               QAbstractItemModel::createIndex(i, 0));
        }
        i++;
    }
}

void AddNodeModel::updateNodesList()
{
    beginResetModel();

    m_modelList.clear();

    QString defaultNodePath = m_effectManager->settings()->defaultResourcePath() + "/defaultnodes";
    loadNodesFromPath(defaultNodePath);

    auto customNodesPaths = m_effectManager->settings()->customNodesPaths();
    for (const auto &nodesPath : std::as_const(customNodesPaths))
         loadNodesFromPath(nodesPath);

    static QString envCustomNodePath = qEnvironmentVariable("QQEM_CUSTOM_NODES_PATH");
    if (!envCustomNodePath.isEmpty())
        loadNodesFromPath(envCustomNodePath);

    endResetModel();
}
