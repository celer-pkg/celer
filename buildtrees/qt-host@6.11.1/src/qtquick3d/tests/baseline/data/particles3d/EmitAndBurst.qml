// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick.Timeline

Item {
    id: mainWindow
    width: 400
    height: 400

    Timeline {
        id: timeline
        enabled: true
        startFrame: 0
        endFrame: 120
        animations: [
            TimelineAnimation {
                id: timelineAnimation
                running: true
                duration: 2000
                from: timeline.currentFrame
                to: 120
            }
        ]
        keyframeGroups: [
            KeyframeGroup {
                target: psystem
                property: "time"
                Keyframe { frame: 0; value: 0 }
                Keyframe { frame: 120; value: 2000 }
            }
        ]
    }

    View3D {
        id: view3D
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            position.z: 600
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
                materials: DefaultMaterial {
                }
            }
        }

        Node {
            ParticleSystem3D {
                id: psystem
                useRandomSeed: false
                running: false

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
                    emitRate: 100
                    SequentialAnimation on emitRate {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation {
                            duration: 1000
                            easing.type: Easing.InOutQuad
                            to: 100
                        }
                        NumberAnimation {
                            duration: 1000
                            easing.type: Easing.InOutQuad
                            to: 5
                        }
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
            }
        }
    }
}
