// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: rainSystem

    property alias scene : particlePlane.scene
    property alias center : particlePlane.sceneCenter
    property alias extents : particlePlane.sceneExtents
    property alias enabled : emitter.enabled
    property alias excluded : particlePlane.excludedNodes
    property alias geometry : particlePlane.geometry

    LineParticle3D {
        id: rainParticle
        sprite: Texture {
            source: "images/dot.png"
        }
        maxAmount: 180000
        color: "#c044488F"
        segmentCount: 3
        texcoordMode: LineParticle3D.Fill
        particleScale: 0.5
    }
    SpriteParticle3D {
        id: splashParticle
        sprite: Texture {
            source: "images/dot.png"
        }
        maxAmount: 300000
        color: "#c044488F"
        particleScale: 2
        fadeOutDuration: 500
        fadeOutEffect: SpriteParticle3D.FadeOpacity

    }
    TrailEmitter3D {
        follow: rainParticle
        DynamicBurst3D {
            id: splashBurst
            amount: 20
            amountVariation: 2
            triggerMode: DynamicBurst3D.TriggerEnd
        }
        emitBursts: splashBurst
        particle: splashParticle
        velocity: VectorDirection3D {
            direction: Qt.vector3d(0, 0, 10)
            directionVariation: Qt.vector3d(50, 50, 1)
        }
        emitMode: ParticleEmitter3D.SurfaceReflected
        particleScale: 0.8
        particleScaleVariation: 0.3
        particleEndScale: 0.1
        particleEndScaleVariation: 0.1
        lifeSpan: 300
    }
    TrailEmitter3D {
        id: poolEmitter
        lifeSpan: 500
        particle: poolParticle
        particleScale: 10
        follow: rainParticle
        emitBursts: poolBurst
        emitMode: ParticleEmitter3D.SurfaceReflected

        SpriteParticle3D {
            id: poolParticle
            color: "#07ecf9ff"
            maxAmount: 30000
            sprite: poolTexture
            fadeOutEffect: Particle3D.FadeOpacity
            fadeInEffect: Particle3D.FadeScale
            fadeOutDuration: 400
            fadeInDuration: 100
            Texture {
                id: poolTexture
                source: "images/ripple.png"
            }
        }

        DynamicBurst3D {
            id: poolBurst
            triggerMode: DynamicBurst3D.TriggerEnd
            amount: 1
        }
    }

    ParticleEmitter3D {
        id: emitter
        particle: rainParticle
        reversed: true
        lifeSpan: 1000
        shape: ParticleSceneShape3D {
            id: particlePlane
        }
        velocity: VectorDirection3D {
            direction: Qt.vector3d(0, 800, 0)
            directionVariation: Qt.vector3d(2, 0, 2)
        }
        particleRotationVariation: Qt.vector3d(0, 90, 0)
        particleRotationVelocity: Qt.vector3d(0, 60, 0)
        particleRotationVelocityVariation: Qt.vector3d(0, 30, 0)
        emitRate: 4000
    }
    Gravity3D {
        magnitude: 100
    }
}
