// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "nodeview.h"
#include "effectmanager.h"

NodeView::NodeView(QQuickItem *parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    m_nodesModel = new NodesModel();
    m_arrowsModel = new ArrowsModel();

    // Create main and output nodes
    // These always exist
    NodesModel::Node n1{0, 0, 220, 50, 80, 80, "Main"};
    NodesModel::Node n2{1, 1, 220, 400, 80, 80, "Output"};
    initializeNodeSize(n1);
    initializeNodeSize(n2);
    n1.nextNodeId = 1;
    m_nodesModel->m_nodesList << n1;
    m_nodesModel->m_nodesList << n2;

    // Connect source & output by default
    ArrowsModel::Arrow a1{0, 0, 0, 0, 0, 1};
    m_arrowsModel->m_arrowsList << a1;

    updateArrowsPositions();
    m_activeArrow.startNodeId = -1;
    m_activeArrow.endNodeId = -1;

    connect(m_nodesModel, &QAbstractItemModel::modelReset,
            this, &NodeView::updateCodeSelectorModel);
}

void NodeView::mouseMoveEvent(QMouseEvent *event)
{
    // Nodes not moved with right button
    if (event->buttons() & Qt::RightButton)
        return;

    m_nodesModel->beginResetModel();
    QPointF movement = event->position() - m_pressPosition;
    for (auto &node : m_nodesModel->m_nodesList) {
        if (node.selected) {
            float newX = node.startX + movement.x();
            float newY = node.startY + movement.y();
            // Don't allow dragging outside the visible nodeview area.
            newX = std::max(newX, float(-node.width / 2.0f));
            newX = std::min(newX, float(width() - node.width / 2.0f));
            newY = std::max(newY, float(-node.height / 2.0f));
            newY = std::min(newY, float(height() - node.height / 2.0f));
            node.x = newX;
            node.y = newY;
        }
    }
    m_nodesModel->endResetModel();

    updateArrowsPositions();
}

