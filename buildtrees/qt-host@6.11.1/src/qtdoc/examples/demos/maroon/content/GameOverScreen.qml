// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Particles

Item {
    id: gameOverScreen
    width: 320
    height: 400
    property GameCanvas gameCanvas

    Image {
        id: img
        source: "gfx/text-gameover.png"
        anchors.centerIn: parent
    }

    ParticleSystem {
        anchors.fill: parent
        ImageParticle {
            id: cloud
            source: "gfx/cloud.png"
            alphaVariation: 0.25
            opacity: 0.25
        }

        Wander {
            xVariance: 100;
            pace: 1;
        }

        Emitter {
            id: cloudLeft
            width: 160
            height: 160
            anchors.right: parent.left
            emitRate: 0.5
            lifeSpan: 12000
            velocity: PointDirection{ x: 64; xVariation: 2; yVariation: 2 }
            size: 160
        }

        Emitter {
            id: cloudRight
            width: 160
            height: 160
            anchors.left: parent.right
            emitRate: 0.5
            lifeSpan: 12000
            velocity: PointDirection{ x: -64; xVariation: 2; yVariation: 2 }
            size: 160
        }
    }


    Text {
        visible: gameOverScreen.gameCanvas != undefined
        text: "You saved " + gameOverScreen.gameCanvas.score + " fishes!"
        anchors.top: img.bottom
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        font.bold: true
        color: "#000000"
        opacity: 0.5
    }

    Image {
        source: "gfx/button-play.png"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        MouseArea {
            anchors.fill: parent
            onClicked: gameOverScreen.gameCanvas.gameOver = false//This will actually trigger the state change in Main.qml
        }
    }
}
