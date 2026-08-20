// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [file]
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Templates as T

ApplicationWindow {
    width: 600
    height: 400
    visible: true

    component Tomato: Label {
        id: tomato
        objectName: text
        horizontalAlignment: Label.AlignHCenter
        verticalAlignment: Label.AlignVCenter
        width: Math.max(200, contentWidth * 1.5, contentWidth * 1.5)
        height: width
        color: skinColor

        function eat() { print("Ate " + text) }
        function ditch() { print("Threw " + text) }
        function squash() { print("Squashed " + text) }

        property color skinColor: "tomato"

        background: Rectangle {
            color: tomato.skinColor
            radius: width / 2
        }

        ContextMenu.menu: contextMenu
    }

    Menu {
        id: contextMenu

        readonly property Tomato triggerItem: parent as Tomato
        readonly property string triggerItemText: triggerItem?.text ?? ""

        MenuItem {
            text: qsTr("Eat %1").arg(contextMenu.triggerItemText)
            onTriggered: contextMenu.triggerItem.eat()
        }
        MenuItem {
            text: qsTr("Throw %1").arg(contextMenu.triggerItemText)
            onTriggered: contextMenu.triggerItem.ditch()
        }
        MenuItem {
            text: qsTr("Squash %1").arg(contextMenu.triggerItemText)
            onTriggered: contextMenu.triggerItem.squash()
        }
    }

    Row {
        anchors.centerIn: parent

        Tomato {
            text: qsTr("tomato")
        }

        Tomato {
            text: qsTr("really ripe tomato")
            skinColor: "maroon"
        }
    }
}
//! [file]