void NodeView::mousePressEvent(QMouseEvent *event)
{
    m_mousePressed = true;

    setFocus(true);

    QPointF p = event->position();
    m_pressPosition = p;
    bool shiftPressed = (event->modifiers() & Qt::ShiftModifier);

    m_nodesModel->beginResetModel();
    m_arrowsModel->beginResetModel();

    // Reset the nodes starting positions
    for (auto &node : m_nodesModel->m_nodesList) {
        if (node.selected) {
            node.startX = node.x;
            node.startY = node.y;
        }
    }

    bool connectorPressed = false;
    if (!m_activeArrowEnabled) {
        m_activeArrow.startNodeId = -1;
        m_activeArrow.endNodeId = -1;
    }
    // Check node connectors first
    for (auto &node : m_nodesModel->m_nodesList) {
        float size = 20;
        // Start is bottom side and end top side of the node.
        QPointF startConnectorCenter = { node.x + node.width / 2, node.y + node.height};
        QPointF endConnectorCenter = { node.x + node.width / 2, node.y };
        QRectF startConnector(startConnectorCenter.x() - size / 2, startConnectorCenter.y() - size / 2, size, size);
        QRectF endConnector(endConnectorCenter.x() - size / 2, endConnectorCenter.y() - size / 2, size, size);
        if (startConnector.contains(p)) {
            m_activeArrowStartPoint = startConnectorCenter;
            emit activeArrowStartPointChanged();
            m_activeArrowEndPoint = startConnectorCenter;
            emit activeArrowEndPointChanged();
            connectorPressed = true;
            m_activeArrow.startNodeId = node.nodeId;
            // Remove possible already existing arrow
            for (auto &arrow : m_arrowsModel->m_arrowsList) {
                if (arrow.startNodeId == node.nodeId) {
                    // Update nextNode
                    node.nextNodeId = -1;
                    m_arrowsModel->m_arrowsList.removeAll(arrow);
                }
            }
        } else if (endConnector.contains(p)) {
            m_activeArrowStartPoint = endConnectorCenter;
            emit activeArrowStartPointChanged();
            m_activeArrowEndPoint = endConnectorCenter;
            emit activeArrowEndPointChanged();
            connectorPressed = true;
            m_activeArrow.endNodeId = node.nodeId;
            // Remove possible already existing arrow
            for (auto &arrow : m_arrowsModel->m_arrowsList) {
                if (arrow.endNodeId == node.nodeId) {
                    // Update nextNode
                    if (auto n = m_nodesModel->getNodeWithId(arrow.startNodeId))
                        n->nextNodeId = -1;
                    m_arrowsModel->m_arrowsList.removeAll(arrow);
                }
            }
        }
        // Don't allow connecting same node out & in together
        if (m_activeArrow.startNodeId != -1 && m_activeArrow.startNodeId == m_activeArrow.endNodeId) {
            m_activeArrow.startNodeId = -1;
            m_activeArrow.endNodeId = -1;
        }
    }

    // True when pressing a node that was already selected
    bool selectedNodeUnderMouse = false;
    NodesModel::Node newlySelectedNode;
    if (!connectorPressed) {
        for (auto &node : m_nodesModel->m_nodesList) {
            node.startX = node.x;
            node.startY = node.y;
            QRectF nodeArea(node.x, node.y, node.width, node.height);
            if (nodeArea.contains(p)) {
                if (node.selected)
                    selectedNodeUnderMouse = true;
                if (shiftPressed)
                    node.selected = !node.selected;
                else
                    node.selected = true;
                if (node.selected) {
                    newlySelectedNode = node;
                    break;
                }
            }
        }
    }

    // Deselected previous nodes
    if (!shiftPressed && !selectedNodeUnderMouse) {
        for (auto &node : m_nodesModel->m_nodesList) {
            if (newlySelectedNode != node)
                node.selected = false;
        }
    }

    // Check if both arrow ends are connected
    if (m_activeArrow.startNodeId >= 0 && m_activeArrow.endNodeId >= 0) {
        m_arrowsModel->m_arrowsList.append(m_activeArrow);
        connectorPressed = false;
        updateArrowsPositions();
        // Update nextNode
        auto n1 = m_nodesModel->getNodeWithId(m_activeArrow.startNodeId);
        if (n1)
            n1->nextNodeId = m_activeArrow.endNodeId;
    }

    if (m_activeArrowEnabled != connectorPressed) {
        m_activeArrowEnabled = connectorPressed;
        emit activeArrowEnabledChanged();
    }

    // Check first selected node
    NodesModel::Node *selectedNode = nullptr;
    for (auto &node : m_nodesModel->m_nodesList) {
        if (node.selected) {
            selectedNode = &node;
            break;
        }
    }
    setSelectedNode(selectedNode);

    m_nodesModel->endResetModel();
    m_arrowsModel->endResetModel();

    // Right button opens the context menu
    if (m_effectNodeSelected && event->button() == Qt::RightButton)
        Q_EMIT openNodeContextMenu();

    // Check if active (connected) nodes have changed
    updateActiveNodesList();
}

void NodeView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_mousePressed = false;
}

void NodeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // When double clicking custom nodes or main node
    if (m_selectedNodeId != -1 && m_selectedNodeId != 1)
        Q_EMIT doubleClickNode();
}

void NodeView::hoverMoveEvent(QHoverEvent *event)
{
    // If start/end is connected, other side follows mouse
    if (m_activeArrow.startNodeId >= 0) {
        m_activeArrowEndPoint = event->position();
        emit activeArrowEndPointChanged();
    } else if (m_activeArrow.endNodeId >= 0) {
        m_activeArrowStartPoint = event->position();
        emit activeArrowStartPointChanged();
    }

    // Pass event forward, to e.g. keep SplitView mouse cursor
    // changes functioning correctly.
    event->setAccepted(false);
}

void NodeView::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
}

void NodeView::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
}

// Selects the node with \a nodeId and deselects other nodes.
void NodeView::selectSingleNode(int nodeId)
{
    m_nodesModel->beginResetModel();
    // Check first selected node
    NodesModel::Node *selectedNode = nullptr;
    for (auto &node : m_nodesModel->m_nodesList) {
        if (node.nodeId == nodeId) {
            node.selected = true;
            selectedNode = &node;
        } else {
            node.selected = false;
        }
    }
    setSelectedNode(selectedNode);
    m_nodesModel->endResetModel();
}

// Selects the Main node and deselects other nodes.
void NodeView::selectMainNode()
{
    m_nodesModel->beginResetModel();
    NodesModel::Node *selectedNode = nullptr;
    for (auto &node : m_nodesModel->m_nodesList) {
        if (node.type == 0) {
            node.selected = true;
            selectedNode = &node;
        } else {
            node.selected = false;
        }
    }
    setSelectedNode(selectedNode);
    m_nodesModel->endResetModel();
}

