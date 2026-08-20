// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    anchors.fill: parent

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#101010"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
            clipFar: 2000
        }

        PointLight {
            position: Qt.vector3d(200, 600, 400)
            brightness: 40
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
        }

        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(30, 15, 15)
            eulerRotation.x: -90
            materials: [
                PrincipledMaterial {
                    baseColor: Qt.rgba(1.0, 1.0, 1.0, 1.0)
                }
            ]
        }

        Node {
            id: rotatingNode
            position.y: 150
            position.z: -200
            NumberAnimation on eulerRotation.y {
                running: true
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 10000
            }
            Model {
                x: 300
                source: "#Cube"
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.9, 0.9, 0.6, 1.0)
                }
                eulerRotation.x: rotatingNode.eulerRotation.y
                eulerRotation.z: 20
            }
            Model {
                x: -300
                source: "#Sphere"
                scale: Qt.vector3d(2.5, 2.5, 2.5)
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(0.9, 0.6, 0.6, 1.0)
                    opacity: 0.4
                }
            }
        }

        //! [particle system]
        ParticleSystem3D {
            id: psystem

            // Start so that the snowing is in full steam
            startTime: 15000
            //! [particle system]

            //! [sprite particle]
            SpriteParticle3D {
                id: snowParticle
                sprite: Texture {
                    source: "images/snowflake.png"
                }
                maxAmount: 1500 * sliderIntensity.sliderValue
                color: "#ffffff"
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.5);
                fadeInDuration: 1000
                fadeOutDuration: 1000
            }
            //! [sprite particle]

            //! [particle emitter]
            ParticleEmitter3D {
                id: emitter
                particle: snowParticle
                position: Qt.vector3d(0, 1000, -350)
                depthBias: -100
                scale: Qt.vector3d(15.0, 0.0, 15.0)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                }
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(50, 50, 50);
                particleScale: 2.0
                particleScaleVariation: 0.5;
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, sliderVelocityY.sliderValue, 0)
                    directionVariation: Qt.vector3d(0, sliderVelocityY.sliderValue * 0.4, 0)
                }
                emitRate: sliderEmitRate.sliderValue * sliderIntensity.sliderValue
                lifeSpan: 15000
            }
            //! [particle emitter]

            //! [particle affectors]
            Wander3D {
                enabled: checkBoxWanderEnabled.checked
                globalAmount: Qt.vector3d(sliderWanderGlobalAmount.sliderValue, 0, sliderWanderGlobalAmount.sliderValue)
                globalPace: Qt.vector3d(sliderWanderGlobalPace.sliderValue, 0, sliderWanderGlobalPace.sliderValue)
                uniqueAmount: Qt.vector3d(sliderWanderUniqueAmount.sliderValue, 0, sliderWanderUniqueAmount.sliderValue)
                uniquePace: Qt.vector3d(sliderWanderUniquePace.sliderValue, 0, sliderWanderUniquePace.sliderValue)
                uniqueAmountVariation: sliderWanderUniqueAmountVariation.sliderValue
                uniquePaceVariation: sliderWanderUniquePaceVariation.sliderValue
            }
            PointRotator3D {
                enabled: checkBoxRotatorEnabled.checked
                pivotPoint: Qt.vector3d(0, 0, -350)
                direction: Qt.vector3d(0, 1, 0)
                magnitude: sliderRotatorMagnitude.sliderValue
            }
            //! [particle affectors]
        }
    }

    SettingsView {
        CustomLabel {
            text: "Intensity of snowfall"
        }
        CustomSlider {
            id: sliderIntensity
            sliderValue: 5
            fromValue: 1
            toValue: 20
        }
        CustomLabel {
            text: "Emit Rate"
        }
        CustomSlider {
            id: sliderEmitRate
            sliderValue: 50
            fromValue: 1
            toValue: 100
        }
        CustomLabel {
            text: "Start Velocity Y"
        }
        CustomSlider {
            id: sliderVelocityY
            sliderValue: -100
            fromValue: -50
            toValue: -500
        }
        CustomLabel {
            text: "System Rotation"
        }
        CustomSlider {
            id: sliderRotation
            sliderValue: 0
            fromValue: -90
            toValue: 90
            onSliderValueChanged: {
                psystem.eulerRotation.z = sliderValue;
            }
        }
        CustomCheckBox {
            id: checkBoxWanderEnabled
            text: "Wander: enabled"
            checked: true
        }
        CustomLabel {
            text: "Wander: globalAmount"
        }
        CustomSlider {
            id: sliderWanderGlobalAmount
            sliderValue: 50
            fromValue: 0
            toValue: 100
        }
        CustomLabel {
            text: "Wander: globalPace"
        }
        CustomSlider {
            id: sliderWanderGlobalPace
            sliderValue: 0.2
            fromValue: 0
            toValue: 2.0
        }
        CustomLabel {
            text: "Wander: uniqueAmount"
        }
        CustomSlider {
            id: sliderWanderUniqueAmount
            sliderValue: 50
            fromValue: 0
            toValue: 100
        }
        CustomLabel {
            text: "Wander: uniquePace"
        }
        CustomSlider {
            id: sliderWanderUniquePace
            sliderValue: 0.2
            fromValue: 0
            toValue: 2.0
        }
        CustomLabel {
            text: "Wander: uniqueAmountVariation"
        }
        CustomSlider {
            id: sliderWanderUniqueAmountVariation
            sliderValue: 0.5
            fromValue: 0.0
            toValue: 1.0
        }
        CustomLabel {
            text: "Wander: uniquePaceVariation"
        }
        CustomSlider {
            id: sliderWanderUniquePaceVariation
            sliderValue: 0.5
            fromValue: 0.0
            toValue: 1.0
        }
        CustomCheckBox {
            id: checkBoxRotatorEnabled
            text: "PointRotator: enabled"
            checked: true
        }
        CustomLabel {
            text: "PointRotator: magnitude"
        }
        CustomSlider {
            id: sliderRotatorMagnitude
            sliderValue: 0.0
            fromValue: -100.0
            toValue: 100.0
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
