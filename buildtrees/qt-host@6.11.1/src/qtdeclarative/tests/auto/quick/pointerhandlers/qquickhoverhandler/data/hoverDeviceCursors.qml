import QtQuick

Item {
    width: 200; height: 200

    Row {
        objectName: "indicators"
        Text {
            text: "🖊"
            visible: stylus.hovered
        }
        Text {
            text: "✐"
            visible: stylusEraser.hovered
        }
        Text {
            text: "🖌"
            visible: airbrush.hovered
        }
        Text {
            text: "⌫"
            visible: airbrushEraser.hovered
        }
        Text {
            text: "🖯"
            visible: mouse.hovered
        }
        Text {
            text: "🖰"
            visible: conflictingMouse.hovered
        }
    }

    Rectangle {
        width: 100; height: 100
        anchors.centerIn: parent
        border.color: "black"

        HoverHandler {
            id: stylus
            objectName: "stylus"
            acceptedDevices: PointerDevice.Stylus
            acceptedPointerTypes: PointerDevice.Pen
            cursorShape: Qt.CrossCursor
        }

        HoverHandler {
            id: stylusEraser
            objectName: "stylus eraser"
            acceptedDevices: PointerDevice.Stylus
            acceptedPointerTypes: PointerDevice.Eraser
            cursorShape: Qt.PointingHandCursor
        }

        HoverHandler {
            id: airbrush
            objectName: "airbrush"
            acceptedDevices: PointerDevice.Airbrush
            acceptedPointerTypes: PointerDevice.Pen
            cursorShape: Qt.BusyCursor
        }

        HoverHandler {
            id: airbrushEraser
            objectName: "airbrush eraser"
            acceptedDevices: PointerDevice.Airbrush
            acceptedPointerTypes: PointerDevice.Eraser
            cursorShape: Qt.OpenHandCursor
        }

        HoverHandler {
            id: mouse
            objectName: "mouse"
            acceptedDevices: PointerDevice.Mouse
            // acceptedPointerTypes can be omitted because Mouse is not ambiguous.
            // When a genuine mouse move is sent, there's a conflict, and this one should win.
            cursorShape: Qt.IBeamCursor
        }

        HoverHandler {
            id: conflictingMouse
            objectName: "conflictingMouse"
            acceptedDevices: PointerDevice.Mouse
            // acceptedPointerTypes can be omitted because Mouse is not ambiguous.
            // When a genuine mouse move is sent, there's a conflict, and this one should lose.
            cursorShape: Qt.ClosedHandCursor
        }
    }
}
