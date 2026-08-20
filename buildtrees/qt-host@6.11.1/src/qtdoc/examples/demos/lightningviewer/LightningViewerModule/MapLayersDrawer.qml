// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

Item {
    id: drawer
    property int dragMargin: 0
    property alias lightningLayerVisible: lightningLayerButton.checked
    property alias distanceLayerVisible: distanceButton.checked
    property real position: minPosition
    property bool opened: false

    function toggle() {
        if (state === open_state)
            close();
        else
            open();
    }
    function close() {
        if (state !== close_state)
            state = close_state;
    }
    function open() {
        if (state !== open_state)
            state = open_state;
    }

    Rectangle {
        id: background
        anchors.left: drawer.left
        anchors.bottom: drawer.bottom
        anchors.bottomMargin: -radius
        width: drawer.width
        height: drawer.height * drawer.position + radius
        opacity: 0.9
        radius: drawer.height * 0.2 * (drawer.position - drawer.minPosition)
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00414A" }
            GradientStop { position: 1.0; color: "#0C1C1F" }
        }
        Rectangle {
            color: "#2CDE85"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 6
            width: Window.window.width * 0.056
            height: Window.window.height * 0.003
            radius: height
        }

    }

    Item {
        id: dragItem
        width: parent.width
        height: drawer.height

        DragHandler {
            id: dragHandler
            grabPermissions: PointerHandler.TakeOverForbidden
            xAxis.enabled: false
            onActiveChanged: {
                positionBehavior.enabled = !active;
                if (!active)
                    drawer.decideState();
            }
        }

        onYChanged: {
            if (!dragHandler.active) {
                y = (1 - drawer.position) * drawer.height;
                return;
            }
            let position = 1 - y / drawer.height
            if (1.0 < position) {
                y = 0;
                return;
            } else if (position < drawer.minPosition) {
                position = drawer.minPosition;
                y = (1.0 - drawer.minPosition) * drawer.height;
            }
            drawer.position = position
        }

        Component.onCompleted: y = (1 - drawer.position) * drawer.height;
        Connections {
            target: drawer
            function onPositionChanged() { updateY(); }
            function onHeightChanged() { updateY(); }
            function updateY() { dragItem.y = (1 - drawer.position) * drawer.height; }
        }
    }

    Item {
        width: drawer.width
        height: drawer.height - drawer.dragMargin
        y: drawer.dragMargin + drawer.height * (1 - drawer.position)
        opacity: drawer.position
        ColumnLayout {
            anchors.fill: parent
            spacing: 18
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
                RowLayout {
                    anchors.fill: parent
                    spacing: 10
                    Item { Layout.fillWidth: true }
                    IconLabel {
                        implicitWidth: drawer.iconSize
                        implicitHeight: drawer.iconSize
                        icon.source: "icons/map-layers.svg"
                        icon.color: "#ABF2CE"
                    }
                    Text {
                        text: qsTr("Map Layers")
                        Layout.fillHeight: true
                        verticalAlignment: Qt.AlignVCenter
                        font.pixelSize: 16
                        font.weight: 700
                        color: "#ABF2CE"
                        rightPadding: 2
                    }
                    Item { Layout.fillWidth: true }
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                RowLayout {
                    anchors.fill: parent
                    spacing: 16
                    Item { Layout.fillWidth: true }
                    Item {
                        Layout.preferredWidth: drawer.buttonSize
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignTop
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            RoundButton {
                                id: lightningLayerButton
                                Layout.preferredWidth: drawer.buttonSize
                                Layout.preferredHeight: drawer.buttonSize
                                icon.source: "icons/lightning-layer.svg"
                                icon.color: drawer.getIconColor(pressed, checked, hovered)
                                icon.width: drawer.iconSize
                                icon.height: drawer.iconSize
                                checkable: true
                                leftPadding: checked
                                             ? (drawer.buttonSize - drawer.iconSize) / 2
                                             : (drawer.buttonSize - drawer.iconSize) / 2 - 2
                                topPadding: checked
                                            ? (drawer.buttonSize - drawer.iconSize) / 2
                                            : (drawer.buttonSize - drawer.iconSize) / 2 - 2
                            }
                            Text {
                                text: qsTr("Lightning\nLive")
                                Layout.fillWidth: true
                                horizontalAlignment: Qt.AlignHCenter
                                color: drawer.getTextColor(lightningLayerButton.checked,
                                                           lightningLayerButton.hovered)
                                font.pixelSize: 12
                            }
                            Item { Layout.fillHeight: true }
                        }
                    }
                    Item {
                        Layout.preferredWidth: drawer.buttonSize
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignTop
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            RoundButton {
                                id: distanceButton
                                Layout.preferredWidth: drawer.buttonSize
                                Layout.preferredHeight: drawer.buttonSize
                                icon.source: "icons/distance.svg"
                                icon.color: drawer.getIconColor(pressed, checked, hovered)
                                icon.width: drawer.iconSize
                                icon.height: drawer.iconSize
                                checkable: true
                                leftPadding: checked
                                             ? (drawer.buttonSize - drawer.iconSize) / 2
                                             : (drawer.buttonSize - drawer.iconSize) / 2 - 2
                                topPadding: checked
                                            ? (drawer.buttonSize - drawer.iconSize) / 2
                                            : (drawer.buttonSize - drawer.iconSize) / 2 - 2
                            }
                            Text {
                                text: qsTr("Distance\n& Time")
                                Layout.fillWidth: true
                                horizontalAlignment: Qt.AlignHCenter
                                color: drawer.getTextColor(distanceButton.checked,
                                                           distanceButton.hovered)
                                font.pixelSize: 12
                            }
                            Item { Layout.fillHeight: true }
                        }
                    }
                    Item { Layout.fillWidth: true }
                }
            }
        }
    }

    readonly property int buttonSize: 64
    readonly property int iconSize: 24
    readonly property real minPosition: height > 0 ? dragMargin / height : 0.2
    readonly property real maxPosition: 1.0
    onPositionChanged: {
        if (position < minPosition)
            position = minPosition
        if (maxPosition < position)
            position = maxPosition
    }
    Behavior on position {
        id: positionBehavior
        NumberAnimation { duration: 100 }
    }

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

    readonly property string open_state: "open_state"
    readonly property string close_state: "close_state"
    function decideState() {
        if (state === open_state) {
            state = (drawer.position < drawer.maxPosition) ? close_state : open_state
        } else {
            state = (drawer.minPosition < drawer.position) ? open_state : close_state
        }
    }

    states: [
        State {
            name: drawer.open_state
            PropertyChanges {
                drawer.opened: true
                drawer.position: drawer.maxPosition
            }
        },
        State {
            name: drawer.close_state
            PropertyChanges {
                drawer.opened: false
                drawer.position: drawer.minPosition
            }
        }
    ]
    state: close_state
}
