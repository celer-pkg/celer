import QtQuick
import QtQuick.VectorImage

Rectangle {
    id: topLevelItem
    width: 580
    height: 240

    ListModel {
        id: renderers
        ListElement { renderer: VectorImage.GeometryRenderer }
        ListElement { renderer: VectorImage.CurveRenderer }
    }

    ListModel {
        id: files
        ListElement { src: "../shared/svg/mask.svg" }
        ListElement { src: "../shared/svg/mask_rect.svg" }
        ListElement { src: "../shared/svg/mask_rect_duplicate.svg" }
        ListElement { src: "../shared/svg/mask_content_units.svg" }
        ListElement { src: "../shared/svg/mask_userunits.svg" }
    }

    Column {
        spacing: 40
        anchors.fill: parent
        Repeater {
            model: renderers

            Row {
                spacing: 20
                height: 100

                Repeater {
                    model: files

                    VectorImage {
                        source: src
                        preferredRendererType: renderer
                        width: 100
                        height: 100
                    }
                }
            }
        }
    }
}
