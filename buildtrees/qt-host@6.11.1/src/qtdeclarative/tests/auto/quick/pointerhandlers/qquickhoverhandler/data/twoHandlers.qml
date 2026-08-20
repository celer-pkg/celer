import QtQuick

Row {
    objectName: "parentitem"
    width: 320
    height: 240
    spacing: 4
    Rectangle {
        objectName: "left"
        width: 158
        height: 160
        color: hh1.hovered ? "blue" : "lightgray"
        HoverHandler {
            id: hh1
            objectName: "left"
        }
    }
    Rectangle {
        objectName: "right"
        width: 158
        height: 160
        color: hh2.hovered ? "blue" : "lightgray"
        HoverHandler {
            id: hh2
            objectName: "right"
        }
    }
}
