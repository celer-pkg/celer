// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: shockwave
    visible: true
    ParticleEmitter3D {
        id: pressurewaveemitter

        enabled: shockwave.visible
        particleScale: 0
        particleEndScaleVariation: 0
        particleEndScale: 30
        emitRate: 0
        lifeSpan: 100
        velocity: burstDirection

        ModelParticle3D {
            id: modelParticle
            sortMode: Particle3D.SortDistance
            fadeOutDuration: 50
            fadeInDuration: 50
        }

        VectorDirection3D {
            id: burstDirection
            direction.y: 0
            directionVariation.x: 0
            directionVariation.z: 0
            directionVariation.y: 0
        }

        Model {
            source: "#Sphere"
            castsReflections: true
            receivesReflections: true
            materials: [shockwaveMat]
            receivesShadows: true
            scale.y: 1
            castsShadows: true
            instancing: modelParticle.instanceTable
            scale.z: 1
            scale.x: 1
        }

        DynamicBurst3D {
            id: dynamicBurst6
            enabled: true
            duration: 0
            amount: 1
            time: 0
            triggerMode: DynamicBurst3D.TriggerTime
        }

        particle: modelParticle
    }

    Connections {
        target: shockwaveTimer
        function onTriggered() {
            shockwave.reset()
        }
    }

    Connections {
        target: shockwaveTimer
        function onTriggered() {
            pressurewave1.reset()
        }
    }

    Connections {
        target: shockwaveTimer
        function onTriggered() {
            debris.reset()
        }
    }

    Connections {
        target: shockwaveTimer
        function onTriggered() {
            debris.startTime = debris.time
        }
    }

    Connections {
        target: shockwaveTimer
        function onTriggered() {
            debris1.reset()
        }
    }

    Connections {
        target: shockwaveTimer
        function onTriggered() {
            debris1.startTime = debris1.time
        }
    }

    Timer {
        id: shockwaveTimer
        running: shockwave.visible
        repeat: true
        interval: 5000
        triggeredOnStart: true
    }

    ParticleSystem3D {
        id: debris
        visible: true
        ParticleEmitter3D {
            id: pressurewaveemitter1
            emitBursts: [dynamicBurst7]
            shape: targetShape10
            particleScaleVariation: 0.5
            particleRotationVelocityVariation.z: 0
            particleRotationVelocityVariation.x: 0
            lifeSpanVariation: 0
            system: debris

            enabled: shockwave.visible
            particle: modelParticle7
            particleScale: 0.5
            particleEndScale: 0
            lifeSpan: 3000
            ModelParticle3D {
                id: modelParticle7
                colorVariation.w: 0
                colorVariation.z: 0
                colorVariation.y: 0
                colorVariation.x: 0
                alignMode: Particle3D.AlignTowardsTarget
                sortMode: Particle3D.SortDistance
                maxAmount: 500000
                fadeInDuration: 1500
                fadeOutDuration: 1500
            }

            Model {
                source: "#Sphere"
                castsReflections: true
                receivesReflections: true
                materials: [shockwavedebrisMat]
                receivesShadows: true
                scale.y: 1
                castsShadows: true
                instancing: modelParticle7.instanceTable
                scale.z: 5
                scale.x: 3
            }

            Attractor3D {
                id: particleAttractor
                y: 0
                useCachedPositions: false
                durationVariation: (debris.time - debris.startTime) * 0.5
                hideAtEnd: false
                shape: targetShape10
                particles: [modelParticle7]
                system: debris
                duration: (debris.time - debris.startTime) * 0.5
            }

            DynamicBurst3D {
                id: dynamicBurst7
                enabled: true
                duration: 10
                amount: 500
                time: 100
                triggerMode: DynamicBurst3D.TriggerTime
            }

            particleEndScaleVariation: 0
            emitRate: 0
        }

        ParticleModelShape3D {
            id: targetShape10
            fill: true
            delegate: Model {
                source: "#Sphere"
                eulerRotation.z: 0
                scale.y: 0.01
                eulerRotation.y: 0
                eulerRotation.x: 0
                scale.z: (debris.time - debris.startTime) * 0.01
                scale.x: (debris.time - debris.startTime) * 0.01
            }
        }
    }

    ParticleSystem3D {
        id: debris1
        ParticleEmitter3D {
            id: pressurewaveemitter2
            system: debris1

            enabled: shockwave.visible
            particle: modelParticle8
            particleScale: 1
            particleScaleVariation: 0.0
            particleRotationVelocityVariation.x: 0
            particleEndScale: 0
            lifeSpan: 3000
            particleRotationVelocityVariation.z: 0
            emitBursts: [dynamicBurst8]
            ModelParticle3D {
                id: modelParticle8
                colorVariation.w: 0
                colorVariation.z: 0
                colorVariation.y: 0
                colorVariation.x: 0
                alignTargetPosition.y: 1
                maxAmount: 500000
                fadeInDuration: 1500
                alignMode: Particle3D.AlignTowardsTarget
                sortMode: Particle3D.SortDistance
                fadeOutDuration: 1500
            }

            Model {
                source: "#Sphere"
                castsReflections: true
                receivesReflections: true
                materials: [shockwavedebrisMat]
                receivesShadows: true
                scale.y: 1
                castsShadows: true
                instancing: modelParticle8.instanceTable
                scale.z: 1
                scale.x: 2
            }

            Attractor3D {
                id: particleAttractor1
                y: 0
                system: debris1
                durationVariation: (debris1.time - debris1.startTime) * 0.5
                hideAtEnd: false
                duration: (debris1.time - debris1.startTime) * 0.5
                useCachedPositions: false
                particles: [modelParticle8]
                shape: targetShape11
            }

            DynamicBurst3D {
                id: dynamicBurst8
                enabled: true
                duration: 10
                amount: 500
                time: 0
                triggerMode: DynamicBurst3D.TriggerTime
            }
            particleEndScaleVariation: 0
            lifeSpanVariation: 3000
            emitRate: 0
            shape: targetShape11
        }

        ParticleModelShape3D {
            id: targetShape11
            fill: true
            delegate: Model {
                source: "#Sphere"
                eulerRotation.z: 0
                scale.y: 0.01
                eulerRotation.x: 0
                eulerRotation.y: 0
                scale.z: (debris1.time - debris1.startTime) * 0.05
                scale.x: (debris1.time - debris1.startTime) * 0.05
            }
        }
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: shockwaveMat
            fragmentShader: "shaders/shockwave.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            cullMode: Material.BackFaceCulling
            property color baseColor: "#ffffff"
            sourceBlend: CustomMaterial.One
            vertexShader: "shaders/shockwave.vert"
            objectName: "Shockwave Material"
        }

        CustomMaterial {
            id: shockwaveMat2
            fragmentShader: "shaders/shockwave.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.One
            cullMode: Material.BackFaceCulling
            property color baseColor: "#85e1fc"
            sourceBlend: CustomMaterial.One
            vertexShader: "shaders/shockwave.vert"
            objectName: "Shockwave Material"
        }

        CustomMaterial {
            id: shockwavedebrisMat
            fragmentShader: "shaders/debris.frag"
            depthDrawMode: Material.NeverDepthDraw
            property real brightness: 10
            property TextureInput dfMask: debristexinput
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.One
            cullMode: Material.NoCulling
            property color baseColor: "#3355ff"
            sourceBlend: CustomMaterial.SrcAlpha
            TextureInput {
                id: debristexinput
                enabled: true
                texture: debristex
            }

            Texture {
                id: debristex
                source: "images/debris.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            vertexShader: "shaders/debris.vert"
            objectName: "Debris Material"
        }
    }

    ParticleSystem3D {
        id: pressurewave1
        y: 9
        ParticleEmitter3D {
            particleRotationVariation.y: 90
            particleRotationVelocityVariation.y: 360
            emitBursts: [diskburst]
            velocity: flatdiskdir
            emitRate: 0
            enabled: shockwave.visible
            particle: flatshockwave
            ModelParticle3D {
                id: flatshockwave
                maxAmount: 100
                fadeOutDuration: 1500
                fadeInDuration: 0
                sortMode: Particle3D.SortDistance
            }

            VectorDirection3D {
                id: flatdiskdir
                direction.y: 1
                directionVariation.z: 1
            }

            Model {
                source: "#Sphere"
                instancing: flatshockwave.instanceTable
                receivesReflections: true
                castsReflections: true
                receivesShadows: true
                materials: [shockwaveMat2]
                scale.y: 0.05
                castsShadows: true
            }

            DynamicBurst3D {
                id: diskburst
                time: 0
                duration: 0
                enabled: true
                triggerMode: DynamicBurst3D.TriggerTime
                amount: 2
            }
            particleEndScaleVariation: 10
            particleEndScale: 40
            particleScale: 0
            lifeSpan: 1500
        }
    }
}
