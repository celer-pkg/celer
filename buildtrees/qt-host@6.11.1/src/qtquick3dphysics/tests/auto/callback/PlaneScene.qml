// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Physics

View3D {
    id: viewport
    property real elapsedTime: 0
    property bool reportKinematicKinematicCollisions: false
    property bool reportStaticKinematicCollisions: false

    environment: SceneEnvironment {
        clearColor: "#d6dbdf"
        backgroundMode: SceneEnvironment.Color
    }

    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 1000)
        clipFar: 2000
        clipNear: 1
    }

    DirectionalLight {
        eulerRotation.x: -45
        eulerRotation.y: 45
        castsShadow: false
        brightness: 1
        shadowFactor: 100
    }

    PhysicsWorld {
        id: world
        scene: viewport.scene
        //maximumTimestep: 0.1
        //minimumTimestep: 0.1
        forceDebugDraw: true
        enableCCD: true
        reportKinematicKinematicCollisions: viewport.reportKinematicKinematicCollisions
        reportStaticKinematicCollisions: viewport.reportStaticKinematicCollisions
    }

    Connections {
        target: world
        function onFrameDone(timeStep) {
            viewport.elapsedTime += timeStep
        }
    }

    StaticRigidBody {
        position: Qt.vector3d(0, -100, 0)
        eulerRotation: Qt.vector3d(-90, 0, 0)
        collisionShapes: PlaneShape {}
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(500, 500, 0)
            materials: PrincipledMaterial {
                baseColor: "green"
            }
            castsShadows: false
            receivesShadows: true
        }
    }
}
