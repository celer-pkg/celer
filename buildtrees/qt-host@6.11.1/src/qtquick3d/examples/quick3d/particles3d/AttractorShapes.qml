// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow

    anchors.fill: parent

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 0)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Model shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.02, 0.02, 0.15)
                materials: PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                }
            }
        }

        Model {
            id: emittingSphere
            source: "#Sphere"
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            materials: PrincipledMaterial {
                opacity: 0.4
            }

            position: Qt.vector3d(-200, 100, 0)
            SequentialAnimation on x {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    to: -100
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: -200
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                system: psystem
                particle: particleRed
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: true
                }
                emitRate: 200
                lifeSpan: 2000
            }

            ParticleEmitter3D {
                id: emitter2
                system: psystem
                particle: particleGreen
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: true
                }
                emitRate: 200
                lifeSpan: 5000
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(200, 200, 0)
                    directionVariation: Qt.vector3d(50, 50, 50)
                }
                particleRotationVelocityVariation: Qt.vector3d(500.0, 500.0, 500.0)
            }

            ParticleEmitter3D {
                system: psystem
                particle: particleWhite
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: true
                }
                emitRate: 200
                lifeSpan: 2000
                depthBias: 10
            }
        }

        Model {
            id: targetSphere
            source: "#Sphere"
            materials: PrincipledMaterial {
                opacity: 0.2
            }
            y: -200
            SequentialAnimation on x {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    to: 200
                    duration: 3500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 0
                    duration: 3500
                    easing.type: Easing.InOutQuad
                }
            }
            Attractor3D {
                system: psystem
                particles: [particleRed]
                // Attract into a position
                positionVariation: Qt.vector3d(10, 10, 10)
                duration: sliderDuration2.sliderValue
                durationVariation: sliderDuration2Variation.sliderValue
                hideAtEnd: checkBoxHide2.checked
            }

            Attractor3D {
                system: psystem
                particles: [particleGreen]
                // Attract into a shape
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: false
                }
                duration: sliderDuration1.sliderValue
                durationVariation: sliderDuration1Variation.sliderValue
                hideAtEnd: checkBoxHide1.checked
                useCachedPositions: false

                SequentialAnimation on scale {
                    running: true
                    loops: Animation.Infinite
                    Vector3dAnimation {
                        to: Qt.vector3d(2, 2, 2)
                        duration: 3500
                        easing.type: Easing.InOutQuad
                    }
                    Vector3dAnimation {
                        to: Qt.vector3d(0.1, 0.1, 0.1)
                        duration: 3500
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            // Particles
            ModelParticle3D {
                id: particleRed
                delegate: particleComponent
                maxAmount: 400
                color: "#ff0000"
                colorVariation: Qt.vector4d(0, 0, 0, 0.5)
                alignMode: Particle3D.AlignTowardsTarget
                alignTargetPosition: targetSphere.position
            }
            ModelParticle3D {
                id: particleGreen
                delegate: particleComponent
                maxAmount: 1000
                color: "#00ff00"
                colorVariation: Qt.vector4d(0, 0, 0, 0.5)
            }
            SpriteParticle3D {
                id: particleWhite
                sprite: Texture {
                    source: "images/dot.png"
                }
                maxAmount: 400
                color: "#ffffff"
                colorVariation: Qt.vector4d(0, 0, 0, 0.5)
            }

            // Attractor inside ParticleSystem
            Attractor3D {
                particles: [particleWhite]
                // Attract into a position
                position: Qt.vector3d(-100, -250, 0)
                positionVariation: Qt.vector3d(100, 10, 10)
                duration: 1500
            }

            Wander3D {
                particles: [particleGreen]
                uniqueAmount: Qt.vector3d(10, 10, 10)
                uniquePace: Qt.vector3d(1, 1, 1)
                uniqueAmountVariation: 8
                uniquePaceVariation: 0.8
                fadeInDuration: 500
                fadeOutDuration: 2000
            }
        }
    }

    SettingsView {
        CustomLabel {
            text: "Attractor 1: Duration"
        }
        CustomSlider {
            id: sliderDuration1
            sliderValue: 3000
            fromValue: 1000
            toValue: 5000
        }
        CustomLabel {
            text: "Attractor 1: Duration Variation"
        }
        CustomSlider {
            id: sliderDuration1Variation
            sliderValue: 500
            fromValue: 0
            toValue: 2000
        }
        CustomCheckBox {
            id: checkBoxHide1
            text: "Attractor 1: Hide at end"
            checked: false
        }
        Item {
            width: 1
            height: 40
        }
        CustomLabel {
            text: "Attractor 2: Duration"
        }
        CustomSlider {
            id: sliderDuration2
            sliderValue: 1500
            fromValue: 1000
            toValue: 5000
        }
        CustomLabel {
            text: "Attractor 2: Duration Variation"
        }
        CustomSlider {
            id: sliderDuration2Variation
            sliderValue: 500
            fromValue: 0
            toValue: 2000
        }
        CustomCheckBox {
            id: checkBoxHide2
            text: "Attractor 2: Hide at end"
            checked: false
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
