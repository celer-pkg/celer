// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Particles
import "samegame.js" as Logic

Item {
    id: gameCanvas
    property bool gameOver: true
    property int score: 0
    property int highScore: 0
    property int moves: 0
    property string mode: ""
    property ParticleSystem ps: particleSystem
    //For easy theming
    property alias backgroundVisible: bg.visible
    property string background: "gfx/background.png"
    property string blockFile: "Block.qml"
    property int blockSize: Settings.blockSize
    onBlockFileChanged: Logic.changeBlock(blockFile);
    property alias particlePack: auxLoader.source
    //For multiplayer
    property int score2: 0
    property int curTurn: 1
    property bool autoTurnChange: false
    signal swapPlayers
    property bool swapping: false
    //onSwapPlayers: if (autoTurnChange) Logic.turnChange();//Now implemented below
    //For puzzle
    property url level
    property bool puzzleWon: false
    signal puzzleLost //Since root is tracking the puzzle progress
    function showPuzzleEnd (won) {
        if (won) {
            smokeParticle.color = Qt.rgba(0,1,0,0);
            puzzleWin.play();
        } else {
            smokeParticle.color = Qt.rgba(1,0,0,0);
            puzzleFail.play();
            puzzleLost();
        }
    }
    function showPuzzleGoal (str) {
        puzzleTextBubble.opacity = 1;
        puzzleTextLabel.text = str;
    }
    Image {
        id: bg
        z: -1
        anchors.fill: parent
        source: gameCanvas.background;
        fillMode: Image.PreserveAspectCrop
    }

    MouseArea {
        anchors.fill: parent
        onClicked: mouse => {
            if (puzzleTextBubble.opacity == 1) {
                puzzleTextBubble.opacity = 0;
                Logic.finishLoadingMap();
            } else if (!gameCanvas.swapping) {
                Logic.handleClick(mouse.x,mouse.y);
            }
        }
    }

    Image {
        id: highScoreTextBubble
        opacity: gameCanvas.mode == "arcade" && gameCanvas.gameOver && gameCanvas.score == gameCanvas.highScore ? 1 : 0
        Behavior on opacity { NumberAnimation {} }
        anchors.centerIn: parent
        z: 10
        source: "gfx/bubble-highscore.png"
        Image {
            anchors.centerIn: parent
            source: "gfx/text-highscore-new.png"
            rotation: -10
        }
    }

    Image {
        id: puzzleTextBubble
        anchors.centerIn: parent
        opacity: 0
        Behavior on opacity { NumberAnimation {} }
        z: 10
        source: "gfx/bubble-puzzle.png"
        Connections {
            target: gameCanvas
            function onModeChanged(mode) {
                if (mode != "puzzle" && puzzleTextBubble.opacity > 0)
                    puzzleTextBubble.opacity = 0;
            }
        }
        Text {
            id: puzzleTextLabel
            width: parent.width - 24
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            font.pixelSize: 24
            font.bold: true
            wrapMode: Text.WordWrap
        }
    }
    onModeChanged: {
        p1WonImg.opacity = 0;
        p2WonImg.opacity = 0;
    }
    SmokeText { id: puzzleWin; source: "gfx/icon-ok.png"; system: particleSystem }
    SmokeText { id: puzzleFail; source: "gfx/icon-fail.png"; system: particleSystem }

    onSwapPlayers: {
        smokeParticle.color = "yellow"
        Logic.turnChange();
        if (curTurn == 1) {
            p1Text.play();
        } else {
            p2Text.play();
        }
        clickDelay.running = true;
    }
    SequentialAnimation {
        id: clickDelay
        ScriptAction { script: gameCanvas.swapping = true; }
        PauseAnimation { duration: 750 }
        ScriptAction { script: gameCanvas.swapping = false; }
    }

    SmokeText {
        id: p1Text; source: "gfx/text-p1-go.png";
        system: particleSystem; playerNum: 1
        opacity: p1WonImg.opacity + p2WonImg.opacity > 0 ? 0 : 1
    }

    SmokeText {
        id: p2Text; source: "gfx/text-p2-go.png";
        system: particleSystem; playerNum: 2
        opacity: p1WonImg.opacity + p2WonImg.opacity > 0 ? 0 : 1
    }

    onGameOverChanged: {
        if (gameCanvas.mode == "multiplayer") {
            if (gameCanvas.score >= gameCanvas.score2) {
                p1WonImg.opacity = 1;
            } else {
                p2WonImg.opacity = 1;
            }
        }
    }
    Image {
        id: p1WonImg
        source: "gfx/text-p1-won.png"
        anchors.centerIn: parent
        opacity: 0
        Behavior on opacity { NumberAnimation {} }
        z: 10
    }
    Image {
        id: p2WonImg
        source: "gfx/text-p2-won.png"
        anchors.centerIn: parent
        opacity: 0
        Behavior on opacity { NumberAnimation {} }
        z: 10
    }

    ParticleSystem{
        id: particleSystem;
        anchors.fill: parent
        z: 5
        ImageParticle {
            id: smokeParticle
            groups: ["smoke"]
            source: "gfx/particle-smoke.png"
            alpha: 0.1
            alphaVariation: 0.1
            color: "yellow"
        }
        Loader {
            id: auxLoader
            anchors.fill: parent
            source: "PrimaryPack.qml"
            onItemChanged: {
                if (item && "particleSystem" in item)
                    item.particleSystem = particleSystem
                if (item && "gameArea" in item)
                    item.gameArea = gameCanvas
            }
        }
    }
}

