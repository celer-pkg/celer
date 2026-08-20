// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    width: 1536
    height: 600
    id: mainWindow

    property real fontSize: 14

    Text {
        x : mainWindow.width / 6
        text: "Default"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }
    Text {
        x : 3 * mainWindow.width / 6 - 50
        text: "SurfaceNormal"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }
    Text {
        id: textbox
        x : 5 * mainWindow.width / 6 - 50
        text: "SurfaceReflected"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }

    component ParticleSystem : ParticleSystem3D {
        id: psystem
        property int systemEmitMode : ParticleEmitter3D.Default
        useRandomSeed: false
        running: false
        startTime: 2000

        Model {
            source: "#Sphere"
            position: Qt.vector3d(0, 0, 0)

            materials: [DefaultMaterial {
                    diffuseColor: "white"
                    lighting: DefaultMaterial.NoLighting
                }
            ]
        }

        SpriteParticle3D {
            id: sprites
            color: "red"
            particleScale: 10.0
            maxAmount: 200
        }

        ParticleEmitter3D {
            id: emitter
            position: Qt.vector3d(0, 0, 0)
            system: psystem
            particle: sprites
            emitMode: systemEmitMode
            emitRate: 5
            lifeSpan: 3000
            shape: ParticleShape3D {
                fill: false
                type: ParticleShape3D.Sphere
            }
            velocity: VectorDirection3D {
                direction: Qt.vector3d(0, 0, 100)
            }
        }
    }

    View3D {
        anchors.topMargin: textbox.height
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#101010"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.VeryHigh
        }
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 400, 500)
            clipFar: 2000
        }

        ParticleSystem {
            position: Qt.vector3d(-500, 300, 0)
        }
        ParticleSystem {
            position: Qt.vector3d(0, 300, 0)
            systemEmitMode: ParticleEmitter3D.SurfaceNormal
        }
        ParticleSystem {
            position: Qt.vector3d(500, 300, 0)
            systemEmitMode: ParticleEmitter3D.SurfaceReflected
        }
    }
}
