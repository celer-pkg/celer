import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 600
    height: 400

    Pane {
        anchors.fill: parent

        ContextMenu.menu: Menu {
            id: uhOh

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
