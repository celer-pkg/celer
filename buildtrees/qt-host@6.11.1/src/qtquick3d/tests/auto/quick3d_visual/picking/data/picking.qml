import QtQuick
import QtQuick3D

View3D {
    id: view
    objectName: "view"
    anchors.fill: parent
    environment: SceneEnvironment {
        backgroundMode: SceneEnvironment.Color
        clearColor: "black"
    }
    OrthographicCamera { z: 600 }
    DirectionalLight { }
    Model {
        id: model1
        objectName: "model1"
        source: "#Cube"
        pickable: true
        materials: PrincipledMaterial {
            baseColor: "red"
        }
    }
    Model {
        id: model2
        objectName: "model2"
        source: "#Cube"
        pickable: true
        position: Qt.vector3d(50.0, 50.0, -50.0)
        materials: PrincipledMaterial {
            baseColor: "green"
        }
    }
    Node {
        objectName: "item2dNode"
        z: 200
        Rectangle {
            id: item2d
            objectName: "item2d"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "#808080"
            opacity: 0.5
            // Not enabled, so by default picking goes through
            enabled: false
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle"
            }
        }
    }
    InstanceList {
        id: instanceList
        instances: [
            InstanceListEntry {
                position: Qt.vector3d(-200, 0, 200)
                color: "magenta"
            },
            InstanceListEntry {
                position: Qt.vector3d(-25, 75, -100)
                color: "blue"
            },
            InstanceListEntry {
                position: Qt.vector3d(200, 0, 0)
                color: "orange"
            }
        ]
    }
    Model {
        id: instancedModel
        objectName: "instancedModel"
        source: "#Cube"
        pickable: true
        instancing: instanceList
        materials: PrincipledMaterial {
            baseColor: "white"
        }
    }
}
