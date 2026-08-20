// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem

    property bool designMode: !designModeToggle.toggled
    property real designModeAnimated: 1.0 - designModeToggle.toggledAnimated

    function setDesignMode(mode) {
        designModeToggle.toggled = !mode;
    }

    function setDesignModeInstantly(mode) {
        mainView.switchDesignModeInstantly = true;
        designModeToggle.toggled = !mode;
        mainView.switchDesignModeInstantly = false;
    }

    Rectangle {
        width: parent.width
        height: 50
        color: backgroundColor1
    }
    CustomModeToggle {
        id: designModeToggle
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height - 10
        textOff: "CODE"
        textOn: "DESIGN"
        description: rootItem.designMode ? "Switch to Code mode" : "Switch to Design mode"
        animatedToggle: !mainView.switchDesignModeInstantly
    }
    Item {
        id: designModeToolbar
        anchors.left: designModeToggle.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        height: parent.height
        opacity: designModeAnimated
        visible: opacity
        CustomIconButton {
            id: alignHorizontallyButton
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height * 0.6
            width: height
            icon: "images/icon_layout_nodes.png"
            description: "Align nodes horizontally"
            onClicked: {
                nodeViewItem.layoutNodes(false);
            }
        }
        CustomIconButton {
            id: distributeVerticallyButton
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: alignHorizontallyButton.right
            height: parent.height * 0.6
            width: height
            icon: "images/icon_distribute_nodes.png"
            description: "Distribute nodes vertically"
            onClicked: {
                nodeViewItem.layoutNodes(true);
            }
        }
    }
    Item {
        id: codeModeToolbar
        anchors.left: designModeToggle.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        height: parent.height
        opacity: 1.0 - designModeAnimated
        visible: opacity
        ComboBox {
            anchors.verticalCenter: parent.verticalCenter
            height: 40
            width: parent.width * 0.5
            model: effectManager.nodeView.codeSelectorModel
            currentIndex: effectManager.nodeView.codeSelectorIndex
            onActivated: {
                var nodeId = effectManager.nodeView.getNodeIdWithName(currentText);
                effectManager.nodeView.selectSingleNode(nodeId);
            }
        }
    }
}
