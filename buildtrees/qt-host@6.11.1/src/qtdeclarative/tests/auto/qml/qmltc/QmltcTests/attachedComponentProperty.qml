import QtQuick
import QtQuick.Templates

Popup {
    Overlay.modal: Rectangle {
        color: 'grey'
    }
    Overlay.objectName: "foos"

    property Component b: Rectangle {
        color: 'green'
    }
}
