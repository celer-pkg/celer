// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

CustomPopup {
    id: rootItem

    property int startNodeId: -1
    property int endNodeId: -1
    property string selectedNodeFile
    property string selectedNodeName
    property string selectedNodeDescription
    property string selectedNodeRequires
    property var selectedNodeProperties: []
    property bool selectedNodeCanBeAdded: true

    property real showHideAnimationSpeed: 400
    // All currently open node groups
    property var openGroups: []

    function reset() {
        startNodeId = -1;
        endNodeId = -1;
        selectedNodeFile = "";
        selectedNodeName = "";
        selectedNodeDescription = "";
        selectedNodeRequires = "";
        selectedNodeProperties = [];
        selectedNodeCanBeAdded = true;
    }

    function addSelectedNode() {
        effectManager.addEffectNode(selectedNodeFile, rootItem.startNodeId, rootItem.endNodeId);
        rootItem.close();
    }

    modal: true
    width: 700
    height: 500
    padding: 10

    Text {
        id: dialogTitle
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Add Effect Node"
        font.pixelSize: 16
        color: mainView.foregroundColor2
    }
    Row {
        anchors.top: dialogTitle.bottom
        anchors.topMargin: 10
        anchors.bottom: buttonBar.top
        anchors.bottomMargin: 10
        width: parent.width
        Component {
            id: sectionHeading
            Item {
                required property string section
                readonly property bool show: openGroups.includes(section)
                width: effectsListView.width - scrollBar.width - 5
                height: 40
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    color: mainView.backgroundColor2
                    border.width: 1
                    border.color: mainView.foregroundColor1
                }
                Image {
                    x: 10
                    source: "images/icon_arrow.png"
                    anchors.verticalCenter: parent.verticalCenter
                    rotation: show ? 90 : 0
                    scale: 0.5
                    Behavior on rotation {
                        NumberAnimation {
                            duration: showHideAnimationSpeed
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    x: 30
                    color: mainView.foregroundColor2
                    font.bold: true
                    font.pixelSize: 12
                    text: parent.section
                    font.capitalization: Font.AllUppercase
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // Show/hide the group
                        const index = openGroups.indexOf(section);
                        if (index > -1)
                            openGroups.splice(index, 1);
                        else
                            openGroups.push(section);
                        // Triggering manually required for array properties
                        // so 'show' is updated.
                        openGroupsChanged();
                        effectManager.showHideAddNodeGroup(parent.section, show);
                    }
                }
            }
        }
        ListView {
            id: effectsListView
            width: parent.width * 0.35
            height: parent.height
            model: effectManager.addNodeModel
            clip: true
            cacheBuffer: 10000
            ScrollBar.vertical: ScrollBar {
                id: scrollBar
                policy: ScrollBar.AlwaysOn
            }
            section.property: "group"
            section.criteria: ViewSection.FullString
            section.delegate: sectionHeading
            delegate: Item {
                property real showAnimated: model.show
                x: 2
                width: effectsListView.width - scrollBar.width - 9
                height: showAnimated * 30
                visible: opacity
                opacity: showAnimated
                Behavior on showAnimated {
                    NumberAnimation {
                        duration: showHideAnimationSpeed
                        easing.type: Easing.InOutQuad
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: mainView.backgroundColor1
                    border.color: (selectedNodeFile === model.file) ? highlightColor : "transparent"
                    border.width: 2
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: model.name
                    color: mainView.foregroundColor2
                    font.pixelSize: 14
                    elide: Text.ElideRight
                }
                Image {
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.4
                    width: height
                    source: "images/icon_requires.png"
                    mipmap: true
                    visible: model.requires !== ""
                    opacity: 0.2
                    MouseArea {
                        id: requiresIconMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                    ToolTip {
                        parent: parent
                        visible: requiresIconMouseArea.containsMouse
                        delay: 1000
                        text: "Requires: " + model.requires
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        selectedNodeName = model.name;
                        selectedNodeFile = model.file;
                        selectedNodeDescription = model.description;
                        selectedNodeRequires = model.requires;
                        selectedNodeProperties = model.properties;
                        selectedNodeCanBeAdded = model.canBeAdded;
                    }
                    onDoubleClicked: {
                        if (model.canBeAdded)
                            addSelectedNode();
                    }
                }
            }
            BusyIndicator {
                id: refreshNodesIndicator
                anchors.centerIn: parent
                opacity: 0.0
                running: opacity > 0
                visible: running
                function show() {
                    nodesLoadingIndicatorAnimation.restart();
                }
                SequentialAnimation {
                    id: nodesLoadingIndicatorAnimation
                    NumberAnimation {
                        target: refreshNodesIndicator
                        property: "opacity"
                        to: 1.0
                        duration: 250
                        easing.type: Easing.InOutQuad
                    }
                    PauseAnimation {
                        duration: 500
                    }
                    NumberAnimation {
                        target: refreshNodesIndicator
                        property: "opacity"
                        to: 0.0
                        duration: 250
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
        Flickable {
            id: nodeInfoFlickable
            readonly property int padding: 10
            width: parent.width - effectsListView.width
            height: parent.height
            contentHeight: nodeInfo.height
            clip: true
            ScrollBar.vertical: ScrollBar {
                width: 15
                active: true
            }
            Column {
                id: nodeInfo
                y: nodeInfoFlickable.padding
                x: nodeInfoFlickable.padding
                width: parent.width - nodeInfoFlickable.padding * 3
                spacing: nodeInfoFlickable.padding
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: selectedNodeName
                    font.pixelSize: 16
                    font.bold: true
                    color: mainView.foregroundColor2
                }
                Text {
                    width: parent.width
                    text: selectedNodeDescription
                    font.pixelSize: 14
                    color: mainView.foregroundColor2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
                Text {
                    width: parent.width
                    text: "Requires: " + selectedNodeRequires
                    font.pixelSize: 14
                    font.bold: true
                    color: mainView.foregroundColor2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    visible: selectedNodeRequires !== ""
                }
                Text {
                    font.pixelSize: 12
                    font.bold: true
                    color: mainView.foregroundColor2
                    text: "PROPERTIES:"
                    visible: selectedNodeProperties.length > 0
                }
                Rectangle {
                    id: propertiesArea
                    width: parent.width
                    height: childrenRect.height + nodeInfoFlickable.padding
                    color: mainView.backgroundColor2
                    border.width: 1
                    border.color: mainView.foregroundColor1
                    visible: selectedNodeProperties.length > 0
                    Column {
                        width: parent.width - nodeInfoFlickable.padding * 2
                        x: nodeInfoFlickable.padding
                        y: nodeInfoFlickable.padding * 0.5
                        Repeater {
                            model: selectedNodeProperties
                            Text {
                                property var nodeProperty: selectedNodeProperties[index]
                                width: parent.width
                                elide: Text.ElideRight
                                font.pixelSize: 14
                                color: mainView.foregroundColor2
                                text: "<b>" + nodeProperty.type + "</b> " + nodeProperty.name
                            }
                        }
                    }
                }
                Item {
                    width: 1
                    height: nodeInfoFlickable.padding
                }
            }
        }
    }
    Item {
        id: buttonBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        height: 40
        Button {
            id: cancelButton
            anchors.left: parent.left
            height: parent.height
            text: "Cancel"
            onPressed: {
                rootItem.close();
            }
        }
        Button {
            id: refreshButton
            anchors.left: cancelButton.right
            anchors.leftMargin: 10
            height: parent.height
            text: "Refresh"
            onPressed: {
                refreshNodesIndicator.show();
                effectManager.refreshAddNodesList();
            }
        }
        Button {
            id: addButton
            anchors.right: parent.right
            height: parent.height
            text: "Add"
            enabled: selectedNodeFile != "" && selectedNodeCanBeAdded
            onPressed: {
                rootItem.addSelectedNode();
            }
        }
    }
}
