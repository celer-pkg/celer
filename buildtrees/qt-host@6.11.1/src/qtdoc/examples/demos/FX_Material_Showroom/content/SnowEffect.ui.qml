// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: snow
    ParticleEmitter3D {
        id: modelShapeEmitter3

        enabled: snow.visible
        y: 400
        particleScale: 0.3
        z: 0.00002
        particleEndScaleVariation: 0.3
        lifeSpan: 15000
        particleScaleVariation: 0.3
        particleRotationVariation.y: 360
        shape: targetShape3
        particleRotationVariation.x: 360
        emitRate: 1000
        particleRotationVelocityVariation.y: 90
        system: snow
        particleRotationVelocityVariation.z: 90
        particleEndScale: 0.3
        particleRotationVariation.z: 360
        particleRotationVelocityVariation.x: 90
        particle: particleRed3
        ModelParticle3D {
            id: particleRed3
            color: "#ffffff"
            fadeOutEffect: Particle3D.FadeNone
            fadeInEffect: Particle3D.FadeNone
            colorVariation.z: 1
            sortMode: Particle3D.SortDistance
            unifiedColorVariation: false
            maxAmount: 10000
            fadeInDuration: 500
            colorVariation.x: 1
            colorVariation.y: 1
            hasTransparency: true
            fadeOutDuration: 500
        }
    }

    ParticleModelShape3D {
        id: targetShape3
        delegate: Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            scale.x: 10
            scale.y: 10
        }
        fill: false
    }

    Gravity3D {
        id: gravity3
        magnitude: 10
        direction.y: -0.1
    }

    Model {
        source: "#Cube"
        castsReflections: true
        castsShadows: true
        instancing: particleRed3.instanceTable
        receivesShadows: true
        materials: [snowMat]
        receivesReflections: true
    }

    Wander3D {
        id: wander
        globalPaceStart.y: 0
        globalPace.y: 0
        uniqueAmountVariation: 1
        uniquePaceVariation: 1
        uniquePace.z: 0.3
        uniquePace.y: 0
        uniquePace.x: 0.3
        uniqueAmount.z: 10
        uniqueAmount.x: 10
        globalPaceStart.z: 0.3
        globalPaceStart.x: 0.3
        globalPace.z: 0
        globalPace.x: 0
        globalAmount.z: 0
        globalAmount.x: 0
        particles: [particleRed3]
        system: snow
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: snowMat
            cullMode: Material.BackFaceCulling
            property color baseColor: "#ffffff"
            property TextureInput dfMask: dfMask_snow
            TextureInput {
                id: dfMask_snow
                texture: dfMask_snowtex
                enabled: true
            }

            Texture {
                id: dfMask_snowtex
                source: "file:content/images/snowflake_DF_multi.tga"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            fragmentShader: "shaders/snow.frag"
            sourceBlend: CustomMaterial.SrcAlpha
            shadingMode: CustomMaterial.Shaded
            vertexShader: "shaders/snow.vert"
            depthDrawMode: Material.NeverDepthDraw
            objectName: "Snow Material"
        }
    }
}
