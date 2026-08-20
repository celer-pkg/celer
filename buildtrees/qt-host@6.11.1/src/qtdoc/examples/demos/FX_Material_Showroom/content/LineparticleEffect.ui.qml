// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: lineparticleEffect

    ParticleEmitter3D {
        id: particleEmitter
        particleEndScaleVariation: 1
        particleScaleVariation: 0
        system: lineparticleEffect
        enabled: lineparticleEffect.visible
        particleScale: 0
        particleRotationVariation.y: 180
        particleRotationVelocityVariation.y: 200
        emitRate: 5
        particleRotationVelocityVariation.x: 200
        particleRotationVelocityVariation.z: 200
        particleRotationVariation.z: 180
        lifeSpanVariation: 5000
        velocity: dir3d
        particleEndScale: 1
        VectorDirection3D {
            id: dir3d
            directionVariation.z: 50
            direction.y: 15
            direction.z: 0
            directionVariation.y: 15
            directionVariation.x: 50
        }

        PointRotator3D {
            id: pointRotator
            magnitude: 100
            particles: [lineParticle]
            system: lineparticleEffect
        }
        lifeSpan: 5000
        particle: lineParticle
        particleRotationVariation.x: 180
    }

    LineParticle3D {
        id: lineParticle
        sprite: lineSprite
        fadeOutDuration: 2500
        fadeInDuration: 2500
        color: "#7fffffff"
        billboard: false
        colorVariation.w: 0.5
        colorVariation.z: 0.5
        colorVariation.y: 0.5
        colorVariation.x: 0.5
        blendMode: SpriteParticle3D.Screen
        alignMode: Particle3D.AlignNone
        particleScale: 1
        eolFadeOutDuration: 500
        lengthVariation: 500
        length: 500
        alphaFade: 0.1
        segmentCount: 200

        Texture {
            id: lineSprite
            source: "images/lineSprite.tif"
        }
    }

    Node {
        id: __materialLibrary__
    }

    ParticleSystem3D {
        id: lineparticlesystem_thick
        ParticleEmitter3D {
            id: lineparticle_thick_emitter
            particle: lineparticle_Thick
            emitRate: 5
            particleEndScaleVariation: 15
            particleRotationVariation.y: 180
            particleRotationVelocityVariation.z: 200
            particleRotationVariation.x: 180
            lifeSpanVariation: 5000
            velocity: lineparticle_thick_dir
            enabled: lineparticleEffect.visible
            lifeSpan: 5000
            particleEndScale: 15
            particleScale: 0
            VectorDirection3D {
                id: lineparticle_thick_dir
                directionVariation.y: 15
                direction.y: 15
                directionVariation.x: 50
                directionVariation.z: 50
                direction.z: 0
            }

            PointRotator3D {
                id: lineparticle_thick_rotator
                magnitude: 100
                particles: [lineparticle_Thick]
                system: lineparticlesystem_thick
            }
            particleRotationVariation.z: 180
            particleScaleVariation: 0
            particleRotationVelocityVariation.y: 200
            particleRotationVelocityVariation.x: 200
            system: lineparticlesystem_thick
        }

        LineParticle3D {
            id: lineparticle_Thick
            texcoordMode: LineParticle3D.Fill
            texcoordMultiplier: 1
            sprite: lineSprite
            particleScale: 1
            fadeOutDuration: 2500
            fadeInDuration: 2500
            blendMode: SpriteParticle3D.Screen
            color: "#40ffbb00"
            billboard: false
            colorVariation.w: 0.25
            colorVariation.z: 0.25
            colorVariation.y: 0.25
            colorVariation.x: 0.25
            sortMode: Particle3D.SortDistance
            scaleMultiplier: 1
            length: 500
            alphaFade: 0
            segmentCount: 200
            alignMode: Particle3D.AlignNone
            lengthVariation: 500
            eolFadeOutDuration: 500
        }
    }

    ParticleSystem3D {
        ParticleEmitter3D {
            lifeSpanVariation: 5000
            emitRate: 20
            particleScaleVariation: 0
            enabled: lineparticleEffect.visible
            particle: spriteParticle
            VectorDirection3D {
                id: dotvector
                direction.y: 15
                directionVariation.x: 50
                directionVariation.z: 50
                direction.z: 0
                directionVariation.y: 15
            }

            PointRotator3D {
                id: dotrotator
                magnitude: 50
                system: lineparticlesystem_thick
                particles: [spriteParticle]
            }

            Wander3D {
                uniquePace.z: 1
                uniquePaceVariation: 1
                uniquePace.x: 1
                uniquePace.y: 1
                uniqueAmount.x: 1
                uniqueAmountVariation: 1
                system: lineparticlesystem_thick
                uniqueAmount.y: 1
                uniqueAmount.z: 1
                particles: [spriteParticle]
            }

            SpriteParticle3D {
                id: spriteParticle
                sprite: dotsprite
                billboard: true
                color: "#ffcc40"
                maxAmount: 1000
                blendMode: SpriteParticle3D.Screen
                colorVariation.w: 0.5
                colorVariation.z: 0.5
                colorVariation.y: 0.5
                colorVariation.x: 0.5
                sortMode: Particle3D.SortDistance
            }

            Texture {
                id: dotsprite
                source: "images/dotSprite.tif"
            }
            particleEndScaleVariation: 1
            particleEndScale: 1
            particleScale: 0
            system: lineparticlesystem_thick
            velocity: dotvector
            lifeSpan: 5000
        }
    }
}
