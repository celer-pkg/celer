// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: fire

    required property Camera sceneCamera

    ParticleEmitter3D {
        id: modelShapeEmitter4
        x: 0
        y: -45.815
        particleScale: 20

        enabled: fire.visible
        emitRate: 50
        particleRotation.x: 0
        particleRotationVariation.x: 360
        lifeSpanVariation: 0
        particleRotationVariation.z: 360
        system: fire
        particleEndScaleVariation: 10
        particleRotationVelocityVariation.y: 90
        particleEndScale: 10
        particleRotationVelocityVariation.z: 90
        ModelParticle3D {
            id: particleRed4
            color: "#ffffff"
            unifiedColorVariation: false
            fadeInDuration: 1250
            alignTargetPosition: fire.sceneCamera.position
            hasTransparency: true
            colorVariation.z: 1
            sortMode: Particle3D.SortDistance
            fadeOutDuration: 250
            colorVariation.y: 1
            alignMode: Particle3D.AlignNone
            maxAmount: 100
            colorVariation.x: 1
            colorVariation.w: 1
        }
        particle: particleRed4
        particleRotation.y: 360
        particleScaleVariation: 20
        particleRotationVelocity.y: 0
        particleRotationVelocityVariation.x: 90
        shape: targetShape4
        lifeSpan: 1500
        particleRotationVariation.y: 360
        particleRotationVelocity.x: 0
        particleRotationVelocity.z: 0
        z: -5.27116
        particleRotation.z: 360
    }

    ParticleModelShape3D {
        id: targetShape4
        fill: false
        delegate: Model {
            source: "#Rectangle"
            scale.x: 1
            eulerRotation.x: -90
            scale.y: 1
        }
    }

    Gravity3D {
        id: gravity4
        magnitude: 300
        direction.y: 1
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        castsShadows: true
        receivesShadows: true
        scale.z: 0.1
        instancing: particleRed4.instanceTable
        materials: [fireMat]
        scale.y: 0.1
        scale.x: 0.1
        receivesReflections: true
    }

    Wander3D {
        id: wander2
        uniqueAmount.x: 10
        uniquePace.x: 0.1
        globalPaceStart.z: 0.1
        globalAmount.z: 5
        system: fire
        uniquePaceVariation: 1
        globalPace.y: 0.1
        uniquePace.y: 0.1
        particles: [particleRed4]
        uniquePace.z: 0.1
        globalPaceStart.y: 0.1
        uniqueAmount.z: 10
        globalPaceStart.x: 0.1
        globalPace.x: 0.1
        uniqueAmountVariation: 1
        globalAmount.x: 5
        globalPace.z: 0.1
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: fireMat
            cullMode: Material.NoCulling
            property color baseColor: "#d97111"
            property TextureInput dfMask: firetexinput
            depthDrawMode: Material.NeverDepthDraw
            objectName: "Fire Material"
            sourceBlend: CustomMaterial.SrcAlpha
            destinationBlend: CustomMaterial.One
            TextureInput {
                id: firetexinput
                texture: firetex
                enabled: true
            }

            Texture {
                id: firetex
                source: "file:content/images/fire.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            vertexShader: "shaders/fire.vert"
            fragmentShader: "shaders/fire.frag"
            shadingMode: CustomMaterial.Shaded
        }
    }
}