void NodeView::setSelectedNode(NodesModel::Node *selectedNode)
{
    if (m_nodesModel->m_selectedNode == selectedNode)
        return;

    m_nodesModel->setSelectedNode(selectedNode);
    bool effectNodeSelected = (selectedNode != nullptr && selectedNode->type == 2);
    if (effectNodeSelected != m_effectNodeSelected) {
        m_effectNodeSelected = effectNodeSelected;
        Q_EMIT effectNodeSelectedChanged();
    }

    bool mainNodeSelected = (selectedNode != nullptr && selectedNode->type == 0);
    if (mainNodeSelected != m_mainNodeSelected) {
        m_mainNodeSelected = mainNodeSelected;
        Q_EMIT mainNodeSelectedChanged();
    }

    m_selectedNodeId = selectedNode ? selectedNode->nodeId : -1;
    Q_EMIT selectedNodeIdChanged();

    updateCodeSelectorIndex();

    Q_EMIT selectedNodeFragmentCodeChanged();
    Q_EMIT selectedNodeVertexCodeChanged();
    Q_EMIT selectedNodeQmlCodeChanged();
    Q_EMIT selectedNodeNameChanged();
    Q_EMIT selectedNodeDescriptionChanged();
}

NodesModel *NodeView::nodesModel() const
{
    return m_nodesModel;
}

ArrowsModel *NodeView::arrowsModel() const
{
    return m_arrowsModel;
}

QPointF NodeView::activeArrowStartPoint() const
{
    return m_activeArrowStartPoint;
}

QPointF NodeView::activeArrowEndPoint() const
{
    return m_activeArrowEndPoint;
}

bool NodeView::activeArrowEnabled() const
{
    return m_activeArrowEnabled;
}

void NodeView::updateArrowsPositions()
{
    m_arrowsModel->beginResetModel();
    for (auto &arrow : m_arrowsModel->m_arrowsList) {
        auto node = m_nodesModel->getNodeWithId(arrow.startNodeId);
        auto node2 = m_nodesModel->getNodeWithId(arrow.endNodeId);
        if (node && node2) {
            arrow.startX = node->x + node->width / 2;
            arrow.startY = node->y + node->height;
            arrow.endX = node2->x + node2->width / 2;
            arrow.endY = node2->y;
        }
    }
    m_arrowsModel->endResetModel();
}

void NodeView::updateActiveNodesList()
{
    QList<NodesModel::Node *> nodes;
    QList<int> nodesIds;
    auto node = m_nodesModel->getNodeWithId(0);
    if (!node)
        return;
    nodes << node;
    int nodeId = node ? node->nextNodeId : -1;
    while (nodeId > 0) {
        auto n = m_nodesModel->getNodeWithId(nodeId);
        if (n) {
            nodes << n;
            if (!n->disabled)
                nodesIds << n->nodeId;
            nodeId = n->nextNodeId;
        } else {
            break;
        }
    }
    m_activeNodesIds = nodesIds;

    // Update info of graph being complete (source -> output)
    bool nodeGraphComplete = false;
    if (!nodes.isEmpty() && nodes.last()->type == 1)
        nodeGraphComplete = true;

    if (nodeGraphComplete != m_nodeGraphComplete) {
        m_nodeGraphComplete = nodeGraphComplete;
        Q_EMIT nodeGraphCompleteChanged();
    }

    if (m_activeNodesList != nodes) {
        m_activeNodesList = nodes;
        // Don't emit until fully initialized to avoid shader errors
        if (m_initialized)
            Q_EMIT activeNodesListChanged();
    }
}

void NodeView::deleteSelectedNodes()
{
    QList<int> nodes;
    for (auto &node : m_nodesModel->m_nodesList) {
        if (node.selected && node.type == 2)
            nodes << node.nodeId;
    }
    m_effectManager->deleteEffectNodes(nodes);
}

QString NodeView::selectedNodeName() const {
    if (auto selectedNode = m_nodesModel->m_selectedNode)
        return selectedNode->name;
    return QString();
}

