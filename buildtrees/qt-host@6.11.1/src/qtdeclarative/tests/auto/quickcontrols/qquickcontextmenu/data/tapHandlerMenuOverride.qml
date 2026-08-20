import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 600
    height: 400

    TextArea {
        id: textArea
        objectName: "textArea"
        text: qsTr("Yes! We have no tomatoes\nOnly bananas and potatoes")

        anchors.fill: parent
        anchors.margins: 20

        // TextArea has a built-in context menu, but we'll open a custom one instead
        TapHandler {
            acceptedButtons: Qt.RightButton
            onPressedChanged: if (pressed) windowMenu.popup()
        }
    }

    Menu {
        id: windowMenu
        objectName: "windowMenu"

        MenuItem {
            text: qsTr("Eat banana")
        }
        MenuItem {
            text: qsTr("Fry potato")
        }
        MenuItem {
            text: qsTr("Boil potato")
        }
    }
}
