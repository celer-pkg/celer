import QtQuick
import QtQuick.Shapes.DesignHelpers

Item {
    id: root
    width: 256
    height: 256

    StarShape {
        objectName: "shape"
        anchors.centerIn: parent
    }
}
