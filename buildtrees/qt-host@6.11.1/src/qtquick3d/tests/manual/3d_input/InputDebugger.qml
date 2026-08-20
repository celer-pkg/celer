// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Window

Rectangle {
    height: 256
    width: 256

    color: "white"
    border.color: "black"
    border.width: 5

    MouseArea {
        objectName: "background rgb dot controller"
        anchors.fill: parent
        cursorShape: Qt.CrossCursor
        onPressed: function(mouse) {
            debugLabel.text = "(" + Math.floor(mouse.x) + ", " + Math.floor(mouse.y) + ")"
            debugPoint.visible = true
            debugPoint.x = mouse.x - 10
            debugPoint.y = mouse.y - 10
            debugPoint.color = "red"
        }
        onReleased: function(mouse) {
            debugLabel.text = "(" + Math.floor(mouse.x) + ", " + Math.floor(mouse.y) + ")"
            debugPoint.visible = true
            debugPoint.x = mouse.x - 10
            debugPoint.y = mouse.y - 10
            debugPoint.color = "green"
        }
        onPositionChanged: function(mouse) {
            debugLabel.text = "(" + Math.floor(mouse.x) + ", " + Math.floor(mouse.y) + ")"
            debugPoint.visible = true
            debugPoint.x = mouse.x - 10
            debugPoint.y = mouse.y - 10
            debugPoint.color = "blue"
        }
    }

    Rectangle {
        id: debugPoint
        height: 20
        width: 20
        radius: 10
        color: "red"
        visible: false
    }

    ColumnLayout {
        anchors.centerIn: parent
        Label {
            id: debugLabel
            text: "no input"
        }
        Button {
            text: "Click Me"
            onClicked: {
                debugLabel.text("clicked 'Me'")
            }
        }
        Rectangle {
            width: 100; height: 40
            color: btap.pressed ? "tomato" : bhover.hovered ? "wheat" : "beige"
            HoverHandler {
                id: bhover
                cursorShape: Qt.OpenHandCursor
            }
            TapHandler {
                id: btap
                objectName: "me too"
                onTapped: btxt.text = "clicked @ " + point.position.x.toFixed(1) + ", " + point.position.y.toFixed(1)
            }
            Text {
                id: btxt
                anchors.centerIn: parent
                text: "click me too"
            }
        }
        TextInput {
            id: textInput
            text: "Some Text"
            focus: true
        }
        Text {
            text: "text cursor pos " + textInput.cursorPosition
        }
        Text {
            Layout.maximumWidth: 220
            elide: Text.ElideRight
            text: "focus " + Window.activeFocusItem
        }
    }
}
