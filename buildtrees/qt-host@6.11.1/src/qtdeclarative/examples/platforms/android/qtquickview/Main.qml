// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls

Rectangle {
    id: mainRectangle

    color: colorStringFormat

    property string colorStringFormat: "#1CB669"

    signal onClicked()

    Text {
        id: helloText

        text: "First QML View"
        color: "white"
        font.pointSize: 72
        font.bold: true
        wrapMode: Text.WordWrap
        width: mainRectangle.width
        horizontalAlignment: Text.AlignHCenter

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: mainRectangle.height / 40
        }
    }


    Text {
        id: changeColorText

        text: "Tap button to change Java view background color"
        color: "white"
        font.pointSize: 48
        wrapMode: Text.WordWrap
        padding: 20
        width: mainRectangle.width
        horizontalAlignment: Text.AlignHCenter

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: helloText.bottom;
            topMargin: mainRectangle.height / 10
        }
    }

    Button {
        id: button

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: changeColorText.bottom
            topMargin: mainRectangle.height / 30
        }

        onClicked: mainRectangle.onClicked()

        background: Rectangle {
            id: buttonBackground

            radius: 14
            color: "#6200EE"
            opacity: button.down ? 0.6 : 1
            scale: button.down ? 0.9 : 1
        }

        contentItem: Text {
            id: buttonText

            text: "CHANGE COLOR"
            color: "white"
            font.pointSize: 56
            font.bold: true
            padding: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
