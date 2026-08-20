import QtQuick
import QtQuick.VectorImage

Rectangle {
    id: topLevelItem
    width: 200
    height: 200

    ListModel {
        id: renderers
        ListElement { renderer: VectorImage.GeometryRenderer; src: "../shared/svg/animateOpacity.svg" }
        ListElement { renderer: VectorImage.CurveRenderer; src: "../shared/svg/animateOpacity.svg" }
        ListElement { renderer: VectorImage.GeometryRenderer; src: "../shared/svg/animateFillStrokeOpacity.svg" }
        ListElement { renderer: VectorImage.CurveRenderer; src: "../shared/svg/animateFillStrokeOpacity.svg" }
    }

    Grid {
        columns: 2
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
