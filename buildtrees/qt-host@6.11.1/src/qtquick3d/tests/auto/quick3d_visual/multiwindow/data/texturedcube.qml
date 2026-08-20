import QtQuick
import QtQuick3D

View3D {
    anchors.fill: parent
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "black"
    }
    PerspectiveCamera { z: 600 }
    DirectionalLight { }
    Model {
        source: "#Cube"
        scale: Qt.vector3d(2, 2, 2)
        eulerRotation.x: 30
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                source: "qt_logo_rect.png"
            }
        }
    }
}
