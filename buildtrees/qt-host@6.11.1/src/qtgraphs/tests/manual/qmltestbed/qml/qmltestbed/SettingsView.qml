// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem
    property real showState: settings.showSettingsView ? 1.0 : 0.0
    readonly property real posX: settingsDrawer.visible ? Window.window.width - settingsDrawer.x : 0

    default property alias content: settingsArea.children

    width: settingsDrawer.width
    height: settingsDrawer.height

    Button {
        x: (settingsDrawer.visible) ? settingsDrawer.x - width : Window.window.width - width
        anchors.top: parent.top
        width: settings.iconSize
        height: width
        opacity: rootItem.showState * 0.6 + 0.4
        visible: opacity
        icon.width: width * 0.3
        icon.height: height * 0.3
        icon.source: "images/icon_settings.png"
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            settings.showSettingsView = !settings.showSettingsView;
        }
    }

    Drawer {
        id: settingsDrawer
        modal: false
        edge: Qt.RightEdge
        interactive: false
        leftInset: -10
        topInset: -20
        bottomInset: -20
        topMargin: 0
        visible: settings.showSettingsView

        background: Rectangle {
            color: "#000000"
            border.color: "#808080"
            border.width: 1
            opacity: 0.8
        }

        ScrollView {
            id: scrollView
            anchors.fill: parent
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.interactive: false
            contentHeight: settingsArea.height + 40
            clip: true
            Column {
                id: settingsArea
                y: 20
                spacing: 10
            }
        }

        enter: Transition {
            NumberAnimation {
                property: "position"
                to: 1.0
                duration: 800
                easing.type: Easing.InOutQuad
            }
        }

        exit: Transition {
            NumberAnimation {
                property: "position"
                to: 0.0
                duration: 800
                easing.type: Easing.InOutQuad
            }
        }
    }
}
