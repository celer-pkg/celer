// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Window
import QtQuick.Effects
import "CloudyRectMaterial"

Rectangle {
    id: mainWindow

    // Multiplier for resolution independency
    readonly property real dp: 0.2 + Math.min(width, height) / 1200
    // Color used everywhere in the example
    property color mainColor: "#b0b0b0"

    width: 1280
    height: 720
    color: mainColor

    Settings {
        id: settings
    }

    Settings {
        id: defaultSettings
    }

    // Animate the mainColor
    SequentialAnimation on mainColor {
        loops: Animation.Infinite
        ColorAnimation {
            to: "#60c0d0"
            duration: 8000
        }
        ColorAnimation {
            to: "#80d090"
            duration: 8000
        }
        ColorAnimation {
            to: "#d0c0b0"
            duration: 8000
        }
        ColorAnimation {
            to: "#b0b0b0"
            duration: 8000
        }
    }

    CloudyRectMaterial {
        id: cloudyRectMaterialLight
        timeRunning: true
        electricCloudColor: Qt.lighter(mainColor, 1.4)
        visible: settings.showCustomMaterial
    }
    CloudyRectMaterial {
        id: cloudyRectMaterialDark
        timeRunning: true
        electricCloudColor: Qt.lighter(mainColor, 0.6)
        visible: settings.showCustomMaterial
    }

    Item {
        id: mainArea
        anchors.left: settingsView.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        Rectangle {
            id: sourceItem
            anchors.centerIn: parent
            width: settings.itemSize * 2
            height: settings.itemSize
            radius: settings.radius
            topRightRadius: settings.individualRadius ? settings.radiustr : undefined
            bottomRightRadius: settings.individualRadius ? settings.radiusbr : undefined
            bottomLeftRadius: settings.individualRadius ? settings.radiusbl : undefined
            topLeftRadius: settings.individualRadius ? settings.radiustl : undefined
            gradient: Gradient {
                id: gradient
                property real lightLevelUp: 1.05 - settings.offsetY * 0.004 * settings.opacity
                property real lightLevelDown: 1.05 + settings.offsetY * 0.004 * settings.opacity
                GradientStop {
                    color: Qt.lighter(mainColor, gradient.lightLevelUp)
                    position: 0
                }
                GradientStop {
                    color: Qt.lighter(mainColor, gradient.lightLevelDown)
                    position: 1
                }
            }
            Image {
                anchors.centerIn: parent
                height: parent.height * 0.6
                width: height
                source: "images/qt_logo_white_rgb.png"
                opacity: 0.1
            }
        }

        RectangularShadow {
            id: rectangularShadow1
            anchors.fill: sourceItem
            offset.x: settings.offsetX
            offset.y: settings.offsetY
            radius: settings.radius
            topRightRadius: settings.individualRadius ? settings.radiustr : undefined
            bottomRightRadius: settings.individualRadius ? settings.radiusbr : undefined
            bottomLeftRadius: settings.individualRadius ? settings.radiusbl : undefined
            topLeftRadius: settings.individualRadius ? settings.radiustl : undefined
            blur: settings.blur
            spread: settings.spread
            opacity: settings.opacity
            color: Qt.lighter(mainColor, 1.3)
            z: -1
            material: settings.showCustomMaterial ? cloudyRectMaterialLight : null
            Rectangle {
                z: 1
                x: parent.material.x - 1
                y: parent.material.y - 1
                width: parent.material.width + 2
                height: parent.material.height + 2
                color: "transparent"
                border.width: 1
                border.color: "#ffffff"
                visible: settings.showDebug
            }
        }

        RectangularShadow {
            id: rectangularShadow2
            anchors.fill: sourceItem
            offset.x: -settings.offsetX
            offset.y: -settings.offsetY
            radius: settings.radius
            topRightRadius: settings.individualRadius ? settings.radiustr : undefined
            bottomRightRadius: settings.individualRadius ? settings.radiusbr : undefined
            bottomLeftRadius: settings.individualRadius ? settings.radiusbl : undefined
            topLeftRadius: settings.individualRadius ? settings.radiustl : undefined
            blur: settings.blur
            spread: settings.spread
            opacity: settings.opacity
            color: Qt.lighter(mainColor, 0.7)
            z: -2
            material: settings.showCustomMaterial ? cloudyRectMaterialDark : null
            Rectangle {
                z: 1
                x: parent.material.x - 1
                y: parent.material.y - 1
                width: parent.material.width + 2
                height: parent.material.height + 2
                color: "transparent"
                border.width: 1
                border.color: "#000000"
                visible: settings.showDebug
            }
        }
        Text {
            anchors.horizontalCenter: lightArea.horizontalCenter
            anchors.bottom: lightArea.top
            color: "#202020"
            font.pixelSize: 16 * dp
            text: "Offset (" + settings.offsetX.toFixed(0) + ", " +
                  settings.offsetY.toFixed(0) + ")"
        }
        Rectangle {
            id: lightArea
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            width: settings.settingsViewWidth * 0.4
            height: width
            radius: 5
            color: Qt.lighter(mainWindow.mainColor, 0.7)
            border.width: 1
            border.color: Qt.lighter(mainWindow.mainColor, 0.5)
            Item {
                id: lightItem
                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: width
                    radius: width / 2
                    color: "#e0e0e0"
                    RectangularShadow {
                        anchors.centerIn: parent
                        width: parent.width * 3.0
                        height: width
                        radius: width / 2
                        blur: width
                        color: "#fff9f0"
                        opacity: settings.opacity
                    }
                }
            }
            MouseArea {
                id: lightMouseArea
                anchors.fill: parent
                preventStealing: true
                Component.onCompleted: {
                    settingsView.resetPosition();
                }
                onPositionChanged:
                    (mouse)=> {
                        settingsView.updatePosition(mouse.x, mouse.y);
                    }
                onPressed:
                    (mouse)=> {
                        settingsView.updatePosition(mouse.x, mouse.y);
                    }
            }
        }
    }

    SettingsView {
        id: settingsView
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 20
        visible: settings.showSettingsView
    }
}
