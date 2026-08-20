import QtQuick
import QtQuick.Controls

Item {
    width: 400
    height: 400

    property alias textArea: textArea
    property alias userContextMenu: userContextMenu

    TextArea {
        id: textArea
        anchors.fill: parent
    }

    Item {
        anchors.fill: textArea

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: userContextMenu.popup()
        }
    }

    Menu {
        id: userContextMenu
        objectName: "userContextMenu"

        MenuItem {
            text: qsTr("Font...")
        }

        MenuItem {
            text: qsTr("Color...")
        }
    }
}
