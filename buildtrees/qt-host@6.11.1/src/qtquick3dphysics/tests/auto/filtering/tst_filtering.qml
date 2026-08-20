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
        scene: sceneA.scene
        minimumTimestep: 15
        maximumTimestep: 15
    }

    PhysicsWorld {
        scene: sceneB.scene
        minimumTimestep: 15
        maximumTimestep: 15
    }

    PhysicsWorld {
        scene: sceneC.scene
        minimumTimestep: 15
        maximumTimestep: 15
    }

    PhysicsWorld {
        scene: sceneD.scene
        minimumTimestep: 15
        maximumTimestep: 15
    }

    BoxesScene {
        width: parent.width/2
        height: parent.height/2
        id: sceneA
    }

    BoxesScene {
        width: parent.width/2
        height: parent.height/2
        x: parent.width/2
        id: sceneB
        filterIgnoreGroups: [1]
    }

    BoxesScene {
        width: parent.width/2
        height: parent.height/2
        y: parent.height/2
        id: sceneC
        filterIgnoreGroups: [1,2]
    }

    BoxesScene {
        width: parent.width/2
        height: parent.height/2
        x: parent.width/2
        y: parent.height/2
        id: sceneD
        filterIgnoreGroups: [1,2,3]
    }

    TestCase {
        name: "sceneA"
        when: sceneA.numBouncesTop >= 3
        function empty() {}
    }

    TestCase {
        name: "sceneB"
        when: sceneB.numBouncesMiddle >= 3
        function empty() {}
    }

    TestCase {
        name: "sceneC"
        when: sceneC.numBouncesBottom >= 3
        function empty() {}
    }

    TestCase {
        name: "sceneD"
        when: sceneD.numBouncesFloor >= 3
        function empty() {}
    }
}
