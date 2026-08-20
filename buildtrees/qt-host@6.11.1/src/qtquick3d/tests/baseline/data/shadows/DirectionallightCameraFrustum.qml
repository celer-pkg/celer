import QtQuick
import QtQuick3D

Item
{
    id: window
    width: 800
    height: 600
    visible: true

    Item
    {
        width: parent.width
        height: parent.height

        View3D
        {
            anchors.fill: parent
            id: view
            camera: mainCamera

            Model
            {
                source: "#Rectangle"
                castsShadows: true
                receivesShadows: true
                scale: Qt.vector3d(10000, 10000, 10000)
                position: Qt.vector3d(100, 0, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)

                materials: [
                    DefaultMaterial {
                        diffuseColor: Qt.rgba(0.1, 0.8, 0.1, 1.0)
                    }
                ]
            }

            DirectionalLight
            {
                color: Qt.rgba(1.0, 1.0, 1.0, 1.0)
                position: Qt.vector3d(0, 3000, 0)
                castsShadow: true
                brightness: 2.0
                eulerRotation: Qt.vector3d(-45, 20, 0)
                visible: true
                shadowFactor: 100
                shadowMapFar: 20000
                shadowBias: 20
            }

            PerspectiveCamera
            {
                id: mainCamera
                position: Qt.vector3d(0, 5000, 5000)
                eulerRotation: Qt.vector3d(-40, -10, 0)
                clipFar: 20000
                clipNear: 1000
            }

            Model
            {
                source: "#Cube"
                castsShadows: true
                receivesShadows: true
                position: Qt.vector3d(1000, 1000, 0)
                scale: Qt.vector3d(10, 10, 10)
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
                receivesShadows: true
                position: Qt.vector3d(-1000, 1000, 0)
                scale: Qt.vector3d(10, 10, 10)
                materials: [
                    DefaultMaterial {
                        diffuseColor: Qt.rgba(1.0, 0.9, 0.7, 1.0)
                    }
                ]
            }
        }
    }
}
