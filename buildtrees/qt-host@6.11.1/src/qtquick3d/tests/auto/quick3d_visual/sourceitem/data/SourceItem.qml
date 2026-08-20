import QtQuick
import QtQuick3D

Item {
    id: rootItem
    anchors.fill: parent

    Rectangle {
        id: redRect
        width: 256
        height: 256
        color: Qt.rgba(1, 0, 0, 1)
    }

    // We’re using a ShaderEffectSource to ensure we test that source
    // items with cleanup jobs are handled correctly as well.
    ShaderEffectSource {
        id: redRectShaderEffect
        width: 256
        height: 256
        sourceItem: redRect
    }

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }
        PerspectiveCamera { z: 600 }

        Node {
            id: sceneRoot
            objectName: "sceneRoot"
            SourceItemModel {
                sourceItem: redRectShaderEffect
            }
        }
    }
}
