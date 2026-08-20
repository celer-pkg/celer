import QtQuick
import QtQuick.VectorImage

Rectangle{
    id: topLevelItem
    width: 1000
    height: 820

    ListModel {
        id: files
        ListElement { src: "../shared/svg/extended_features/blur.svg" }
        ListElement { src: "../shared/svg/extended_features/blur2.svg" }
        ListElement { src: "../shared/svg/extended_features/boxGauss.svg" }
        ListElement { src: "../shared/svg/extended_features/filterandmask.svg" }
    }

    Grid {
        spacing: 10
        columns: 2
        anchors.fill: parent
        Repeater {
            model: files

            VectorImage {
                source: src
                preferredRendererType: VectorImage.CurveRenderer
                width: 400
                height: 400
                fillMode: VectorImage.PreserveAspectFit
            }
        }
    }
}
