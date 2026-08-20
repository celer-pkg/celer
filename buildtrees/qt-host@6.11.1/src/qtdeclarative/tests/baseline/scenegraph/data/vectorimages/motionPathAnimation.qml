import QtQuick
import QtQuick.VectorImage

Rectangle { 
    id: topLevelItem
    width: 800
    height: 900

    ListModel {
        id: files
        ListElement { src: "../shared/svg/animateMotionPath.svg" }
        ListElement { src: "../shared/svg/animateComplexMotionPath.svg" }
        ListElement { src: "../shared/svg/animateMotionPathPauseAtStart.svg" }
    }

    Grid {
        columns: 2
        anchors.fill: parent
        Repeater {
            model: files
            VectorImage {
                source: src
                preferredRendererType: VectorImage.CurveRenderer
            }
        }
    }
}
