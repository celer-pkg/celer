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

        // Model shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.4, 0.4, 0.4)
                materials: PrincipledMaterial {
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            // Particles
            ModelParticle3D {
                id: particleNone
                delegate: particleComponent
                maxAmount: 40
                color: "#ffffff"
                fadeInEffect: ModelParticle3D.FadeNone
                fadeOutEffect: ModelParticle3D.FadeNone
                hasTransparency: false
            }
            ModelParticle3D {
                id: particleOpacity
                delegate: particleComponent
                maxAmount: 40
                color: "#ffffff"
                fadeInEffect: ModelParticle3D.FadeOpacity
                fadeOutEffect: ModelParticle3D.FadeOpacity
                fadeInDuration: sliderFadeInDuration.sliderValue
                fadeOutDuration: sliderFadeOutDuration.sliderValue
            }
            ModelParticle3D {
                id: particleScale
                delegate: particleComponent
                maxAmount: 40
                color: "#ffffff"
                fadeInEffect: ModelParticle3D.FadeScale
                fadeOutEffect: ModelParticle3D.FadeScale
                fadeInDuration: sliderFadeInDuration.sliderValue
                fadeOutDuration: sliderFadeOutDuration.sliderValue
            }
            ModelParticle3D {
                id: particleScaleOpacity
                delegate: particleComponent
                maxAmount: 40
                color: "#ffffff"
                fadeInEffect: ModelParticle3D.FadeScale
                fadeOutEffect: ModelParticle3D.FadeOpacity
                fadeInDuration: sliderFadeInDuration.sliderValue
                fadeOutDuration: sliderFadeOutDuration.sliderValue
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                particle: particleNone
                position: Qt.vector3d(300, 200, 0)
                particleScaleVariation: 0.5
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-200, 0, 0)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 80
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "NONE"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleOpacity
                position: Qt.vector3d(300, 100, 0)
                particleScaleVariation: 0.5
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-200, 0, 0)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 80
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "OPACITY"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleScale
                position: Qt.vector3d(300, 0, 0)
                particleScaleVariation: 0.5
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-200, 0, 0)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 80
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "SCALE"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleScaleOpacity
                position: Qt.vector3d(300, -100, 0)
                particleScaleVariation: 0.5
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-200, 0, 0)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 80
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "SCALE -> OPACITY"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
        }
    }

    SettingsView {
        CustomLabel {
            text: "Fade In Duration (ms)"
        }
        CustomSlider {
            id: sliderFadeInDuration
            sliderValue: 1500
            fromValue: 0
            toValue: 4000
        }
        CustomLabel {
            text: "Fade Out Duration (ms)"
        }
        CustomSlider {
            id: sliderFadeOutDuration
            sliderValue: 1500
            fromValue: 0
            toValue: 4000
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
