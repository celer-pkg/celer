import QtQuick
import QtQuick.VectorImage
import Qt.labs.folderlistmodel
Rectangle{
    id: topLevelItem
    width: 650
    height: 900
    Column {
        anchors.fill: parent
        spacing: 20
        Repeater {
            model: FolderListModel {
                folder: Qt.resolvedUrl("../shared/svg/extended_features/")
                nameFilters: [ "mask*.svg"]
            }
            Item {
                width: 650
                height: childrenRect.height
                VectorImage {
                    width: 300
                    height: implicitHeight * width / implicitWidth
                    source: fileUrl
                    clip: true
                    preferredRendererType: VectorImage.GeometryRenderer
                }
                VectorImage {
                    x: 350
                    width: 300
                    height: implicitHeight * width / implicitWidth
                    source: fileUrl
                    clip: true
                    preferredRendererType: VectorImage.CurveRenderer
                }
            }
        }
    }
}