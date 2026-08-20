import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 600
    height: 400

    Drawer {
        width: 100
        height: window.height
    }

    TextField {
        text: "Test"
        anchors.centerIn: parent

        ContextMenu.menu: Menu {
            MenuItem {
                text: "Cut"
            }
        }
    }
}
