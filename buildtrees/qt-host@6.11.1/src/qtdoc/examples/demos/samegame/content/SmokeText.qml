// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Particles

Item {
    id: smokeText
    z: 10
    property alias source: img.source
    property alias system: emitter.system
    property int playerNum: 1
    function play() {
        anim.running = true;
    }
    anchors.centerIn: parent
    Image {
        opacity: 0
        id: img
        anchors.centerIn: parent
        rotation: smokeText.playerNum == 1 ? -8 : -5
        Emitter {
            id: emitter
            group: "smoke"
            anchors.fill: parent
            shape: MaskShape { source: img.source }
            enabled: false
            emitRate: 1000
            lifeSpan: 600
            size: 64
            endSize: 32
            velocity: AngleDirection { angleVariation: 360; magnitudeVariation: 160 }
        }
    }
    SequentialAnimation {
        id: anim
        running: false
        PauseAnimation { duration: 500}
        ParallelAnimation {
            NumberAnimation { target: img; property: "opacity"; from: 0.1; to: 1.0 }
            NumberAnimation { target: img; property: "scale"; from: 0.1; to: 1.0 }
        }
        PauseAnimation { duration: 250}
        ScriptAction { script: emitter.pulse(100); }
        NumberAnimation { target: img; property: "opacity"; from: 1.0; to: 0.0 }
    }
}
