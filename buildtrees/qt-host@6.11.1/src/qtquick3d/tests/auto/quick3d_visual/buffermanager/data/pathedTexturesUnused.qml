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
        source: "noise1.jpg"
    }

    Texture {
        source: "noise2.jpg"
    }

    Texture {
        source: "noise3.jpg"
    }

    Texture {
        source: "noise4.jpg"
    }

    Texture {
        source: "noise5.jpg"
    }
}