import QtQuick

Window {
    id: root
    width: 640
    height: 480
    visible: true

    Column {
        Text {
            text: qsTr("text with positional arguments is allowed $1 %2").arg(root.width).arg(root.height)
        }
        Text {
            text: qsTr(`templated literal without arguments is allowed`)
        }
        Text {
            text: `${qsTr("translate command inside templated literal is allowed")}`
        }
        Text {
            text: qsTr(`templated literal with arguments is not allowed ${root.width}, ${root.height}`)
        }
    }
}
