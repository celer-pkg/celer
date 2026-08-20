// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Particles3D

Node {
    property bool inGravityField: false
    signal teleporterTriggered

    PrincipledMaterial {
        id: wallMateriall
        baseColor: "#ff776644"
        roughness: 0.7
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: triggerMaterial
        baseColor: "#66aaff"
        roughness: 0.5
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: doorMaterial
        baseColor: "#ff10670e"
        roughness: 0.5
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: whiteMaterial
        baseColor: "white"
        roughness: 0.8
    }

    Texture {
        id: tileColor_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles107_1K_Color.jpg"
    }
    Texture {
        id: tileNormal_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles107_1K_NormalGL.jpg"
    }
    Texture {
        id: tileRoughness_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles107_1K_Roughness.jpg"
    }

    Texture {
        id: blackTileColor_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tiles108_1K_Color.jpg"
    }

    PrincipledMaterial {
        id: internalWallMaterial
        baseColorMap: tileColor_texture
        roughnessMap: tileRoughness_texture
        roughness: 1
        normalMap: tileNormal_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    PrincipledMaterial {
        id: testFloor
        baseColorMap: blackTileColor_texture
        roughnessMap: tileRoughness_texture
        roughness: 1
        normalMap: tileNormal_texture
        normalStrength: 0.5
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    Texture {
        id: warningColor_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tape001_1K_Color.jpg"
    }
    Texture {
        id: warningNormal_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tape001_1K_NormalGL.jpg"
    }
    Texture {
        id: warningRoughness_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/Tape001_1K_Roughness.jpg"
    }

    PrincipledMaterial {
        id: warningMaterial
        baseColorMap: warningColor_texture
        roughnessMap: warningRoughness_texture
        roughness: 1
        normalMap: warningNormal_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    StaticRigidBody {
        id: building
        position: Qt.vector3d(-165.234, -99.9, -443.476)
        collisionShapes: TriangleMeshShape {
            source: "meshes/building.mesh"
        }
        Model {
            source: "meshes/building.mesh"
            materials: [testFloor, internalWallMaterial, whiteMaterial, warningMaterial]
        }
    }

    TriggerBody {
        id: trigger
        position: Qt.vector3d(-162.316, -16.3654, -743.701)
        scale: "13.26, 8.33, 5.58"
        collisionShapes: BoxShape {}
        Model {
            source: "#Cube"
            materials: triggerMaterial
            opacity: 0.1
        }
        onBodyEntered: inGravityField = true
        onBodyExited: inGravityField = false
    }

    Timer {
        id: teleportTriggerDelay
        interval: 250
        onTriggered: teleporterTriggered() // emit signal
    }

    TriggerBody {
        id: teleportTrigger
        position: Qt.vector3d(-762.058, 100, 424.581)
        scale: Qt.vector3d(1, 4, 1)
        collisionShapes: BoxShape {}
        Model {
            source: "#Cylinder"
            materials: PrincipledMaterial {
                baseColor: "red"
            }
            opacity: 0.2
        }
        onBodyEntered: teleportTriggerDelay.start()
        onBodyExited: teleportTriggerDelay.stop()
    }

    ParticleSystem3D {
        id: psystem

        startTime: 5000

        SpriteParticle3D {
            id: spriteParticle
            sprite: Texture {
                source: "maps/sphere.png"
            }
            maxAmount: 10000
            color: "#aaccff"
            colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.5)
            fadeInDuration: 1000
            fadeOutDuration: 1000
            billboard: true
        }

        ParticleEmitter3D {
            id: emitter
            particle: spriteParticle
            position: Qt.vector3d(-200, -600, -750)
            depthBias: -100
            scale: Qt.vector3d(12.0, 0.5, 5.0)
            shape: ParticleShape3D {
                type: ParticleShape3D.Cube
            }
            particleScale: 2.0
            particleScaleVariation: 1.0
            velocity: VectorDirection3D {
                direction: Qt.vector3d(0, 100, 0)
                directionVariation: Qt.vector3d(20, 50, 20)
            }
            emitRate: 1000
            lifeSpan: 10000
        }

        SpriteParticle3D {
            id: teleportParticle
            sprite: Texture {
                source: "maps/sphere.png"
            }
            maxAmount: 5000
            color: "#ffccaa"
            colorVariation: Qt.vector4d(0.2, 0.2, 0.2, 0.5)
            fadeInDuration: 500
            fadeOutDuration: 500
            billboard: true
        }

        ParticleEmitter3D {
            id: teleportEmitter
            particle: teleportParticle
            position: Qt.vector3d(-712.058, 100, 424.581)
            depthBias: -100
            scale: Qt.vector3d(0.1, 4, 1)
            shape: ParticleShape3D {
                type: ParticleShape3D.Cube
            }
            particleScale: 0.5
            particleScaleVariation: 1.0
            velocity: VectorDirection3D {
                direction: Qt.vector3d(-100, 0, 0)
                directionVariation: Qt.vector3d(20, 50, 20)
            }
            emitRate: 5000
            lifeSpan: 1000
        }
    }

    //! [door]
    TriggerBody {
        id: doorTrigger
        position: Qt.vector3d(864.5, 200.4, -461)
        scale: Qt.vector3d(1.5, 2, 3)
        collisionShapes: BoxShape {}
        onBodyEntered: door.open = true
        onBodyExited: door.open = false
    }

    Node {
        id: doorNode
        position: Qt.vector3d(864.5, 200, -461)
        DynamicRigidBody {
            id: door
            property bool open: false
            isKinematic: true
            scale: Qt.vector3d(0.8, 2, 0.1)
            kinematicPosition.x: open ? 79 : 0
            collisionShapes: BoxShape {}
            Model {
                source: "#Cube"
                materials: doorMaterial
            }
            Behavior on kinematicPosition.x {
                NumberAnimation {
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
    //! [door]

    Texture {
        id: sign_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        flipV: true
        flipU: true
        source: "maps/sign.png"
    }

    PrincipledMaterial {
        id: sign_material
        baseColorMap: sign_texture
    }

    Model {
        id: sign

        position: Qt.vector3d(704.498, 269.974, -476.206)
        rotation: Qt.quaternion(0, 1, 0, 0)
        scale: Qt.vector3d(1.8, 1, 0.0687316)
        source: "#Cube"
        materials: sign_material
    }
}
