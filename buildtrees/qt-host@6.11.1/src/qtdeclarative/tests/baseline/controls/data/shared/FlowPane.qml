import QtQuick
import QtQuick.Controls

Pane {
    width: Math.min(implicitWidth, 800)
    height: Math.min(implicitHeight, 800)

    default property alias content: flow.children

    Flow {
        id: flow
        anchors.fill: parent
        spacing: 5
    }
}
