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
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            property real cameraAnim: 0
            NumberAnimation {
                target: camera
                property: "cameraAnim"
                running: true
                loops: Animation.Infinite
                from: 0
                to: 2 * Math.PI
                duration: 40000
            }
            position: Qt.vector3d(Math.sin(cameraAnim * 3.0) * 400, 200, 600 + Math.cos(cameraAnim) * 200)
            eulerRotation: Qt.vector3d(-20, Math.sin(cameraAnim * 3.0) * 30, 0)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 0)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Background stars
        ParticleSystem3D {
            id: psystem2
            startTime: 10000
            SpriteParticle3D {
                id: particle3
                sprite: Texture {
                    source: "images/star3.png"
                }
                maxAmount: 1000
                color: "#40ffffff"
                colorVariation: Qt.vector4d(0.1, 0.1, 0.1, 0.2)
                fadeInEffect: SpriteParticle3D.FadeScale
                fadeInDuration: 2000
                fadeOutEffect: SpriteParticle3D.FadeScale
                fadeOutDuration: 2000
                alignMode: SpriteParticle3D.AlignTowardsTarget
                alignTargetPosition: camera.position
            }
            ParticleEmitter3D {
                particle: particle3
                shape: ParticleShape3D {
                    fill: true
                    type: ParticleShape3D.Sphere
                }
                position: Qt.vector3d(0, -500, -400)
                scale: Qt.vector3d(30, 30, 15)
                emitRate: 100
                lifeSpan: 10000
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 50)
                particleScale: 5.0
                particleScaleVariation: 3.0
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 0, 0)
                    directionVariation: Qt.vector3d(10, 10, 10)
                }
            }
        }

        ParticleSystem3D {
            id: psystem
            running: false
            SequentialAnimation on time {
                loops: Animation.Infinite
                PauseAnimation {
                    duration: 1500
                }
                NumberAnimation {
                    to: 5000
                    duration: 5000
                    easing.type: Easing.InOutQuad
                }
                PauseAnimation {
                    duration: 1500
                }
                NumberAnimation {
                    to: 0
                    duration: 5000
                    easing.type: Easing.InOutQuad
                }
            }

            SpriteParticle3D {
                id: particle1
                sprite: Texture {
                    source: "images/star3.png"
                }
                maxAmount: 4096
                colorTable: Texture {
                    source: "images/color_table5.png"
                }
                color: "#d0ffffff"
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.4)
                particleScale: 15.0
                billboard: true
                fadeInEffect: SpriteParticle3D.FadeNone
                fadeOutEffect: SpriteParticle3D.FadeNone
            }

            SpriteParticle3D {
                id: particle2
                sprite: Texture {
                    source: "images/dot.png"
                }
                maxAmount: 4096
                colorTable: Texture {
                    source: "images/color_table4.png"
                }
                color: "#60ffffff"
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.4)
                particleScale: 6.0
                billboard: true
                fadeInEffect: SpriteParticle3D.FadeNone
                fadeOutEffect: SpriteParticle3D.FadeNone
            }

            ParticleEmitter3D {
                particle: particle1
                scale: Qt.vector3d(5.0, 5.0, 5.0)
                shape: ParticleCustomShape3D {
                    source: "data/qt_logo_in_4096.cbor"
                }
                lifeSpan: 5001
                emitBursts: [
                    EmitBurst3D {
                        time: 0
                        amount: 4096
                    }
                ]
                depthBias: -200
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(80, 80, 80)
                particleScaleVariation: 0.5
                particleEndScale: 4.0
                particleEndScaleVariation: 2.0
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-150, 100, 0)
                    directionVariation: Qt.vector3d(150, 100, 100)
                }
            }

            ParticleEmitter3D {
                particle: particle2
                scale: Qt.vector3d(5.0, 5.0, 5.0)
                shape: ParticleCustomShape3D {
                    source: "data/qt_logo_out_4096.cbor"
                }
                lifeSpan: 5001
                emitBursts: [
                    EmitBurst3D {
                        time: 0
                        amount: 4096
                    }
                ]
                particleScale: 2.0
                particleEndScale: 1.0
                particleScaleVariation: 1.5
                particleEndScaleVariation: 0.8
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 200, 0)
                    directionVariation: Qt.vector3d(50, 50, 50)
                }
            }

            Attractor3D {
                particles: [particle1]
                position: Qt.vector3d(-200, 0, 0)
                scale: Qt.vector3d(4.0, 4.0, 4.0)
                shape: ParticleCustomShape3D {
                    source: "data/heart_4096.cbor"
                    randomizeData: true
                }
                duration: 4000
                durationVariation: 1000
            }
            Attractor3D {
                particles: [particle2]
                position: Qt.vector3d(200, 0, 0)
                scale: Qt.vector3d(6.0, 6.0, 6.0)
                shape: ParticleCustomShape3D {
                    source: "data/qt_logo_in_4096.cbor"
                }
                duration: 4000
                durationVariation: 1000
            }
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem, psystem2]
    }
}
