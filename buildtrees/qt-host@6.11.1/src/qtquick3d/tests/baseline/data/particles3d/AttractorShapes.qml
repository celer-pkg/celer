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
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 0)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Model shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.15, 0.02, 0.02)
                eulerRotation: Qt.vector3d(20,-20,20)
                materials: DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                }
            }
        }

        ParticleSystem3D {
            id: psystem
            useRandomSeed: false
            running: false

            // Particles
            ModelParticle3D {
                id: particleRed
                delegate: particleComponent
                maxAmount: 1000
                color: "#ff0000"
                colorVariation: Qt.vector4d(0, 0, 0, 0.8)
            }
            ModelParticle3D {
                id: particleGreen
                delegate: particleComponent
                maxAmount: 1000
                color: "#00ff00"
                colorVariation: Qt.vector4d(0, 0, 0, 0.8)
            }
            ModelParticle3D {
                id: particleWhite
                delegate: particleComponent
                maxAmount: 1000
                color: "#ffffff"
                colorVariation: Qt.vector4d(0, 0, 0, 0.8)
            }

            Model {
                id: emittingSphere
                source: "#Sphere"
                scale: Qt.vector3d(2.0, 2.0, 2.0)
                materials: DefaultMaterial {
                    opacity: 0.2
                }

                position.x: -300
                position.y: -200
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                system: psystem
                position: emittingSphere.position
                particle: particleRed
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: true
                }
                emitRate: 500
                lifeSpan: 2000
            }

            ParticleEmitter3D {
                id: emitter2
                system: psystem
                position: emittingSphere.position
                particle: particleGreen
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: true
                }
                emitRate: 200
                lifeSpan: 5000
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 400, 0)
                    directionVariation: Qt.vector3d(50, 50, 50)
                }
                particleRotationVelocityVariation: Qt.vector3d(500.0, 500.0, 500.0)
            }

            ParticleEmitter3D {
                system: psystem
                particle: particleWhite
                position: Qt.vector3d(100, 50, 0)
                scale: Qt.vector3d(5.0, 5.0, 5.0)
                shape: ParticleCustomShape3D {
                    source: "qt_logo_in_4096.cbor"
                }
                emitRate: 500
                lifeSpan: 5000
            }

            Model {
                id: targetSphere
                source: "#Sphere"
                materials: DefaultMaterial {
                    opacity: 0.2
                }
                position.y: -200
            }

            Wander3D {
                particles: [particleGreen]
                uniqueAmount: Qt.vector3d(10, 10, 10)
                uniquePace: Qt.vector3d(1, 1, 1)
                uniqueAmountVariation: 8
                uniquePaceVariation: 0.8
            }

            Attractor3D {
                particles: [particleRed]
                // Attract into a position
                position: targetSphere.position
                positionVariation: Qt.vector3d(10, 10, 10)
                duration: 1500
                durationVariation: 500
                hideAtEnd: false
            }

            Attractor3D {
                particles: [particleGreen]
                // Attract into a shape
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: false
                }
                duration: 3000
                durationVariation: 500
                hideAtEnd: false
            }
        }
    }
}
