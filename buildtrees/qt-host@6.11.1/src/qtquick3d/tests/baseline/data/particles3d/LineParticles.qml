// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers
import QtQuick.Timeline

Item {
    id: mainWindow
    width: 1200
    height: 600

    property real systemtime: 0
    property real fontSize: 14

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

    component LineEmitter : Node {
        id: root
        property alias emitterPosition : emitter.position
        property alias emitterScale : emitter.scale
        property alias emitRate : emitter.emitRate
        property alias particleCount : particle.maxAmount
        property real particleOpacity : 1.0
        property bool textureEnabled : false
        property bool variableLengthEnabled : false
        property alias lineSegmentCount : particle.segmentCount
        property alias particleScale : particle.particleScale
        property alias particleTexcoordMode : particle.texcoordMode
        property alias particleTexcoordScale : particle.texcoordMultiplier
        property alias particleEolFadeDuration : particle.eolFadeOutDuration
        property real lineLength : 50.0
        property alias lifespan : emitter.lifeSpan
        property real velocityX : 120.0
        property real velocityY : 0.0
        property real velocityZ : 0.0
        property real velocityVariation : 5.0
        property var system
        property string description : "desc"
        property real fontSize : 14.0
        property alias fadeOutDuration : particle.fadeOutDuration
        property alias fadeInDuration : particle.fadeInDuration

        ParticleEmitter3D {
            id: emitter
            system: root.system
            Texture {
                id: tex
                source: "images/qt_logo.png"
            }
            particle: LineParticle3D {
                id: particle
                sprite: textureEnabled ? tex : null
                maxAmount: 20
                color: Qt.rgba(1.0, 1.0, 1.0, particleOpacity)
                fadeInDuration: 0
                fadeOutDuration: 500
                texcoordMode: LineParticle3D.Relative
                billboard: false
                length: variableLengthEnabled ? -1.0 : root.lineLength
                lengthDeltaMin: variableLengthEnabled ? root.lineLength / root.lineSegmentCount : 10.0
                eolFadeOutDuration: 0
                particleScale: 20.0
                segmentCount: 5
            }
            velocity: VectorDirection3D {
                direction: Qt.vector3d(root.velocityX, root.velocityY, root.velocityZ)
                directionVariation: Qt.vector3d(root.velocityVariation, root.velocityVariation, root.velocityVariation)
            }
            emitRate: 2
            lifeSpan: 3000
            Text {
                x: -100
                text: description
                font.pixelSize: root.fontSize
                color: "white"
            }
        }
    }

    component LineEmitterSystem : ParticleSystem3D {

        property real gravityStrength: 0.0
        property bool variableLengthEnabled: false
        property int lifespan: 3000

        useRandomSeed: false
        running: false

        id: psystem

        Gravity3D {
            position: Qt.vector3d(600, 100, 0)
            magnitude: psystem.gravityStrength
            direction: Qt.vector3d(1, 0, 0)
        }

        LineEmitter {
            description: "length 50"
            fontSize: mainWindow.fontSize
            system: psystem
            emitterPosition: Qt.vector3d(0, 0, 0)
            variableLengthEnabled: psystem.variableLengthEnabled
            lifespan: psystem.lifespan
        }

        LineEmitter {
            description: "length 100"
            fontSize: mainWindow.fontSize
            system: psystem
            emitterPosition: Qt.vector3d(0, 50, 0)
            lineLength: 100
            velocityX: 240
            lifespan: psystem.lifespan - 1000
            variableLengthEnabled: psystem.variableLengthEnabled
        }

        LineEmitter {
            description: "End-of-Life fade"
            fontSize: mainWindow.fontSize
            system: psystem
            emitterPosition: Qt.vector3d(0, 100, 0)
            particleEolFadeDuration: 1000
            fadeOutDuration: 0
            variableLengthEnabled: psystem.variableLengthEnabled
            lifespan: psystem.lifespan
        }

        LineEmitter {
            description: "textured"
            fontSize: mainWindow.fontSize
            system: psystem
            emitterPosition: Qt.vector3d(0, 150, 0)
            textureEnabled: true
            variableLengthEnabled: psystem.variableLengthEnabled
            lifespan: psystem.lifespan
        }

        LineEmitter {
            description: "textured, absolute coordinates"
            fontSize: mainWindow.fontSize
            system: psystem
            emitterPosition: Qt.vector3d(0, 200, 0)
            textureEnabled: true
            particleTexcoordMode: LineParticle3D.Absolute
            variableLengthEnabled: psystem.variableLengthEnabled
            lifespan: psystem.lifespan
        }

        LineEmitter {
            description: "textured, filled"
            fontSize: mainWindow.fontSize
            system: psystem
            emitterPosition: Qt.vector3d(0, 250, 0)
            textureEnabled: true
            particleTexcoordMode: LineParticle3D.Fill
            variableLengthEnabled: psystem.variableLengthEnabled
            lifespan: psystem.lifespan
        }
    }

    Text {
        x : mainWindow.width / 4
        text: "Fixed length"
        font.pixelSize: mainWindow.fontSize
        color: "black"
    }
    Text {
        id: textbox
        x : 3 * mainWindow.width / 4
        text: "Variable length"
        font.pixelSize: mainWindow.fontSize
        color: "black"
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
            position: Qt.vector3d(-50, 100, 500)
            clipFar: 2000
        }

        Model {
            source: "#Cube"
            position: Qt.vector3d(-400, -50, 0)
            eulerRotation.y: mainWindow.systemtime * 0.1
            scale: Qt.vector3d(0.5, 0.5, 0.001)
            materials: [
                DefaultMaterial {
                    diffuseColor: "white"
                    lighting: DefaultMaterial.NoLighting
                }
            ]
        }

        LineEmitterSystem {
            id: fixedSize
            position: Qt.vector3d(-500, 0, 0)
            time: mainWindow.systemtime
        }

        LineEmitterSystem {
            id: variableSize
            position: Qt.vector3d(100, 0, 0)
            gravityStrength: 150.0
            variableLengthEnabled: true
            lifespan: 2000
            time: mainWindow.systemtime
        }
    }
}
