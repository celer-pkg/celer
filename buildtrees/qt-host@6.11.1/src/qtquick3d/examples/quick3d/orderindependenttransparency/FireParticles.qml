// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Node {
    id: root
    property ParticleSystem3D ps
    property real size: 1.0
    property alias particle : fireEmitter.particle

    ParticleEmitter3D {
        id: fireEmitter
        system: root.ps
        particleScale: 2
        particleEndScale: 1
        particleScaleVariation: 1
        particleEndScaleVariation: 1
        velocity: VectorDirection3D {
            direction: Qt.vector3d(0, 100 * root.size, 0)
            directionVariation: Qt.vector3d(30, 10, 30)
        }
        emitRate: 100
        lifeSpan: 300 * root.size
        lifeSpanVariation: 200
    }
}

