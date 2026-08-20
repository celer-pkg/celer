import QtQuick

Window {
    // Attach to window
    property var margins: SafeArea.margins
    property real marginsTop: SafeArea.margins.top
    property real marginsLeft: SafeArea.margins.left
    property real marginsRight: SafeArea.margins.right
    property real marginsBottom: SafeArea.margins.bottom

    Item {
        // Attach to item
        objectName: "item"
        property var margins: SafeArea.margins
        property real marginsTop: SafeArea.margins.top
        property real marginsLeft: SafeArea.margins.left
        property real marginsRight: SafeArea.margins.right
        property real marginsBottom: SafeArea.margins.bottom
    }
}
