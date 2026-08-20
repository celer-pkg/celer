// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: bubbles

    required property Camera sceneCamera

    ParticleEmitter3D {
        id: modelShapeEmitter6
        x: -0
        y: -8.389
        particleRotation.z: 0
        enabled: bubbles.visible
        velocity: vectorDirection2
        particle: particleRed8
        particleRotationVariation.x: 0
        particleRotationVelocityVariation.x: 0
        lifeSpan: 5000
        ModelParticle3D {
            id: particleRed8
            color: "#ffffff"
            sortMode: Particle3D.SortDistance
            alignMode: Particle3D.AlignTowardsStartVelocity
            colorVariation.w: 0.25
            colorVariation.y: 0.25
            colorVariation.x: 0.25
            maxAmount: 5000
            alignTargetPosition: bubbles.sceneCamera.position
            fadeInDuration: 0
            hasTransparency: true
            unifiedColorVariation: false
            colorVariation.z: 0
            fadeOutDuration: 0
        }
        particleRotationVelocity.z: 0
        particleRotation.y: 0
        particleEndScaleVariation: 0.25
        shape: targetShape7
        system: bubbles
        particleRotationVelocity.x: 0
        particleRotationVelocityVariation.y: 0
        particleScale: 0.25
        particleScaleVariation: 0.25
        particleRotationVariation.y: 0
        particleEndScale: 0.25
        z: 9.0413
        particleRotationVelocity.y: 0
        particleRotationVelocityVariation.z: 0
        particleRotation.x: 0
        lifeSpanVariation: 5000
        emitRate: 50
        particleRotationVariation.z: 0
    }

    ParticleModelShape3D {
        id: targetShape7
        fill: true
        delegate: Model {
            source: "#Sphere"
            scale.y: 1
            scale.x: 1
            scale.z: 1
        }
    }

    Gravity3D {
        id: gravity8
        magnitude: 9.8
        direction.y: -1
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        receivesReflections: true
        materials: [bubbleMat]
        receivesShadows: true
        scale.y: 1
        castsShadows: true
        instancing: particleRed8.instanceTable
        scale.z: 1
        scale.x: 1
    }

    Wander3D {
        id: wander5
        uniqueAmount.y: 0
        globalAmount.y: 0
        globalPace.x: 0.1
        uniqueAmountVariation: 1
        globalPaceStart.z: 0.1
        uniqueAmount.z: 0
        uniquePaceVariation: 1
        particles: [particleRed8]
        globalAmount.z: 0
        uniquePace.z: 0.1
        globalPace.y: 0.1
        globalAmount.x: 0
        system: bubbles
        uniqueAmount.x: 0
        globalPaceStart.y: 0.1
        globalPaceStart.x: 0.1
        globalPace.z: 0.1
        uniquePace.y: 0.1
        uniquePace.x: 0.1
    }

    VectorDirection3D {
        id: vectorDirection2
        direction.z: 0
        directionVariation.x: 50
        directionVariation.z: 50
        directionVariation.y: 100
        direction.y: 100
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: bubbleMat
            fragmentShader: "shaders/bubble.frag"
            objectName: "New Material"
            sourceBlend: CustomMaterial.SrcAlpha
            depthDrawMode: Material.NeverDepthDraw
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            property color baseColor: "#ffffff"
            vertexShader: "shaders/bubble.vert"
            shadingMode: CustomMaterial.Shaded
            cullMode: Material.BackFaceCulling
        }
    }
}
