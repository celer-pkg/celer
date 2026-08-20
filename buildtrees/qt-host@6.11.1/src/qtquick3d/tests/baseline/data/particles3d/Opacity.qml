// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick.Timeline

Item {
    id: mainWindow

    property real systemtime: 0

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
                target: mainWindow
                property: "systemtime"
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
            position.z: 700
        }

        PointLight {
            position: Qt.vector3d(0, 400, 300)
            brightness: 10
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                }
            }
        }

        component CustomParticleSystem: ParticleSystem3D {
            id: psystem
            useRandomSeed: false
            running: false
            time: systemtime

            // Particles
            ModelParticle3D {
                id: particleModel
                delegate: particleComponent
                maxAmount: 80
                color: "#ffffff"
            }
            SpriteParticle3D {
                id: particleSprite
                sprite: Texture {
                    source: "images/sphere.png"
                }
                maxAmount: 80
                color: "#ffffff"
                billboard: true
                particleScale: 50.0
            }

            ParticleEmitter3D {
                id: emitter1
                particle: particleModel
                position: Qt.vector3d(0, 10, 0)
                particleScale: 0.2
                particleEndScale: 1.5
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 150, 0)
                    directionVariation: Qt.vector3d(20, 0, 20)
                }
                lifeSpan: 3000
                emitRate: 20
            }
            ParticleEmitter3D {
                id: emitter2
                particle: particleSprite
                position: Qt.vector3d(0, 10, 0)
                particleScale: 0.2
                particleEndScale: 1.5
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, -150, 0)
                    directionVariation: Qt.vector3d(20, 0, 20)
                }
                lifeSpan: 3000
                emitRate: 20
            }
        }

        CustomParticleSystem {
            x: -300
            opacity: 1.0
        }
        CustomParticleSystem {
            x: -100
            opacity: 0.66
        }
        CustomParticleSystem {
            x: 100
            opacity: 0.33
        }
        CustomParticleSystem {
            x: 300
            opacity: 0.0
        }
    }
}
