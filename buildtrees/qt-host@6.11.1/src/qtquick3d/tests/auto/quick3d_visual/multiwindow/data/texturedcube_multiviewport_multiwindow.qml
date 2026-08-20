import QtQuick
import QtQuick3D

Window {
    visible: true
    width: 640; height: 480

    Rectangle {
        id: contentSource
        visible: false
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
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }
        importScene: scene
        PerspectiveCamera { z: 600 }
    }

    Window {
        objectName: "window2"
        visible: true
        width: 320; height: 240

        View3D {
            anchors.fill: parent
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Color
                clearColor: "black"
            }
            importScene: scene
            PerspectiveCamera { z: 600 }
        }
    }

    Timer {
        running: true
        interval: 3000
        repeat: false
        onTriggered: changeToSourceItemBasedTexture()
    }
}
