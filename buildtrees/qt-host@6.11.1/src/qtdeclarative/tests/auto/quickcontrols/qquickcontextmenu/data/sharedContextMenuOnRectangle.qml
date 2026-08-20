pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Templates as T

ApplicationWindow {
    width: 600
    height: 400

    Menu {
        id: contextMenu

        readonly property string triggerItemText: (parent as T.Label)?.text ?? ""

        MenuItem {
            text: qsTr("Eat %1").arg(contextMenu.triggerItemText)
        }
        MenuItem {
            text: qsTr("Throw %1").arg(contextMenu.triggerItemText)
        }
        MenuItem {
            text: qsTr("Squash %1").arg(contextMenu.triggerItemText)
        }
    }

    component Tomato: Label {
        id: root
        objectName: text
        horizontalAlignment: Label.AlignHCenter
        verticalAlignment: Label.AlignVCenter
        width: Math.max(200, contentWidth * 1.5, contentWidth * 1.5)
        height: width
        color: skinColor

        property color skinColor: "tomato"

        background: Rectangle {
            color: root.skinColor
            radius: width / 2
        }

        ContextMenu.menu: contextMenu
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
