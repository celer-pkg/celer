// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: sparks

    ParticleEmitter3D {
        id: lineSpark
        particleScale: 0
        lifeSpan: 100
        velocity: targetDirection
        enabled: sparks.visible
        lifeSpanVariation: 100
        particleEndScaleVariation: 1
        particleEndScale: 1
        emitRate: 2500
        particle: sparkParticle
        system: sparks

        ModelParticle3D {
            id: sparkParticle
            color: "#ffffff"

            unifiedColorVariation: false
            fadeInDuration: 50
            hasTransparency: true
            sortMode: Particle3D.SortDistance
            fadeOutDuration: 50
            alignMode: Particle3D.AlignTowardsStartVelocity
            maxAmount: 50000
        }

        TargetDirection3D {
            id: targetDirection
            position.x: -3
            magnitudeVariation: 100
            magnitude: 100
            positionVariation.z: 1
            positionVariation.y: 1
            positionVariation.x: 1
        }
    }

    Gravity3D {
        id: gravity5
        direction.y: -1
        magnitude: 8.9
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        scale.z: 0.75
        receivesShadows: true
        castsShadows: true
        instancing: sparkParticle.instanceTable
        materials: [sparkMat]
        scale.y: .1
        receivesReflections: true
        scale.x: .1
    }

    Wander3D {
        id: wander3
        uniqueAmount.y: 0
        uniqueAmount.x: 0
        globalAmount.z: 0
        globalPaceStart.z: 0.1
        uniquePace.x: 0.1
        system: sparks
        uniquePaceVariation: 1
        globalPace.y: 0.1
        uniquePace.y: 0.1
        uniquePace.z: 0.1
        globalPaceStart.y: 0.1
        uniqueAmount.z: 0
        globalPaceStart.x: 0.1
        globalPace.x: 0.1
        uniqueAmountVariation: 1
        globalAmount.x: 0
        globalPace.z: 0.1
    }

    staticFlags: 0

    ParticleSystem3D {
        id: dotSparks
        ParticleEmitter3D {
            id: lineSpark1
            enabled: sparks.visible
            shape: targetShape16
            velocity: vectorDirection8
            particleRotationVariation.z: 360
            particleRotationVariation.y: 360
            particleRotationVariation.x: 360
            lifeSpan: 100
            particleEndScale: 1
            system: dotSparks
            emitRate: 100
            ModelParticle3D {
                id: sparkParticle1
                color: "#ffffff"
                alignMode: Particle3D.AlignTowardsStartVelocity

                property bool castsReflections: true
                maxAmount: 50000
                sortMode: Particle3D.SortDistance
                fadeInDuration: 50
                fadeOutDuration: 50
                unifiedColorVariation: false
                hasTransparency: true
            }
            particle: sparkParticle1
            particleRotationVelocityVariation.y: 360
            particleScaleVariation: 1
            particleRotationVelocityVariation.z: 360
            particleEndScaleVariation: 1
            lifeSpanVariation: 100
            particleRotationVelocityVariation.x: 360
        }

        Gravity3D {
            id: gravity12
            magnitude: 8.9
            direction.y: -1
        }

        Model {
            source: "#Sphere"
            castsReflections: true
            scale.x: 0.3
            scale.y: 0.1
            instancing: sparkParticle1.instanceTable
            materials: [sparkMat]
            receivesReflections: true
            castsShadows: true
            scale.z: 0.1
            receivesShadows: true
        }

        VectorDirection3D {
            id: vectorDirection8
            direction.x: -100
            direction.z: 0
            directionVariation.z: 100
            directionVariation.y: 100
            directionVariation.x: 100
            direction.y: 0
        }

        Wander3D {
            id: wander11
            system: dotSparks
            uniquePaceVariation: 1
            uniquePace.z: 0.1
            globalPaceStart.x: 0.1
            globalAmount.x: 0
            uniqueAmount.y: 10
            uniquePace.x: 0.1
            globalPace.x: 0.1
            uniqueAmount.z: 10
            uniqueAmount.x: 10
            uniquePace.y: 0.1
            uniqueAmountVariation: 1
            globalAmount.z: 0
            globalPaceStart.z: 0.1
            globalPace.z: 0.1
            globalPace.y: 0.1
            globalPaceStart.y: 0.1
        }

        ParticleModelShape3D {
            id: targetShape16
            fill: true
            delegate: Model {
                source: "#Sphere"
                eulerRotation.x: -90
                scale.x: 1
                scale.y: 1
                scale.z: 1
            }
        }

        staticFlags: 0
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: sparkMat
            cullMode: Material.NoCulling
            property color baseColor: "#e5b16d"
            property real brightness: 10
            property TextureInput dfMask: sparktexinput
            depthDrawMode: Material.NeverDepthDraw
            objectName: "New Material"
            sourceBlend: CustomMaterial.SrcAlpha
            destinationBlend: CustomMaterial.One
            TextureInput {
                id: sparktexinput
                texture: sparktex
                enabled: true
            }

            Texture {
                id: sparktex
                source: "file:content/images/spark.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
            }
            vertexShader: "shaders/spark.vert"
            fragmentShader: "shaders/spark.frag"
            shadingMode: CustomMaterial.Shaded
        }
    }
}
