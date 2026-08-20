// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: button
    property string text: ""
    property color buttonColor: "white"
    property color textColor: "black"
    property bool available: true
    property alias fontPointSize: buttonText.font.pointSize

    signal clicked()

    state: "NORMAL"

    Rectangle {
        id: buttonRect
        anchors.fill: parent
        radius: 10
        color: buttonColor
        visible: button.available
        Text {
            id: buttonText
            anchors.fill: parent
            anchors.rightMargin: parent.width * 0.05
            anchors.leftMargin: parent.width * 0.05
            anchors.bottomMargin: parent.height * 0.20
            anchors.topMargin: parent.height * 0.20
            text: button.text
            color: textColor
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPointSize: 8
            font.pointSize: 64
            font.family: "Helvetica"
            font.weight: Font.Light
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            button.clicked();
        }
        onPressed: {
            button.state = "PRESSED";
        }
        onReleased: {
            button.state = "NORMAL";
        }
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: buttonRect
                color: button.buttonColor
                border.color: "transparent"
            }
            PropertyChanges {
                target: buttonText
                color: button.textColor
            }
        },
        State {
            name: "PRESSED"
            PropertyChanges {
                target: buttonRect
                color: "transparent"
                border.color: button.buttonColor
            }
            PropertyChanges {
                target: buttonText
                color: button.buttonColor
            }
        }
    ]
}
