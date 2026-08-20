import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

View3D {
    id: view
    objectName: "view"
    anchors.fill: parent
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "black"
    }

    Node {
        id: originNode
        PerspectiveCamera {
            id: cameraNode
            z: 600
        }
    }

    DirectionalLight {
        ambientColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
    }
    Model {
        id: model1
        objectName: "model1"
        source: "#Cube"
        pickable: true
        position: Qt.vector3d(50.0, 50.0, -50.0)
        materials: PrincipledMaterial {
            baseColor: "green"
        }
    }

//    Model {
//        source: "#Sphere"
//        z: -100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "pink"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }

//    Model {
//        source: "#Sphere"
//        y: 100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "pink"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }

//    Model {
//        source: "#Sphere"
//        z: -100
//        y: 100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "yellow"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }
//    Model {
//        source: "#Sphere"
//        x: 100
//        y: 100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "yellow"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }

//    Model {
//        source: "#Sphere"
//        z: -100
//        x: 100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "orange"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }

//    Model {
//        source: "#Sphere"
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "orange"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }

//    Model {
//        source: "#Sphere"
//        z: -100
//        x: 100
//        y: 100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "white"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }

//    Model {
//        source: "#Sphere"
//        z: 100
//        scale: Qt.vector3d(0.02, 0.02, 0.02)
//        materials: PrincipledMaterial {
//            baseColor: "white"
//            lighting: PrincipledMaterial.NoLighting
//        }
//    }


    OrbitCameraController {
        camera: cameraNode
        origin: originNode
    }
}
