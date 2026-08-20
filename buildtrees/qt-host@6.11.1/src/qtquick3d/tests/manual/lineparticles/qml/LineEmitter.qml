// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers


Node {
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
