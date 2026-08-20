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
            position: Qt.vector3d(0, 100, 600)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 100)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Models shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.1, 0.1, 0.1)
                materials: PrincipledMaterial {
                }
            }
        }

        Component {
            id: particleComponent2
            Model {
                source: "#Cylinder"
                scale: Qt.vector3d(0.1, 0.2, 0.1)
                materials: PrincipledMaterial {
                    metalness: 0.5
                    roughness: 0
                    specularAmount: 1.0
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            SequentialAnimation on eulerRotation.y {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    to: 180
                    duration: 5000
                    easing.type: Easing.InOutCirc
                }
                NumberAnimation {
                    to: 0
                    duration: 5000
                    easing.type: Easing.InOutCirc
                }
            }

            property vector4d cVar: Qt.vector4d(sliderRedVariation.sliderValue,
                                                sliderGreenVariation.sliderValue,
                                                sliderBlueVariation.sliderValue,
                                                sliderAlphaVariation.sliderValue)
            // Particles
            ModelParticle3D {
                id: particleRed
                delegate: particleComponent
                maxAmount: 250
                color: "#ff0000"
                colorVariation: psystem.cVar
                unifiedColorVariation: checkBoxUnified.checked
            }
            ModelParticle3D {
                id: particleGreen
                delegate: particleComponent
                maxAmount: 250
                color: "#00ff00"
                colorVariation: psystem.cVar
                unifiedColorVariation: checkBoxUnified.checked
            }
            ModelParticle3D {
                id: particleBlue
                delegate: particleComponent
                maxAmount: 250
                color: "#0000ff"
                colorVariation: psystem.cVar
                unifiedColorVariation: checkBoxUnified.checked
            }
            ModelParticle3D {
                id: particleWhite
                delegate: particleComponent2
                maxAmount: 250
                color: "#ffffff"
                colorVariation: psystem.cVar
                unifiedColorVariation: checkBoxUnified.checked
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                particle: particleRed
                position: Qt.vector3d(400, 50, 0)
                particleScaleVariation: 0.8
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-350, 150, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 100
                lifeSpan: 2500
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "RED"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleGreen
                position: Qt.vector3d(400, 0, 0)
                particleScaleVariation: 0.8
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-350, 150, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 100
                lifeSpan: 2500
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "GREEN"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleBlue
                position: Qt.vector3d(400, -50, 0)
                particleScaleVariation: 0.8
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-350, 150, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 100
                lifeSpan: 2500
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "BLUE"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleWhite
                position: Qt.vector3d(400, -100, 0)
                particleScaleVariation: 0.8
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-350, 150, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 100
                lifeSpan: 2500
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "WHITE"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }

            Gravity3D {
                // Enable to affect only some of the particles
                //particles: [particleRed, particleBlue, particleGreen]
                direction: Qt.vector3d(0, 1, 0)
                magnitude: -100
            }
        }
    }

    SettingsView {
        CustomCheckBox {
            id: checkBoxUnified
            text: "Unified Variation"
            checked: false
        }
        CustomLabel {
            text: "Red Variation"
        }
        CustomSlider {
            id: sliderRedVariation
            sliderValue: 0
            fromValue: 0.0
            toValue: 1.0
        }
        CustomLabel {
            text: "Green Variation"
        }
        CustomSlider {
            id: sliderGreenVariation
            sliderValue: 0
            fromValue: 0.0
            toValue: 1.0
        }
        CustomLabel {
            text: "Blue Variation"
        }
        CustomSlider {
            id: sliderBlueVariation
            sliderValue: 0
            fromValue: 0.0
            toValue: 1.0
        }
        CustomLabel {
            text: "Alpha Variation"
        }
        CustomSlider {
            id: sliderAlphaVariation
            sliderValue: 0
            fromValue: 0.0
            toValue: 1.0
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
