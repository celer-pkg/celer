import QtQuick
import QtQuick.VectorImage

Rectangle { 
    id: topLevelItem
    width: 800
    height: 200

    ListModel {
        id: renderers
        ListElement { renderer: VectorImage.GeometryRenderer; src: "../shared/svg/animateRotateTransform.svg" }
        ListElement { renderer: VectorImage.CurveRenderer; src: "../shared/svg/animateRotateTransform.svg" }
        ListElement { renderer: VectorImage.GeometryRenderer; src: "../shared/svg/animateRotateTransformRemove.svg" }
        ListElement { renderer: VectorImage.CurveRenderer; src: "../shared/svg/animateRotateTransformRemove.svg" }		
    }

    Row {
        Repeater {
            model: renderers

            VectorImage {
                layer.enabled: true
                layer.samples: 4
                source: src
                preferredRendererType: renderer
            }
        }
    }
}
