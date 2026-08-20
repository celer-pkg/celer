// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Particles

Item {
    id: block
    property bool dying: false
    property bool spawned: false
    property int type: 0
    property ParticleSystem particleSystem

    Behavior on x {
        enabled: block.spawned;
        SpringAnimation{ spring: 2; damping: 0.2 }
    }
    Behavior on y {
        SpringAnimation{ spring: 2; damping: 0.2 }
    }

    Image {
        id: img
        source: {
            if (block.type == 0){
                "gfx/red.png";
            } else if (block.type == 1) {
                "gfx/blue.png";
            } else if (block.type == 2) {
                "gfx/green.png";
            } else {
                "gfx/yellow.png";
            }
        }
        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
        anchors.fill: parent
    }

    //Foreground particles
    BlockEmitter {
        id: particles
        system: block.particleSystem
        group: {
            if (block.type == 0){
                "red";
            } else if (block.type == 1) {
                "blue";
            } else if (block.type == 2) {
                "green";
            } else {
                "yellow";
            }
        }
        anchors.fill: parent
    }

    //Paint particles on the background
    PaintEmitter {
        id: particles2
        system: block.particleSystem
    }

    states: [
        State {
            name: "AliveState"; when: block.spawned == true && block.dying == false
            PropertyChanges { img.opacity: 1 }
        },

        State {
            name: "DeathState"; when: block.dying == true
            StateChangeScript { script: {block.particleSystem.paused = false; particles.pulse(100); particles2.pulse(100);} }
            PropertyChanges { img.opacity: 0 }
            StateChangeScript { script: block.destroy(1000); }
        }
    ]
}
