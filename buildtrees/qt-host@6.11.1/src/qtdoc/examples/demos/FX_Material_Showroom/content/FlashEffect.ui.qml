// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: flash
    ParticleEmitter3D {
        id: burstEmitter1
        velocity: burstDirection1
        particle: modelParticle1
        particleScale: 0
        particleEndScale: 30

        enabled: flash.visible
        lifeSpan: 1000
        emitBursts: [emitBurst1]
        ModelParticle3D {
            id: modelParticle1
            sortMode: Particle3D.SortDistance
            fadeInDuration: burstEmitter1.lifeSpan / 4
            fadeOutDuration: modelParticle1.fadeInDuration * 3
        }

        VectorDirection3D {
            id: burstDirection1
            directionVariation.x: 0
            directionVariation.z: 0
            direction.y: 0
            directionVariation.y: 0
        }

        EmitBurst3D {
            id: emitBurst1
            duration: 100
            amount: 1
            time: 0
        }

        Model {
            source: "#Sphere"
            castsReflections: true
            receivesReflections: true
            materials: [flashMat]
            scale.y: 1
            receivesShadows: true
            castsShadows: true
            instancing: modelParticle1.instanceTable
            scale.z: 1
            scale.x: 1
        }
        emitRate: 1
        particleEndScaleVariation: 0
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: flashMat
            fragmentShader: "shaders/flash.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.One
            cullMode: Material.NoCulling
            property color baseColor: "#bbddff"
            sourceBlend: CustomMaterial.One
            vertexShader: "shaders/flash.vert"
            objectName: "Flash Material"
        }
    }
}
