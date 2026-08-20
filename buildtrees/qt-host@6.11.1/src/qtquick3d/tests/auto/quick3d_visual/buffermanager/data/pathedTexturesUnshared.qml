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
        source: "noise1.jpg"
    }

    Texture {
        id: tex2
        source: "noise2.jpg"
    }

    Texture {
        id: tex3
        source: "noise3.jpg"
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
                source: "noise4.jpg"
            }
        }
    }
    
    Model {
        source: "#Cube"
        materials: DefaultMaterial {
            diffuseMap: Texture {
                source: "noise5.jpg"
            }
        }
    }
}