// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem
    property real showState: AppSettings.showSettingsView ? 1.0 : 0.0

    default property alias content: settingsArea.children

    width: settingsDrawer.width
    height: settingsDrawer.height

    Button {
        x: (settingsDrawer.visible) ? settingsDrawer.x - width : Window.window.width - width
        anchors.top: parent.top
        width: AppSettings.iconSize
        height: width
        opacity: rootItem.showState * 0.6 + 0.4
        visible: opacity
        icon.width: width * 0.3
        icon.height: height * 0.3
        icon.source: "qrc:/images/icon_settings.png"
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            AppSettings.showSettingsView = !AppSettings.showSettingsView;
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
        topMargin: 10
        visible: AppSettings.showSettingsView

        background: Rectangle {
            color: "#80404040"
            border.color: "#000000"
            border.width: 1
            opacity: 0.8
        }

        Column {
            id: settingsArea
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
