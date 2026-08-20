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
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        // Particles light
        PointLight {
            id: pointLight
            property real animatedBrightness: 1.0
            position: psystemStar.position
            brightness: sliderLightBrightness.sliderValue * animatedBrightness
            // Add some liveness to the light
            SequentialAnimation {
                loops: Animation.Infinite
                running: true
                NumberAnimation {
                    target: pointLight
                    property: "animatedBrightness"
                    to: 1.2
                    duration: 1200
                    easing.type: Easing.OutElastic
                }
                NumberAnimation {
                    target: pointLight
                    property: "animatedBrightness"
                    to: 1.0
                    duration: 1600
                    easing.type: Easing.OutElastic
                }
            }
        }

        // General light to the scene
        PointLight {
            position: Qt.vector3d(300, 500, 600)
            // Brigness coming from the amount of trail particles
            brightness: 0.1 + sliderSmokeEmitRate.sliderValue * 0.4 + sliderStardustEmitRate.sliderValue * 0.1
            ambientColor: Qt.rgba(0.4, 0.1, 0.1, 1.0)
            color: "#ff8080"
        }

        // Animate the camera position & rotation
        Node {
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 300, 800)
                eulerRotation.x: -20
                SequentialAnimation on z {
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 500
                        duration: 4000
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 800
                        duration: 4000
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                NumberAnimation {
                    to: -20
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 20
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // Background wall
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(80, 50, 0)
            z: -500
            y: -600
            materials: PrincipledMaterial {
                baseColor: "#201010"
            }
        }

        // Qt Cube model
        Model {
            source: "#Cube"
            z: 10
            y: 20
            scale: Qt.vector3d(2.0, 2.0, 2.0)

            NumberAnimation on eulerRotation.y {
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 5000
            }
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "images/qt_logo2.png"
                }
                normalMap: Texture {
                    source: "images/qt_logo2_n.png"
                }
            }
        }

        // System for the trail particles
        ParticleSystem3D {
            id: psystemTrail

            SpriteParticle3D {
                id: smokeParticle
                sprite: Texture {
                    source: "images/smoke_sprite.png"
                }
                maxAmount: 600
                spriteSequence: SpriteSequence3D {
                    frameCount: 15
                    interpolate: true
                }
                billboard: true
                blendMode: SpriteParticle3D.Screen
                colorTable: Texture {
                    source: "images/color_table2.png"
                }
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.8)
                fadeInEffect: Particle3D.FadeScale
                fadeInDuration: 100
                fadeOutEffect: Particle3D.FadeOpacity
                fadeOutDuration: 1500
            }

            ParticleEmitter3D {
                id: smokeEmitter
                particle: smokeParticle
                position: psystemStar.position
                particleScale: 20
                particleScaleVariation: 5
                particleEndScale: 30
                particleEndScaleVariation: 10
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 40)
                emitRate: sliderSmokeEmitRate.sliderValue
                lifeSpan: sliderSmokeLifeSpan.sliderValue
                lifeSpanVariation: 1000
            }

            SpriteParticle3D {
                id: dustParticle
                sprite: Texture {
                    source: "images/dust.png"
                }
                maxAmount: 600
                blendMode: SpriteParticle3D.Screen
                color: "#ff8080"
                colorVariation: Qt.vector4d(0.2, 0.4, 0.4, 0.6)
                billboard: true
                fadeInDuration: 200
                fadeOutDuration: 1500
            }

            ParticleEmitter3D {
                id: dustEmitter
                particle: dustParticle
                position: psystemStar.position
                emitRate: sliderStardustEmitRate.sliderValue
                lifeSpan: sliderStardustLifeSpan.sliderValue
                lifeSpanVariation: 1000
                particleScale: 20.0
                particleScaleVariation: 10.0
                particleRotationVariation: Qt.vector3d(40, 40, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 10)
            }

            Wander3D {
                uniqueAmount: Qt.vector3d(40, 40, 0)
                uniqueAmountVariation: 0.8
                uniquePace: Qt.vector3d(0.1, 0.1, 0)
                uniquePaceVariation: 0.8
                fadeInDuration: 3000
            }
        }

        // System for the star sphere particles
        ParticleSystem3D {
            id: psystemStar
            position: Qt.vector3d(0, -200, 0)

            // Animate x & y in a heart shape
            // This could be done also with Timeline & Keyframes
            SequentialAnimation on x {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    to: -300
                    duration: 1500
                    easing.type: Easing.OutSine
                }
                NumberAnimation {
                    to: 300
                    duration: 3000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 0
                    duration: 1500
                    easing.type: Easing.InSine
                }
            }
            SequentialAnimation on y {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    to: 300
                    duration: 2250
                    easing.type: Easing.OutSine
                }
                NumberAnimation {
                    to: 150
                    duration: 750
                    easing.type: Easing.InQuad
                }
                NumberAnimation {
                    to: 300
                    duration: 750
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    to: -300
                    duration: 2250
                    easing.type: Easing.InSine
                }
            }

            Vector3dAnimation {
                running: true
                loops: Animation.Infinite
                target: psystemStar
                property: "eulerRotation"
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(0, 360, -720)
                duration: 4000
            }

            SpriteParticle3D {
                id: dotParticle
                sprite: Texture {
                    source: "images/sphere.png"
                }
                maxAmount: 200
                color: Qt.rgba(1.0, 0.8, 0.8, sliderLightBrightness.sliderValue / 300)
                colorVariation: Qt.vector4d(0.0, 0.4, 0.4, 0.2)
                blendMode: SpriteParticle3D.Screen
            }

            ParticleEmitter3D {
                id: dotParticleEmitter
                particle: dotParticle
                scale: Qt.vector3d(0.8, 0.8, 0.8)
                particleScale: 2.5
                particleScaleVariation: 2.0
                emitRate: 200
                lifeSpan: 1000
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: false
                }
            }
        }
    }

    SettingsView {
        CustomLabel {
            text: "Smoke emitrate"
        }
        CustomSlider {
            id: sliderSmokeEmitRate
            sliderValue: 50
            fromValue: 0
            toValue: 100
        }
        CustomLabel {
            text: "Smoke lifeSpan"
        }
        CustomSlider {
            id: sliderSmokeLifeSpan
            sliderValue: 3000
            fromValue: 1000
            toValue: 5000
        }
        CustomLabel {
            text: "Stardust emitrate"
        }
        CustomSlider {
            id: sliderStardustEmitRate
            sliderValue: 50
            fromValue: 0
            toValue: 100
        }
        CustomLabel {
            text: "Stardust lifeSpan"
        }
        CustomSlider {
            id: sliderStardustLifeSpan
            sliderValue: 4000
            fromValue: 1000
            toValue: 5000
        }
        CustomLabel {
            text: "Star brightness"
        }
        CustomSlider {
            id: sliderLightBrightness
            sliderValue: 50
            fromValue: 1
            toValue: 200
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystemStar, psystemTrail]
    }
}
