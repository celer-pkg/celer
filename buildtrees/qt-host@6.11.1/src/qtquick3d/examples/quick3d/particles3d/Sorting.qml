// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    anchors.fill: parent

    View3D {
        id: view
        anchors.fill: parent

        PointLight {
            color: "white"
            ambientColor: "gray"
            brightness: 2.0
        }

        Model {
            source: "#Rectangle"
            eulerRotation: Qt.vector3d(-90, 0, 0)
            scale: Qt.vector3d(10, 10, 1)
            materials: [ PrincipledMaterial {
                    baseColor: "red"
                }
            ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.3, 0.3, 0.3)
            materials: [PrincipledMaterial {
                    baseColor: "white"
                }
            ]
        }

        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.1, 0.1, 0.1)
                materials: PrincipledMaterial {
                    metalness: 0.5
                    roughness: 0
                    specularAmount: 1.0
                }
                opacity: 0.75
            }
        }

        ParticleSystem3D {
            id: psystem

            ModelParticle3D {
                id: modelParticle
                delegate: particleComponent
                maxAmount: 60
                color: Qt.rgba(0.5, 0.5, 0.5, 0.5)
                colorVariation: Qt.vector4d(0.5, 0.5, 0.5, 0.15)
                sortMode: sortModeSelectionBox.index
            }

            SpriteParticle3D {
                id: spriteParticle
                color: Qt.rgba(0.75, 0.75, 0.75, 0.75)
                colorVariation: Qt.vector4d(0.5, 0.5, 0.5, 0.05)
                maxAmount: 60
                billboard: true
                sortMode: sortModeSelectionBox.index
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                x: 35
                particle: spriteParticle
                particleScaleVariation: 0.3
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 5, 15)
                    directionVariation: Qt.vector3d(0, 0, 5)
                }
                emitRate: 10
                lifeSpan: 6000
                // Sort emitters based on camera rotation
                depthBias: cameraController.rot - 180
            }
            ParticleEmitter3D {
                x: -35
                particle: modelParticle
                particleScaleVariation: 0.2
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 5, 15)
                }
                emitRate: 10
                lifeSpan: 6000
            }
        }

        Node {
            id: cameraController
            property real rot: 0.0
            PropertyAnimation {
                target: cameraController
                property: "rot"
                from: 0.0
                to: 360.0
                duration: 10000
                loops: Animation.Infinite
                running: true
                paused: !checkBoxRotateCamera.checked
            }
            eulerRotation: Qt.vector3d(0, rot, 0)
            PerspectiveCamera {
                y: 20
                z: 100
            }
        }

        environment: SceneEnvironment {
            clearColor: "darkGray"
            backgroundMode: SceneEnvironment.Color
        }
    }
    SettingsView {
        id: settingsView
        CustomSelectionBox {
            id: sortModeSelectionBox
            text: "Mode"
            values: ["SortNone", "SortNewest", "SortOldest", "SortDistance"]
            parentWidth: settingsView.width
        }
        CustomCheckBox {
            id: checkBoxRotateCamera
            text: "Rotate camera"
            checked: false
        }
    }
    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
