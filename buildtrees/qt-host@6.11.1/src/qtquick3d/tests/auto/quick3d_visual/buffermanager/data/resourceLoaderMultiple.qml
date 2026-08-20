import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import io.qt.tests.auto.BufferManager

View3D {
    width: 640
    height: 480
    id: view1
    anchors.fill: parent

    PerspectiveCamera {
        id: camera1
    }

    GridGeometry {
        id: grid1
        horizontalLines: 10
        verticalLines: 10
    }
    GridGeometry {
        id: grid2
        horizontalLines: 8
        verticalLines: 8
        horizontalStep: 0.1
        verticalStep: 0.1
    }

    GridGeometry {
        id: grid3
        horizontalLines: 18
        verticalLines: 18
        horizontalStep: 30.0
        verticalStep: 20.5
    }
    GridGeometry {
        id: grid4
        horizontalLines: 18
        verticalLines: 18
        horizontalStep: 30.0
        verticalStep: 20.5
    }

    ResourceLoader {
        meshSources: [
            "#Cube",
            "#Sphere",
            "random1.mesh",
            "random2.mesh"
        ]
        textures: [
            Texture {
                source: "noise1.jpg"
            },
            Texture {
                source: "noise1.jpg"
                generateMipmaps: true
            },
            Texture {
                source: "noise2.jpg"
            },
            Texture {
                textureData: GradientTexture { }
            },
            Texture {
                id: qmlTexture
                sourceItem: Rectangle {
                    width: 200
                    height: 200
                    color: "blue"
                }
            }
        ]

        geometries: [
            grid1,
            grid2,
            grid3
        ]
    }

    Node {
        Node {
            Node {
                ResourceLoader {
                    id: secondLoader
                    meshSources: "#Cone"
                    textures: Texture { textureData: GradientTexture { } }
                    geometries: grid4
                }
            }
        }
    }
}

