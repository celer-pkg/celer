// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers

ParticleSystem3D {

    property real gravityStrength: 0.0
    property bool variableLengthEnabled: false
    property int lifespan: 3000

    id: psystem

    Gravity3D {
        position: Qt.vector3d(600, 100, 0)
        magnitude: psystem.gravityStrength
        direction: Qt.vector3d(1, 0, 0)
    }

    LineEmitter {
        description: "length 50"
        fontSize: mainWindow.fontSize
        system: psystem
        emitterPosition: Qt.vector3d(0, 0, 0)
        variableLengthEnabled: psystem.variableLengthEnabled
        lifespan: psystem.lifespan
    }

    LineEmitter {
        description: "length 100"
        fontSize: mainWindow.fontSize
        system: psystem
        emitterPosition: Qt.vector3d(0, 50, 0)
        lineLength: 100
        velocityX: 240
        lifespan: psystem.lifespan - 1000
        variableLengthEnabled: psystem.variableLengthEnabled
    }

    LineEmitter {
        description: "End-of-Life fade"
        fontSize: mainWindow.fontSize
        system: psystem
        emitterPosition: Qt.vector3d(0, 100, 0)
        particleEolFadeDuration: 1000
        fadeOutDuration: 0
        variableLengthEnabled: psystem.variableLengthEnabled
        lifespan: psystem.lifespan
    }

    LineEmitter {
        description: "textured"
        fontSize: mainWindow.fontSize
        system: psystem
        emitterPosition: Qt.vector3d(0, 150, 0)
        textureEnabled: true
        variableLengthEnabled: psystem.variableLengthEnabled
        lifespan: psystem.lifespan
    }

    LineEmitter {
        description: "textured, absolute coordinates"
        fontSize: mainWindow.fontSize
        system: psystem
        emitterPosition: Qt.vector3d(0, 200, 0)
        textureEnabled: true
        particleTexcoordMode: LineParticle3D.Absolute
        variableLengthEnabled: psystem.variableLengthEnabled
        lifespan: psystem.lifespan
    }

    LineEmitter {
        description: "textured, filled"
        fontSize: mainWindow.fontSize
        system: psystem
        emitterPosition: Qt.vector3d(0, 250, 0)
        textureEnabled: true
        particleTexcoordMode: LineParticle3D.Fill
        variableLengthEnabled: psystem.variableLengthEnabled
        lifespan: psystem.lifespan
    }
}
