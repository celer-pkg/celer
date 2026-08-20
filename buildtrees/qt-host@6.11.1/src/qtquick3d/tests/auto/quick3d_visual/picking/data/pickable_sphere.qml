import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

View3D {
    id: view
    objectName: "view"
    anchors.centerIn: parent
    width: 100
    height: 100
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "green"
    }
    PerspectiveCamera { z: 600 }
    DirectionalLight { }
    Model {
        id: model1
        objectName: "model1"
        pickable: true
        geometry: SphereGeometry { radius: 200.0 }
        materials: PrincipledMaterial { }
    }
}
