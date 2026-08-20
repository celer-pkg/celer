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
         id: model3
         objectName: "model3"
         source: "#Cone"
         pickable: true
	 y: -50
         materials: PrincipledMaterial {
             baseColor: "pink"
         }
     }
}
