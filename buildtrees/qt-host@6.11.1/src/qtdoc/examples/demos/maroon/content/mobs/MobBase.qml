// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtMultimedia
import examples.Maroon
import "../logic.js" as Logic

Item  {
    id: container
    property string name: "Fish"
    property int col: 0
    property real hp: 3
    property real damage: 1
    property real speed: 0.25
    property int rof: 30 //In ticks
    property int fireCounter: 0
    property bool dying: false
    width: (parent as GameCanvas)?.squareSize ?? 0
    height: (parent as GameCanvas)?.squareSize ?? 0
    x: col * width
    z: 1001
    function fire() { }

    function die() {
        if (dying)
            return;
        dying = true;
        bubble.jumpTo("burst");
        if (fishSprite.currentSprite == "front")
            fishSprite.jumpTo(Math.random() > 0.5 ? "left" : "right" );
        fishSwim.start();
        Logic.gameState.score += 1;
        killedSound.play();
        bubble.scale = 0.9
        destroy(350);
    }

    function inked() {
        if (hp > 0)
            ink.jumpTo("dirty");
    }

    function hit(dmg) {
        hp -= dmg;

        if (hp <= 0)
            Logic.killMob(col, container);
    }

    Component.onCompleted: spawnSound.play()

    SoundEffect {
        id: spawnSound
        source: "../audio/catch.wav"
    }
    SoundEffect {
        id: killedSound
        source: "../audio/catch-action.wav"
    }

    SpriteSequence {
        id: fishSprite
        width: 64
        height: 64
        interpolate: false
        goalSprite: ""

        Sprite {
            name: "left"
            source: "../gfx/mob-idle.png"
            frameWidth: 64
            frameHeight: 64
            frameCount: 1
            frameDuration: 800
            frameDurationVariation: 400
            to: { "front" : 1 }
        }

        Sprite {
            name: "front"
            source: "../gfx/mob-idle.png"
            frameCount: 1
            frameX: 64
            frameWidth: 64
            frameHeight: 64
            frameDuration: 800
            frameDurationVariation: 400
            to: { "left" : 1, "right" : 1 }
        }

        Sprite {
            name: "right"
            source: "../gfx/mob-idle.png"
            frameCount: 1
            frameX: 128
            frameWidth: 64
            frameHeight: 64
            frameDuration: 800
            frameDurationVariation: 400
            to: { "front" : 1 }
        }


        Sprite { //WORKAROUND: This prevents the triggering of a rendering error which is currently under investigation.
            name: "dummy"
            source: "../gfx/melee-idle.png"
            frameCount: 8
            frameWidth: 64
            frameHeight: 64
            frameX: 0
            frameDuration: 200
        }

        NumberAnimation on x {
            id: fishSwim
            running: false
            property bool goingLeft: fishSprite.currentSprite == "right"
            to: goingLeft ? -360 : 360
            duration: 300
        }
    }

    SpriteSequence {
        id: bubble
        scale: 0.4 + (0.2  * container.hp)
        interpolate: false
        goalSprite: ""

        Behavior on scale {
            NumberAnimation { duration: 150; easing.type: Easing.OutBack }
        }

        Sprite {
            name: "big"
            source: "../gfx/catch.png"
            frameCount: 1
            to: { "burst" : 0 }
        }

        Sprite {
            name: "burst"
            source: "../gfx/catch-action.png"
            frameCount: 3
            frameX: 64
            frameDuration: 200
        }

        Sprite { //WORKAROUND: This prevents the triggering of a rendering error which is currently under investigation.
            name: "dummy"
            source: "../gfx/melee-idle.png"
            frameCount: 8
            frameWidth: 64
            frameHeight: 64
            frameX: 0
            frameDuration: 200
        }
        SequentialAnimation on width {
            loops: Animation.Infinite
            NumberAnimation { from: container.width * 1; to: container.width * 1.1; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: container.width * 1.1; to: container.width * 1; duration: 1000; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on height {
            loops: Animation.Infinite
            NumberAnimation { from: container.height * 1; to: container.height * 1.15; duration: 1200; easing.type: Easing.InOutQuad }
            NumberAnimation { from: container.height * 1.15; to: container.height * 1; duration: 1000; easing.type: Easing.InOutQuad }
        }
    }

    SpriteSequence {
        id: ink
        scale: bubble.scale
        goalSprite: ""

        Sprite {
            name: "clean"
            source: "../gfx/projectile-action.png"
            frameCount: 1
            frameX: 0
            frameWidth: 64
            frameHeight: 64
        }
        Sprite {
            name: "dirty"
            source: "../gfx/projectile-action.png"
            frameCount: 3
            frameX: 64
            frameWidth: 64
            frameHeight: 64
            frameDuration: 150
            to: {"clean":1}
        }

        Sprite { //WORKAROUND: This prevents the triggering of a rendering error which is currently under investigation.
            name: "dummy"
            source: "../gfx/melee-idle.png"
            frameCount: 8
            frameWidth: 64
            frameHeight: 64
            frameX: 0
            frameDuration: 200
        }
        SequentialAnimation on width {
            loops: Animation.Infinite
            NumberAnimation { from: container.width * 1; to: container.width * 1.1; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: container.width * 1.1; to: container.width * 1; duration: 1000; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on height {
            loops: Animation.Infinite
            NumberAnimation { from: container.height * 1; to: container.height * 1.15; duration: 1200; easing.type: Easing.InOutQuad }
            NumberAnimation { from: container.height * 1.15; to: container.height * 1; duration: 1000; easing.type: Easing.InOutQuad }
        }

    }

    SequentialAnimation on x {
        loops: Animation.Infinite
        NumberAnimation { from: container.x; to: container.x - 5; duration: 900; easing.type: Easing.InOutQuad }
        NumberAnimation { from: container.x - 5; to: container.x; duration: 900; easing.type: Easing.InOutQuad }
    }
}
