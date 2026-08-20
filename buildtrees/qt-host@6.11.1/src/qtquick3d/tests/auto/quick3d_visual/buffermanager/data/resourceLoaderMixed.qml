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

    DirectionalLight {

    }



    Model {
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                source: "noise1.jpg"
            }
            emissiveMap: namedTexture
        }
    }

    Model {
        source: "#Cone"
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                source: "noise3.jpg"
            }
            emissiveMap: namedTexture2
        }
    }

    Model {
        geometry: grid1
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                source: "noise2.jpg"
                generateMipmaps: true
            }
        }
    }

    Model {
        geometry: GridGeometry { }
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                textureData: GradientTexture { }
            }
            emissiveMap: qmlTexture
        }
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
                id: namedTexture
                source: "noise1.jpg"
                generateMipmaps: true
            },
            Texture {
                source: "noise2.jpg"
            },
            Texture {
                id: namedTexture2
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
}

