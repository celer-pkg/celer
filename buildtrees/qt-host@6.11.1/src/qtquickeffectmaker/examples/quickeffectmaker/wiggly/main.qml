// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Material
import "WigglyEffect"

Window {
    id: mainWindow

    // Helper for UI scalability.
    // Pixel multiplier which depends on the window size.
    readonly property real px: width / defaultWindowWidth
    readonly property real defaultWindowWidth: 1280
    readonly property real defaultWindowHeight: 720

    Material.theme: Material.Dark
    Material.accent: Material.Pink

    width: defaultWindowWidth
    height: defaultWindowHeight
    visible: true
    title: qsTr("Wiggly")
    color: "#2e2f30"

    // Custom font
    FontLoader {
        id: font1
        // "Injekuta Bl" font from: https://typodermicfonts.com
        // Creative Commons Zero license https://creativecommons.org/publicdomain/zero/1.0/
        source: "injekuta_bl.otf"
    }

    //! [sourceitem]
    Rectangle {
        id: sourceItem
        anchors.centerIn: parent
        width: textItem.width + 60 * mainWindow.px
        height: textItem.height + 30 * mainWindow.px
        color: "#d0d0d0d0"
        border.color: "#d0ffffff"
        border.width: 4 * mainWindow.px
        radius: 20 * mainWindow.px
        layer.enabled: true
        layer.smooth: true
        visible: false
        Text {
            id: textItem
            anchors.centerIn: parent
            text: wigglyTextField.text
            font.family: font1.font.family
            font.pixelSize: Math.min(200 * mainWindow.px, 0.8 * mainWindow.width / text.length)
        }
    }
    //! [sourceitem]

    //! [wigglyeffect]
    WigglyEffect {
        id: wigglyEffect
        source: sourceItem
        anchors.fill: sourceItem
        timeRunning: true
        wigglyAmountX: wigglyAmountXSlider.value
        wigglyAmountY: wigglyAmountYSlider.value
        electricCloudColor.a: electricSwitch.checked ? 1.0 : 0.0
        wigglyShadows: 0.5
    }
    //! [wigglyeffect]

    // Toolbar background
    Rectangle {
        anchors.fill: toolbar
        anchors.leftMargin: -40 * mainWindow.px
        anchors.rightMargin: -20 * mainWindow.px
        color: "#40000000"
        border.width: 2
        border.color: "#20ffffff"
        radius: height * 0.5
     }

    // Toolbar with settings
    Row {
        id: toolbar
        height: wigglyTextField.height + 40 * mainWindow.px
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20 * mainWindow.px
        spacing: 20 * mainWindow.px
        //! [settings]
        TextField {
            id: wigglyTextField
            anchors.verticalCenter: parent.verticalCenter
            width: mainWindow.width * 0.3
            text: "Wiggly"
        }
        Slider {
            id: wigglyAmountYSlider
            anchors.verticalCenter: parent.verticalCenter
            width: mainWindow.width * 0.15
            from: 0
            to: 100
            value: 40
        }
        Slider {
            id: wigglyAmountXSlider
            anchors.verticalCenter: parent.verticalCenter
            width: mainWindow.width * 0.15
            from: 0
            to: 100
            value: 20
        }
        Switch {
            id: electricSwitch
            anchors.verticalCenter: parent.verticalCenter
        }
        //! [settings]
    }
}
