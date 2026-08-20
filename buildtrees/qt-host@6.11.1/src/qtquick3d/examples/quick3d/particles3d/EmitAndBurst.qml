// Copyright (C) 2023 The Qt Company Ltd.
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

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            z: 600
            y: -20
        }

        PointLight {
            position: Qt.vector3d(0, 400, 300)
            brightness: 40
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        // Model shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: PrincipledMaterial {
                }
            }
        }

        Node {
            SequentialAnimation on eulerRotation.x {
                loops: Animation.Infinite
                NumberAnimation {
                    to: -80
                    duration: 2500
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: 0
                    duration: 2500
                    easing.type: Easing.InOutQuad
                }
            }
            ParticleSystem3D {
                id: psystem

                // Particles
                ModelParticle3D {
                    id: particleWhite
                    delegate: particleComponent
                    maxAmount: 1200
                    color: "#ffffff"
                }

                ParticleEmitter3D {
                    id: emitter1
                    particle: particleWhite
                    position: Qt.vector3d(-300, -200, 0)
                    particleScale: 0.2
                    particleEndScale: 1.0
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 200, 0)
                        directionVariation: Qt.vector3d(20, 0, 20)
                    }
                    lifeSpan: 3000
                    emitRate: 100
                    Timer {
                        running: true
                        repeat: true
                        interval: 500
                        onTriggered: {
                            emitter1.enabled = !emitter1.enabled
                        }
                    }
                }
                Node {
                    position: emitter1.position
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: "Enabling\nEmitter"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }

                ParticleEmitter3D {
                    id: emitter2
                    particle: particleWhite
                    position: Qt.vector3d(-100, -200, 0)
                    particleScale: 0.2
                    particleEndScale: 1.0
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 200, 0)
                        directionVariation: Qt.vector3d(20, 0, 20)
                    }
                    lifeSpan: 3000
                    emitRate: 0.1
                    SequentialAnimation {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            target: emitter2
                            property: "emitRate"
                            duration: 2000
                            easing.type: Easing.InOutQuad
                            to: 100
                        }
                        NumberAnimation {
                            target: emitter2
                            property: "emitRate"
                            duration: 2000
                            easing.type: Easing.InOutQuad
                            to: 0.1
                        }
                    }
                }
                Node {
                    position: emitter2.position
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: "Animated\nemitRate"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }

                ParticleEmitter3D {
                    id: emitter3
                    particle: particleWhite
                    position: Qt.vector3d(100, -200, 0)
                    particleScale: 0.2
                    particleEndScale: 1.0
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 200, 0)
                        directionVariation: Qt.vector3d(20, 0, 20)
                    }
                    lifeSpan: 3000
                    Timer {
                        running: true
                        repeat: true
                        interval: 500
                        onTriggered: {
                            emitter3.burst(50);
                        }
                    }
                }
                Node {
                    position: emitter3.position
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: "Burst"
                        font.pointSize: AppSettings.fontSizeLarge
                        color: "#ffffff"
                    }
                }

                ParticleEmitter3D {
                    id: emitter4
                    particle: particleWhite
                    position: Qt.vector3d(300, -200, 0)
                    particleScale: 0.2
                    particleEndScale: 1.0
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 200, 0)
                        directionVariation: Qt.vector3d(20, 0, 20)
                    }
                    lifeSpan: 3000
                    emitRate: 10
                    Timer {
                        running: true
                        repeat: true
                        interval: 500
                        onTriggered: {
                            emitter4.burst(50);
                        }
                    }
                }
                Node {
                    position: emitter4.position
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: "Emit and\nBurst"
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
