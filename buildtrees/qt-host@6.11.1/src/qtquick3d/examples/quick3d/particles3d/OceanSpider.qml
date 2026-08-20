// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow

    readonly property real tentacleEmitRate: 120
    readonly property real cameraDistance: sliderCameraDistance.sliderValue
    property real cameraDistanceSmoothed: cameraDistance
    property real tentacleWideness: 0

    anchors.fill: parent

    // Animate tentacle movement
    SequentialAnimation {
        loops: Animation.Infinite
        running: true
        NumberAnimation {
            target: mainWindow
            property: "tentacleWideness"
            to: 30
            duration: 6000
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: mainWindow
            property: "tentacleWideness"
            to: -30
            duration: 6000
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on cameraDistanceSmoothed {
        SmoothedAnimation {
            velocity: 100
            duration: 1000
        }
    }

    // Background ocean gradient
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                color: "#005060"
                position: 0.0
            }
            GradientStop {
                color: "#000000"
                position: 1.0
            }
        }
    }

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        // Camera rotating the spider
        Node {
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, mainWindow.cameraDistanceSmoothed)
            }

            NumberAnimation on eulerRotation.y {
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 20000
            }

            SequentialAnimation on eulerRotation.x {
                loops: Animation.Infinite
                NumberAnimation {
                    to: -50
                    duration: 6000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 0
                    duration: 6000
                    easing.type: Easing.InOutQuad
                }
            }
        }

        PointLight {
            position: Qt.vector3d(0, 400, 0)
            brightness: 200
        }

        // Body of the spider
        Model {
            id: spiderBody
            source: "#Sphere"
            scale: Qt.vector3d(1.0, 0.6, 1.2)

            PrincipledMaterial {
                id: spiderMaterial
                baseColor: tentacleParticle.color
                metalness: 1.0
                roughness: 0.6
                normalMap: Texture {
                    source: "images/leather_n.png"
                }
                normalStrength: 0.8
            }
            materials: [ spiderMaterial ]
            Model {
                source: "#Cone"
                scale: Qt.vector3d(0.4, 0.5, 0.5)
                position: Qt.vector3d(20, 10, -20)
                eulerRotation: Qt.vector3d(-40, -80, 0)
                materials: [ spiderMaterial ]
            }
            Model {
                source: "#Cone"
                scale: Qt.vector3d(0.4, 0.5, 0.5)
                position: Qt.vector3d(-20, 10, -20)
                eulerRotation: Qt.vector3d(-40, 80, 0)
                materials: [ spiderMaterial ]
            }
            SequentialAnimation on y {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 100
                    duration: 2500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 0
                    duration: 2500
                    easing.type: Easing.InOutQuad
                }
            }

            // System for tentacles particles
            ParticleSystem3D {
                id: psystemTentacles
                SpriteParticle3D {
                    id: tentacleParticle
                    sprite: Texture {
                        source: "images/dot.png"
                    }
                    maxAmount: 8 * mainWindow.tentacleEmitRate * 1.5
                    color: "#000000"
                    colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.2)
                    fadeInEffect: SpriteParticle3D.FadeNone
                    fadeOutDuration: 800
                    billboard: true
                    SequentialAnimation {
                        running: true
                        loops: Animation.Infinite
                        ColorAnimation {
                            target: tentacleParticle
                            property: "color"
                            to: "#202000"
                            duration: 2000
                        }
                        ColorAnimation {
                            target: tentacleParticle
                            property: "color"
                            to: "#200000"
                            duration: 2000
                        }
                        ColorAnimation {
                            target: tentacleParticle
                            property: "color"
                            to: "#000020"
                            duration: 2000
                        }
                        ColorAnimation {
                            target: tentacleParticle
                            property: "color"
                            to: "#002000"
                            duration: 2000
                        }
                    }
                    blendMode: SpriteParticle3D.Screen
                }

                // Emitters for all 8 tentacles
                component TentacleEmitter: Node {
                    id: tentacleNode
                    property real movementAmount: 1.0
                    ParticleEmitter3D {
                        z: 55
                        eulerRotation.x: 80 + mainWindow.tentacleWideness * tentacleNode.movementAmount
                        system: psystemTentacles
                        particle: tentacleParticle
                        velocity: VectorDirection3D {
                            direction: Qt.vector3d(0, 300, 0)
                        }
                        particleScale: 6.0
                        particleEndScale: 0.5
                        emitRate: mainWindow.tentacleEmitRate
                        lifeSpan: 1500
                    }
                }
                TentacleEmitter {
                    eulerRotation.y: 0.5 * 45
                    movementAmount: 1.8
                }
                TentacleEmitter {
                    eulerRotation.y: 1.5 * 45
                    movementAmount: 1.3
                }
                TentacleEmitter {
                    eulerRotation.y: 2.5 * 45
                    movementAmount: 1.5
                }
                TentacleEmitter {
                    eulerRotation.y: 3.5 * 45
                    movementAmount: 2.2
                }
                TentacleEmitter {
                    eulerRotation.y: 4.5 * 45
                    movementAmount: 1.6
                }
                TentacleEmitter {
                    eulerRotation.y: 5.5 * 45
                    movementAmount: 1.1
                }
                TentacleEmitter {
                    eulerRotation.y: 6.5 * 45
                    movementAmount: 1.5
                }
                TentacleEmitter {
                    eulerRotation.y: 7.5 * 45
                    movementAmount: 2.1
                }

                Gravity3D {
                    direction: Qt.vector3d(0, 1, 0)
                    magnitude: -300
                }

                Wander3D {
                    globalAmount: Qt.vector3d(15, 15, 15)
                    globalPace: Qt.vector3d(1.0, 1.0, 1.0)
                    fadeInDuration: 1000
                    Vector3dAnimation on globalPaceStart {
                        loops: Animation.Infinite
                        duration: 8000
                        from: Qt.vector3d(0, 0, 0)
                        to: Qt.vector3d(5 * Math.PI * 2, 3 * Math.PI * 2, Math.PI * 2)
                    }
                }
            }

            // Emitters for dust/smoke particles
            ParticleEmitter3D {
                id: smokeEmitter
                system: psystemDust
                particle: smokeParticle
                particleScale: 10
                particleScaleVariation: 5
                particleEndScale: 60
                particleEndScaleVariation: 30
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 40)
                emitRate: sliderDustEmitRate.sliderValue
                lifeSpan: 3000
                lifeSpanVariation: 1000
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 0, 200)
                    directionVariation: Qt.vector3d(50, 50, 50)
                }
            }
            ParticleEmitter3D {
                id: dustEmitter
                system: psystemDust
                particle: dustParticle
                emitRate: sliderDustEmitRate.sliderValue * 0.5
                lifeSpan: 3000
                lifeSpanVariation: 1000
                particleScale: 10.0
                particleScaleVariation: 5.0
                particleEndScale: 30.0
                particleEndScaleVariation: 10.0
                particleRotationVariation: Qt.vector3d(20, 20, 180)
                particleRotationVelocityVariation: Qt.vector3d(5, 5, 50)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 0, 100)
                    directionVariation: Qt.vector3d(20, 20, 20)
                }
            }

        }

        // System for dust/smoke particles
        ParticleSystem3D {
            id: psystemDust
            SpriteParticle3D {
                id: smokeParticle
                sprite: Texture {
                    source: "images/smoke_sprite.png"
                }
                maxAmount: 200
                spriteSequence: SpriteSequence3D {
                    frameCount: 15
                    interpolate: true
                }
                billboard: true
                color: tentacleParticle.color
                fadeInEffect: SpriteParticle3D.FadeNone
                fadeOutEffect: Particle3D.FadeOpacity
                fadeOutDuration: 1500
                blendMode: SpriteParticle3D.Screen
            }

            SpriteParticle3D {
                id: dustParticle
                sprite: Texture {
                    source: "images/dust.png"
                }
                maxAmount: 100
                color: "#ffffff"
                colorVariation: Qt.vector4d(0.4, 0.4, 0.4, 0.4)
                billboard: true
                fadeInDuration: 200
                fadeOutDuration: 1500
                blendMode: SpriteParticle3D.Screen
            }
        }

        // System for bubbles particles
        ParticleSystem3D {
            id: psystemBubbles
            position: Qt.vector3d(0, 100, -200)
            SpriteParticle3D {
                id: bubbleParticle
                sprite: Texture {
                    source: "images/sphere.png"
                }
                maxAmount: 300
                color: "#80ffffff"
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.2)
                fadeInDuration: 1000
                fadeOutDuration: 1000
                blendMode: SpriteParticle3D.Screen
                billboard: true
            }

            ParticleEmitter3D {
                particle: bubbleParticle
                scale: Qt.vector3d(12, 12, 12)
                particleScale: 2.0
                particleScaleVariation: 1.0
                particleEndScale: 1.0
                particleEndScaleVariation: 0.5
                emitRate: 100
                lifeSpan: 3000
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                }
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 0, 200)
                    directionVariation: Qt.vector3d(20, 20, 100)
                }
            }
        }
    }

    SettingsView {
        CustomLabel {
            text: "Camera Distance"
        }
        CustomSlider {
            id: sliderCameraDistance
            sliderValue: 400
            fromValue: 200
            toValue: 800
        }
        CustomLabel {
            text: "Mystical Dust"
        }
        CustomSlider {
            id: sliderDustEmitRate
            sliderValue: 20
            fromValue: 0
            toValue: 50
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystemTentacles, psystemBubbles, psystemDust]
    }
}
