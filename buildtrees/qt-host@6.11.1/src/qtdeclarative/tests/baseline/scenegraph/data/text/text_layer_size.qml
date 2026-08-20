import QtQuick

Item {
    width: 200
    height: 110
    Item {
        implicitWidth: 100
        implicitHeight: 100

        scale: 10

        Item {
            implicitWidth: 100
            implicitHeight: 100

            Text {
                text: "f"
                anchors.centerIn: parent
            }
        }

        Item {
            x: 10
            implicitWidth: 100
            implicitHeight: 100
            layer.enabled: true
            layer.textureSize: Qt.size(1000, 1000)

            Text {
                text: "f"
                anchors.centerIn: parent
            }
        }
    }
}
