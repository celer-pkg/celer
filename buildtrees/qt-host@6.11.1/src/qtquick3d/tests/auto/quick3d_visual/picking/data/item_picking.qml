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
        objectName: "item2dPlusZNode"
        z: 100
        Rectangle {
            id: item2d
            objectName: "item2dPlusZ"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "blue"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle +Z"
                color: "white"
            }
        }
    }
    Node {
        objectName: "item2dMinusZNode"
        z: -100
        eulerRotation.y: 180
        Rectangle {
            id: item2d2
            objectName: "item2dMinusZ"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "blue"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle -Z"
                color: "white"
            }
        }
    }

    Node {
        objectName: "item2dMinusXNode"
        x: -100
        eulerRotation.y: -90
        Rectangle {
            id: item2d3
            objectName: "item2dMinusX"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "red"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle -X"
                color: "white"
            }
        }
    }
    Node {
        objectName: "item2dPlusXNode"
        x: 100
        eulerRotation.y: 90
        Rectangle {
            id: item2d4
            objectName: "item2dPlusX"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "red"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle +X"
                color: "white"
            }
        }
    }

    Node {
        objectName: "item2dPlusYNode"
        y: 100
        eulerRotation.x: -90
        Rectangle {
            id: item2d5
            objectName: "item2dPlusY"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "green"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle +Y"
                color: "white"
            }
        }
    }

    Node {
        objectName: "item2dMinusYNode"
        y: -100
        eulerRotation.x: 90
        Rectangle {
            id: item2d6
            objectName: "item2dMinusY"
            anchors.centerIn: parent
            width: 150
            height: 150
            border.width: 2
            border.color: "#ffffff"
            color: "green"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rectangle -Y"
                color: "white"
            }
        }
    }


    // rotate item

    Node {
        x: 300
        objectName: "item2dRotationNode"
        eulerRotation.x: -90
        Rectangle {
            objectName: "item2dRotation"
            anchors.centerIn: parent
            width: 100
            height: 100
            border.width: 2
            border.color: "#ffffff"
            color: "yellow"
            opacity: 0.5
            rotation: 45
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Rotation"
                color: "white"
            }
        }
    }

    // translate item
    Node {
        x: 500
        eulerRotation.x: -90
        objectName: "item2dTranslationNode"
        Rectangle {
            objectName: "item2dTranslation"
            width: 100
            height: 100
            x: -100
            y: -100
            border.width: 2
            border.color: "#ffffff"
            color: "yellow"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Translation"
                color: "white"
            }
        }
    }

    // scale Item
    Node {
        x: 700
        eulerRotation.x: -90
        objectName: "item2dScaleNode"
        Rectangle {
            objectName: "item2dScale"
            width: 100
            height: 100
            scale: 2
            anchors.centerIn: parent
            border.width: 2
            border.color: "#ffffff"
            color: "yellow"
            opacity: 0.5
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Scale"
                color: "white"
            }
        }
    }


    // complex qml transform
    Node {
        x: 1000
        eulerRotation.x: -90
        objectName: "item2dComplexNode"
        Rectangle {
            objectName: "item2dComplex"
            width: 100
            height: 100
            border.width: 2
            border.color: "#ffffff"
            color: "orange"
            opacity: 0.5
            transform: [
                Translate {
                    x: -50
                    y: 50
                },
                Rotation {
                    origin.x: 25
                    origin.y: 25
                    angle: 90
                },
                Scale {
                    xScale: 2
                    yScale: 2
                }


            ]

            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Transforms"
                color: "white"
            }
        }
    }

    Node {
        x: 1100
        eulerRotation.x: -90
        objectName: "item2dMadnessNode"
        Rectangle {
            objectName: "item2dMadness"
            width: 100
            height: 100
            border.width: 2
            border.color: "#ffffff"
            color: "salmon"
            opacity: 0.5
            transform: [
                Rotation {
                    axis.x: 1
                    axis.y: 0
                    axis.z: 0
                    angle: -45
                }
            ]

            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "Madness!"
                color: "white"
            }
        }
    }
}
