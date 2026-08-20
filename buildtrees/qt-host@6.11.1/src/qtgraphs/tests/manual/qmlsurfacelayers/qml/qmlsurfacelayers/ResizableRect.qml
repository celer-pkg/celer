// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick

Rectangle {
    id: rect

    property color rectangleColor
    border.color: rectangleColor
    color: Qt.rgba(rectangleColor.r, rectangleColor.g, rectangleColor.b, 0.5)
    property int rulersSize: 10

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.ClosedHandCursor
        drag {
            target: parent
            minimumX: 0
            minimumY: 0
            maximumX: parent.parent.width - parent.width
            maximumY: parent.parent.height - parent.height
        }
    }
    Rectangle {
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        color: rectangleColor
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.verticalCenter

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeHorCursor
            drag{ target: parent; axis: Drag.XAxis }
            onMouseXChanged: {
                if (drag.active){
                    rect.width = rect.width - mouseX
                    rect.x = rect.x + mouseX
                    if (rect.width < 30)
                        rect.width = 30
                }
            }
        }
    }

    Rectangle {
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        color: rectangleColor
        anchors.horizontalCenter: parent.right
        anchors.verticalCenter: parent.verticalCenter

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeHorCursor
            drag{ target: parent; axis: Drag.XAxis }
            onMouseXChanged: {
                if (drag.active){
                    rect.width = rect.width + mouseX
                    if (rect.width < 50)
                        rect.width = 50
                }
            }
        }
    }

    Rectangle {
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        x: parent.x / 2
        y: 0
        color: rectangleColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.top

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeVerCursor
            drag{ target: parent; axis: Drag.YAxis }
            onMouseYChanged: {
                if (drag.active){
                    rect.height = rect.height - mouseY
                    rect.y = rect.y + mouseY
                    if (rect.height < 50)
                        rect.height = 50
                }
            }
        }
    }

    Rectangle {
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        x: parent.x / 2
        y: parent.y
        color: rectangleColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.bottom

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeVerCursor
            drag{ target: parent; axis: Drag.YAxis }
            onMouseYChanged: {
                if (drag.active){
                    rect.height = rect.height + mouseY
                    if (rect.height < 50)
                        rect.height = 50
                }
            }
        }
    }
}
