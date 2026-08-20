// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: switchMap

    property bool opened: false
    property int mapType: MapTypeConfig.map
    property int orientation: Qt.Vertical

    implicitWidth: width
    implicitHeight: height
    width: (orientation === Qt.Vertical) ? 80 : (opened ? 150 : 80)
    height: (orientation === Qt.Horizontal) ? 80 : (opened ? 150 : 80)
    Behavior on width  { NumberAnimation { duration: switchMap.behaviorDuration } }
    Behavior on height { NumberAnimation { duration: switchMap.behaviorDuration } }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        radius: 20
        opacity: switchMap.opened * 0.55
        Behavior on opacity { NumberAnimation { duration: switchMap.behaviorDuration } }
    }

    Item {
        anchors.fill: parent
        anchors.margins: 10
        Button {
            readonly property bool selected: switchMap.mapType === MapTypeConfig.map
            z: 1 + checked
            x: switchMap.getButtonX(selected)
            y: switchMap.getButtonY(selected)
            width: switchMap.buttonSize
            height: switchMap.buttonSize
            icon.source: "icons/map.svg"
            icon.color: switchMap.getButtonBgColor(enabled, checked)
            icon.width: switchMap.iconSize
            icon.height: switchMap.iconSize
            checked: switchMap.opened && selected
            visible: switchMap.opened + selected
            onClicked: switchMap.select(MapTypeConfig.map);
            Behavior on x { NumberAnimation { duration: switchMap.behaviorDuration } }
            Behavior on y { NumberAnimation { duration: switchMap.behaviorDuration } }
        }
        Button {
            readonly property bool selected: switchMap.mapType === MapTypeConfig.globe
            z: 1 + checked
            x: switchMap.getButtonX(selected)
            y: switchMap.getButtonY(selected)
            width: switchMap.buttonSize
            height: switchMap.buttonSize
            icon.source: "icons/globe.svg"
            icon.color: switchMap.getButtonBgColor(enabled, checked)
            icon.width: switchMap.iconSize
            icon.height: switchMap.iconSize
            checked: switchMap.opened && selected
            visible: switchMap.opened + selected
            onClicked: switchMap.select(MapTypeConfig.globe);
            Behavior on x { NumberAnimation { duration: switchMap.behaviorDuration } }
            Behavior on y { NumberAnimation { duration: switchMap.behaviorDuration } }
        }
        MouseArea {
            z: 3
            anchors.fill: parent
            enabled: !switchMap.opened
            onClicked: switchMap.opened = !switchMap.opened;
        }
    }

    Timer {
        id: timer
        interval: switchMap.showTimeInterval
        running: false
        onTriggered: switchMap.opened = false;
    }

    readonly property int behaviorDuration: 250
    readonly property int showTimeInterval: 3000

    function select(mapType) {
        timer.restart()
        if (switchMap.mapType !== mapType)
            switchMap.mapType = mapType;
    }

    readonly property color disabled_color: "#808080"
    readonly property color checked_color: "#80EBB6"
    readonly property color normal_color: "#2CDE85"
    function getButtonBgColor(enabled, checked) {
        return !enabled ? "#808080" : checked ? "#80EBB6" : "#2CDE85";
    }

    readonly property int iconSize: 24
    readonly property int buttonSize: 60
    readonly property int spacing: 10
    function getButtonX(selected) {
        const vertical_ = switchMap.orientation === Qt.Vertical;
        return selected ? 0 : vertical_ ? 0 : buttonSize + spacing;
    }
    function getButtonY(selected) {
        const horizontal_ = switchMap.orientation === Qt.Horizontal;
        return selected ? 0 : horizontal_ ? 0 : buttonSize + spacing;
    }

    onOpenedChanged: {
        if (opened)
            timer.start();
    }
}
