import QtQuick
import QtQuick3D

Rectangle {
    width: 640
    height: 480
    color: "white"

    View3D {
        id: viewport
        anchors.fill: parent

        property real myScale: 0.01

        Node {
            PerspectiveCamera {
                position: Qt.vector3d(0, 0, 10)
                clipFar: 11
                clipNear: 0.001
            }

            DirectionalLight {
                eulerRotation.x: -45
                eulerRotation.y: 45
                brightness: 1
            }

            Node {
                Model {
                    source: "#Cube"
                    materials: PrincipledMaterial {
                        baseColor: "red"
                    }
                    scale: Qt.vector3d(viewport.myScale, viewport.myScale, viewport.myScale)
                }
                eulerRotation.y: 240
            }
        }
    }
}
