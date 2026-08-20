import QtQuick
import QtQuick.Controls

Item {
    width: 400
    height: 400

    property alias textArea: textArea
    property alias userContextMenu: userContextMenu
    property alias ourContextMenu: ourContextMenu

    TextArea {
        id: textArea
        anchors.fill: parent

        ContextMenu.menu: Menu {
            id: ourContextMenu
            objectName: "ourContextMenu"
            popupType: Popup.Item

            MenuItem {
                text: "ContextMenu menu item"
            }
        }

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