void NodeView::setSelectedNodeName(const QString &name) {
    if (auto selectedNode = m_nodesModel->m_selectedNode) {
        if (selectedNode->name == name)
            return;

        // Make sure the name is unique
        QString newName = getUniqueNodeName(name);
        m_nodesModel->beginResetModel();
        selectedNode->name = newName;
        m_nodesModel->endResetModel();
        Q_EMIT selectedNodeNameChanged();
    }
}

QString NodeView::selectedNodeDescription() const {
    if (auto selectedNode = m_nodesModel->m_selectedNode)
        return selectedNode->description;
    return QString();
}

void NodeView::setSelectedNodeDescription(const QString &description) {
    if (auto selectedNode = m_nodesModel->m_selectedNode) {
        if (selectedNode->description == description)
            return;

        m_nodesModel->beginResetModel();
        selectedNode->description = description;
        m_nodesModel->endResetModel();
        Q_EMIT selectedNodeDescriptionChanged();
    }
}

QString NodeView::selectedNodeFragmentCode() const
{
    if (auto selectedNode = m_nodesModel->m_selectedNode)
        return selectedNode->fragmentCode;
    return QString();
}

void NodeView::setSelectedNodeFragmentCode(const QString &newSelectedNodeFragmentCode)
{
    if (auto selectedNode = m_nodesModel->m_selectedNode) {
        if (selectedNode->fragmentCode == newSelectedNodeFragmentCode)
            return;
        selectedNode->fragmentCode = newSelectedNodeFragmentCode;
        Q_EMIT selectedNodeFragmentCodeChanged();
    }
}

QString NodeView::selectedNodeVertexCode() const
{
    if (auto selectedNode = m_nodesModel->m_selectedNode)
        return selectedNode->vertexCode;
    return QString();
}

void NodeView::setSelectedNodeVertexCode(const QString &newSelectedNodeVertexCode)
{
    if (auto selectedNode = m_nodesModel->m_selectedNode) {
        if (selectedNode->vertexCode == newSelectedNodeVertexCode)
            return;
        selectedNode->vertexCode = newSelectedNodeVertexCode;
        Q_EMIT selectedNodeVertexCodeChanged();
    }
}

QString NodeView::selectedNodeQmlCode() const
{
    if (auto selectedNode = m_nodesModel->m_selectedNode)
        return selectedNode->qmlCode;
    return QString();
}

void NodeView::setSelectedNodeQmlCode(const QString &newSelectedNodeQmlCode)
{
    if (auto selectedNode = m_nodesModel->m_selectedNode) {
        if (selectedNode->qmlCode == newSelectedNodeQmlCode)
            return;
        selectedNode->qmlCode = newSelectedNodeQmlCode;
        Q_EMIT selectedNodeQmlCodeChanged();
    }
}

bool NodeView::nodeGraphComplete() const
{
    return m_nodeGraphComplete;
}

bool NodeView::effectNodeSelected() const
{
    return m_effectNodeSelected;
}

bool NodeView::mainNodeSelected() const
{
    return m_mainNodeSelected;
}

int NodeView::selectedNodeId() const
{
    return m_selectedNodeId;
}

// Layout the nodes currently in active list
void NodeView::layoutNodes(bool distribute)
{
    if (m_activeNodesList.size() < 2)
        return;

    // Make sure that also non-connected nodes
    // are inside the visible nodeview area.
    if (!distribute) {
        for (auto &node : m_nodesModel->m_nodesList) {
            float newX = node.x;
            float newY = node.y;
            newX = std::max(newX, float(-node.width / 2.0f));
            newX = std::min(newX, float(width() - node.width / 2.0f));
            newY = std::max(newY, float(-node.height / 2.0f));
            newY = std::min(newY, float(height() - node.height / 2.0f));
            node.x = newX;
            node.y = newY;
        }
    }

    const int nodeCount = m_activeNodesList.size();
    const float areaHeight = height();
    const float sideMargin = areaHeight * 0.05f;
    const float marginY = areaHeight * 0.02f;
    const float areaWCenter = width() / 2.0f;
    float origFullHeight = m_activeNodesList.last()->y - m_activeNodesList.first()->y;
    origFullHeight = std::max(origFullHeight, 100.0f);
    const float firstY = sideMargin;
    const float lastY = areaHeight - firstY - m_activeNodesList.last()->height;
    const float newFullHeight = lastY - firstY;
    const float yScaling = newFullHeight / origFullHeight;
    int i = 0;
    m_nodesModel->beginResetModel();
    for (const auto &n : m_activeNodesList) {
        if (!distribute)
            n->x = areaWCenter - n->width / 2.0f;

        if (i == 0) {
            n->y = firstY;
        } else if (i == nodeCount - 1) {
            n->y = lastY;
        } else {
            if (distribute) {
                n->y = sideMargin + (float(i) / (nodeCount - 1)) * (areaHeight - sideMargin * 2.0f - n->height);
            } else {
                n->y = yScaling * n->y;
                // Make sure that middle nodes Y are between some limits
                if (auto previousNode = m_activeNodesList.at(i - 1))
                    n->y = std::max(n->y, previousNode->y + previousNode->height + marginY);
                n->y = std::max(n->y, firstY + marginY + m_activeNodesList.first()->height);
                n->y = std::min(n->y, lastY - marginY - n->height);
            }
        }
        i++;
    }

    m_nodesModel->endResetModel();
    updateArrowsPositions();
}

