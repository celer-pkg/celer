// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

ParticleSystem3D {
    id: psystem
    property real spriteScale: 2

    useRandomSeed: false
    startTime: 5000

    SpriteParticle3D {
        id: spriteParticle
        sprite: Texture {
            source: "qrc:/qt/qml/Quick3DAssets/Snow/snow.png"
        }
        maxAmount: 20000
        color: "#ffffff"
        colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.5)
        fadeInDuration: 500
        fadeOutDuration: 500
        billboard: true
        spriteSequence: SpriteSequence3D {
            frameCount: 8
            frameIndex: 0
            interpolate: false
            randomStart: true
            duration: 16000
        }
    }

    ParticleEmitter3D {
        id: emitter

        system: psystem
        particle: spriteParticle
        position: Qt.vector3d(0, 350, 0)
        depthBias: -10
        scale: Qt.vector3d(15.0, 5.0, 15.0)
        shape: ParticleShape3D {
            type: ParticleShape3D.Sphere
        }
        particleScale: psystem.spriteScale
        particleScaleVariation: psystem.spriteScale * 0.5
        velocity: VectorDirection3D {
            direction: Qt.vector3d(0, -100, 0)
            directionVariation: Qt.vector3d(20, 50, 20)
        }
        emitRate: 2000
        lifeSpan: 2000
    }

    PointRotator3D {
        pivotPoint: Qt.vector3d(0, 0, 0)
        direction: Qt.vector3d(0, 1, 0)
        magnitude: 20
        system: psystem
    }

    ShapeAffector3D {
        id: carBottomMask
        scale: Qt.vector3d(5.3, 1, 2.5)
        position: Qt.vector3d(-20.0, 0, 0)
        eulerRotation.y: 45
        shapeType: ShapeAffector3D.Sphere
    }

    ShapeAffector3D {
        id: carInteriorMask
        eulerRotation.y: 45
        scale: Qt.vector3d(2.5, 1.5, 2.05)
        position:Qt.vector3d(-25, 67, 6)
        shapeType: ShapeAffector3D.Sphere
    }
}
