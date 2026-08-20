// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: explosion
    visible: true
    ParticleEmitter3D {
        id: burstEmitter5
        particleScaleVariation: 0
        enabled: explosion.visible
        system: explosion
        velocity: burstDirection5
        particleRotationVelocityVariation.y: 90
        particle: modelParticle5
        particleScale: 0
        particleRotationVelocityVariation.x: 90
        particleEndScale: 5
        lifeSpan: 500
        particleRotationVelocityVariation.z: 90
        emitBursts: [dynamicBurst3]
        ModelParticle3D {
            id: modelParticle5
            color: "#ffffff"
            maxAmount: 1000
            fadeInDuration: 500
            fadeOutDuration: 500
        }

        VectorDirection3D {
            id: burstDirection5
            directionVariation.x: 25
            directionVariation.z: 25
            directionVariation.y: 25
            direction.y: 0
        }

        Model {
            source: "#Sphere"
            castsReflections: true
            receivesReflections: true
            materials: [explosion_fireMat]
            receivesShadows: true
            scale.y: 1
            castsShadows: true
            instancing: modelParticle5.instanceTable
            scale.z: 1
            scale.x: 1
        }

        DynamicBurst3D {
            id: dynamicBurst3
            enabled: explosion.visible
            duration: 1
            amount: 100
            time: 1000
        }
        lifeSpanVariation: lifeSpan
        particleEndScaleVariation: particleEndScale
        emitRate: 0
    }
    running: true

    Timer {
        id: explosionTimer
        running: explosion.visible
        interval: 10000
        triggeredOnStart: true
        repeat: true
    }

    Connections {
        target: explosionTimer
        function onTriggered() {
            explosion_flash.reset()
        }
    }

    Connections {
        target: explosionTimer
        function onTriggered() {
            explosion_smoke.reset()
        }
    }

    Connections {
        target: explosionTimer
        function onTriggered() {
            explosion.reset()
        }
    }

    Connections {
        target: explosionTimer
        function onTriggered() {
            explosion_sparks.reset()
        }
    }

    ParticleSystem3D {
        id: explosion_shockwave
        ParticleEmitter3D {
            id: burstEmitter2
            system: explosion_shockwave
            velocity: burstDirection2
            particle: modelParticle2
            particleScale: 0
            particleEndScale: 30
            enabled: explosion.visible
            lifeSpan: 1000
            emitBursts: [dynamicBurst2]
            ModelParticle3D {
                id: modelParticle2
                fadeInDuration: 100
                fadeOutDuration: 900
            }

            VectorDirection3D {
                id: burstDirection2
                directionVariation.x: 0
                directionVariation.z: 0
                direction.y: 0
                directionVariation.y: 0
            }

            Model {
                source: "#Sphere"
                castsReflections: true
                receivesReflections: true
                materials: [explosion_shockwaveMat]
                scale.y: 1
                receivesShadows: true
                castsShadows: true
                instancing: modelParticle2.instanceTable
                scale.z: 1
                scale.x: 1
            }

            DynamicBurst3D {
                id: dynamicBurst2
                time: 1000
                enabled: explosion.visible
                duration: 10
                amount: 1
            }
            emitRate: 0
            particleEndScaleVariation: 0
        }
    }

    ParticleSystem3D {
        id: explosion_flash
        visible: true
        ParticleEmitter3D {
            id: burstEmitter3
            enabled: explosion.visible
            system: explosion_flash
            velocity: burstDirection3
            particle: modelParticle3
            particleScale: 0
            particleEndScale: 60
            lifeSpan: 1000
            emitBursts: [dynamicBurst1]
            ModelParticle3D {
                id: modelParticle3
                fadeInDuration: burstEmitter3.lifeSpan / 4
                fadeOutDuration: modelParticle3.fadeInDuration * 3
            }

            VectorDirection3D {
                id: burstDirection3
                directionVariation.x: 0
                directionVariation.z: 0
                directionVariation.y: 0
                direction.y: 0
            }

            Model {
                source: "#Sphere"
                castsReflections: true
                receivesReflections: true
                materials: [explosion_flashMat]
                receivesShadows: true
                scale.y: 1
                castsShadows: true
                instancing: modelParticle3.instanceTable
                scale.z: 1
                scale.x: 1
            }

            DynamicBurst3D {
                id: dynamicBurst1
                time: 1000
                enabled: explosion.visible
                duration: 10
                amount: 1
            }
            particleEndScaleVariation: 0
            emitRate: 0
        }
    }

    ParticleSystem3D {
        id: explosion_smoke
        visible: true
        running: true
        ParticleEmitter3D {
            id: burstEmitter4
            enabled: explosion.visible
            lifeSpanVariation: lifeSpan
            particleRotationVelocityVariation.z: 90
            particleRotationVelocityVariation.y: 90
            particleRotationVelocityVariation.x: 90
            system: explosion_smoke
            velocity: burstDirection4
            particle: modelParticle4
            particleScale: 0
            particleEndScale: 10
            lifeSpan: 5000
            emitBursts: [dynamicBurst]
            ModelParticle3D {
                id: modelParticle4
                color: "#33ffffff"
                maxAmount: 1000
                fadeInDuration: 1000
                fadeOutDuration: 4000
            }

            VectorDirection3D {
                id: burstDirection4
                directionVariation.x: 100
                directionVariation.z: 100
                directionVariation.y: 100
                direction.y: 0
            }

            Model {
                source: "#Sphere"
                castsReflections: true
                receivesReflections: true
                materials: [explosion_smokeMat]
                receivesShadows: true
                scale.y: 1
                castsShadows: true
                instancing: modelParticle4.instanceTable
                scale.z: 1
                scale.x: 1
            }

            DynamicBurst3D {
                id: dynamicBurst
                time: 1250
                enabled: explosion.visible
                amount: 100
                duration: 100
            }

            particleEndScaleVariation: particleEndScale / 2
            emitRate: 0
        }
    }



    ParticleSystem3D {
        id: explosion_sparks
        visible: true
        ParticleEmitter3D {
            id: burstEmitter6
            system: explosion_sparks
            velocity: burstDirection6
            enabled: explosion.visible
            particleRotationVelocityVariation.y: 90
            particle: modelParticle6
            particleScale: 0.1
            particleRotationVelocityVariation.x: 90
            particleScaleVariation: particleScale
            particleEndScale: 0.5
            lifeSpan: 3000
            particleRotationVelocityVariation.z: 90
            emitBursts: [dynamicBurst4]
            ModelParticle3D {
                id: modelParticle6
                color: "#ffffff"
                maxAmount: 1000
                fadeInDuration: 500
                fadeOutDuration: 500
            }

            VectorDirection3D {
                id: burstDirection6
                directionVariation.x: 200
                directionVariation.z: 200
                direction.y: 0
                directionVariation.y: 200
            }

            Model {
                source: "#Sphere"
                castsReflections: true
                receivesReflections: true
                materials: [explosion_sparkMat]
                scale.y: 1
                receivesShadows: true
                castsShadows: true
                instancing: modelParticle6.instanceTable
                scale.z: 1
                scale.x: 1
            }

            DynamicBurst3D {
                id: dynamicBurst4
                enabled: explosion.visible
                duration: 100
                amount: 500
                time: 1000
            }

            Gravity3D {
                id: gravity7
                particles: [modelParticle6]
                system: explosion_sparks
            }
            lifeSpanVariation: lifeSpan
            particleEndScaleVariation: particleEndScale
            emitRate: 0
        }
        running: true
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: explosion_shockwaveMat
            fragmentShader: "shaders/shockwave.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            property color baseColor: "#ffffff"
            cullMode: Material.BackFaceCulling
            sourceBlend: CustomMaterial.SrcAlpha
            vertexShader: "shaders/shockwave.vert"
            objectName: "New Material"
        }

        CustomMaterial {
            id: explosion_flashMat
            fragmentShader: "shaders/flash.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.One
            property color baseColor: "#ffaa11"
            cullMode: Material.NoCulling
            sourceBlend: CustomMaterial.One
            vertexShader: "shaders/flash.vert"
            objectName: "New Material"
        }

        CustomMaterial {
            id: explosion_smokeMat
            fragmentShader: "shaders/smoke.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            cullMode: Material.BackFaceCulling
            property color baseColor: "#222222"
            property color sssColor: "#222222"
            property real roughness: 0.5
            property real density: 0.5
            property real metalness: 0.0
            sourceBlend: CustomMaterial.SrcAlpha

            property TextureInput dfMask: smoketexinput1
            TextureInput {
                id: smoketexinput1
                enabled: true
                texture: smoketex1
            }

            Texture {
                id: smoketex1
                source: "file:content/images/smoke.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            vertexShader: "shaders/smoke.vert"
            objectName: "New Material"
        }

        CustomMaterial {
            id: explosion_fireMat
            fragmentShader: "shaders/fire.frag"
            depthDrawMode: Material.NeverDepthDraw
            shadingMode: CustomMaterial.Shaded
            destinationBlend: CustomMaterial.One
            cullMode: Material.BackFaceCulling
            property TextureInput dfMask: firetexinput1
            property color baseColor: "#d97111"
            sourceBlend: CustomMaterial.SrcAlpha
            TextureInput {
                id: firetexinput1
                enabled: true
                texture: firetex1
            }

            Texture {
                id: firetex1
                source: "file:content/images/fire.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            vertexShader: "shaders/fire.vert"
            objectName: "New Material"
        }

        CustomMaterial {
            id: explosion_sparkMat
            cullMode: Material.NoCulling
            property color baseColor: "#e5b16d"
            property real brightness: 10
            property TextureInput dfMask: sparktexinput
            depthDrawMode: Material.NeverDepthDraw
            objectName: "New Material"
            sourceBlend: CustomMaterial.SrcAlpha
            destinationBlend: CustomMaterial.One
            TextureInput {
                id: sparktexinput
                texture: sparktex
                enabled: true
            }

            Texture {
                id: sparktex
                source: "file:content/images/spark.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
            }
            vertexShader: "shaders/spark.vert"
            fragmentShader: "shaders/spark.frag"
            shadingMode: CustomMaterial.Shaded
        }
    }
}
