// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import ToDoList

Item {
    id: root

    implicitWidth: 52
    implicitHeight: 80

    property int radius: 8
    property bool isAm: true
    property color rectColor: "#41CD52"

    Item {
        id: greenArea

        width: root.implicitWidth
        height: root.implicitHeight / 2

        Rectangle {
            id: mainRect

            implicitWidth: greenArea.width
            implicitHeight: (greenArea.height - root.radius)
            anchors.top: greenArea.top
            anchors.topMargin: root.radius
            color: root.rectColor
        }

        Rectangle {
            id: roundRect

            implicitWidth: greenArea.width
            implicitHeight: root.radius * 2
            anchors.top: greenArea.top
            anchors.horizontalCenter: greenArea.horizontalCenter
            radius: root.radius
            color: root.rectColor
        }

        MouseArea {
            anchors.fill: greenArea
        }
    }

    Rectangle {
        id: bigRect

        anchors.fill: root
        radius: 8
        color: "transparent"
        border.color: Constants.secondaryColor
    }

    Column {
        id: col

        anchors.centerIn: root
        spacing: 8

        Label {
            text: "AM"
            color: Constants.secondaryColor
            font.pixelSize: 16
            anchors.horizontalCenter: col.horizontalCenter
        }

        Rectangle {
            id: separator

            height: 1
            width: root.width
            color: Constants.secondaryColor
        }

        Label {
            text: "PM"
            color: Constants.secondaryColor
            font.pixelSize: 16
            anchors.horizontalCenter: col.horizontalCenter
        }
    }

    TapHandler {
        gesturePolicy: TapHandler.WithinBounds
        onTapped: root.isAm = !root.isAm
    }

    states: [
        State {
            name: "am"
            when: root.isAm
            PropertyChanges {
                greenArea.y: 0
                greenArea.rotation: 0
            }
        },
        State {
            name: "pm"
            when: !root.isAm
            PropertyChanges {
                greenArea.y: root.height / 2
                greenArea.rotation: 180
            }
        }
    ]
}
