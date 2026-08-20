// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

// This is the first screen.
// It shows the logo and emit a startButtonClicked signal
// when the user press the "PLAY" button.

Item {
    id: newGameScreen
    width: 320
    height: 480

    signal startButtonClicked

    Image {
        source: "gfx/logo.png"
        anchors.top: parent.top
        anchors.topMargin: 60
    }

    Image {
        source: "gfx/logo-fish.png"
        anchors.top: parent.top

        SequentialAnimation on x {
            loops: Animation.Infinite
            NumberAnimation { from: newGameScreen.x + 148; to: newGameScreen.x + 25; duration: 2000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: newGameScreen.x + 25; to: newGameScreen.x + 148; duration: 1600; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on anchors.topMargin {
            loops: Animation.Infinite
            NumberAnimation { from: 100; to: 60; duration: 1600; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 60; to: 100; duration: 2000; easing.type: Easing.InOutQuad }
        }
    }

    Image {
        source: "gfx/logo-bubble.png"
        anchors.top: parent.top

        SequentialAnimation on x {
            loops: Animation.Infinite
            NumberAnimation { from: newGameScreen.x + 140; to: newGameScreen.x + 40; duration: 2000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: newGameScreen.x + 40; to: newGameScreen.x + 140; duration: 1600; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on anchors.topMargin {
            loops: Animation.Infinite
            NumberAnimation { from: 100; to: 60; duration: 1600; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 60; to: 100; duration: 2000; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on width {
            loops: Animation.Infinite
            NumberAnimation { from: 140; to: 160; duration: 1000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 160; to: 140; duration: 800; easing.type: Easing.InOutQuad }
        }
        SequentialAnimation on height {
            loops: Animation.Infinite
            NumberAnimation { from: 150; to: 140; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 140; to: 150; duration: 1000; easing.type: Easing.InOutQuad }
        }
    }

    Image {
        source: "gfx/button-play.png"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 60
        MouseArea {
            anchors.fill: parent
            onClicked: newGameScreen.startButtonClicked()
        }
    }
}
