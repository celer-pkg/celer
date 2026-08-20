// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: rootItem
    property real showState: 1.0

    default property alias content: settingsArea.children

    width: settingsArea.width
    height: settingsArea.height
    x: parent.width - showState * width + (1 - showState) * 30

    Behavior on showState {
        NumberAnimation {
            duration: 800
            easing.type: Easing.InOutQuad
        }
    }

    // Background
    Rectangle {
        anchors.fill: settingsArea
        anchors.margins: -10
        color: "#80404040"
        border.color: "#000000"
        border.width: 1
        opacity: 0.8
        MouseArea {
            anchors.fill: parent
            onPressed: {}
        }
    }

    Item {
        anchors.right: parent.left
        anchors.rightMargin: 30
        anchors.top: parent.top
        width: 16 + Math.max(rootItem.width, rootItem.height) * 0.05
        height: width
        opacity: rootItem.showState * 0.6 + 0.4
        visible: opacity
        Image {
            anchors.centerIn: parent
            width: parent.width * 0.3
            height: width
            source: "images/icon_settings.png"
            mipmap: true
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                settings.showSettingsView = !settings.showSettingsView; // qmllint disable unqualified
            }
        }
    }

    Column {
        id: settingsArea
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20
    }
}
