// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtMultimedia
import examples.Maroon

TowerBase {
    hp: 4
    range: 0.1
    damage: 1
    rof: 40
    income: 0

    SpriteSequence {
        id: sprite
        width: 64
        height: 64
        interpolate: false
        goalSprite: ""

        Sprite {
            name: "idle"
            source: "../gfx/melee-idle.png"
            frameCount: 8
            frameDuration: 250
        }

        Sprite {
            name: "shoot"
            source: "../gfx/melee-action.png"
            frameCount: 2
            frameDuration: 200
            to: { "idle" : 1 }
        }
    }

    function fire() {
        shootSound.play()
        sprite.jumpTo("shoot")
    }

    function die() { dieImpl(); }

    SoundEffect {
        id: shootSound
        source: "../audio/melee-action.wav"
    }
}
