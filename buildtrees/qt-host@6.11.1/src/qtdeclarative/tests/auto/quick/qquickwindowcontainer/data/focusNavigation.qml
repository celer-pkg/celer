import QtQuick

Window {
    id: topLevel
    width: 300
    height: 400
    visible: true
    color: "orange"

    Row {
        anchors.bottom: parent.bottom
        spacing: 10
        Rectangle {
            objectName: "rect1"
            width: 100
            height: 30
            border.width: 2
            border.color: activeFocus ? "blue" : "black"
            focusPolicy: Qt.TabFocus
        }
        Rectangle {
            objectName: "rect2"
            width: 100
            height: 30
            border.width: 2
            border.color: activeFocus ? "blue" : "black"
            focusPolicy: Qt.TabFocus
        }
    }

    WindowContainer {
        window: Window {
            objectName: "childWindow"
            width: 300
            height: 200
            visible: true
            color: "cyan"

            Row {
                anchors.bottom: parent.bottom
                spacing: 10
                Rectangle {
                    objectName: "rect1embedded"
                    width: 100
                    height: 30
                    border.width: 2
                    border.color: activeFocus ? "blue" : "black"
                    focusPolicy: Qt.TabFocus
                }
                Rectangle {
                    objectName: "rect2embedded"
                    width: 100
                    height: 30
                    border.width: 2
                    border.color: activeFocus ? "blue" : "black"
                    focusPolicy: Qt.TabFocus
                }
            }
        }
    }
}
