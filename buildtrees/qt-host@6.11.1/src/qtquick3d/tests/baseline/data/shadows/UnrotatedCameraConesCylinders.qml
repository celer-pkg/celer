// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    visible: true
    width: 640
    height: 480

    function addConesCylindersTriple(parentNode, z_pos) {
        const newObject = Qt.createQmlObject(`
            import QtQuick
            import QtQuick3D

            Node {
                scale: Qt.vector3d(0.01, 0.01, 0.01)
                property var z_positions: [0, 100, 200]

                PrincipledMaterial {
                    id: material
                    baseColor: "gray"
                }

                Model {
                    source: "#Cone"
                    position: Qt.vector3d(0, 450, z_positions[0])
                    eulerRotation.z: 180
                    scale.y: 5
                    materials: material
                }

                Model {
                    source: "#Cone"
                    position.z: z_positions[1]
                    scale.y: 2.5
                    materials: material
                }

                Model {
                    source: "#Cylinder"
                    position: Qt.vector3d(0, 175, z_positions[2])
                    materials: material
                    scale.y: 3.5
                }
            }
            `,
            parentNode,
            "ConesCylinders"
        );
        newObject.z_positions = [z_pos, z_pos - 125, z_pos - 250];
    }

    View3D {
        id: view
        anchors.fill: parent
        camera: camera
        environment: SceneEnvironment {
            clearColor: "lightblue"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            clipFar: 15000
            clipNear: 0.3
            position: Qt.vector3d(2.5, 2.5, 2.5)
        }

        Node {
            DirectionalLight {
                visible: true
                castsShadow: true
                shadowFactor: 100
                eulerRotation: Qt.vector3d(-40, -120, 0)
                shadowBias: 0.2
                csmNumSplits: 2
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                softShadowQuality: Light.Hard
                shadowMapFar: 150
                lockShadowmapTexels: true
            }
        }

        Model {
            id: ground
            source: "#Cube"
            scale: Qt.vector3d(150, 0.01, 150)
            z: -5500
            materials: PrincipledMaterial {
                baseColor: "gray"
            }
            castsShadows: false
        }

        RandomInstancing {
            id: spawner
            position:  InstanceRange {
                from: Qt.vector3d(2000, 0, 2000)
                to: Qt.vector3d(-2000, 0, -2000)
            }
            instanceCount : 10
            randomSeed: 1234
        }

        CustomMaterial {
            id: mat
            depthDrawMode: Material.OpaquePrePassDepthDraw
        }

        Model {
            materials: mat
            instancing: spawner
            source: "#Cube"
            castsShadows: true
            receivesShadows: true
            scale: Qt.vector3d(0.01, 0.01, 0.01)
        }

        Node {
            id: shapeSpawner
            Component.onCompleted: {
                var z_pos = 0
                for (var i = 0; i < 10; i++) {
                    addConesCylindersTriple(this, z_pos)
                    z_pos -= 1450
                }
            }
        }
    }

    WasdController {
        id: wasd
        controlledObject: camera
        speed: 0.1
        shiftSpeed: 10
        mouseEnabled: true
    }
}
