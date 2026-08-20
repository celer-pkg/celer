// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Particles
import "content/logic.js" as Logic

Window {
    visible: true
    width: 320
    height: 480

    Item {
        id: root
        property var gameState
        property bool passedSplash: false

        Image {
            source:"content/gfx/background.png"
            anchors.bottom: view.bottom

            ParticleSystem {
                id: particles
                anchors.fill: parent

                ImageParticle {
                    id: bubble
                    anchors.fill: parent
                    source: "content/gfx/catch.png"
                    opacity: 0.25
                }

                Wander {
                    xVariance: 25;
                    pace: 25;
                }

                Emitter {
                    width: parent.width
                    height: 150
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    startTime: 15000

                    emitRate: 2
                    lifeSpan: 15000

                    acceleration: PointDirection{ y: -6; xVariation: 2; yVariation: 2 }

                    size: 24
                    sizeVariation: 16
                }
            }
        }

        Column {
            id: view
            y: -(height - 480)
            width: 320

            GameOverScreen { gameCanvas: gameCanvasId }

            Item {
                id: canvasArea
                width: 320
                height: 480

                Row {
                    height: childrenRect.height
                    Image {
                        id: wave
                        y: 30
                        source:"content/gfx/wave.png"
                    }
                    Image {
                        y: 30
                        source:"content/gfx/wave.png"
                    }
                    NumberAnimation on x { from: 0; to: -(wave.width); duration: 16000; loops: Animation.Infinite }
                    SequentialAnimation on y {
                        loops: Animation.Infinite
                        NumberAnimation { from: root.y - 2; to: root.y + 2; duration: 1600; easing.type: Easing.InOutQuad }
                        NumberAnimation { from: root.y + 2; to: root.y - 2; duration: 1600; easing.type: Easing.InOutQuad }
                    }
                }

                Row {
                    opacity: 0.5
                    Image {
                        id: wave2
                        y: 25
                        source: "content/gfx/wave.png"
                    }
                    Image {
                        y: 25
                        source: "content/gfx/wave.png"
                    }
                    NumberAnimation on x { from: -(wave2.width); to: 0; duration: 32000; loops: Animation.Infinite }
                    SequentialAnimation on y {
                        loops: Animation.Infinite
                        NumberAnimation { from: root.y + 2; to: root.y - 2; duration: 1600; easing.type: Easing.InOutQuad }
                        NumberAnimation { from: root.y - 2; to: root.y + 2; duration: 1600; easing.type: Easing.InOutQuad }
                    }
                }

                Image {
                    source: "content/gfx/sunlight.png"
                    opacity: 0.02
                    y: 0
                    anchors.horizontalCenter: parent.horizontalCenter
                    transformOrigin: Item.Top
                    SequentialAnimation on rotation {
                        loops: Animation.Infinite
                        NumberAnimation { from: -10; to: 10; duration: 8000; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 10; to: -10; duration: 8000; easing.type: Easing.InOutSine }
                    }
                }

                Image {
                    source: "content/gfx/sunlight.png"
                    opacity: 0.04
                    y: 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    transformOrigin: Item.Top
                    SequentialAnimation on rotation {
                        loops: Animation.Infinite
                        NumberAnimation { from: 10; to: -10; duration: 8000; easing.type: Easing.InOutSine }
                        NumberAnimation { from: -10; to: 10; duration: 8000; easing.type: Easing.InOutSine }
                    }
                }

                Image {
                    source: "content/gfx/grid.png"
                    opacity: 0.5
                }

                GameCanvas {
                    id: gameCanvasId
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 20
                    x: 32
                    focus: true
                }

                InfoBar { anchors.bottom: canvas.top; anchors.bottomMargin: 6; width: parent.width; canvas: gameCanvasId }

                //3..2..1..go
                Timer {
                    id: countdownTimer
                    interval: 1000
                    running: root.countdown < 5
                    repeat: true
                    onTriggered: root.countdown++
                }
                Repeater {
                    model: ["content/gfx/text-blank.png", "content/gfx/text-3.png", "content/gfx/text-2.png", "content/gfx/text-1.png", "content/gfx/text-go.png"]
                    delegate: Image {
                        required property string modelData
                        required property int index

                        visible: root.countdown <= index
                        opacity: root.countdown == index ? 0.5 : 0.1
                        scale: root.countdown >= index ? 1.0 : 0.0
                        source: modelData
                        Behavior on opacity { NumberAnimation {} }
                        Behavior on scale { NumberAnimation {} }
                    }
                }
            }

            NewGameScreen {
                onStartButtonClicked: root.passedSplash = true
            }
        }

        property int countdown: 10
        Timer {
            id: gameStarter
            interval: 4000
            running: false
            repeat: false
            onTriggered: Logic.startGame(gameCanvasId);
        }

        states: [
            State {
                name: "gameOn"; when: root.gameState.gameOver == false && root.passedSplash
                PropertyChanges { view { y: -(view.height - 960) } }
                StateChangeScript { script: root.countdown = 0; }
                PropertyChanges { gameStarter { running: true } }
            },
            State {
                name: "gameOver"; when: root.gameState.gameOver == true
                PropertyChanges { view { y: 0 } }
            }
        ]

        transitions: Transition {
            NumberAnimation { properties: "x,y"; duration: 1200; easing.type: Easing.OutQuad }
        }

        Component.onCompleted: gameState = Logic.newGameState(gameCanvasId);
    }
}
