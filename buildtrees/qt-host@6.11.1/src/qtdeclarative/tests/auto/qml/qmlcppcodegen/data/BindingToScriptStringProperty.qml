pragma Strict
import QtQuick

Item {
    id: root
    height: 300
    property bool c: false

    Rectangle {
        id: top
        objectName: "top"
        anchors.top: root.top
        anchors.bottom: bottom.top
    }
    Rectangle {
        id: bottom
        objectName: "bottom"
        anchors.top: root.verticalCenter
        anchors.bottom: root.bottom
    }
    states: State {
        when: root.c
        AnchorChanges {
            target: bottom
            anchors.top: root.bottom
        }
    }
}
