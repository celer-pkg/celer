// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: icon
    property alias source: loader.source
    property bool preferWidth: loader.implicitWidth > loader.implicitHeight
    width: 128
    height: 128

    opacity: grid.selectedItem == null || grid.selectedItem == icon ? 1.0 : 0.0
    Behavior on opacity {
        NumberAnimation { duration: window.animationDuration; easing.type: window.animationEasing }
    }

    Behavior on scale {
        NumberAnimation { duration: window.animationDuration; easing.type: window.animationEasing }
    }

    Rectangle {
        anchors.fill: parent
        color: "darkslategray"
        border.color: icon.scale === 1.0 ? "lavender" : "transparent"
        border.width: 2
        radius: icon.scale === 1.0 ? 10 : 0

        Behavior on border.color {
            ColorAnimation { duration: window.animationDuration; easing.type: window.animationEasing }
        }
    }

    Item {
        id: iconItem
        x: 2
        y: 2
        width: parent.width - 4
        height: parent.height - 4
        clip: icon.scale === 1.0
        Loader {
            id: loader
            width: preferWidth ? parent.width : parent.height * (implicitWidth / implicitHeight)
            height: preferWidth ? parent.width * (implicitHeight / implicitWidth) : parent.height

            anchors.centerIn: parent

            onLoaded: {
                item.animations.loops = Animation.Infinite
            }
        }

        PinchHandler {
            rotationAxis.enabled: false
            enabled: grid.selectedItem === icon

            onActiveChanged: {
                if (!active) {
                    iconItem.scale = 1
                    iconItem.x = 2
                    iconItem.y = 2
                    iconItem.width = iconItem.parent.width - 4
                    iconItem.height = iconItem.parent.height - 4
                }
            }
        }
    }

    TapHandler {
        enabled: icon.opacity === 1.0
        onTapped: {
            grid.zoom(icon)
        }
    }
}
