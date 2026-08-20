import QtQuick
import QtQuick.Window
import QtQuick3D

Rectangle {
    width: 640
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    // Two sets of view3D's since scoped lights dont work
    View3D {
        anchors.fill: parent

        DirectionalLight {
            color: "black"
            ambientColor: "white"
        }

        PerspectiveCamera {
            z: 500
        }

        // Lit Diffuse
        Node {
            id: litAmbientNode
            y: -200

            Model {
                source: "#Cube"
                x: -100
                materials: DefaultMaterial {
                    diffuseColor: "#FFA800"
                }
            }
            Model {
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseColor: "green"
                }
            }
            Model {
                x:100
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseColor: "grey"
                }
            }
        }
    }


    View3D {
        anchors.fill: parent

        PerspectiveCamera {
            z: 500
        }


        // Unlit
        Model {
            source: "#Cube"
            x: -100
            materials: DefaultMaterial {
                lighting: DefaultMaterial.NoLighting
                diffuseColor: "#FFA800"
            }
        }
        Model {
            source: "#Cube"
            materials: DefaultMaterial {
                lighting: DefaultMaterial.NoLighting
                diffuseColor: "green"
            }
        }
        Model {
            x:100
            source: "#Cube"
            materials: DefaultMaterial {
                lighting: DefaultMaterial.NoLighting
                diffuseColor: "grey"
            }
        }

        // QML in 3D
        Node {
            y: -50
            x: -150
            z: 50
            Rectangle {
                color: "#FFA800"
                width: 100
                height: 100
            }

            Rectangle {
                color: "green"
                x: 100
                width: 100
                height: 100
            }

            Rectangle {
                color: "grey"
                x: 200
                width: 100
                height: 100
            }
        }

        DirectionalLight {
            //color: "black"
            //ambientColor: "white"
        }

        // Lit Diffuse
        Node {
            id: litNode
            y: 100

            Model {
                source: "#Cube"
                x: -100
                materials: DefaultMaterial {
                    diffuseColor: "#FFA800"
                }
            }
            Model {
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseColor: "green"
                }
            }
            Model {
                x:100
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseColor: "grey"
                }
            }
        }
    }

    // QML in 2D
    Item {
        id: container
        y: 8.5
        x: 181
        property real size: 92.75
        Rectangle {
            color: "#FFA800"
            width: container.size
            height: container.size
        }

        Rectangle {
            color: "green"
            x: container.size
            width: container.size
            height: container.size
        }

        Rectangle {
            color: "grey"
            x: container.size * 2
            width: container.size
            height: container.size
        }
    }
}

