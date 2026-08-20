// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [import]
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
//! [import]

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 10, 300)
            clipNear: 1.0
            NumberAnimation on z {
                from: 200
                to: -100
                duration: 40 * 1000
            }
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: Qt.rgba(0.5, 0.5, 0.5, 1.0)
        }

        RandomInstancing {
            id: randomInstancing
            instanceCount: 800

            position: InstanceRange {
                from: Qt.vector3d(-200, 0, -200)
                to: Qt.vector3d(200, 1, 200)
            }
            scale: InstanceRange {
                from: Qt.vector3d(10, 10, 10)
                to: Qt.vector3d(10, 10, 10)
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(0, 0, 0)
            }
            color: InstanceRange {
                from: "grey"
                to: "white"
                proportional: true
            }

            randomSeed: 2021
        }

        PrincipledMaterial {
            id: marbleMaterial
            baseColorMap: Texture {
                source: "maps/baseColor.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            opacityChannel: Material.A
            metalnessMap: Texture {
                source: "maps/occlusionRoughnessMetallic.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            metalnessChannel: Material.B
            roughnessMap: Texture {
                source: "maps/occlusionRoughnessMetallic.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            roughnessChannel: Material.G
            metalness: 1
            roughness: 1
            normalMap: Texture {
                source: "maps/normal.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
        }

        //! [example]
        LodManager {
            camera: camera
            distances: [100, 140, 180]
            fadeDistance: 10

            Model {
                scale: Qt.vector3d(100, 100, 100);
                source: "meshes/marble_bust_01_LOD_0.mesh"
                materials: [ marbleMaterial ]
            }

            Model {
                scale: Qt.vector3d(100, 100, 100);
                source: "meshes/marble_bust_01_LOD_1.mesh"
                materials: [ marbleMaterial ]
            }

            Model {
                scale: Qt.vector3d(100, 100, 100);
                source: "meshes/marble_bust_01_LOD_2.mesh"
                materials: [ marbleMaterial ]
            }

            Model {
                scale: Qt.vector3d(100, 100, 100);
                source: "meshes/marble_bust_01_LOD_3.mesh"
                materials: [ marbleMaterial ]
            }
        }
        //! [example]

        LodManager {
            camera: camera
            distances: [50, 100, 150]

            Model {
                instancing: randomInstancing
                source: "#Sphere"
                materials: [
                    PrincipledMaterial {
                        metalness: 0
                        roughness: 1
                        baseColor: "red"
                    }
                ]
                scale: Qt.vector3d(0.005, 0.005, 0.005)
            }

            Model {
                instancing: randomInstancing
                source: "#Cylinder"
                materials: [
                    PrincipledMaterial {
                        metalness: 0
                        roughness: 1
                        baseColor: "orange"
                    }
                ]
                scale: Qt.vector3d(0.005, 0.005, 0.005)
            }

            Model {
                instancing: randomInstancing
                source: "#Cube"
                materials: [
                    PrincipledMaterial {
                        metalness: 0
                        roughness: 1
                        baseColor: "yellow"
                    }
                ]
                scale: Qt.vector3d(0.005, 0.005, 0.005)
            }

            Model {
                instancing: randomInstancing
                source: "#Rectangle"
                materials: [
                    PrincipledMaterial {
                        metalness: 0
                        roughness: 1
                        baseColor: "green"
                    }
                ]
                scale: Qt.vector3d(0.005, 0.005, 0.005)
            }
        }
    }

    DebugView {
        source: view
    }
}
