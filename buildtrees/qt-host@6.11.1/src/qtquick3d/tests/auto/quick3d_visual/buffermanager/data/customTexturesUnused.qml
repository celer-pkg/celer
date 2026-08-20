import QtQuick
import QtQuick3D
import io.qt.tests.auto.BufferManager

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
        textureData: GradientTexture {
            id: textData1
            width: 256
            height: 256
            startColor: "red"
            endColor: "blue"
        }
    }

    Texture {
        id: tex2
        textureData: textData1
    }

    GradientTexture {
        id: textData2
        width: 128
        height: 128
        startColor: "green"
        endColor: "yellow"
    }

    Texture {
        id: tex3
        textureData: textData2
    }
}
