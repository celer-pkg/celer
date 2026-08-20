import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 600
    height: 400
    // visible: true // for manual testing

    Rectangle {
        objectName: "tomato"
        x: 100
        y: 100
        width: 100
        height: 100
        color: "tomato"
        radius: width / 2

        ContextMenu.menu: Menu {
            MenuItem {
                text: qsTr("Eat tomato")
            }
            MenuItem {
                text: qsTr("Throw tomato")
            }
            MenuItem {
                text: qsTr("Squash tomato")
            }
        }
    }
}
