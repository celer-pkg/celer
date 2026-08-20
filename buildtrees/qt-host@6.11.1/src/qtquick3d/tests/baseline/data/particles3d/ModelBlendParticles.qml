// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick.Timeline

Item {
    id: mainWindow
    width: 400
    height: 400
    visible: true

    View3D {
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#404040"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: cameraRotation
            property real rot: 0.0
            eulerRotation: Qt.vector3d(-45, 0, 0)
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 1250)
                clipFar: 2000
            }
        }

        DirectionalLight {
            brightness: 50
            eulerRotation: Qt.vector3d(-10, -90, 0)
            castsShadow: true
            shadowFactor: 25
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
        }

        Model {
            source: "#Rectangle"
            eulerRotation: Qt.vector3d(-90, 0, 0)
            y: -30
            scale: Qt.vector3d(13, 13, 1)
            receivesShadows: true
            castsShadows: false
            materials: [
                DefaultMaterial {
                    diffuseColor: "#0c100c"
                }
            ]
        }

        ParticleSystem3D {
            id: psystem
            running: false
            useRandomSeed: false
            NumberAnimation on time {
                    loops: 1
                    from:0
                    to: 8000
                    duration: 10
                }

            Component {
                id: modelComponent
                Model {
                    source: "#Cube"
                    scale: Qt.vector3d(1, 1, 1)
                    receivesShadows: false
                    eulerRotation: Qt.vector3d(90, 0, 0)
                    position: Qt.vector3d(-400, 50, 0)
                    materials: [
                        PrincipledMaterial {
                            baseColor: "#41cd52"
                            metalness: 0.8
                            roughness: 0.1
                            specularAmount: 1
                            cullMode: Material.NoCulling
                        }
                    ]
                }
            }

            Node {
                id: translateNode
                x: 150
            }

            ModelBlendParticle3D {
                id: particle
                delegate: modelComponent
                endNode: translateNode
                modelBlendMode: "Explode"
                endTime: 1500
            }

            ParticleEmitter3D {
                id: emitter
                system: psystem
                particle: particle
                lifeSpan: particle.modelBlendMode === ModelBlendParticle3D.Explode ? 40000 : 4000
                emitRate: particle.maxAmount / 10
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(50, 50, 0)
                    directionVariation: Qt.vector3d(0, 10, 10)
                }
                particleRotation: Qt.vector3d(20, 0, 3)
                particleRotationVariation: Qt.vector3d(4, 0, 1)
                particleScale: 1.0
                particleEndScale: 1.0
            }
        }
    }
}
