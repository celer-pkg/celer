// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow

    property int smokeAmount: 20
    property int explosionAmount: 100

    anchors.fill: parent

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
        }

        PointLight {
            id: light1
            position: Qt.vector3d(0, 400, 100)
            brightness: 5
            ambientColor: Qt.rgba(0.4, 0.3, 0.3, 1.0)
        }

        ParticleSystem3D {
            id: psystem

            SpriteParticle3D {
                id: spriteParticle
                sprite: Texture {
                    source: "images/sphere.png"
                }
                maxAmount: 3
                color: "#ffff80"
                colorTable: Texture {
                    source: "images/color_table5.png"
                }
                particleScale: 40.0
                fadeInEffect: SpriteParticle3D.FadeNone
                fadeOutEffect: SpriteParticle3D.FadeNone
            }
            SpriteParticle3D {
                id: spriteTrailParticle
                sprite: Texture {
                    source: "images/sphere.png"
                }
                maxAmount: 1000
                fadeInDuration: 500
                fadeOutDuration: 500
                fadeInEffect: SpriteParticle3D.FadeScale
                fadeOutEffect: SpriteParticle3D.FadeScale
                particleScale: 10.0
                colorTable: Texture {
                    source: "images/color_table3.png"
                }
                lights: [ light1 ]
            }
            SpriteParticle3D {
                id: explosionParticle
                sprite: Texture {
                    source: "images/star3.png"
                }
                maxAmount: 1000
                fadeInEffect: SpriteParticle3D.FadeScale
                fadeInDuration: 100
                fadeOutDuration: 500
                particleScale: 10.0
                colorTable: Texture {
                    source: "images/color_table3.png"
                }
                lights: [ light1 ]
            }
            SpriteParticle3D {
                id: smokeParticle
                sprite: Texture {
                    source: "images/explosion_01_strip13.png"
                }
                maxAmount: 1000
                spriteSequence: SpriteSequence3D {
                    frameCount: 13
                    interpolate: true
                }
                billboard: true
                color: "#2fffffff"
                colorVariation: Qt.vector4d(0.5, 0.5, 0.5, 0.1)
                unifiedColorVariation: true
                fadeOutEffect: Particle3D.FadeOpacity
                fadeOutDuration: 2000
                lights: [ light1 ]
            }

            ParticleEmitter3D {
                particle: spriteParticle
                position: Qt.vector3d(0, -100, 0)
                scale: Qt.vector3d(1, 1, 1)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Cube
                }
                particleScaleVariation: 0.2
                particleEndScale: 1.5
                particleEndScaleVariation: 0.5
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 400, 0)
                    directionVariation: Qt.vector3d(40, 40, 0)
                }
                emitRate: 1
                lifeSpan: 2500
                lifeSpanVariation: 500
                depthBias: -10
            }

            TrailEmitter3D {
                id: spriteTrailEmitter
                particle: spriteTrailParticle
                follow: spriteParticle
                particleScale: 2.0
                particleScaleVariation: 0.5
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, -10, 0)
                    directionVariation: Qt.vector3d(2, 2, 0)
                }
                emitRate: 100
                lifeSpan: 2000
                lifeSpanVariation: 500
            }

            TrailEmitter3D {
                id: spriteSmokeTrailEmitter
                particle: smokeParticle
                follow: spriteParticle
                particleScale: 8
                particleScaleVariation: 4
                particleEndScale: 45
                particleEndScaleVariation: 15
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 40)
                emitRate: 0
                lifeSpan: 3000
                lifeSpanVariation: 1000
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 0, 0)
                    directionVariation: Qt.vector3d(mainWindow.smokeAmount * 0.6, mainWindow.smokeAmount * 0.6, 0)
                }
                depthBias: -20
                emitBursts: [
                    DynamicBurst3D {
                        enabled: checkBoxStartBurst.checked
                        triggerMode: DynamicBurst3D.TriggerStart
                        amount: mainWindow.smokeAmount
                        amountVariation: mainWindow.smokeAmount * 0.4
                    },
                    DynamicBurst3D {
                        enabled: checkBoxEndBurst.checked
                        triggerMode: DynamicBurst3D.TriggerEnd
                        amount: mainWindow.smokeAmount
                        amountVariation: mainWindow.smokeAmount * 0.4
                    }
                ]
            }

            TrailEmitter3D {
                particle: explosionParticle
                follow: spriteParticle
                particleScale: 1.0
                particleScaleVariation: 0.8
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 40)
                emitRate: 0
                lifeSpan: 3000
                lifeSpanVariation: 1000
                velocity: TargetDirection3D {
                    position: Qt.vector3d(0, 300, 0)
                    positionVariation: Qt.vector3d(100, 100, 0)
                    normalized: true
                    magnitude: 20 + mainWindow.explosionAmount * 0.2
                    magnitudeVariation: magnitude * 0.5
                }
                emitBursts: [
                    DynamicBurst3D {
                        enabled: checkBoxExplosionBurst.checked
                        triggerMode: DynamicBurst3D.TriggerEnd
                        amount: mainWindow.explosionAmount
                        amountVariation: mainWindow.explosionAmount * 0.4
                    }
                ]
            }
            Gravity3D {
                direction: Qt.vector3d(0, 1, 0)
                magnitude: -200
                particles: [spriteParticle]
            }
            Gravity3D {
                direction: Qt.vector3d(0, 1, 0)
                magnitude: -40
                particles: [explosionParticle]
            }
        }
    }

    SettingsView {
        CustomCheckBox {
            id: checkBoxStartBurst
            text: "Smoke burst at start"
            checked: true
        }
        CustomCheckBox {
            id: checkBoxEndBurst
            text: "Smoke burst at end"
            checked: true
        }
        CustomLabel {
            text: "Smoke amount"
        }
        CustomSlider {
            id: smokeSlider
            sliderValue: mainWindow.smokeAmount
            fromValue: 5
            toValue: 50
            onSliderValueChanged: mainWindow.smokeAmount = sliderValue;
        }
        CustomCheckBox {
            id: checkBoxExplosionBurst
            text: "Explosion burst at end"
            checked: true
        }
        CustomLabel {
            text: "Explosion amount"
        }
        CustomSlider {
            id: explosionSlider
            sliderValue: mainWindow.explosionAmount
            fromValue: 20
            toValue: 200
            onSliderValueChanged: mainWindow.explosionAmount = sliderValue;
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
