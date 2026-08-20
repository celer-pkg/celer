// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Tests the character controller with a trigger box and a static body

import QtCore
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import QtQuick

Item {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        id: world
        gravity: Qt.vector3d(0, -9.82, 0)
        running: true
        forceDebugDraw: true
        typicalLength: 1
        typicalSpeed: 10
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#151a3f"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera1
            position: Qt.vector3d(0, 5, 10)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            clipFar: 50
            clipNear: 0.01
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
        }

        StaticRigidBody {
            collisionShapes: BoxShape {
                extents: Qt.vector3d(8, 2, 4)
            }
        }

        TriggerBody {
            id: triggerBox
            position: Qt.vector3d(2, 4, 0)
            collisionShapes: BoxShape {
                extents: Qt.vector3d(2, 1, 2)
            }
            receiveTriggerReports: true
            property bool entered: false
            onBodyEntered: function(body) {
                entered = true
            }
        }

        CharacterController {
            id: characterUp
            property bool collided: false
            position: Qt.vector3d(2, 8, 0)

            gravity: world.gravity
            onCollisionsChanged: {
               collided = true;
            }

            sendTriggerReports: true

            collisionShapes:  CapsuleShape {
                height: 1
                diameter: 1
            }

            Model {
                eulerRotation.z: 90
                scale: Qt.vector3d(0.01, 0.01, 0.01)
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }

        CharacterController {
            id: characterDown
            property bool collided: false
            position: Qt.vector3d(-2, -20, 0)
            gravity: Qt.vector3d(-world.gravity.x, -world.gravity.y, -world.gravity.z)

            onCollisionsChanged: {
               collided = true
            }

            sendTriggerReports: true

            collisionShapes:  CapsuleShape {
                height: 1
                diameter: 1
            }

            Model {
                eulerRotation.z: 90
                scale: Qt.vector3d(0.01, 0.01, 0.01)
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
            }
        }

        TestCase {
            name: "character up"
            when: characterUp.collisions === CharacterController.Down
            function triggered() {}
        }

        TestCase {
            name: "character down"
            when: characterDown.collisions === CharacterController.Up
            function triggered() {}
        }

        TestCase {
            name: "trigger box"
            when: triggerBox.entered
            function triggered() {}
        }
    }
}
