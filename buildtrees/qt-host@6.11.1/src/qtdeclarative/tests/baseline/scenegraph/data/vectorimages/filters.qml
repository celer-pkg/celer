import QtQuick
import QtQuick.VectorImage

Rectangle {
    id: topLevelItem
    width: 1000
    height: 700

    ListModel {
        id: files
        ListElement { src: "../shared/svg/extended_features/box.svg" }
        ListElement { src: "../shared/svg/extended_features/boxColor.svg" }
        ListElement { src: "../shared/svg/extended_features/fecolormatrix.svg" }
        ListElement { src: "../shared/svg/extended_features/fecolormatrixSimple.svg" }
        ListElement { src: "../shared/svg/extended_features/feoffset.svg" }
        ListElement { src: "../shared/svg/extended_features/feComposite.svg" }
        ListElement { src: "../shared/svg/extended_features/feBlend.svg" }
        ListElement { src: "../shared/svg/extended_features/femergenode.svg" }
    }

    Grid {
        columns: 4
        anchors.fill: parent
        spacing: 20
        Repeater {
            model: files
            VectorImage {
                source: src
                preferredRendererType: VectorImage.CurveRenderer
                height: 440
                width: 220
                fillMode: VectorImage.PreserveAspectFit
            }
        }
    }
}
