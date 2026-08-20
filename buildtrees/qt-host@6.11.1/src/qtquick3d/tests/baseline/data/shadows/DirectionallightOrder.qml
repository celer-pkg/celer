import QtQuick3D
import QtQuick

Item {
    visible: true
    width: 1280
    height: 480
    View3D {
        width: parent.width/2
        height: parent.height

        PerspectiveCamera {
            z: 500
        }

        DirectionalLight {
            eulerRotation.x: 45
            eulerRotation.y: -45
            castsShadow: true
            shadowFactor: 75
            brightness: 0.75
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 0.75
        }

        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "lightgreen"
            }
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(5, 0.1, 5)
            position: Qt.vector3d(0, 100, 0)
            materials: PrincipledMaterial {
                baseColor: "lightgreen"
            }
        }
    }
    View3D {
        width: parent.width/2
        height: parent.height
        x: parent.width/2

        PerspectiveCamera {
            z: 500
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 0.75
        }

        DirectionalLight {
            eulerRotation.x: 45
            eulerRotation.y: -45
            castsShadow: true
            shadowFactor: 75
            brightness: 0.75
        }

        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "lightgreen"
            }
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(5, 0.1, 5)
            position: Qt.vector3d(0, 100, 0)
            materials: PrincipledMaterial {
                baseColor: "lightgreen"
            }
        }
    }
}
