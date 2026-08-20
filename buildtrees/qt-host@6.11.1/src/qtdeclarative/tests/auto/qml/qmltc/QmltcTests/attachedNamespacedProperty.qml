import QtQuick as Q
import QtQuick.Templates as T

T.Popup {
    T.Overlay.modal: Q.Rectangle {
        color: 'grey'
    }
    T.Overlay.objectName: "foos"
}
