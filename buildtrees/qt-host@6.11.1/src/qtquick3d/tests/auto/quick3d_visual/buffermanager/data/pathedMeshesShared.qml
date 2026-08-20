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

    // primitive
    Model {
        source: "#Cube"
        materials: DefaultMaterial {

        }
    }

    Model {
        source: "#Sphere"
        // No material
    }

    // From mesh file
    Model {
        source: "random1.mesh"
        materials: DefaultMaterial {

        }
    }

    Model {
        source: "random2.mesh"
        // No material
    }

    Model {
        source: "#Cube"
        materials: DefaultMaterial {

        }
    }

    Model {
        source: "#Sphere"
        // No material
    }

    // From mesh file
    Model {
        source: "random1.mesh"
        materials: DefaultMaterial {

        }
    }

    Model {
        source: "random2.mesh"
        // No material
    }

}

