// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtMultimedia
import examples.Maroon
import "../logic.js" as Logic

TowerBase {
    id: container
    hp: 2
    range: 6
    damage: 0 // By projectile
    rof: 40
    income: 0
    property var targetMob
    property real realDamage: 1
    function fire() {
        proj.x = 32 - proj.width / 2
        proj.y = 0
        targetMob = Logic.gameState.mobs[col][0]
        projAnim.to = targetMob.y - container.y -10
        projAnim.start()
        shootSound.play()
        sprite.jumpTo("shoot")
    }
    function die() { dieImpl(); }
    Image {
        id: proj
        SequentialAnimation on y {
            id: projAnim
            running: false
            property real to: 1000
            SmoothedAnimation {
                to: projAnim.to
                velocity: 400
            }
            ScriptAction {
                script: {
                    if (container.targetMob && container.targetMob.hit) {
                        container.targetMob.hit(realDamage)
                        container.targetMob.inked()
                        projSound.play()
                    }
                }
            }
            PropertyAction {
                value: 1000;
            }
        }
        source: "../gfx/projectile.png"
        Component.onCompleted: {
            y = 1000
        }
    }

    SoundEffect {
        id: shootSound
        source: "../audio/shooter-action.wav"
    }
    SoundEffect {
        id: projSound
        source: "../audio/projectile-action.wav"
    }

    SpriteSequence {
        id: sprite
        width: 64
        height: 64
        interpolate: false
        goalSprite: ""

        Sprite {
            name: "idle"
            source: "../gfx/shooter-idle.png"
            frameCount: 4
            frameDuration: 250
        }

        Sprite {
            name: "shoot"
            source: "../gfx/shooter-action.png"
            frameCount: 5
            frameDuration: 90
            to: { "idle" : 1 }
        }

        SequentialAnimation on x {
            loops: Animation.Infinite
            NumberAnimation { from: sprite.x; to: sprite.x - 4; duration: 900; easing.type: Easing.InOutQuad }
            NumberAnimation { from: sprite.x - 4; to: sprite.x; duration: 900; easing.type: Easing.InOutQuad }
        }
    }
}
