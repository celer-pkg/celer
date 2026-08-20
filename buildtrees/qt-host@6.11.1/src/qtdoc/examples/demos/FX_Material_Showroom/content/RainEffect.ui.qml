// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: rain

    required property Camera sceneCamera

    ParticleEmitter3D {

        enabled: rain.visible
        id: modelShapeEmitter1
        y: 500
        particleScale: 1.5
        particleScaleVariation: modelShapeEmitter1.particleScale
        particleEndScale: modelShapeEmitter1.particleScale
        system: rain
        ModelParticle3D {
            id: particleRed1
            color: "#ffffff"
            alignMode: Particle3D.AlignTowardsStartVelocity
            alignTargetPosition: rain.sceneCamera.position
            maxAmount: 50000
            hasTransparency: true
            fadeOutDuration: 100
            unifiedColorVariation: true
            sortMode: Particle3D.SortDistance
            fadeInDuration: 1000
        }
        shape: targetShape1
        particleRotationVariation.y: 360
        particle: particleRed1
        emitRate: 100
        lifeSpan: 3000
        particleEndScaleVariation: modelShapeEmitter1.particleScale
    }

    ParticleModelShape3D {
        id: targetShape1
        fill: false
        delegate: Model {
            source: "#Rectangle"
            scale.x: 15
            eulerRotation.x: -90
            scale.y: 15
        }
    }

    Gravity3D {
        id: gravity1
        direction.y: -1
        magnitude: 98 * 5
    }

    Model {
        source: "#Cube"
        castsReflections: true
        receivesShadows: true
        materials: [rainMat]
        instancing: particleRed1.instanceTable
        castsShadows: true
        receivesReflections: true
    }

    ParticleSystem3D {
        id: ripples
        y: 0
        ParticleEmitter3D {
            id: modelShapeEmitter5
            lifeSpanVariation: 250

            enabled: rain.visible
            particleScale: 0
            lifeSpan: 250
            system: ripples
            ModelParticle3D {
                id: particleRed5
                color: "#ffffff"
                alignMode: Particle3D.AlignTowardsStartVelocity
                maxAmount: 50000
                colorVariation.y: 0
                sortMode: Particle3D.SortDistance
                fadeInDuration: 20
                fadeOutDuration: 80
                unifiedColorVariation: true
                colorVariation.w: 0
                hasTransparency: true
                colorVariation.x: 0
                colorVariation.z: 0
            }
            particle: particleRed5
            particleRotationVelocityVariation.y: 0
            particleScaleVariation: 0
            particleRotationVelocityVariation.z: 0
            shape: targetShape5
            particleEndScale: 0.25
            emitRate: 5000
            particleRotationVariation.x: 0
            particleRotationVariation.z: 0
            particleEndScaleVariation: 0.25
            particleRotationVelocityVariation.x: 0
            particleRotationVariation.y: 360
        }

        ParticleModelShape3D {
            id: targetShape5
            fill: false
            delegate: Model {
                source: "#Rectangle"
                eulerRotation.x: -90
                scale.x: 15
                scale.y: 15
            }
        }

        Model {
            source: "#Sphere"
            castsReflections: true
            scale.x: 1
            scale.y: 1
            instancing: particleRed5.instanceTable
            materials: [splashMat]
            receivesReflections: true
            castsShadows: true
            scale.z: 1
            receivesShadows: true
        }
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: rainMat
            fragmentShader: "shaders/raindrop.frag"
            objectName: "New Material"
            property color baseColor: "white"
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            sourceBlend: CustomMaterial.SrcAlpha
            vertexShader: "shaders/raindrop.vert"
            depthDrawMode: Material.NeverDepthDraw
            cullMode: Material.BackFaceCulling
            property TextureInput rainmap: texture_rain
            TextureInput {
                id: texture_rain
                texture: raintex
                enabled: true
            }

            Texture {
                id: raintex
                source: "file:content/images/raindrops_multi.tga"
                scaleV: 3
                scaleU: 3
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
        }

        CustomMaterial {
            id: splashMat
            fragmentShader: "shaders/rainsplash.frag"
            objectName: "New Material"
            property color baseColor: "white"
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            sourceBlend: CustomMaterial.SrcAlpha
            vertexShader: "shaders/raindrop.vert"
            depthDrawMode: Material.NeverDepthDraw
            cullMode: Material.BackFaceCulling
            property TextureInput rainmap: texture_splash
            TextureInput {
                id: texture_splash
                texture: splashtex
                enabled: true
            }

            Texture {
                id: splashtex
                source: "file:content/images/rainsplash.png"
                scaleV: 3
                scaleU: 3
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
        }
    }
}
