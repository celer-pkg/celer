// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Node {
    id: smoke
    property real size: 1.0
    property color color: "gray"
    property real colorVariation: 0.5

    SpriteParticle3D {
        id: smokeSpriteParticle
        sprite: ParticleResources.smokeSpriteTexture
        maxAmount: 200
        spriteSequence: SpriteSequence3D {
            frameCount: 15
            interpolate: true
        }
        billboard: true
        color: smoke.color
        colorVariation: Qt.vector4d(smoke.colorVariation, smoke.colorVariation, smoke.colorVariation, 0.2)
        unifiedColorVariation: true
        fadeOutEffect: Particle3D.FadeOpacity
        fadeOutDuration: 2000
    }

    ParticleEmitter3D {
        system: ParticleResources.system
        enabled: smoke.visible
        id: smokeEmitter
        particle: smokeSpriteParticle
        particleScale: 25 * smoke.size
        particleScaleVariation: 10 * smoke.size
        particleEndScale: 35 * smoke.size
        particleEndScaleVariation: 15 * smoke.size
        particleRotationVariation: Qt.vector3d(0, 0, 180)
        particleRotationVelocityVariation: Qt.vector3d(0, 0, 40)
        emitRate: 30
        lifeSpan: 2000
        lifeSpanVariation: 1000
        velocity: VectorDirection3D {
            direction: Qt.vector3d(50.0, 70, -50.0)
            directionVariation: Qt.vector3d(10, 10, 10)
        }
    }
}
