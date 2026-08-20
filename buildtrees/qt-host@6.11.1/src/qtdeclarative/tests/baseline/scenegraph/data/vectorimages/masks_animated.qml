import QtQuick
import QtQuick.VectorImage

Rectangle {
    id: topLevelItem
    width: 350
    height: 500

    ListModel {
        id: renderers
        ListElement { renderer: VectorImage.GeometryRenderer }
        ListElement { renderer: VectorImage.CurveRenderer }
    }

    ListModel {
        id: files
        ListElement { src: "../shared/svg/mask_animation_translate.svg" }
		ListElement { src: "../shared/svg/mask_animation_rotate.svg" }
		ListElement { src: "../shared/svg/mask_animation_scale.svg" }
    }

    Column {
        spacing: 40
        anchors.fill: parent
        Repeater {
            model: renderers

            Row {
                spacing: 20
                height: 200

                Repeater {
                    model: files

                    VectorImage {
						clip: true
                        source: src
                        preferredRendererType: renderer
                        width: 100
                        height: 400
                    }
                }
            }
        }
    }
}
