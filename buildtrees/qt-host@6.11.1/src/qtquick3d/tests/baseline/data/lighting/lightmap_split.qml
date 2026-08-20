import QtQuick
import QtQuick3D

Rectangle {
    width: 640
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        width: parent.width/2
        height: parent.height

        PerspectiveCamera {
            z: 300
            y: 100
        }

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
            lightmapper: Lightmapper {
                texelsPerUnit: 0.3
                source: "lightmaps/box.bin"
            }
        }

        PointLight {
            bakeMode: Light.BakeModeAll
            y: 190
            brightness: 5
            castsShadow: true
            shadowFactor: 75
        }

        Box {
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: true
                key: "box"
            }
            scale: Qt.vector3d(100, 100, 100)
        }
    }

    View3D {
        width: parent.width/2
        height: parent.height
        x: parent.width/2

        PerspectiveCamera {
            z: 300
            y: 100
        }

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
            lightmapper: Lightmapper {
                texelsPerUnit: 0.1
                source: "lightmaps/box_low.bin"
            }
        }

        PointLight {
            bakeMode: Light.BakeModeAll
            y: 90
            brightness: 5
            castsShadow: true
            shadowFactor: 75
        }

        Box {
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: true
                key: "box"
            }
            scale: Qt.vector3d(100, 100, 100)
        }
    }
}
