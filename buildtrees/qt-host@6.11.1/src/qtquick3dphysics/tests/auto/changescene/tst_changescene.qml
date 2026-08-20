// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtCore
import QtTest
import QtQuick3D
import QtQuick3D.Physics
import QtQuick

Item {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        id: world
        scene: sceneA.scene
    }

    ImpellerScene {
        width: parent.width/2
        height: parent.height/2
        id: sceneA
    }

    ImpellerScene {
        width: parent.width/2
        height: parent.height/2
        x: parent.width/2
        id: sceneB
    }

    ImpellerScene {
        width: parent.width/2
        height: parent.height/2
        y: parent.height/2
        id: sceneC
    }

    ImpellerScene {
        width: parent.width/2
        height: parent.height/2
        x: parent.width/2
        y: parent.height/2
        id: sceneD
    }

    TestCase {
        name: "sceneA"
        when: sceneA.numBounces == 1
        function test_next() { next() }
    }

    TestCase {
        name: "sceneB"
        when: sceneB.numBounces > 0
        function test_next() { next() }
    }

    TestCase {
        name: "sceneA2"
        when: sceneA.numBounces == 5
        function test_next() { next() }
    }

    TestCase {
        name: "sceneB2"
        when: sceneB.numBounces == 5
        function test_next() { next() }
    }

    TestCase {
        name: "sceneC"
        when: sceneC.numBounces == 1
        function test_next() { next() }
    }

    TestCase {
        name: "sceneD"
        when: sceneD.numBounces == 1
        function empty() {}
    }

    property int step: 0
    function next() {
        step += 1;
        if (step === 1) {
            world.scene = sceneB.scene
        } else if (step === 2) {
            world.scene = sceneA.scene
        } else if (step === 3) {
            world.scene = sceneB.scene
        } else if (step === 4) {
            world.scene = sceneC.scene
        } else if (step === 5) {
            world.scene = sceneD.scene
        }
    }
}
