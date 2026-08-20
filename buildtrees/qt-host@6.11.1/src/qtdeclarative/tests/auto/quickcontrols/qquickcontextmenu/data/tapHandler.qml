import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 600
    height: 400

    signal contextMenuOpened

    contentItem.ContextMenu.menu: Menu {
        objectName: "menu"
        onOpened: window.contextMenuOpened()

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

    TapHandler {
        objectName: "tapHandler"
        acceptedButtons: Qt.RightButton
        // Ensure that it grabs mouse on press, as the default gesture policy results in a passive grab.
        gesturePolicy: TapHandler.WithinBounds
    }
}
