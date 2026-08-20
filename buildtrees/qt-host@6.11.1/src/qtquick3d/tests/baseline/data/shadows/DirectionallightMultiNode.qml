import QtQuick
import QtQuick3D

Item
{
    id: window
    width: 800
    height: 600
    visible: true

    Node
    {
        id: sceneRoot

        DirectionalLight
        {
            color: Qt.rgba(1.0, 1.0, 1.0, 1.0)
            castsShadow: true
            brightness: 2.0
            position: Qt.vector3d(0, 300, 0)
            eulerRotation: Qt.vector3d(-45, 20, 0)
            visible: true
            shadowFactor: 100
            shadowMapQuality: Light.ShadowMapQualityHigh
        }

        PerspectiveCamera
        {
            id: mainCamera
            position: Qt.vector3d(0, 500, 500)
            eulerRotation: Qt.vector3d(-40, -10, 0)
            clipFar: 50000
            clipNear: 10
        }

        Model
        {
            source: "#Cube"
            castsShadows: true
            receivesShadows: false
            position: Qt.vector3d(100, 100, 0)
            scale: Qt.vector3d(1, 1, 1)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(1.0, 0.9, 0.7, 1.0)
                }
            ]
        }

        Model
        {
            source: "#Cube"
            castsShadows: true
            receivesShadows: false
            position: Qt.vector3d(-100, 100, 0)
            scale: Qt.vector3d(1, 1, 1)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(1.0, 0.9, 0.7, 1.0)
                }
            ]
        }
    }

    Item
    {
        width: parent.width
        height: parent.height

        View3D
        {
            anchors.fill: parent
            id: view
            camera: mainCamera
            importScene: sceneRoot

            Model
            {
                source: "#Rectangle"
                castsShadows: true
                receivesShadows: true
                scale: Qt.vector3d(10, 10, 10)
                position: Qt.vector3d(0, 0, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)

                materials: [
                    DefaultMaterial {
                        diffuseColor: Qt.rgba(0.1, 0.8, 0.1, 1.0)
                    }
                ]
            }
        }
    }
}
