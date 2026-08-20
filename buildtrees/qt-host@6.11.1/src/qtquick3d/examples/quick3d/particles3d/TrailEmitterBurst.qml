// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow

    anchors.fill: parent

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

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
            position: Qt.vector3d(400, 600, 400)
            brightness: 80
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.5, 0.5, 0.5)
                materials: PrincipledMaterial {
                }
            }
        }

        Component {
            id: particleComponentStar
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(0.1, 0.1, 0.0)
                materials: PrincipledMaterial {
                    baseColorMap: Texture {
                        source: "images/star2.png"
                    }
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            // Particles
            ModelParticle3D {
                id: particleCube
                delegate: particleComponent
                maxAmount: 4
                color: "#ffffff"
                colorVariation: Qt.vector4d(0.4, 0.4, 0.4, 0.0)
                unifiedColorVariation: true
            }
            ModelParticle3D {
                id: particleSpark
                delegate: particleComponentStar
                maxAmount: 600
                fadeInDuration: 200
                fadeOutDuration: 500
                color: "#ffffff"
                colorVariation: Qt.vector4d(0.4, 0.4, 0.4, 0.8)
                unifiedColorVariation: true

            }
            ModelParticle3D {
                id: particleSpark2
                delegate: particleComponentStar
                maxAmount: 1000
                fadeInDuration: 200
                fadeOutDuration: 500
                color: "#ff6060"
                colorVariation: Qt.vector4d(0.5, 0.2, 0.2, 0.5)
                unifiedColorVariation: true
            }
            ModelParticle3D {
                id: particleStar
                delegate: particleComponentStar
                maxAmount: 1000
                color: "#ffee60"
                colorVariation: Qt.vector4d(0.6, 0.6, 0.2, 0.5)
                unifiedColorVariation: true
            }

            ParticleEmitter3D {
                particle: particleCube
                position: Qt.vector3d(-550, -300, 0)
                particleScaleVariation: 0.4
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(250, 400, 0)
                    directionVariation: Qt.vector3d(50, 50, 50)
                }
                emitRate: 1
                lifeSpan: 4000
            }

            TrailEmitter3D {
                id: trailEmitter
                particle: particleSpark
                follow: particleCube
                particleScale: 1.5
                particleScaleVariation: 0.5
                particleRotationVariation: Qt.vector3d(20, 20, 180)
                particleRotationVelocityVariation: Qt.vector3d(100, 100, 100);
                velocity: VectorDirection3D {
                    directionVariation: Qt.vector3d(20, 20, 20)
                }
                emitRate: sliderEmitRate.sliderValue
                lifeSpan: 1000
            }

            TrailEmitter3D {
                id: trailEmitter2
                particle: particleSpark2
                follow: particleCube
                particleScale: 2.5
                particleScaleVariation: 0.5
                particleRotationVariation: Qt.vector3d(20, 20, 180)
                particleRotationVelocityVariation: Qt.vector3d(100, 100, 100);
                velocity: VectorDirection3D {
                    directionVariation: Qt.vector3d(100, 100, 100)
                }
                lifeSpan: 1000
            }

            ParticleEmitter3D {
                id: starEmitter
                particle: particleStar
                particleScale: 5.0
                particleScaleVariation: 3.0
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(100, 100, 100);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 500, 0)
                    directionVariation: Qt.vector3d(80, 80, 80)
                }
                lifeSpan: 1000
            }

            Gravity3D {
                direction: Qt.vector3d(0, 1, 0)
                magnitude: -200
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            trailEmitter2.burst(50);
            var pos = view3D.mapTo3DScene(Qt.vector3d(mouseX, mouseY, camera.z));
            starEmitter.burst(sliderBurstAmount.sliderValue,
                              sliderBurstDuration.sliderValue,
                              pos);

        }
    }

    Text {
        anchors.centerIn: parent
        font.pointSize: AppSettings.fontSizeLarge
        color: "#ffffff"
        text: qsTr("Click to burst!")
    }

    SettingsView {
        CustomLabel {
            text: "Burst amount"
        }
        CustomSlider {
            id: sliderBurstAmount
            sliderValue: 100
            fromValue: 10
            toValue: 200
        }
        CustomLabel {
            text: "Burst duration"
        }
        CustomSlider {
            id: sliderBurstDuration
            sliderValue: 100
            fromValue: 0
            toValue: 1000
        }
        CustomLabel {
            text: "Trail emitRate"
        }
        CustomSlider {
            id: sliderEmitRate
            sliderValue: 100
            fromValue: 0
            toValue: 150
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
