import QtQuick
import QtQuick.Shapes.DesignHelpers

Window {
    width: rectangleShape.width + 10
    height: rectangleShape.height + 10

    property alias rectangleShape: rectangleShape

    RectangleShape {
        id: rectangleShape
        width: 120
        height: 90
        anchors.centerIn: parent
    }
}
