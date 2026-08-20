// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: ambientParticleEffect
    ParticleEmitter3D {
        id: modelShapeEmitter18
        particleScaleVariation: 1
        particleEndScaleVariation: 1
        emitRate: 1000
        lifeSpanVariation: 100
        enabled: ambientParticleEffect.visible
        particleEndScale: 1
        SpriteParticle3D {
            id: modelShapeParticle
            color: "#8085edff"
            blendMode: SpriteParticle3D.Screen
            billboard: true
            sprite: ambientdot
            colorVariation.w: 0.5
            colorVariation.z: 0.25
            colorVariation.y: 0.25
            colorVariation.x: 0.25
            particleScale: 2
            fadeOutDuration: 2500
            fadeInDuration: 2500
            maxAmount: 5000

            Texture {
                id: ambientdot
                source: "images/ambientdot.tif"
            }
        }

        Wander3D {
            id: wander13
            uniquePaceVariation: 1
            uniquePace.z: 1
            uniquePace.y: 1
            uniquePace.x: 1
            uniqueAmountVariation: 1
            uniqueAmount.z: 1
            uniqueAmount.y: 1
            uniqueAmount.x: 1
            particles: [modelShapeParticle]
            system: ambientParticleEffect
        }

        Attractor3D {
            id: attractor1
            useCachedPositions: false
            hideAtEnd: false
            durationVariation: 5000
            duration: 10000
            positionVariation.z: 500
            positionVariation.y: 500
            positionVariation.x: 500
            particles: [modelShapeParticle]
            system: ambientParticleEffect
        }
        lifeSpan: 5000
        particle: modelShapeParticle
        shape: targetShape19
    }

    ParticleModelShape3D {
        id: targetShape19
        delegate: Model {
            source: "#Cube"
            scale.z: 10
            scale.y: 10
            scale.x: 10
        }
        fill: true
    }

    Node {
        id: __materialLibrary__
    }
}
