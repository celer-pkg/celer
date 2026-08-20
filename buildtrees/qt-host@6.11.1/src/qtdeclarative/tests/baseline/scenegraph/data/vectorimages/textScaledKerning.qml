import QtQuick
import QtQuick.VectorImage
import Qt.labs.folderlistmodel

Rectangle{
    id: topLevelItem
    width: 800
    height: 200

    Row {
        anchors.fill: parent
        Repeater {
            model: [ VectorImage.GeometryRenderer, VectorImage.CurveRenderer ]
            VectorImage {
                width: parent.width / 2
                height: parent.height
                source: "../shared/svg/text_scaled_kerning.svg"
                clip: true
                preferredRendererType: modelData
            }
        }
    }
}
