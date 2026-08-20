// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: smoke
    ParticleEmitter3D {
        id: modelShapeEmitter2

        enabled: smoke.visible
        x: 0
        y: 25.243
        particleScale: 10
        particleScaleVariation: 5
        particleRotationVelocity.z: 30
        particleRotationVelocity.y: 30
        particleRotationVelocity.x: 30
        particleRotation.z: 360
        particleRotation.y: 360
        particleRotation.x: 360
        z: -5.27116
        particleEndScaleVariation: 10
        lifeSpan: 6000
        particleRotationVariation.y: 360
        shape: targetShape2
        particleRotationVariation.x: 360
        emitRate: 10
        particleRotationVelocityVariation.y: 30
        system: smoke
        particleRotationVelocityVariation.z: 30
        particleEndScale: 20
        particleRotationVariation.z: 360
        particleRotationVelocityVariation.x: 30
        particle: particleRed2
        ModelParticle3D {
            id: particleRed2
            color: "#ffffff"
            sortMode: Particle3D.SortDistance
            colorVariation.w: 1
            unifiedColorVariation: false
            maxAmount: 150
            fadeInDuration: 2000
            alignMode: Particle3D.AlignNone
            hasTransparency: true
            fadeOutDuration: 2000
        }
        lifeSpanVariation: 0
    }

    ParticleModelShape3D {
        id: targetShape2
        delegate: Model {
            source: "#Rectangle"
            eulerRotation.x: -90
        }
        fill: false
    }

    Gravity3D {
        id: gravity2
        magnitude: 20
        direction.y: 1
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        scale.x: .1
        castsShadows: true
        instancing: particleRed2.instanceTable
        scale.y: .1
        receivesShadows: false
        materials: [smokeMat]
        scale.z: .1
        receivesReflections: true
    }

    Wander3D {
        id: wander1
        uniqueAmount.z: 10
        globalPace.x: 0.1
        globalPace.y: 0.1
        uniqueAmount.x: 10
        uniquePaceVariation: 1
        globalAmount.x: 5
        uniquePace.x: 0.1
        globalPace.z: 0.1
        uniqueAmountVariation: 1
        globalPaceStart.x: 0.1
        globalAmount.z: 5
        system: smoke
        globalPaceStart.y: 0.1
        particles: [particleRed2]
        uniquePace.z: 0.1
        uniquePace.y: 0.1
        globalPaceStart.z: 0.1
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: smokeMat
            TextureInput {
                id: smoketexinput
                enabled: true
                texture: smoketex
            }

            Texture {
                id: smoketex
                source: "file:content/images/smoke.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            cullMode: Material.BackFaceCulling

            property real specular: 0
            property real density: 0.0
            property real opacity: 1
            property real roughness: 1
            property real clearcoat: 1
            property real metalness: 0.0
            property real sssDistortion: 0.1
            property real sssPower: 1
            property real sssScale: 3
            property color baseColor: "#808080"
            property color sssColor: "#737d80"
            property TextureInput dfMask: smoketexinput
            alwaysDirty: false
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            fragmentShader: "shaders/smoke.frag"
            sourceBlend: CustomMaterial.SrcAlpha
            vertexShader: "shaders/smoke.vert"
            shadingMode: CustomMaterial.Shaded
            depthDrawMode: Material.NeverDepthDraw
            objectName: "Smoke Material"
        }
    }
    staticFlags: 0
}
