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
        gravity: Qt.vector3d(0, -490, 0)
        scene: sceneA.scene
    }

    PhysicsWorld {
        gravity: Qt.vector3d(0, -10, 0)
        scene: sceneB.scene
    }

    PhysicsWorld {
        gravity: Qt.vector3d(0, -900, 0)
        scene: sceneC.scene
    }

    PhysicsWorld {
        gravity: Qt.vector3d(0, -1900, 0)
        scene: sceneD.scene
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
        when: sceneA.numBounces > 0
        function empty() {}
    }

    TestCase {
        name: "sceneB"
        when: sceneB.numBounces > 0
        function empty() {}
    }

    TestCase {
        name: "sceneC"
        when: sceneC.numBounces > 0
        function empty() {}
    }

    TestCase {
        name: "sceneD"
        when: sceneD.numBounces > 0
        function empty() {}
    }
}
