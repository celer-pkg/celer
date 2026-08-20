import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: mainWindow
    width: 400
    height: 400
    visible: true

    View3D {
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#404040"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 150)
            clipFar: 2000
        }

        DirectionalLight {
            brightness: 50
            eulerRotation: Qt.vector3d(-90, -90, 0)
            castsShadow: true
            shadowFactor: 25
            shadowMapQuality: Light.ShadowMapQualityHigh
        }

        Model {
            source: "#Rectangle"
            eulerRotation: Qt.vector3d(-90, 0, 0)
            y: -30
            scale: Qt.vector3d(3, 3, 1)
            receivesShadows: true
            materials: [
                DefaultMaterial {
                    diffuseColor: "#0c100c"
                }
            ]
        }

        Model
        {
            source: "#Cube"
            castsShadows: true
            receivesShadows: false
            position: Qt.vector3d(1, 1, 0)
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(1.0, 0.9, 0.7, 1.0)
                }
            ]
        }
    }
}
