import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 600
    height: 400

    Label {
        objectName: "tomato"
        x: 100
        y: 100
        width: 100
        height: 100
        text: "tomato"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

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

        background: Rectangle {
            color: "tomato"
            radius: width / 2
        }
    }
}
