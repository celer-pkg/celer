// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import "logic.js" as Logic

Item {
    id: grid

    property int squareSize: 64
    property int rows: 6
    property int cols: 4
    property Item canvas: grid
    property int score: 0
    property int coins: 100
    property int lives: 3
    property int waveNumber: 0
    property int waveProgress: 0
    property var towers
    property var mobs
    property bool gameRunning: false
    property bool gameOver: false
    property bool errored: false
    property string errorString: ""

    width: cols * squareSize
    height: rows * squareSize

    function freshState() {
        lives = 3
        coins = 100
        score = 0
        waveNumber = 0
        waveProgress = 0
        gameOver = false
        gameRunning = false
        towerMenu.shown = false
        helpButton.comeBack();
    }

    Text {
        id: errorText // Mostly for debug purposes
        text: grid.errorString
        visible: grid.errored
        color: "red"
        font.pixelSize: 18
        wrapMode: Text.WordWrap
        width: parent.width / 1.2
        height: parent.height / 1.2
        anchors.centerIn: parent
        z: 1000
    }

    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: Logic.tick()
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        onClicked: function (mouse) {
            if (towerMenu.visible)
                towerMenu.finish()
            else
                towerMenu.open(mouse.x, mouse.y)
        }
    }

    Image {
        id: towerMenu
        visible: false
        z: 1500
        scale: 0.9
        opacity: 0.7
        property int dragDistance: 16
        property int targetRow: 0
        property int targetCol: 0
        property bool shown: false
        property bool towerExists: false

        function finish() {
            shown = false
        }

        function open(xp,yp) {
            if (!grid.gameRunning)
                return
            targetRow = Logic.row(yp)
            targetCol = Logic.col(xp)
            if (targetRow == 0)
                towerMenu.y = (targetRow + 1) * grid.squareSize
            else
                towerMenu.y = (targetRow - 1) * grid.squareSize
            towerExists = (grid.towers[Logic.towerIdx(targetCol, targetRow)] != null)
            shown = true
            helpButton.goAway();
        }

        states: State {
            name: "shown"; when: towerMenu.shown && !grid.gameOver
            PropertyChanges { towerMenu { visible: true; scale: 1; opacity: 1 } }
        }

        transitions: Transition {
            PropertyAction { property: "visible" }
            NumberAnimation { properties: "opacity,scale"; duration: 500; easing.type: Easing.OutElastic }
        }

        x: -32
        source: "gfx/dialog.png"
        Row {
            id: buttonRow
            height: 100
            anchors.centerIn: parent
            spacing: 8
            BuildButton {
                row: towerMenu.targetRow; col: towerMenu.targetCol
                anchors.verticalCenter: parent.verticalCenter
                towerType: 1; index: 0
                canBuild: !towerMenu.towerExists
                source: "gfx/dialog-melee.png"
                onClicked: towerMenu.finish()
            }
            BuildButton {
                row: towerMenu.targetRow; col: towerMenu.targetCol
                anchors.verticalCenter: parent.verticalCenter
                towerType: 2; index: 1
                canBuild: !towerMenu.towerExists
                source: "gfx/dialog-shooter.png"
                onClicked: towerMenu.finish()
            }
            BuildButton {
                row: towerMenu.targetRow; col: towerMenu.targetCol
                anchors.verticalCenter: parent.verticalCenter
                towerType: 3; index: 2
                canBuild: !towerMenu.towerExists
                source: "gfx/dialog-bomb.png"
                onClicked: towerMenu.finish()
            }
            BuildButton {
                row: towerMenu.targetRow; col: towerMenu.targetCol
                anchors.verticalCenter: parent.verticalCenter
                towerType: 4; index: 3
                canBuild: !towerMenu.towerExists
                source: "gfx/dialog-factory.png"
                onClicked: towerMenu.finish()
            }
        }
    }


    Keys.onPressed: function (event) { // Cheat Codes while Testing
        if (event.key == Qt.Key_Up && (event.modifiers & Qt.ShiftModifier))
            grid.coins += 10;
        if (event.key == Qt.Key_Left && (event.modifiers & Qt.ShiftModifier))
            grid.lives += 1;
        if (event.key == Qt.Key_Down && (event.modifiers & Qt.ShiftModifier))
            Logic.gameState.waveProgress += 1000;
        if (event.key == Qt.Key_Right && (event.modifiers & Qt.ShiftModifier))
            Logic.endGame();
    }

    Image {
        id: helpButton
        z: 1010
        source: "gfx/button-help.png"
        function goAway() {
            helpMA.enabled = false;
            helpButton.opacity = 0;
        }
        function comeBack() {
            helpMA.enabled = true;
            helpButton.opacity = 1;
        }
        Behavior on opacity { NumberAnimation {} }
        MouseArea {
            id: helpMA
            anchors.fill: parent
            onClicked: {helpImage.visible = true; helpButton.visible = false;}
        }

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
    }

    Image {
        id: helpImage
        z: 1010
        source: "gfx/help.png"
        anchors.fill: parent
        visible: false
        MouseArea {
            anchors.fill: parent
            onClicked: helpImage.visible = false;
        }
    }

}
