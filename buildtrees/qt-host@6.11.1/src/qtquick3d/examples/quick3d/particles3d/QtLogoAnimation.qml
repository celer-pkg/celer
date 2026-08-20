// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick.Controls
import QtQuick.Timeline
import QtQuick.Layouts

Item {
    id: mainWindow

    property real cubeRotation: 0

    anchors.fill: parent

    Timeline {
        id: timeline
        enabled: true
        startFrame: 0
        endFrame: 100
        animations: [
            TimelineAnimation {
                id: timelineAnimation
                running: true
                duration: (100 - timeline.currentFrame) * 100 //10000
                from: timeline.currentFrame
                to: 100
            }
        ]
        keyframeGroups: [
            KeyframeGroup {
                target: mainWindow
                property: "cubeRotation"
                Keyframe { frame: 0; value: 0 }
                Keyframe { frame: 100; value: 360 }
            },
            KeyframeGroup {
                target: psystem1
                property: "time"
                Keyframe { frame: 0; value: 3001 }
                Keyframe { frame: 50; value: 0; easing.type: Easing.OutQuad }
                Keyframe { frame: 55; value: 0 }
                Keyframe { frame: 80; value: 3001 }
            },
            KeyframeGroup {
                target: psystem2
                property: "time"
                Keyframe { frame: 50; value: 0; easing.type: Easing.InQuad }
                Keyframe { frame: 80; value: 5000 }
            },
            KeyframeGroup {
                target: qtCube
                property: "opacity"
                Keyframe { frame: 60; value: 0 }
                Keyframe { frame: 70; value: 0.99 }
                Keyframe { frame: 90; value: 0.99 }
                Keyframe { frame: 100; value: 0.0 }
            }
        ]
    }

    View3D {
        id: view3D
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            position.z: 600
        }

        PointLight {
            position: Qt.vector3d(200, 400, 200)
            brightness: 10
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
        }

        // Particle models
        Component {
            id: dotParticleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.02, 0.02, 0.02)
                materials: PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                }
            }
        }
        Component {
            id: smokeParticleComponent
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(6, 6, 6)
                materials: PrincipledMaterial {
                    baseColorMap: Texture { source: "images/smoke.png" }
                    lighting: PrincipledMaterial.NoLighting
                }
                opacity: 0.2
            }
        }

        Component {
            id: starParticleComponent
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(0.5, 0.5, 0.5)
                materials: PrincipledMaterial {
                    baseColorMap: Texture { source: "images/star.png" }
                    lighting: PrincipledMaterial.NoLighting
                    cullMode: PrincipledMaterial.NoCulling
                }
                opacity: 0.2
            }
        }

        Node {
            eulerRotation: Qt.vector3d(20, -40 + mainWindow.cubeRotation, -10 + mainWindow.cubeRotation)

            Model {
                id: qtCube
                source: "#Cube"
                scale: Qt.vector3d(2.0, 2.0, 2.0)
                opacity: 0
                materials: PrincipledMaterial {
                    baseColorMap: Texture { source: "images/qt_logo.png" }
                }
            }

            ParticleSystem3D {
                id: psystem1
                // We animate this system time manually
                running: false

                ModelParticle3D {
                    id: particleWhite
                    delegate: dotParticleComponent
                    maxAmount: 2000
                    color: "#ffffff"
                    colorVariation: Qt.vector4d(0, 0, 0, 0.8)
                    fadeInEffect: ModelParticle3D.FadeNone
                    fadeOutEffect: ModelParticle3D.FadeOpacity
                    fadeOutDuration: 3000
                }

                ParticleEmitter3D {
                    id: emitter1
                    particle: particleWhite
                    scale: Qt.vector3d(2.0, 2.0, 2.0)
                    shape: ParticleShape3D {
                        type: ParticleShape3D.Cube
                        fill: false
                    }
                    velocity: TargetDirection3D {
                        magnitude: -0.6
                        magnitudeVariation: 0.4
                    }
                    lifeSpan: 3000
                    emitBursts: [
                        EmitBurst3D {
                            time: 0
                            amount: 2000
                        }
                    ]
                }

                Wander3D {
                    uniqueAmount: Qt.vector3d(40.0, 40.0, 40.0)
                    uniquePace: Qt.vector3d(0.2, 0.2, 0.2)
                    uniqueAmountVariation: 0.5
                    uniquePaceVariation: 0.5
                    fadeInDuration: 1000
                }
            }
        }

        ParticleSystem3D {
            id: psystem2

            // We animate this system time manually
            running: false

            ModelParticle3D {
                id: smokeParticle
                delegate: smokeParticleComponent
                maxAmount: 20
                color: "#ffffff"
                colorVariation: Qt.vector4d(0, 0, 0, 0.5)
                fadeInEffect: ModelParticle3D.FadeScale
                fadeOutEffect: ModelParticle3D.FadeOpacity
                fadeOutDuration: 2000
            }
            ModelParticle3D {
                id: starParticle
                delegate: starParticleComponent
                maxAmount: 50
                color: "#ffff00"
                colorVariation: Qt.vector4d(0.4, 0.6, 0, 0.1)
                unifiedColorVariation: true
                fadeInEffect: ModelParticle3D.FadeScale
                fadeOutEffect: ModelParticle3D.FadeOpacity
                fadeOutDuration: 2000
            }

            ParticleEmitter3D {
                id: emitter2
                particle: smokeParticle
                scale: Qt.vector3d(0.1, 0.1, 0.1)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                }
                particleRotationVariation: Qt.vector3d(20, 20, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 100)
                velocity: TargetDirection3D {
                    normalized: true
                    magnitude: -200.0
                    magnitudeVariation: 0.5
                }
                lifeSpan: 4000
                emitBursts: [
                    EmitBurst3D {
                        time: 400
                        amount: 20
                        duration: 600
                    }
                ]
            }
            ParticleEmitter3D {
                id: emitter3
                particle: starParticle
                scale: Qt.vector3d(0.1, 0.1, 0.1)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                }
                particleScale: 2.0
                particleScaleVariation: 1.0
                particleEndScale: 5.0
                particleEndScaleVariation: 3.0
                particleRotationVariation: Qt.vector3d(0, 0, 180)
                particleRotationVelocityVariation: Qt.vector3d(0, 0, 200)
                velocity: TargetDirection3D {
                    normalized: true
                    magnitudeVariation: 150
                }
                lifeSpan: 2500
                emitBursts: [
                    EmitBurst3D {
                        time: 1
                        amount: 50
                    }
                ]
            }
        }
    }

    Frame {
        id: toolbar
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        height: 60
        padding: 0
        background: Rectangle {
            color: "#ffffff"
            radius: 4
            opacity: 0.2
        }

        RowLayout {
            anchors.fill: parent
            Button {
                id: playButton
                Layout.leftMargin: 14
                Layout.minimumHeight: toolbar.height - 10
                Layout.minimumWidth: Layout.minimumHeight
                background: Rectangle {
                    color : "transparent"
                }
                icon.source: timelineAnimation.running ? "qrc:/images/icon_pause.png" : "qrc:/images/icon_play.png"
                icon.width: Layout.minimumWidth
                icon.height: Layout.minimumHeight
                icon.color: "transparent"
                onClicked: {
                    // If we are close to end, start from the beginning
                    if (timeline.currentFrame >= timeline.endFrame - 1.0)
                        timeline.currentFrame = 0;

                    timelineAnimation.running = !timelineAnimation.running;
                }
            }

            CustomSlider {
                id: sliderTimelineTime
                Layout.fillWidth: true
                sliderValue: timeline.currentFrame
                sliderEnabled: !timelineAnimation.running || timelineAnimation.paused
                fromValue: 0.0
                toValue: 100.0
                onSliderValueChanged: timeline.currentFrame = sliderValue;
            }
        }
    }

    LoggingView {
        anchors.bottom: toolbar.top
        anchors.bottomMargin: 8
        particleSystems: [psystem1, psystem2]
    }
}
