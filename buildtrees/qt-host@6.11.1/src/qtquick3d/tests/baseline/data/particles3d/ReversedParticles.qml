// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    width: 1024
    height: 600
    id: mainWindow

    property real systemtime: 0
    property real fontSize: 14

    Text {
        x : mainWindow.width / 4
        text: "Normal"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }
    Text {
        id: textbox
        x : 3 * mainWindow.width / 4
        text: "Reversed"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }

    component ParticleSystem : ParticleSystem3D {
        id: psystem
        property bool reversedSystem
        useRandomSeed: false
        running: false

        startTime: 2000

        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, -100, 0)

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
            position: Qt.vector3d(0, -50, 0)
            system: psystem
            particle: sprites
            reversed: reversedSystem
            emitRate: 5
            lifeSpan: 3000
        }
        Gravity3D {

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
            position: Qt.vector3d(-250, 100, 500)
            clipFar: 2000
        }

        ParticleSystem {
            position: Qt.vector3d(-500, 300, 0)
        }
        ParticleSystem {
            position: Qt.vector3d(50, 300, 0)
            reversedSystem: true
        }
    }
}
