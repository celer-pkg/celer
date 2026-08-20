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

    MouseArea {
        anchors.fill: textArea
        acceptedButtons: Qt.RightButton
        onClicked: userContextMenu.popup()
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
