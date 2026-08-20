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

    GradientTexture {
        id: textData2
        width: 128
        height: 128
        startColor: "green"
        endColor: "yellow"
    }

    Model {
        id: model1
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColorMap: tex1
        }
    }

    Model {
        id: model2
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                id: tex2
                textureData: textData2
            }
        }
    }

    Model {
        id: model3
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                id: tex3
                textureData: GradientTexture {
                    id: textData3
                    width: 64
                    height: 64
                    startColor: "black"
                    endColor: "white"
                }
            }
        }
    }
}
