// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Shapes
import QQEMLib 1.0

NodeViewItem {
    id: nodeViewItem

    property bool initialized: false
    readonly property color borderColor: Qt.lighter(mainView.backgroundColor1, 2.0)
    readonly property color connectorColor: "#80d0d040"

    function showAddNodeDialog(startNodeId, endNodeId) {
        addNodeDialog.reset();
        effectManager.updateAddNodeData();
        addNodeDialog.startNodeId = startNodeId;
        addNodeDialog.endNodeId = endNodeId;
        addNodeDialog.open();
    }

    function showRenameNodeDialog() {
        renameNodeDialog.open();
    }

    focus: true
    Component.onCompleted: {
        nodeViewItem.updateActiveNodesList();
        effectManager.initialize();
    }

    Timer {
        running: true
        interval: 1
        onTriggered: {
            // Do the layout when views have correct sizes
            layoutNodes(false);
            initialized = true;
        }
    }

    onDoubleClickNode: {
        mainToolbar.setDesignMode(false);
    }
    onOpenDeleteNodeDialog: {
        mainWindow.deleteNodeAction();
    }
    onOpenNodeContextMenu: {
        nodeContextMenu.popup();
    }

    RenameNodeDialog {
        id: renameNodeDialog
        x: mainWindow.width * 0.5 - width * 0.5
        y: mainWindow.height * 0.5 - height * 0.5
    }

    AddNodeDialog {
        id: addNodeDialog
        x: mainWindow.width * 0.5 - width * 0.5
        y: mainWindow.height * 0.5 - height * 0.5
    }

    Image {
        anchors.fill: parent
        source: "images/nodeview_background.jpg"
    }

    // Render nodes
    Repeater {
        model: nodeViewItem.nodesModel
        delegate: Rectangle {
            width: model.width
            height: model.height
            x: model.x
            y: model.y
            border.color: model.selected ? highlightColor : borderColor
            color: mainView.backgroundColor1
            opacity: model.disabled ? 0.4 : 1.0
            border.width: 2
            radius: model.type > 1 ? 6 : height * 0.5
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                width: model.width - 20
                elide: Text.ElideRight
                text: model.name
                font.pixelSize: 14
                opacity: model.disabled ? 0.5 : 1.0
                color: mainView.foregroundColor2
            }
            Image {
                id: disableIconButton
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 6
                width: 16
                height: 16
                mipmap: true
                visible: model.type === 2
                source: model.disabled ? "images/icon_visibility_off.png" : "images/icon_visibility_on.png"
                opacity: model.disabled ? 1.0 : 0.2
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -4
                    onClicked: {
                        model.disabled = !model.disabled;
                        nodeViewItem.updateActiveNodesList();
                        effectManager.bakeShaders(true);
                    }
                }
            }

            // Connectors
            Rectangle {
                width: 10
                height: 10
                x: parent.width * 0.5 - width * 0.5
                y: -height * 0.5
                radius: width * 0.5
                visible: model.type !== 0
                border.width: 2
                border.color: connectorColor
                color: mainView.backgroundColor1
            }
            Rectangle {
                width: 10
                height: 10
                x: parent.width * 0.5 - width * 0.5
                y: parent.height - height * 0.5
                radius: width * 0.5
                visible: model.type !== 1
                border.width: 2
                border.color: connectorColor
                color: mainView.backgroundColor1
            }
        }
    }

    // Render arrows between nodes
    Repeater {
        model: nodeViewItem.arrowsModel
        delegate: Shape {
            anchors.fill: parent
            ShapePath {
                strokeWidth: 2
                strokeColor: connectorColor
                capStyle: ShapePath.RoundCap
                startX: model.startX
                startY: model.startY
                PathLine {
                    x: model.endX
                    y: model.endY
                }
            }
        }
    }

    // Render '+' into arrows
    Repeater {
        model: nodeViewItem.arrowsModel
        delegate: Rectangle {
            property real arrowWidth: Math.sqrt(Math.pow(model.endX - model.startX, 2) + Math.pow(model.endY - model.startY, 2))

            x: model.startX + (model.endX - model.startX) * 0.5 - width * 0.5
            y: model.startY + (model.endY - model.startY) * 0.5 - height * 0.5
            width: 20
            height: 20
            radius: width * 0.5
            border.width: 1
            color: mainView.backgroundColor1
            border.color: highlightColor
            visible: arrowWidth > 50
            Rectangle {
                width: 8
                height: 2
                anchors.centerIn: parent
                color: mainView.foregroundColor2
            }
            Rectangle {
                width: 2
                height: 8
                anchors.centerIn: parent
                color: mainView.foregroundColor2
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    nodeViewItem.showAddNodeDialog(model.startNodeId, model.endNodeId);
                }
            }
            Text {
                anchors.left: parent.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 16
                color: mainView.foregroundColor1
                visible: nodeViewItem.nodeGraphComplete && nodeViewItem.arrowsModel.rowCount === 1
                text: "Add Node"
            }
        }
    }

    // Add node button
    Rectangle {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        width: 30
        height: 30
        radius: width * 0.5
        border.width: 2
        color: mainView.backgroundColor1
        border.color: borderColor
        Rectangle {
            width: 8
            height: 2
            anchors.centerIn: parent
            color: mainView.foregroundColor2
        }
        Rectangle {
            width: 2
            height: 8
            anchors.centerIn: parent
            color: mainView.foregroundColor2
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                nodeViewItem.showAddNodeDialog(-1, -1);
            }
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.right
            anchors.leftMargin: 10
            font.pixelSize: 16
            color: mainView.foregroundColor1
            text: "Add Node"
        }
    }

    // Active arrow user modifies
    Shape {
        anchors.fill: parent
        visible: nodeViewItem.activeArrowEnabled
        opacity: 0.4
        ShapePath {
            strokeWidth: 4
            strokeColor: connectorColor
            strokeStyle: ShapePath.DashLine
            dashPattern: [2, 2]
            capStyle: ShapePath.RoundCap
            startX: nodeViewItem.activeArrowStartPoint.x
            startY: nodeViewItem.activeArrowStartPoint.y
            PathLine {
                x: nodeViewItem.activeArrowEndPoint.x
                y: nodeViewItem.activeArrowEndPoint.y
            }
        }
    }
}
