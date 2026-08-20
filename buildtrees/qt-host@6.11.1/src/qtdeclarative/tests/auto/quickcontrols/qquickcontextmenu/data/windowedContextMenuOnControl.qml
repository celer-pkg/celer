import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: win
    width: 600
    height: 400

    signal triggered(QtObject item)

    Control {
        objectName: "tomato"
        x: 100
        y: 100
        width: 100
        height: 100

        ContextMenu.menu: Menu {
            popupType: Popup.Window
            MenuItem {
                text: qsTr("Eat tomato")
                onTriggered: win.triggered(this)
            }
            MenuItem {
                text: qsTr("Throw tomato")
                onTriggered: win.triggered(this)
            }
            MenuItem {
                text: qsTr("Squash tomato")
                onTriggered: win.triggered(this)
            }
        }

        background: Rectangle {
            color: "tomato"
            radius: width / 2
        }
    }
}
