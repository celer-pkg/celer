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

            SequentialAnimation {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    target: camera
                    property: "eulerRotation.x"
                    to: -90
                    duration: 20000
                    easing.type: Easing.Linear
                }
                NumberAnimation {
                    target: camera
                    property: "eulerRotation.x"
                    to: 0
                    duration: 2000
                    easing.type: Easing.Linear
                }
            }

            SequentialAnimation {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    target: camera
                    property: "position.y"
                    to: 1400
                    duration: 20000
                    easing.type: Easing.Linear
                }
                NumberAnimation {
                    target: camera
                    property: "position.y"
                    to: 0
                    duration: 2000
                    easing.type: Easing.Linear
                }
            }

            SequentialAnimation {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    target: camera
                    property: "position.z"
                    to: 0
                    duration: 20000
                    easing.type: Easing.Linear
                }
                NumberAnimation {
                    target: camera
                    property: "position.z"
                    to: 600
                    duration: 2000
                    easing.type: Easing.Linear
                }
            }
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
                source: "#Cone"
                scale: Qt.vector3d(0.1, 0.1, 0.1)
                materials: PrincipledMaterial {
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            // Particles
            ModelParticle3D {
                id: particleVelocity
                delegate: particleComponent
                maxAmount: 250
                color: "#ff0000"
                alignMode: Particle3D.AlignTowardsStartVelocity
            }

            ModelParticle3D {
                id: particleCamera
                delegate: particleComponent
                maxAmount: 250
                color: "#00ff00"
                alignMode: Particle3D.AlignTowardsTarget
                alignTargetPosition: camera.position
            }

            ModelParticle3D {
                id: particleNoAlign
                delegate: particleComponent
                maxAmount: 250
                color: "#0000ff"
            }

            SpriteParticle3D {
                id: particleBillboard
                sprite: Texture {
                    source: "images/snowflake.png"
                }
                maxAmount: 250
                color: "#ffffff"
                billboard: true
            }

            SpriteParticle3D {
                id: particleNoBillboard
                sprite: Texture {
                    source: "images/snowflake.png"
                }
                maxAmount: 250
                color: "#aaaaff"
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                particle: particleVelocity
                position: Qt.vector3d(400, 50, 0)
                particleScale: 2
                particleRotation: Qt.vector3d(90, 0, 0)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 100, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "StartVelocity"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }

            ParticleEmitter3D {
                particle: particleCamera
                position: Qt.vector3d(200, 50, 0)
                particleScale: 2
                particleRotation: Qt.vector3d(90, 0, 0)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 100, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Camera"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }

            ParticleEmitter3D {
                particle: particleNoAlign
                position: Qt.vector3d(0, 50, 0)
                particleScale: 2
                particleRotation: Qt.vector3d(90, 0, 0)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 100, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "NoAlign"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }

            ParticleEmitter3D {
                particle: particleBillboard
                position: Qt.vector3d(-200, 50, 0)
                particleScale: 8
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 100, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Billboard"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }

            ParticleEmitter3D {
                particle: particleNoBillboard
                position: Qt.vector3d(-400, 50, 0)
                particleScale: 8
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 100, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 10
                lifeSpan: 4000
                Node {
                    x: 20
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "NoBillboard"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }
            }
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
