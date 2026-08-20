// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: dust

    required property Camera sceneCamera

    ParticleEmitter3D {
        id: dustpointEmitter
        x: 0
        y: -38

        enabled: dust.visible
        emitRate: 2000
        velocity: dustpointDir
        particleRotationVariation.z: 360
        lifeSpanVariation: 6000
        particleRotationVariation.x: 360
        particleRotation.x: 0
        system: dust
        particleEndScale: 0.1
        particleRotationVelocityVariation.y: 5
        particleEndScaleVariation: 0.1
        particleScale: 0.1
        particleRotationVelocityVariation.z: 5
        ModelParticle3D {
            id: particleRed7
            color: "#40ffffff"
            unifiedColorVariation: false
            fadeInDuration: 3000
            alignTargetPosition: dust.sceneCamera.position
            hasTransparency: true
            sortMode: Particle3D.SortDistance
            fadeOutDuration: 3000
            alignMode: Particle3D.AlignNone
            maxAmount: 600000
            colorVariation.w: 0.25
        }
        particle: particleRed7
        particleRotation.y: 0
        particleRotationVelocityVariation.x: 5
        particleRotationVelocity.y: 0
        particleScaleVariation: 0.1
        shape: targetShape6
        lifeSpan: 6000
        particleRotationVariation.y: 360
        particleRotationVelocity.x: 0
        particleRotationVelocity.z: 0
        z: -5.27116
        particleRotation.z: 0
    }

    ParticleEmitter3D {
        id: dustcloudEmitter
        x: 0
        y: -45.815

        enabled: dust.visible
        velocity: vectorDirection1
        particleScale: 5
        emitRate: 20
        particleRotation.x: 0
        particleRotationVariation.x: 360
        lifeSpanVariation: 6000
        particleRotationVariation.z: 360
        system: dust
        particleEndScaleVariation: 2.5
        particleRotationVelocityVariation.y: 10
        particleEndScale: 5
        particleRotationVelocityVariation.z: 10
        ModelParticle3D {
            id: particleRed6
            color: "#ffffff"
            unifiedColorVariation: false
            fadeInDuration: 6000
            alignTargetPosition: dust.sceneCamera.position
            hasTransparency: true
            sortMode: Particle3D.SortDistance
            fadeOutDuration: 6000
            alignMode: Particle3D.AlignNone
            maxAmount: 1500
            colorVariation.w: 1
        }
        particle: particleRed6
        particleRotation.y: 0
        particleScaleVariation: 2.5
        particleRotationVelocity.y: 0
        particleRotationVelocityVariation.x: 10
        shape: targetShape6
        lifeSpan: 6000
        particleRotationVariation.y: 360
        particleRotationVelocity.x: 0
        particleRotationVelocity.z: 0
        z: -5.27116
        particleRotation.z: 0
    }

    ParticleModelShape3D {
        id: targetShape6
        fill: true
        delegate: Model {
            source: "#Cube"
            scale.x: 10
            scale.y: 10
            scale.z: 10
        }
    }

    Gravity3D {
        id: gravity6
        magnitude: 1
        direction.y: -1
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        castsShadows: true
        receivesShadows: true
        scale.z: 1
        instancing: particleRed6.instanceTable
        materials: [dustMat]
        scale.y: 1
        scale.x: 1
        receivesReflections: true
    }

    Wander3D {
        id: wander4
        uniqueAmount.y: 0
        uniqueAmount.x: 0
        uniquePace.x: 100
        globalPaceStart.z: 0.1
        globalAmount.z: 5
        system: dust
        uniquePaceVariation: 1
        globalPace.y: 0.1
        uniquePace.y: 100
        particles: [particleRed6]
        uniquePace.z: 100
        globalPaceStart.y: 0.1
        uniqueAmount.z: 0
        globalPaceStart.x: 0.1
        globalPace.x: 0.1
        uniqueAmountVariation: 1
        globalAmount.x: 5
        globalPace.z: 0.1
    }

    VectorDirection3D {
        id: vectorDirection1
        directionVariation.x: 10
        directionVariation.y: 10
        direction.y: 0
        directionVariation.z: 10
    }

    Model {
        id: dustpoint
        source: "#Sphere"
        receivesShadows: false
        castsShadows: false
        scale.z: 0.1
        instancing: particleRed7.instanceTable
        materials: [dustMat]
        scale.y: 0.1
        receivesReflections: false
        scale.x: 0.1
    }

    VectorDirection3D {
        id: dustpointDir
        directionVariation.x: 10
        directionVariation.y: 10
        direction.y: 0
        directionVariation.z: 10
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: dustMat
            cullMode: Material.BackFaceCulling
            property color baseColor: "#40ffffff"
            property TextureInput dfMask: dusttexinput
            depthDrawMode: Material.NeverDepthDraw
            objectName: "New Material"
            sourceBlend: CustomMaterial.SrcAlpha
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            TextureInput {
                id: dusttexinput
                texture: dusttex
                enabled: true
            }

            Texture {
                id: dusttex
                source: "file:content/images/dust.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            vertexShader: "shaders/dustcloud.vert"
            fragmentShader: "shaders/dustcloud.frag"
            shadingMode: CustomMaterial.Shaded
        }
    }

    staticFlags: 0
}
