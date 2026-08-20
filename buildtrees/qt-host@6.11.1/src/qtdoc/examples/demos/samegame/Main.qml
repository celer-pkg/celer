// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Particles
import "content/samegame.js" as Logic
import "content"

Rectangle {
    id: root
    width: Settings.screenWidth; height: Settings.screenHeight
    property int acc: 0

    readonly property real safeTop: SafeArea.margins.top
    readonly property real safeBottom: SafeArea.margins.bottom
    readonly property real safeLeft: SafeArea.margins.left
    readonly property real safeRight: SafeArea.margins.right

    function loadPuzzle() {
        if (gameCanvas.mode != "")
            Logic.cleanUp();
        Logic.startNewGame(gameCanvas,"puzzle","levels/level"+acc+".qml")
    }
    function nextPuzzle() {
        acc = (acc + 1) % 10;
        loadPuzzle();
    }
    Timer {
        id: gameOverTimer
        interval: 1500
        running : gameCanvas.gameOver && gameCanvas.mode == "puzzle" //mode will be reset by cleanUp();
        repeat  : false
        onTriggered: {
            Logic.cleanUp();
            root.nextPuzzle();
        }
    }

    Image {
        source: "content/gfx/background.png"
        anchors.fill: parent
    }

    GameArea {
        id: gameCanvas
        z: 1
        x: root.safeLeft
        y: Settings.headerHeight + root.safeTop

        width: parent.width - root.safeLeft - root.safeRight
        height: parent.height - Settings.headerHeight - root.safeTop - Settings.footerHeight - root.safeBottom

        backgroundVisible: root.state == "in-game"
        onModeChanged: if (gameCanvas.mode != "puzzle") puzzleWon = false; //UI has stricter constraints on this variable than the game does
        Age {
            groups: ["redspots", "greenspots", "bluespots", "yellowspots"]
            enabled: root.state == ""
            system: gameCanvas.ps
        }

        onPuzzleLost: root.acc--;//So that nextPuzzle() reloads the current one

    }

    Item {
        id: menu
        z: 2
        x: root.safeLeft
        width: parent.width - root.safeLeft - root.safeRight
        anchors.top: parent.top
        anchors.topMargin: root.safeTop
        anchors.bottom: bottomBar.top

        LogoAnimation {
            x: 64
            y: Settings.headerHeight
            particleSystem: gameCanvas.ps
            running: root.state == ""
        }
        Row {
            x: 112
            y: 20
            Image { source: "content/gfx/logo-a.png" }
            Image { source: "content/gfx/logo-m.png" }
            Image { source: "content/gfx/logo-e.png" }
        }

        Column {
            y: 100 + 40
            spacing: Settings.menuButtonSpacing
            width: parent.width
            height: parent.height - (140 + Settings.footerHeight)

            Button {
                width: parent.width
                rotatedButton: true
                imgSrc: Qt.resolvedUrl("content/gfx/but-game-1.png")
                onClicked: {
                    if (root.state == "in-game")
                        return //Prevent double clicking
                    root.state = "in-game"
                    gameCanvas.blockFile = "Block.qml"
                    gameCanvas.background = "gfx/background.png"
                    arcadeTimer.start();
                }
                //Emitted particles don't fade out, because ImageParticle is on the GameArea
                system: gameCanvas.ps
                group: "green"
                Timer {
                    id: arcadeTimer
                    interval: Settings.menuDelay
                    running : false
                    repeat  : false
                    onTriggered: Logic.startNewGame(gameCanvas)
                }
            }

            Button {
                width: parent.width
                rotatedButton: true
                imgSrc: Qt.resolvedUrl("content/gfx/but-game-2.png")
                onClicked: {
                    if (root.state == "in-game")
                        return
                    root.state = "in-game"
                    gameCanvas.blockFile = "Block.qml"
                    gameCanvas.background = "gfx/background.png"
                    twopTimer.start();
                }
                system: gameCanvas.ps
                group: "green"
                Timer {
                    id: twopTimer
                    interval: Settings.menuDelay
                    running : false
                    repeat  : false
                    onTriggered: Logic.startNewGame(gameCanvas, "multiplayer")
                }
            }

            Button {
                width: parent.width
                rotatedButton: true
                imgSrc: Qt.resolvedUrl("content/gfx/but-game-3.png")
                onClicked: {
                    if (root.state == "in-game")
                        return
                    root.state = "in-game"
                    gameCanvas.blockFile = "SimpleBlock.qml"
                    gameCanvas.background = "gfx/background.png"
                    endlessTimer.start();
                }
                system: gameCanvas.ps
                group: "blue"
                Timer {
                    id: endlessTimer
                    interval: Settings.menuDelay
                    running : false
                    repeat  : false
                    onTriggered: Logic.startNewGame(gameCanvas, "endless")
                }
            }

            Button {
                width: parent.width
                rotatedButton: true
                imgSrc: Qt.resolvedUrl("content/gfx/but-game-4.png")
                group: "yellow"
                onClicked: {
                    if (root.state == "in-game")
                        return
                    root.state = "in-game"
                    gameCanvas.blockFile = "PuzzleBlock.qml"
                    gameCanvas.background = "gfx/background.png"
                    puzzleTimer.start();
                }
                Timer {
                    id: puzzleTimer
                    interval: Settings.menuDelay
                    running : false
                    repeat  : false
                    onTriggered: root.loadPuzzle();
                }
                system: gameCanvas.ps
            }
        }
    }

    Image {
        id: scoreBar
        source: "content/gfx/bar.png"
        width: parent.width
        z: 6
        y: -(Settings.headerHeight + root.safeTop)
        height: Settings.headerHeight + root.safeTop
        Behavior on opacity { NumberAnimation {} }
        SamegameText {
            id: arcadeScore
            anchors { right: parent.right; top: parent.top; topMargin: root.safeTop; rightMargin: root.safeRight + 11}
            text: '<font color="#f7d303">P1:</font> ' + gameCanvas.score
            font.pixelSize: Settings.fontPixelSize
            textFormat: Text.StyledText
            color: "white"
            opacity: gameCanvas.mode == "arcade" ? 1 : 0
            Behavior on opacity { NumberAnimation {} }
        }
        SamegameText {
            id: arcadeHighScore
            anchors { left: parent.left; top: parent.top; topMargin: root.safeTop; leftMargin: root.safeLeft + 11}
            text: '<font color="#f7d303">Highscore:</font> ' + gameCanvas.highScore
            opacity: gameCanvas.mode == "arcade" ? 1 : 0
        }
        SamegameText {
            id: p1Score
            anchors { right: parent.right; top: parent.top; topMargin: root.safeTop; rightMargin: root.safeRight + 11}
            text: '<font color="#f7d303">P1:</font> ' + gameCanvas.score
            opacity: gameCanvas.mode == "multiplayer" ? 1 : 0
        }
        SamegameText {
            id: p2Score
            anchors { left: parent.left; top: parent.top; topMargin: root.safeTop; leftMargin: root.safeLeft + 11}
            text: '<font color="#f7d303">P2:</font> ' + gameCanvas.score2
            opacity: gameCanvas.mode == "multiplayer" ? 1 : 0
            rotation: 180
        }
        SamegameText {
            id: puzzleMoves
            anchors { left: parent.left; top: parent.top; topMargin: root.safeTop; leftMargin: root.safeLeft + 11}
            text: '<font color="#f7d303">Moves:</font> ' + gameCanvas.moves
            opacity: gameCanvas.mode == "puzzle" ? 1 : 0
        }
        SamegameText {
            Image {
                source: "content/gfx/icon-time.png"
                x: -20
            }
            id: puzzleTime
            anchors { verticalCenter: parent.verticalCenter; verticalCenterOffset: root.safeTop / 2; horizontalCenter: parent.horizontalCenter; horizontalCenterOffset: 20}
            text: "00:00"
            opacity: gameCanvas.mode == "puzzle" ? 1 : 0
            Timer {
                interval: 1000
                repeat: true
                running: gameCanvas.mode == "puzzle" && !gameCanvas.gameOver
                onTriggered: {
                    var elapsed = Math.floor((new Date() - Logic.gameDuration)/ 1000.0);
                    var mins = Math.floor(elapsed/60.0);
                    var secs = (elapsed % 60);
                    puzzleTime.text =  (mins < 10 ? "0" : "") + mins + ":" + (secs < 10 ? "0" : "") + secs;
                }
            }
        }
        SamegameText {
            id: puzzleScore
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; verticalCenterOffset: root.safeTop / 2; rightMargin: root.safeRight + 11}
            text: '<font color="#f7d303">Score:</font> ' + gameCanvas.score
            opacity: gameCanvas.mode == "puzzle" ? 1 : 0
        }
    }

    Image {
        id: bottomBar
        width: parent.width
        height: Settings.footerHeight + root.safeBottom
        source: "content/gfx/bar.png"
        y: parent.height - Settings.footerHeight - root.safeBottom
        z: 2
        Button {
            id: quitButton
            height: Settings.toolButtonHeight
            imgSrc: Qt.resolvedUrl("content/gfx/but-quit.png")
            onClicked: {Qt.quit(); }
            anchors { left: parent.left; verticalCenter: parent.verticalCenter; verticalCenterOffset: -root.safeBottom / 2; leftMargin: root.safeLeft + 11 }
        }
        Button {
            id: menuButton
            height: Settings.toolButtonHeight
            imgSrc: Qt.resolvedUrl("content/gfx/but-menu.png")
            visible: (root.state == "in-game");
            onClicked: {root.state = ""; Logic.cleanUp(); gameCanvas.mode = ""}
            anchors { left: quitButton.right; verticalCenter: parent.verticalCenter; verticalCenterOffset: -root.safeBottom / 2; leftMargin: 0 }
        }
        Button {
            id: againButton
            height: Settings.toolButtonHeight
            imgSrc: Qt.resolvedUrl("content/gfx/but-game-new.png")
            visible: (root.state == "in-game");
            opacity: gameCanvas.gameOver && (gameCanvas.mode == "arcade" || gameCanvas.mode == "multiplayer")
            Behavior on opacity{ NumberAnimation {} }
            onClicked: {if (gameCanvas.gameOver) { Logic.startNewGame(gameCanvas, gameCanvas.mode);}}
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; verticalCenterOffset: -root.safeBottom / 2; rightMargin: root.safeRight + 11 }
        }
        Button {
            id: nextButton
            height: Settings.toolButtonHeight
            imgSrc: Qt.resolvedUrl("content/gfx/but-puzzle-next.png")
            visible: (root.state == "in-game") && gameCanvas.mode == "puzzle" && gameCanvas.puzzleWon
            opacity: gameCanvas.puzzleWon ? 1 : 0
            Behavior on opacity{ NumberAnimation {} }
            onClicked: {if (gameCanvas.puzzleWon) root.nextPuzzle();}
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; verticalCenterOffset: -root.safeBottom / 2; rightMargin: root.safeRight + 11 }
        }
    }

    Connections {
        target: root
        function onStateChanged() {
            stateChangeAnim.running = true
        }
    }
    SequentialAnimation {
        id: stateChangeAnim
        ParallelAnimation {
            NumberAnimation { target: bottomBar; property: "y"; to: root.height; duration: Settings.menuDelay/2; easing.type: Easing.OutQuad }
            NumberAnimation { target: scoreBar; property: "y"; to: -(Settings.headerHeight + root.safeTop); duration: Settings.menuDelay/2; easing.type: Easing.OutQuad }
        }
        ParallelAnimation {
            NumberAnimation { target: bottomBar; property: "y"; to: root.height - Settings.footerHeight - root.safeBottom; duration: Settings.menuDelay/2; easing.type: Easing.OutBounce}
            NumberAnimation { target: scoreBar; property: "y"; to: root.state == "" ? -(Settings.headerHeight + root.safeTop) : 0; duration: Settings.menuDelay/2; easing.type: Easing.OutBounce}
        }
    }

    states: [
        State {
            name: "in-game"
            PropertyChanges {
                menu {
                    opacity: 0
                    visible: false
                }
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {properties: "x,y,opacity"}
        }
    ]

    //"Debug mode"
    focus: true
    Keys.onAsteriskPressed: Logic.nuke();
    Keys.onSpacePressed: gameCanvas.puzzleWon = true;
}
