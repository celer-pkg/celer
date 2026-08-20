import QtQuick

Item {
    width: 320
    height: 480

    Repeater {
        model: ListModel {
            ListElement {
                rt: Text.NativeRendering
                c: "blue"
                o: -10
            }
            ListElement {
                rt: Text.QtRendering
                c: "red"
                o: 0
            }
            ListElement {
                rt: Text.CurveRendering
                c: "green"
                o: 10
            }
        }

        Text {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            anchors.horizontalCenterOffset: o
            text: "Q"
            color: c
            font.family: "Times New Roman"
            font.pixelSize: 2000
            renderType: rt
            layer.enabled: true
            layer.sourceRect: Qt.rect(0, 0, implicitWidth, implicitHeight)
        }
    }
}
