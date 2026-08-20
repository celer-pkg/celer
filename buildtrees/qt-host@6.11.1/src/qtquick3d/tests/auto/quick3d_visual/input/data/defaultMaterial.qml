import QtQuick
import QtQuick3D

View3D {
    width: 1024; height: 480

    environment: SceneEnvironment {
        clearColor: "#111"
        backgroundMode: SceneEnvironment.Color
    }

    PerspectiveCamera {
        z: 600
    }

    DirectionalLight {
        eulerRotation.y: 90
    }

    DirectionalLight {
        eulerRotation.y: -90
    }

    Node {
        objectName: "left object"
        x: -120
        z: 400
        eulerRotation.y: 10
        Model {
            source: "#Cube"
            pickable: true
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    sourceItem: BusyBox {
                        objectName: "left busybox"
                        layer.enabled: true
                        layer.textureSize: Qt.size(512, 512)
                    }
                }
            }
        }
    }

    Node {
        objectName: "right object"
        x: 120
        z: 400
        eulerRotation.x: 35
        eulerRotation.y: 5
        Model {
            source: "#Cube"
            pickable: true
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    sourceItem: BusyBox {
                        objectName: "right busybox"
                        layer.enabled: true
                        layer.textureSize: Qt.size(512, 512)
                    }
                }
            }
        }
    }
}
