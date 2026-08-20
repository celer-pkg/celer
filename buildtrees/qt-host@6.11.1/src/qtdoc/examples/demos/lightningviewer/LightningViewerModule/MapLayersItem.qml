// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mapLayersItem
    implicitWidth: width
    implicitHeight: 126
    width: opened ? 284 : 90

    property Component mapActionButton: null
    property alias lightningLayerVisible: lightningLayerButton.checked
    property alias distanceLayerVisible: distanceButton.checked
    property bool opened: false

    function toggle() {
        if (opened)
            close();
        else
            open();
    }
    function close() {
        timer.stop();
        opened = false;
    }
    function open() {
        timer.restart();
        opened = true;
    }

    Rectangle {
        anchors.fill: parent
        radius: 25
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#00414A" }
            GradientStop { position: 1.0; color: "#0C1C1F" }
        }
        opacity: mapLayersItem.opened ? 0.9 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 0
        Loader {
            Layout.preferredWidth: item ? (item as Item).width : mapLayersItem.buttonSize
            Layout.preferredHeight: item ? (item as Item).height : mapLayersItem.buttonSize
            sourceComponent: mapLayersItem.mapActionButton
        }
        Item {
            Layout.preferredWidth: 47
            Layout.fillHeight: true
            visible: mapLayersItem.opened
            Rectangle {
                anchors.centerIn: parent
                width: 2
                height: 22
                color: "#2CDE85"
            }
        }
        Item {
            Layout.preferredWidth: mapLayersItem.buttonSize
            Layout.fillHeight: true
            visible: mapLayersItem.opened
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                RoundButton {
                    id: lightningLayerButton
                    Layout.preferredWidth: mapLayersItem.buttonSize
                    Layout.preferredHeight: mapLayersItem.buttonSize
                    icon.source: "icons/lightning-layer.svg"
                    icon.color: mapLayersItem.getIconColor(pressed, checked, hovered)
                    icon.width: mapLayersItem.iconSize
                    icon.height: mapLayersItem.iconSize
                    checkable: true
                    onClicked: timer.restart();
                    leftPadding: checked ? (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2
                                         : (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2 - 2
                    topPadding: checked ? (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2
                                        : (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2 - 2
                }
                Text {
                    text: qsTr("Lightning\nLive")
                    Layout.fillWidth: true
                    horizontalAlignment: Qt.AlignHCenter
                    color: mapLayersItem.getTextColor(lightningLayerButton.checked,
                                                      lightningLayerButton.hovered)
                    font.pixelSize: 12
                }
                Item { Layout.fillHeight: true }
            }
        }
        Item {
            Layout.preferredWidth: 20
            Layout.fillHeight: true
            visible: mapLayersItem.opened
        }
        Item {
            Layout.preferredWidth: mapLayersItem.buttonSize
            Layout.fillHeight: true
            visible: mapLayersItem.opened
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                RoundButton {
                    id: distanceButton
                    Layout.preferredWidth: mapLayersItem.buttonSize
                    Layout.preferredHeight: mapLayersItem.buttonSize
                    icon.source: "icons/distance.svg"
                    icon.color: mapLayersItem.getIconColor(pressed, checked, hovered)
                    icon.width: mapLayersItem.iconSize
                    icon.height: mapLayersItem.iconSize
                    checkable: true
                    onClicked: timer.restart();
                    leftPadding: checked ? (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2
                                         : (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2 - 2
                    topPadding: checked ? (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2
                                        : (mapLayersItem.buttonSize-mapLayersItem.iconSize) / 2 - 2
                }
                Text {
                    text: qsTr("Distance\n& Time")
                    Layout.fillWidth: true
                    horizontalAlignment: Qt.AlignHCenter
                    color: mapLayersItem.getTextColor(distanceButton.checked,
                                                      distanceButton.hovered)
                    font.pixelSize: 12
                }
                Item { Layout.fillHeight: true }
            }
        }
    }

    Behavior on width { NumberAnimation { duration: 200 } }

    Timer {
        id: timer
        interval: 4000
        onTriggered: mapLayersItem.opened = false;
    }

    readonly property int buttonSize: 64
    readonly property int iconSize: 24

    readonly property color normalColor: "#C0F5DA"
    readonly property color checkedColor: "#2CDE85"
    function getIconColor(pressed, checked, hovered) {
        return pressed ?  normalColor
                       : checked ? (hovered ? normalColor : checkedColor)
                                 : (hovered ? checkedColor : normalColor)
    }
    function getTextColor(checked, hovered) {
        return checked && !hovered ? checkedColor : normalColor;
    }
}
