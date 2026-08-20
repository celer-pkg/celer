import QtQuick
import QtQuick3D

Window {
    visible: true
    width: 640
    height: 480

    function reparentView() {
        scene.parent = item2;
    }

    Item {
        id: item1
        width: 640
        height: 480

        View3D {
            anchors.fill: parent
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Color
                clearColor: "black"
            }

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
            PerspectiveCamera { z: 600 }
        }
    }

    Window {
        objectName: "window2"
        visible: true
        width: 640
        height: 480

        Item {
            id: item2
            width: 640
            height: 480
        }
    }
}
