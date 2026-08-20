import QtQuick
import QtQuick3D

Item {
    width: 400
    height: 400
    visible: true

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#808080"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-600, 500, 1500)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            clipFar: 5000
        }

        SpotLight {
            color: Qt.rgba(1.0, 0.9, 0.7, 1.0)
            ambientColor: Qt.rgba(0.0, 0.0, 0.0, 0.0)
            position: Qt.vector3d(0, 250, 0)
            eulerRotation.x: -45
            eulerRotation.y: 90
            shadowMapFar: 2000
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
            visible: true
            castsShadow: true
            brightness: 50
            coneAngle: 150
            innerConeAngle: 100
            shadowBias: 20
        }

        Model {
            source: "#Cube"
            x: -300
            scale: Qt.vector3d(1, 10, 1)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(0.8, 0.6, 0.4, 1.0)
                }
            ]
            receivesShadows: true
        }

        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(30, 30, 30)
            eulerRotation.x: -90
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(0.8, 0.6, 0.4, 1.0)
                }
            ]
        }

        Model {
            source: "#Rectangle"
            z: -400
            scale: Qt.vector3d(30, 30, 30)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(0.8, 0.8, 0.9, 1.0)
                }
            ]
        }

        Model {
            source: "#Sphere"
            position: Qt.vector3d(-600, 0, 0)
            scale: Qt.vector3d(3, 3, 5)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(0.9, 0.9, 0.9, 1.0)
                }
            ]
            receivesShadows: true
        }
    }
}