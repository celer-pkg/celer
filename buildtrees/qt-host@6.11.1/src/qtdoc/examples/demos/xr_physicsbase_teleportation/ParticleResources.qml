// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma Singleton

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Node {
    function init(scene) {
        particleSystem.parent = scene
    }

    property alias system: particleSystem
    property alias smokeSpriteTexture: smokeTexture

    ParticleSystem3D {
        id: particleSystem
    }

    Texture {
        id: smokeTexture
        source: "media/textures/smoke_sprite.png"
    }
}
