// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// A scene with a character controller getting resized until it triggers
// boxes and can move over an obstacle.

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

        StaticRigidBody {
            collisionShapes: BoxShape {
                extents: Qt.vector3d(2, 1, 4)
            }
            position: Qt.vector3d(0, 2, 0)
        }

        TriggerBody {
            id: triggerBoxUp
            position: Qt.vector3d(2, 5, 0)
            collisionShapes: BoxShape {
                extents: Qt.vector3d(2, 1, 2)
            }
            receiveTriggerReports: true
            property bool entered: false
            onBodyEntered: function(body) {
                entered = true
            }
        }

        TriggerBody {
            id: triggerBoxSide
            position: Qt.vector3d(3.5, 2, 0)
            collisionShapes: BoxShape {
                extents: Qt.vector3d(1, 1, 1)
            }
            receiveTriggerReports: true
            property bool entered: false
            onBodyEntered: function(body) {
                entered = true
            }
        }

        TriggerBody {
            id: triggerBoxStep
            position: Qt.vector3d(-6, 3, 0)
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
            id: character
            property bool collided: false
            position: Qt.vector3d(2, 2, 0)

            gravity: world.gravity
            onCollisionsChanged: {
               collided = true;
            }

            sendTriggerReports: true

            Connections {
                target: world
                property real totalAnimationTime: 5000
                function onFrameDone(timeStep) {
                    let progressStep = timeStep / totalAnimationTime
                    animationController.progress += progressStep
                    if (animationController.progress >= 1) {
                        animationController.completeToEnd()
                    }
                }
            }

            AnimationController {
                id: animationController
                animation: ParallelAnimation {
                    NumberAnimation {
                        target: capsuleShape
                        property: "height"
                        to: 2
                        from: 1
                        duration: 5000
                    }
                    NumberAnimation {
                        target: capsuleShape
                        property: "diameter"
                        to: 3
                        from: 1
                        duration: 3000
                    }
                }
            }

            collisionShapes:  CapsuleShape {
                id: capsuleShape
                height: 1
                diameter: 1
            }

            movement.x: -1;

            Model {
                eulerRotation.z: 90
                //scale: Qt.vector3d(0.01, 0.01, 0.01)
                geometry: CapsuleGeometry {
                    height: capsuleShape.height
                    diameter: capsuleShape.diameter
                }
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }

        TestCase {
            name: "character up"
            when: character.collisions === CharacterController.Down
            function triggered() {}
        }

        TestCase {
            name: "trigger box up"
            when: triggerBoxUp.entered
            function triggered() {}
        }

        TestCase {
            name: "trigger box side"
            when: triggerBoxSide.entered
            function triggered() {}
        }

        TestCase {
            name: "trigger box step"
            when: triggerBoxStep.entered
            function triggered() {}
        }
    }
}
