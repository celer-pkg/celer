// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls.Basic

Slider {
    // Height, width and any other size related properties containing odd looking float or other dividers
    // that do not seem to have any logical origin are just arbitrary and based on original design
    // and/or personal preference on what looks nice.
    id: slider
    property int liquidAmount: 0
    property alias box: box
    property alias liquidAmountText: liquidAmountText

    states: State {
        name: "pressed"
        when: slider.pressed
        PropertyChanges {
            target: handle
            scale: 1.1
        }
    }

    transitions: Transition {
        NumberAnimation {
            properties: "scale"
            duration: 10
            easing.type: Easing.InOutQuad
        }
    }

    background: Rectangle {
        id: rectangle
        x: slider.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        width: slider.availableWidth
        height: 4
        radius: 2
        color: Colors.grey

        Rectangle {
            width: slider.visualPosition * parent.width
            height: parent.height
            color: Colors.green
            radius: 2
        }
    }
    handle: Rectangle {
        id: handle
        x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
        anchors.verticalCenter: parent.verticalCenter
        width: 14
        height: width
        radius: 100
        color: Colors.green
        Image {
            id: handleMark
            source: "./images/icons/Polygon.svg"
            anchors.bottom: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 1
        }
        Rectangle {
            id: box
            anchors.bottom: handleMark.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 1
            implicitWidth: liquidAmountText.width + 8
            implicitHeight: liquidAmountText.height + 4
            radius: 4
            color: Colors.currentTheme.background
            border.color: Colors.grey
            Text {
                id: liquidAmountText
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                text: slider.liquidAmount + "ml"
                font.pixelSize: 12
                clip: false
                color: Colors.currentTheme.textColor
            }
        }
    }
}
