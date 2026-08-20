// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef NODEVIEW_H
#define NODEVIEW_H

#include <QtQuick/qquickitem.h>
#include <QList>
#include "nodesmodel.h"
#include "arrowsmodel.h"

class EffectManager;

class NodeView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(NodesModel *nodesModel READ nodesModel NOTIFY nodesModelChanged)
    Q_PROPERTY(ArrowsModel *arrowsModel READ arrowsModel NOTIFY arrowsModelChanged)
    Q_PROPERTY(QPointF activeArrowStartPoint READ activeArrowStartPoint NOTIFY activeArrowStartPointChanged)
    Q_PROPERTY(QPointF activeArrowEndPoint READ activeArrowEndPoint NOTIFY activeArrowEndPointChanged)
    Q_PROPERTY(bool activeArrowEnabled READ activeArrowEnabled NOTIFY activeArrowEnabledChanged)
    Q_PROPERTY(bool nodeGraphComplete READ nodeGraphComplete NOTIFY nodeGraphCompleteChanged)
    Q_PROPERTY(bool effectNodeSelected READ effectNodeSelected NOTIFY effectNodeSelectedChanged)
    Q_PROPERTY(bool mainNodeSelected READ mainNodeSelected NOTIFY mainNodeSelectedChanged)
    Q_PROPERTY(int selectedNodeId READ selectedNodeId NOTIFY selectedNodeIdChanged)
    Q_PROPERTY(QString selectedNodeName READ selectedNodeName WRITE setSelectedNodeName NOTIFY selectedNodeNameChanged)
    Q_PROPERTY(QString selectedNodeDescription READ selectedNodeDescription WRITE setSelectedNodeDescription NOTIFY selectedNodeDescriptionChanged)
    Q_PROPERTY(QString selectedNodeFragmentCode READ selectedNodeFragmentCode WRITE setSelectedNodeFragmentCode NOTIFY selectedNodeFragmentCodeChanged)
    Q_PROPERTY(QString selectedNodeVertexCode READ selectedNodeVertexCode WRITE setSelectedNodeVertexCode NOTIFY selectedNodeVertexCodeChanged)
    Q_PROPERTY(QString selectedNodeQmlCode READ selectedNodeQmlCode WRITE setSelectedNodeQmlCode NOTIFY selectedNodeQmlCodeChanged)
    Q_PROPERTY(QStringList codeSelectorModel READ codeSelectorModel NOTIFY codeSelectorModelChanged)
    Q_PROPERTY(int codeSelectorIndex READ codeSelectorIndex NOTIFY codeSelectorIndexChanged)
    QML_NAMED_ELEMENT(NodeViewItem)
public:
    explicit NodeView(QQuickItem *parent = nullptr);

    NodesModel *nodesModel() const;
    ArrowsModel *arrowsModel() const;

    QPointF activeArrowStartPoint() const;

    QPointF activeArrowEndPoint() const;

    bool activeArrowEnabled() const;

    QString selectedNodeName() const;
    void setSelectedNodeName(const QString &name);
    QString selectedNodeDescription() const;
    void setSelectedNodeDescription(const QString &description);
    QString selectedNodeFragmentCode() const;
    void setSelectedNodeFragmentCode(const QString &newSelectedNodeFragmentCode);
    QString selectedNodeVertexCode() const;
    void setSelectedNodeVertexCode(const QString &newSelectedNodeVertexCode);
    QString selectedNodeQmlCode() const;
    void setSelectedNodeQmlCode(const QString &newSelectedNodeQmlCode);

    bool nodeGraphComplete() const;
    bool effectNodeSelected() const;
    bool mainNodeSelected() const;
    int selectedNodeId() const;
    QStringList codeSelectorModel() const;
    int codeSelectorIndex() const;

    void setSelectedNode(NodesModel::Node *selectedNode);
    void updateCodeSelectorModel();

public Q_SLOTS:
    void updateActiveNodesList();
    void deleteSelectedNodes();
    void layoutNodes(bool distribute);
    void selectSingleNode(int nodeId);
    void selectMainNode();
    int getNodeIdWithName(const QString &name);
    void toggleNodeDisabled();

signals:
    void nodesModelChanged();
    void arrowsModelChanged();

    void activeArrowStartPointChanged();

    void activeArrowEndPointChanged();

    void activeArrowEnabledChanged();
    void activeNodesListChanged();

    void selectedNodeNameChanged();
    void selectedNodeDescriptionChanged();
    void selectedNodeFragmentCodeChanged();
    void selectedNodeVertexCodeChanged();
    void selectedNodeQmlCodeChanged();

    void nodeGraphCompleteChanged();
    void effectNodeSelectedChanged();
    void mainNodeSelectedChanged();
    void selectedNodeIdChanged();

    void codeSelectorModelChanged();
    void codeSelectorIndexChanged();

    // Other than changed singnals
    void doubleClickNode();
    void openDeleteNodeDialog();
    void openNodeContextMenu();


protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    friend class EffectManager;

    void updateArrowsPositions();
    void updateCodeSelectorIndex();
    int getUniqueNodeId();
    void initializeNode(NodesModel::Node &node);
    void initializeNodeSize(NodesModel::Node &node);
    QString getUniqueNodeName(const QString &origName, int counter = 0);

    bool m_mousePressed = false;
    NodesModel *m_nodesModel = nullptr;
    ArrowsModel *m_arrowsModel = nullptr;
    QPointF m_pressPosition;
    QPointF m_activeArrowStartPoint;
    QPointF m_activeArrowEndPoint;
    bool m_activeArrowEnabled;
    ArrowsModel::Arrow m_activeArrow;
    bool m_nodeGraphComplete = false;
    bool m_effectNodeSelected = false;
    bool m_mainNodeSelected = false;
    int m_selectedNodeId = -1;
    bool m_initialized = false;
    EffectManager *m_effectManager = nullptr;
    QList<NodesModel::Node *> m_activeNodesList;
    // Ids of nodes currently connected and not disabled
    QList<int> m_activeNodesIds;
    QStringList m_codeSelectorModel;
    int m_codeSelectorIndex = 0;
};

#endif // NODEVIEW_H
