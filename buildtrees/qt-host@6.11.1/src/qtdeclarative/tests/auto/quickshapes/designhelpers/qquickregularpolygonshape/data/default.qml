import QtQuick
import QtQuick.Shapes.DesignHelpers

Item {
    id: root
    width: 256
    height: 256

    RegularPolygonShape {
        objectName: "shape"
        anchors.centerIn: parent
    }
}
