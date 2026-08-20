import QtQuick
import QtQuick3D

Rectangle {
    visible: true
    width: 1024
    height: 768
    color: "black"

    View3D {
        anchors.fill: parent

        DirectionalLight {
        }

        PerspectiveCamera {
            z: 200
        }


        // 2D in 3D
        Node {
            y: 115.1
            Image {
                width: sourceSize.width * 0.15
                height: sourceSize.height * 0.15
                source: "../shared/maps/oulu_2.jpeg"
            }
        }

        // Lit Material
        Model {
            source: "#Rectangle"
            scale.y: 0.75
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "../shared/maps/oulu_2.jpeg"
                }
                metalness: 0.0
                roughness: 0.0
            }
            z: 70
            x: -50
            y: -37.5
        }

        DefaultMaterial {
            id: defaultMaterial
            diffuseMap: Texture {
                source: "../shared/maps/oulu_2.jpeg"
            }
        }

        // Lit Material
        Model {
            source: "#Rectangle"
            scale.y: 0.75
            materials: defaultMaterial
            z: 70
            x: 50
            y: -37.5
        }
    }


    // QML 2D
    Image {
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        source: "../shared/maps/oulu_2.jpeg"

    }
}
