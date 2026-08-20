import QtQuick
import QtQuick3D

Rectangle {
    width: 1024; height: 480
    color: th.pressed ? "#334" : "black"

    Item {
        anchors.fill: parent
        TapHandler { id: th }
    }

    View3D {
        anchors.fill: parent
        Shortcut {
            sequence: StandardKey.Quit
            onActivated: Qt.quit()
        }

        PerspectiveCamera {
            z: 600
        }

        DirectionalLight { }

        Node {
            objectName: "left object"
            x: -256
            y: 128
            z: 380
            eulerRotation.y: 15
            Rectangle {
                width: 200; height: 200
                color: ma.pressed ? "goldenrod" : "wheat"
                MouseArea {
                    id: ma
                    enabled: false
                    objectName: "left mousearea"
                    anchors.fill: parent
                }
            }
        }

        Model {
            objectName: "right object"
            x: 32
            z: 300
            eulerRotation.y: -35
            source: "#Cube"
            pickable: true
            materials: DefaultMaterial {
                diffuseColor: "lightsteelblue"
            }
        }
    }
}
