import QtQuick
import QtQuick3D

View3D {
    width: 640
    height: 480
    id: view1
    anchors.fill: parent

    PerspectiveCamera {
        id: camera1
    }

    Texture {
        id: tex1
        sourceItem: Rectangle {
            width: 256
            height: 256
            color: "red"
        }
    }

    Texture {
        id: tex2
        sourceItem: Rectangle {
            width: 200
            height: 200
            color: "blue"
            Text {
                anchors.centerIn: parent
                text: "test"
            }
        }
    }

    Rectangle {
        id: item1
        width: 100
        height: 100
        color: "green"
    }

    Texture {
        id: tex3
        sourceItem: item1
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: tex1
        }
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: tex2
        }
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: tex3
        }
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: Texture {
                sourceItem: Item {
                    width: 400
                    height: 400
                }
            }
        }
    }
}
