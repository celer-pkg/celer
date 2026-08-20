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
        eulerRotation.y: -5
    }

    DirectionalLight {
        eulerRotation.y: 2
    }

    Node {
        objectName: "left object"
        x: -120
        z: 400
        Model {
            source: "#Cube"
            pickable: true
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    sourceItem: BusyBox {
                        id: sharedItem
                        objectName: "shared busybox"
                    }
                }
            }
        }
    }

    Node {
        objectName: "right object"
        x: 120
        z: 400
        eulerRotation.x: -25
        eulerRotation.y: 5
        Model {
            source: "#Cube"
            pickable: true
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    sourceItem: sharedItem
                }
            }
        }
    }
}
