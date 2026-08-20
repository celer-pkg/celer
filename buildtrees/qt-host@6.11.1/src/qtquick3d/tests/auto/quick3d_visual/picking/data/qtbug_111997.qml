import QtQuick
import QtQuick3D

View3D {
    id: view
    objectName: "view"
    anchors.centerIn: parent
    width: 100
    height: 100
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "black"
    }
    OrthographicCamera { z: 600 }
    DirectionalLight { }
    Model {
        id: model1
        objectName: "model1"
        materials: [
            PrincipledMaterial {
                baseColor: 'red'
                lighting: PrincipledMaterial.NoLighting
            }
        ]
        scale: Qt.vector3d(10, 10, 10)
        eulerRotation: Qt.vector3d(45, 0, 0)
        pickable: true
    }
}
