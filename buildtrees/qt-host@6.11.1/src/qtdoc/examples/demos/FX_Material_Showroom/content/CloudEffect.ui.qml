// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: cloud

    required property Camera sceneCamera

    ParticleEmitter3D {
        id: modelShapeEmitter7
        x: 0
        y: -45.815
        particleScale: 20
        velocity: vectorDirection3
        particleRotation.z: 0
        particle: particleRed9
        enabled: cloud.visible
        particleRotationVariation.x: 360
        particleRotationVelocityVariation.x: 10
        lifeSpan: 25000
        ModelParticle3D {
            id: particleRed9
            color: "#ffffff"
            sortMode: Particle3D.SortDistance
            alignMode: Particle3D.AlignNone
            maxAmount: 50
            alignTargetPosition: cloud.sceneCamera.position
            fadeInDuration: 12500
            hasTransparency: true
            unifiedColorVariation: false
            fadeOutDuration: 12500
        }
        particleRotationVelocity.z: 0
        particleRotation.y: 0
        particleEndScaleVariation: 0
        shape: targetShape8
        system: cloud
        particleRotationVelocity.x: 0
        particleRotationVelocityVariation.y: 10
        particleScaleVariation: 10
        particleRotationVariation.y: 360
        particleEndScale: 20
        z: -5.27116
        particleRotationVelocity.y: 0
        particleRotationVelocityVariation.z: 10
        particleRotation.x: 0
        lifeSpanVariation: 0
        emitRate: 2
        particleRotationVariation.z: 360
    }

    ParticleModelShape3D {
        id: targetShape8
        fill: true
        delegate: Model {
            source: "#Cube"
            scale.z: 3
            eulerRotation.z: 0
            eulerRotation.y: 0
            scale.x: 3
            scale.y: 3
            eulerRotation.x: 0
        }
    }

    Model {
        source: "#Sphere"
        castsReflections: true
        receivesReflections: true
        materials: [cloudMat]
        scale.y: 0.1
        receivesShadows: true
        castsShadows: true
        instancing: particleRed9.instanceTable
        scale.z: 0.1
        scale.x: 0.1
    }

    Wander3D {
        id: wander6
        uniqueAmount.y: 1
        globalPace.x: 0.1
        uniqueAmountVariation: 1
        uniqueAmount.z: 1
        globalPaceStart.z: 0.1
        uniquePaceVariation: 1
        particles: [particleRed9]
        globalAmount.z: 0
        uniquePace.z: 0.1
        globalPace.y: 0.1
        globalAmount.x: 0
        system: cloud
        uniqueAmount.x: 1
        globalPaceStart.y: 0.1
        globalPaceStart.x: 0.1
        globalPace.z: 0.1
        uniquePace.y: 0.1
        uniquePace.x: 0.1
    }

    VectorDirection3D {
        id: vectorDirection3
        directionVariation.z: 1
        directionVariation.y: 1
        directionVariation.x: 5
        direction.y: 0
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: cloudMat
            TextureInput {
                id: cloudtexinput
                enabled: true
                texture: cloudtex
            }

            Texture {
                id: cloudtex
                source: "file:content/images/cloud.png"
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
            property color baseColor: "#ffffff"
            property color sssColor: "#e5f9ff"
            property TextureInput dfMask: cloudtexinput
            alwaysDirty: false
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            fragmentShader: "shaders/cloud.frag"
            sourceBlend: CustomMaterial.SrcAlpha
            vertexShader: "shaders/cloud.vert"
            shadingMode: CustomMaterial.Shaded
            depthDrawMode: Material.NeverDepthDraw
            objectName: "Cloud Material"
        }
    }
    staticFlags: 0
}