QStringList NodeView::codeSelectorModel() const
{
    return m_codeSelectorModel;
}

int NodeView::codeSelectorIndex() const
{
    return m_codeSelectorIndex;
}

int NodeView::getNodeIdWithName(const QString &name)
{
    for (const auto &node : m_nodesModel->m_nodesList) {
        if (node.name == name)
            return node.nodeId;
    }
    return -1;
}

// Update the code selector (combobox) model
void NodeView::updateCodeSelectorModel()
{
    QStringList codeSelectorModel;
    for (const auto &node : m_nodesModel->m_nodesList) {
        // Hide "Output" node as it currently doesn't contain code
        if (node.type == NodesModel::DestinationNode)
            continue;
        codeSelectorModel << node.name;
    }
    if (codeSelectorModel != m_codeSelectorModel) {
        m_codeSelectorModel = codeSelectorModel;
        Q_EMIT codeSelectorModelChanged();
    }
}

// Update the index of code selector (combobox) to match the currectly selected node
void NodeView::updateCodeSelectorIndex() {
    int index = -1;
    int i = 0;
    for (const auto &node : m_nodesModel->m_nodesList) {
        if (node.nodeId == m_selectedNodeId && node.type != NodesModel::DestinationNode) {
            index = i;
            break;
        }
        if (node.type != NodesModel::DestinationNode)
            i++;
    }
    index = std::min(index, int(m_codeSelectorModel.size() - 1));
    if (index != m_codeSelectorIndex) {
        m_codeSelectorIndex = index;
        Q_EMIT codeSelectorIndexChanged();
    }
}

void NodeView::toggleNodeDisabled()
{
    if (auto node = m_nodesModel->m_selectedNode) {
        m_nodesModel->beginResetModel();
        node->disabled = !node->disabled;
        m_nodesModel->endResetModel();
    }
    updateActiveNodesList();
}


// This will return node id, which is not already in use
int NodeView::getUniqueNodeId()
{
    if (!m_nodesModel)
        return 0;

    int id = 0;
    // Get biggest id + 1
    for (const auto &node : m_nodesModel->m_nodesList)
        id = std::max(id, node.nodeId);
    id++;

    return id;
}

void NodeView::initializeNode(NodesModel::Node &node)
{
    node.type = 2;
    node.x = 20;
    node.y = 60;
    node.nodeId = getUniqueNodeId();
    initializeNodeSize(node);
}

void NodeView::initializeNodeSize(NodesModel::Node &node)
{
    if (node.type == NodesModel::CustomNode) {
        node.width = 150;
        node.height = 50;
    } else {
        node.width = 80;
        node.height = 80;
    }
}

// Returns unique node name, so name appended with a number.
// "Custom" -> "Custom2" -> "Custom3" etc.
QString NodeView::getUniqueNodeName(const QString &origName, int counter)
{
    if (!m_nodesModel)
        return origName;

    // Initially try name as is, then first counter '2'.
    QString counterString = (counter == 0) ? "" : QString::number(counter + 1);
    QString name = origName + counterString;
    counter++;
    // Bail out just in case
    if (counter > 99)
        return name;
    for (const auto &node : m_nodesModel->m_nodesList) {
        if (node.name == name) {
            name = getUniqueNodeName(origName, counter);
        }
    }
    return name;
}
