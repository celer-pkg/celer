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
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 1000)
        }

        PointLight {
            position: Qt.vector3d(200, 400, 800)
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
            y: -100
            ParticleSystem3D {
                id: psystem
                useRandomSeed: false
                running: false

                NumberAnimation {
                    running: true
                    target: psystem
                    property: "eulerRotation.z"
                    from: 0
                    to: 90
                    duration: 2000
                }

                // Particles
                ModelParticle3D {
                    id: particleWhite
                    delegate: particleComponent
                    maxAmount: 250
                    color: "#ffffff"
                }
                ModelParticle3D {
                    id: particleGreen
                    delegate: particleComponent
                    maxAmount: 250
                    color: "#00ff00"
                }
                SpriteParticle3D {
                    id: particleBillboard
                    sprite: Texture {
                        source: "images/sphere.png"
                    }
                    maxAmount: 250
                    color: "#ff0000"
                    billboard: true
                }

                // Emitter directly inside the system
                ParticleEmitter3D {
                    id: emitter3
                    system: psystem
                    particle: particleGreen
                    x: -100
                    particleScale: 1.0
                    particleEndScale: 0.2
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(250, 0, 0)
                        directionVariation: Qt.vector3d(0, 30, 0)
                    }
                    lifeSpan: 3000
                    emitRate: 20
                    Component.onCompleted: emitter3.burst(100)
                }

                // Emitter inside the system + node.
                Node {
                    y: -200
                    scale: Qt.vector3d(2, 2, 2)
                    ParticleEmitter3D {
                        id: emitter1
                        system: psystem
                        particle: particleWhite
                        y: 100
                        x: -50
                        particleScale: 0.2
                        particleEndScale: 1.0
                        particleRotationVariation: Qt.vector3d(180, 180, 180)
                        velocity: VectorDirection3D {
                            direction: Qt.vector3d(0, 200, 0)
                            directionVariation: Qt.vector3d(40, 0, 30)
                        }
                        lifeSpan: 3000
                        emitRate: 50
                        Component.onCompleted: emitter1.burst(100)

                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(0.2, 0.2, 0.2)
                            materials: DefaultMaterial {
                            }
                        }
                    }
                }
            }

            // Emitter outside the system + node
            Node {
                eulerRotation.z: -90
                scale: Qt.vector3d(2, 2, 2)
                ParticleEmitter3D {
                    id: emitter2
                    system: psystem
                    particle: particleBillboard
                    position.y: -50
                    eulerRotation.z: -90
                    particleScale: 4
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 100, 0)
                        directionVariation: Qt.vector3d(20, 0, 30)
                    }
                    lifeSpan: 3000
                    emitRate: 50
                    Component.onCompleted: emitter2.burst(100)
                }
            }
        }
    }
}
