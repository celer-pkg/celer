import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 600
    height: 400

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
