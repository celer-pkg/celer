// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow

    readonly property real meterTicksAngle: 300
    readonly property bool particleNeedle: checkBoxParticlesNeedle.checked
    property real separation: sliderElementSeparation.sliderValue

    anchors.fill: parent

    Behavior on separation {
        SmoothedAnimation {
            velocity: 0.2
            duration: 1000
        }
    }

    View3D {
        id: view3D
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#161610"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 600 - mainWindow.separation * 100)
        }

        PointLight {
            position: Qt.vector3d(200, 400, 300)
            brightness: 5
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Background
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(25, 15, 0)
            z: -300
            materials: PrincipledMaterial {
                baseColor: "#505040"
            }
        }

        Node {
            id: speedometerComponent

            position: Qt.vector3d(0, 100, 0)
            eulerRotation.x: 90 - mainWindow.separation * 75

            // Light which follows the needle
            PointLight {
                // Calculates the direction of the needle
                function getNeedleAngle(startAngle) {
                    return Math.sin(startAngle +
                                    (-(180 / 360) +
                                     (-gaugeItem.value * mainWindow.meterTicksAngle / 360) +
                                     (mainWindow.meterTicksAngle / 2 + 180) / 360) * 2 * Math.PI);
                }

                readonly property real lightRadius: 120
                readonly property real posX: getNeedleAngle(Math.PI) * lightRadius
                readonly property real posY: getNeedleAngle(Math.PI / 2) * lightRadius

                position: Qt.vector3d(posX, 40, -posY)
                color: Qt.rgba(1.0, 0.8, 0.4, 1.0)
                brightness: sliderNeedleBrightness.sliderValue
                quadraticFade: 4.0
            }

            Model {
                y: -4 - mainWindow.separation * 100
                source: "meshes/meter_background.mesh"
                scale: Qt.vector3d(30, 30, 30)
                materials: PrincipledMaterial {
                    baseColor: "#505050"
                    metalness: 1.0
                    roughness: 0.6
                    normalMap: Texture {
                        source: "images/leather_n.png"
                    }
                    normalStrength: 0.4
                }
            }
            Model {
                y: -35 - mainWindow.separation * 100
                source: "#Sphere"
                scale: Qt.vector3d(1.6, 0.2, 1.6)
                materials: PrincipledMaterial {
                    baseColor: "#606060"
                }
            }
            Model {
                y: -mainWindow.separation * 60
                source: "#Cylinder"
                scale: Qt.vector3d(0.2, 0.8, 0.2)
                materials: PrincipledMaterial {
                    baseColor: "#606060"
                }
            }
            Model {
                y: 30 - mainWindow.separation * 60
                source: "#Sphere"
                scale: Qt.vector3d(0.4, 0.1, 0.4)
                materials: PrincipledMaterial {
                    baseColor: "#f0f0f0"
                }
            }

            // Speedometer labels
            Model {
                y: 25 + mainWindow.separation * 20
                source: "#Rectangle"
                scale: Qt.vector3d(4, 4, 4)
                eulerRotation.x: -90
                materials: PrincipledMaterial {
                    alphaMode: PrincipledMaterial.Blend
                    baseColorMap: Texture {
                        source: "images/speedometer_labels.png"
                    }
                }
            }

            Model {
                y: mainWindow.separation * 60
                source: "meshes/meter_edge.mesh"
                scale: Qt.vector3d(30, 30, 30)
                materials: PrincipledMaterial {
                    baseColor: "#b0b0b0"
                }
            }

            Node {
                id: gaugeItem

                property real value: 0
                property real needleSize: 180

                SequentialAnimation {
                    running: checkBoxAnimateSpeed.checked
                    loops: Animation.Infinite
                    NumberAnimation {
                        target: gaugeItem
                        property: "value"
                        duration: 8000
                        to: 1
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: gaugeItem
                        property: "value"
                        duration: 8000
                        to: 0
                        easing.type: Easing.InOutQuad
                    }
                }

                y: 20 - mainWindow.separation * 60
                eulerRotation.z: 90
                eulerRotation.y: -mainWindow.meterTicksAngle * value + (mainWindow.meterTicksAngle/2 - 90)

                Model {
                    position.y: gaugeItem.needleSize / 2
                    source: "#Cylinder"
                    scale: Qt.vector3d(0.05, gaugeItem.needleSize * 0.01, 0.2)
                    materials: PrincipledMaterial {
                        baseColor: "#606060"
                        opacity: mainWindow.particleNeedle ? 0.0 : 1.0
                    }

                    ParticleEmitter3D {
                        system: psystemGauge
                        particle: particleSpark
                        y: 10
                        scale: Qt.vector3d(0.1, 0.8, 0.1)
                        shape: ParticleShape3D {
                            type: ParticleShape3D.Cube
                        }
                        particleScale: 2.0
                        particleScaleVariation: 1.0
                        particleRotationVariation: Qt.vector3d(0, 0, 180)
                        particleRotationVelocityVariation: Qt.vector3d(20, 20, 200);
                        emitRate: 500
                        lifeSpan: 2000
                        lifeSpanVariation: 1000
                    }

                    // Needle particle system
                    // This system rotates together with the needle
                    ParticleSystem3D {
                        id: psystemNeedle
                        running: mainWindow.particleNeedle
                        visible: running
                        SpriteParticle3D {
                            id: needleParticle
                            sprite: Texture {
                                source: "images/dot.png"
                            }
                            maxAmount: 500
                            fadeInDuration: 50
                            fadeOutDuration: 200
                            color: "#40808020"
                            colorVariation: Qt.vector4d(0.2, 0.2, 0.0, 0.2)
                            blendMode: SpriteParticle3D.Screen
                        }

                        ParticleEmitter3D {
                            particle: needleParticle
                            y: -50
                            scale: Qt.vector3d(0.2, 0.0, 0.2)
                            shape: ParticleShape3D {
                                type: ParticleShape3D.Cube
                            }

                            particleScale: 4.0
                            particleScaleVariation: 2.0
                            particleEndScale: 1.0
                            particleEndScaleVariation: 0.5
                            velocity: VectorDirection3D {
                                direction: Qt.vector3d(0, 110, 0)
                                directionVariation: Qt.vector3d(10, 10, 10)
                            }
                            emitRate: sliderNeedleEmitrate.sliderValue
                            lifeSpan: 1000
                        }
                    }
                }
            }
            // Gauge particle system
            ParticleSystem3D {
                id: psystemGauge
                // Particles coming under the needle
                SpriteParticle3D {
                    id: particleSpark
                    sprite: Texture {
                        source: "images/star3.png"
                    }
                    maxAmount: 1500
                    fadeInEffect: SpriteParticle3D.FadeNone
                    fadeOutDuration: 200
                    color: "#40ff0000"
                    colorVariation: Qt.vector4d(0.4, 0.2, 0.2, 0.2)
                    unifiedColorVariation: true
                    blendMode: SpriteParticle3D.Screen
                }
                // Particles rotating the gauge
                SpriteParticle3D {
                    id: particleSpark2
                    sprite: Texture {
                        source: "images/star3.png"
                    }
                    maxAmount: 30
                    fadeInDuration: 1000
                    fadeOutDuration: 1000
                    color: "#20ffffff"
                    colorVariation: Qt.vector4d(0.2, 0.2, 0.5, 0.1)
                    blendMode: SpriteParticle3D.Screen
                }
                ParticleEmitter3D {
                    y: 40 + mainWindow.separation * 60
                    particle: particleSpark2
                    scale: Qt.vector3d(3.8, 0, 3.8)
                    shape: ParticleShape3D {
                        type: ParticleShape3D.Cylinder
                    }
                    emitRate: 10
                    lifeSpan: 3000
                    particleScale: 4.0
                    particleScaleVariation: 3.0
                    particleRotationVariation: Qt.vector3d(20, 20, 180)
                    particleRotationVelocityVariation: Qt.vector3d(10, 10, 50)
                }
                Wander3D {
                    uniqueAmount: Qt.vector3d(10, 0, 10)
                    uniquePace: Qt.vector3d(0.1, 0.1, 0.1)
                    fadeInDuration: 2000
                }
                PointRotator3D {
                    particles: [ particleSpark2 ]
                    magnitude: -20
                }
            }
        }
    }

    SettingsView {
        CustomCheckBox {
            id: checkBoxAnimateSpeed
            text: "Animate Speed"
            checked: true
        }
        CustomLabel {
            text: "Speed"
            opacity: !checkBoxAnimateSpeed.checked ? 1.0 : 0.4
        }
        CustomSlider {
            id: sliderSpeedValue
            sliderValue: gaugeItem.value
            sliderEnabled: !checkBoxAnimateSpeed.checked
            fromValue: 0
            toValue: 1
            onSliderValueChanged: gaugeItem.value = sliderValue;
        }
        CustomCheckBox {
            id: checkBoxParticlesNeedle
            text: "Particles Needle"
            checked: true
        }
        CustomLabel {
            text: "Needle emitRate"
            opacity: checkBoxParticlesNeedle.checked ? 1.0 : 0.4
        }
        CustomSlider {
            id: sliderNeedleEmitrate
            sliderValue: 200
            sliderEnabled: checkBoxParticlesNeedle.checked
            fromValue: 50
            toValue: 500
        }
        CustomLabel {
            text: "Needle Brightness"
        }
        CustomSlider {
            id: sliderNeedleBrightness
            sliderValue: 10
            fromValue: 0
            toValue: 50
        }
        CustomLabel {
            text: "Elements Separation"
        }
        CustomSlider {
            id: sliderElementSeparation
            sliderValue: 0
            fromValue: 0
            toValue: 1
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystemGauge, psystemNeedle]
    }
}
