import QtQuick

// Test case for QTBUG-139561: crash in deliverHoverEventRecursive
// when layer.enabled is bound to containsMouse during hover processing
Item {
    id: root
    width: 400
    height: 300

    Rectangle {
        id: testRect
        objectName: "testRect"
        x: 100
        y: 100
        width: 200
        height: 100
        color: mouseArea.containsMouse ? "wheat" : "lightSteelBlue"

        // This binding causes the crash - layer changes during hover processing
        layer.enabled: mouseArea.containsMouse
        MouseArea {
            id: mouseArea
            objectName: "testMouseArea"
            anchors.fill: parent
            hoverEnabled: true
        }

        Text {
            anchors.centerIn: parent
            text: "Hover me!"
            color: "white"
        }
    }
}
