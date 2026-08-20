// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls.Basic

AbstractButton {
    // Height, width and any other size related properties containing odd looking float or other dividers
    // that do not seem to have any logical origin are just arbitrary and based on original design
    // and/or personal preference on what looks nice.
    id: button
    property string buttonText: ""
    property bool showIcon: true
    property string buttonColor: "grey"
    property alias rectangle: rectangle

    states: State {
        name: "pressed"
        when: button.pressed
        PropertyChanges {
            target: rectangle
            scale: 0.9
        }
    }

    transitions: Transition {
        NumberAnimation {
            properties: "scale"
            duration: 10
            easing.type: Easing.InOutQuad
        }
    }
    contentItem: Rectangle {
        id: rectangle
        border.width: 1
        border.color: Colors.border
        radius: 10
        anchors.fill: parent
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            Text {
                text: button.buttonText
                color: (button.buttonColor
                        == "green") ? Colors.dark.textColor : Colors.currentTheme.textColor
                font.pixelSize: 18
                font.weight: 700
                rightPadding: 8
            }
            Image {
                id: icon
                visible: button.showIcon
                source: (Colors.currentTheme == Colors.dark) ? "./images/icons/keyboard_backspace_white_right.svg" : "./images/icons/keyboard_backspace_black_right.svg"
            }
        }
    }
}
