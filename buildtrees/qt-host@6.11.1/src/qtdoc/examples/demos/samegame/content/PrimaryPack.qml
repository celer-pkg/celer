// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


import QtQuick
import QtQuick.Particles

Item {
    id: container
    property ParticleSystem particleSystem
    property GameArea gameArea
    onGameAreaChanged: bgstacker.parent = gameArea;//Move to direct child of game canvas
    Item {
        id: bgstacker
        z: 0
        ImageParticle {
            groups: ["yellowspots"]
            color: Qt.darker("yellow");//Actually want desaturated...
            system: container.particleSystem
            source: "gfx/particle-paint.png"
            colorVariation: 0.2
            alpha: 0.2
            entryEffect: ImageParticle.None
        }
        ImageParticle {
            groups: ["redspots"]
            system: container.particleSystem
            color: Qt.darker("red");//Actually want desaturated...
            source: "gfx/particle-paint.png"
            colorVariation: 0.2
            alpha: 0.2
            entryEffect: ImageParticle.None
        }
        ImageParticle {
            groups: ["greenspots"]
            system: container.particleSystem
            color: Qt.darker("green");//Actually want desaturated...
            source: "gfx/particle-paint.png"
            colorVariation: 0.2
            alpha: 0.2
            entryEffect: ImageParticle.None
        }
        ImageParticle {
            groups: ["bluespots"]
            system: container.particleSystem
            color: Qt.darker("blue");//Actually want desaturated...
            source: "gfx/particle-paint.png"
            colorVariation: 0.2
            alpha: 0.2
            entryEffect: ImageParticle.None
        }
    }
    ImageParticle {
        groups: ["yellow"]
        system: container.particleSystem
        color: Qt.darker("yellow");//Actually want desaturated...
        source: "gfx/particle-brick.png"
        colorVariation: 0.4
        alpha: 0.1
    }
    ImageParticle {
        groups: ["red"]
        system: container.particleSystem
        color: Qt.darker("red");//Actually want desaturated...
        source: "gfx/particle-brick.png"
        colorVariation: 0.4
        alpha: 0.1
    }
    ImageParticle {
        groups: ["green"]
        system: container.particleSystem
        color: Qt.darker("green");//Actually want desaturated...
        source: "gfx/particle-brick.png"
        colorVariation: 0.4
        alpha: 0.1
    }
    ImageParticle {
        groups: ["blue"]
        system: container.particleSystem
        color: Qt.darker("blue");//Actually want desaturated...
        source: "gfx/particle-brick.png"
        colorVariation: 0.4
        alpha: 0.1
    }
}
