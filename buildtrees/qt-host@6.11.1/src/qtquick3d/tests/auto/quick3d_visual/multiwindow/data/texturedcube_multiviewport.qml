import QtQuick
import QtQuick3D

Item {
    anchors.fill: parent

    Rectangle {
        id: contentSource
        color: "red"
        width: 256; height: 256
    }

    function changeToSourceItemBasedTexture() {
        material.baseColorMap.sourceItem = contentSource;
    }

    Node {
        id: scene
        DirectionalLight { }
        Model {
            source: "#Cube"
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation.x: 30
            materials: PrincipledMaterial {
                id: material
                baseColorMap: Texture {
                    source: "qt_logo_rect.png"
                }
            }
        }
    }

    View3D {
        width: parent.width / 2
        height: parent.height / 2
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }
        importScene: scene
        PerspectiveCamera { z: 600 }
    }

    View3D {
        x: parent.width / 2
        width: parent.width / 2
        height: parent.height / 2
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "red"
        }
        importScene: scene
        PerspectiveCamera { z: 600 }
    }

    View3D {
        y: parent.height / 2
        width: parent.width / 2
        height: parent.height / 2
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "green"
        }
        importScene: scene
        PerspectiveCamera { z: 600 }
    }

    View3D {
        x: parent.width / 2
        y: parent.height / 2
        width: parent.width / 2
        height: parent.height / 2
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "blue"
        }
        importScene: scene
        PerspectiveCamera { z: 600 }
    }
}
