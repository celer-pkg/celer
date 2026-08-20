// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls

Rectangle {
    id: secondaryRectangle

    property int gridRotation: 0

    color: "blue"

    Text {
        id: title

        text: "Second QML View"
        color: "white"
        font.pointSize: 72
        font.bold: true
        wrapMode: Text.WordWrap
        width: secondaryRectangle.width
        horizontalAlignment: Text.AlignHCenter

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: secondaryRectangle.height / 40
        }
    }

    Text {
        id: gridText

        text: "QML Grid type"
        font.pointSize: 48
        color: "white"
        width: secondaryRectangle.width
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        anchors {
            top: title.bottom
            topMargin: secondaryRectangle.height / 10
            horizontalCenter: parent.horizontalCenter
        }
    }

    Grid {
        id: grid

        columns: 3
        rows: 3
        spacing: secondaryRectangle.height / 15
        rotation: gridRotation

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: gridText.bottom
            topMargin: secondaryRectangle.height / 10
        }

        Repeater {
            id: repeater

            model: [
                "green",
                "lightblue",
                "grey",
                "red",
                "black",
                "white",
                "pink",
                "yellow",
                "orange"
            ]

            Rectangle {
                required property string modelData

                height: secondaryRectangle.height / 15
                width: height
                color: modelData
            }
        }
    }
}
