// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: heatwave
    ParticleEmitter3D {
        enabled: heatwave.visible
        id: modelShapeEmitter
        x: -0
        y: -60
        particleEndScaleVariation: 0
        system: heatwave
        z: 0.00002
        particleEndScale: 1
        particle: particleRed
        emitRate: 20
        lifeSpan: 5000
        lifeSpanVariation: 0
        particleRotationVariation.x: 0
        particleRotationVelocityVariation.x: 0
        particleRotationVelocityVariation.z: 0
        particleRotationVelocityVariation.y: 0
        shape: targetShape
        particleRotationVariation.z: 0
        particleRotationVariation.y: 0

        ModelParticle3D {
            id: particleRed
            maxAmount: 100
            color: "#ffffff"
            fadeInDuration: 2000
            fadeOutDuration: 2000
            unifiedColorVariation: true
            hasTransparency: true
            sortMode: Particle3D.SortDistance
            colorVariation.w: 0
            colorVariation.z: 0
            colorVariation.y: 0
            colorVariation.x: 0
        }
    }

    ParticleModelShape3D {
        id: targetShape
        fill: false
        delegate: Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            scale.x: 5.0
            scale.y: 5.0
        }
    }

    Gravity3D {
        id: gravity
        magnitude: 20
        direction.y: 1
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        receivesReflections: true
        receivesShadows: true
        castsShadows: true
        materials: [heatwaveMat]
        scale.z: 1
        scale.y: 1
        scale.x: 1
        instancing: particleRed.instanceTable
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: heatwaveMat
            cullMode: Material.BackFaceCulling
            property color baseColor: "white"
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            fragmentShader: "shaders/heatwave.frag"
            sourceBlend: CustomMaterial.SrcAlpha
            shadingMode: CustomMaterial.Shaded
            vertexShader: "shaders/heatwave.vert"
            depthDrawMode: Material.NeverDepthDraw
            objectName: "New Material"
        }
    }
}
