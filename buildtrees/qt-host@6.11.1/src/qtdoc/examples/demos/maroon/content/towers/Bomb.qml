// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtMultimedia
import examples.Maroon
import "../logic.js" as Logic

TowerBase {
    id: container
    hp: 10
    range: 0.4
    rof: 10
    property real detonationRange: 2.5

    function fire() {
        sound.play()
        sprite.jumpTo("shoot")
        animDelay.start()
    }

    function finishFire() {
        let sCol = Math.max(0, col - 1)
        let eCol = Math.min(Logic.gameState.cols - 1, col + 1)
        let killList = new Array()
        for (let i = sCol; i <= eCol; i++) {
            for (let j = 0; j < Logic.gameState.mobs[i].length; j++)
                if (Math.abs(Logic.gameState.mobs[i][j].y - container.y) < Logic.gameState.squareSize * detonationRange)
                    killList.push(Logic.gameState.mobs[i][j])
            while (killList.length > 0)
                Logic.killMob(i, killList.pop())
        }
        Logic.killTower(row, col);
    }

    Timer {
        id: animDelay
        running: false
        interval: shootState.frameCount * shootState.frameDuration
        onTriggered: container.finishFire()
    }

    function die()
    {
        destroy() // No blink, because we usually meant to die
    }

    SoundEffect {
        id: sound
        source: "../audio/bomb-action.wav"
    }

    SpriteSequence {
        id: sprite
        width: 64
        height: 64
        interpolate: false
        goalSprite: ""

        Sprite {
            name: "idle"
            source: "../gfx/bomb-idle.png"
            frameCount: 4
            frameDuration: 800
        }

        Sprite {
            id: shootState
            name: "shoot"
            source: "../gfx/bomb-action.png"
            frameCount: 6
            frameDuration: 155
            to: { "dying" : 1 } // So that if it takes a frame to clean up, it is on the last frame of the explosion
        }

        Sprite {
            name: "dying"
            source: "../gfx/bomb-action.png"
            frameCount: 1
            frameX: 64 * 5
            frameWidth: 64
            frameHeight: 64
            frameDuration: 155
        }

        SequentialAnimation on x {
            loops: Animation.Infinite
            NumberAnimation { from: sprite.x; to: sprite.x + 4; duration: 900; easing.type: Easing.InOutQuad }
            NumberAnimation { from: sprite.x + 4; to: sprite.x; duration: 900; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on y {
            loops: Animation.Infinite
            NumberAnimation { from: sprite.y; to: sprite.y - 4; duration: 900; easing.type: Easing.InOutQuad }
            NumberAnimation { from: sprite.y - 4; to: sprite.y; duration: 900; easing.type: Easing.InOutQuad }
        }
    }
}
