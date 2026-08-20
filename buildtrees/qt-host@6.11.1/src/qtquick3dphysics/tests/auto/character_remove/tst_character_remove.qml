// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Tests adding and removing character controller

import QtTest
import QtQuick3D
import QtQuick3D.Physics
import QtQuick

Item {
    width: 640
    height: 480
    visible: true

    function randomInRange(min, max) {
        return Math.random() * (max - min) + min
    }

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

        Timer {
            id: spawnTimer
            interval: 100
            running: true
            repeat: true
            onTriggered: {
                var pos = Qt.vector3d(randomInRange(-5, 5),
                                      randomInRange(3, 6),
                                      randomInRange(-1, 1))
                shapeSpawner.createCharacter(pos)
            }
        }

        Timer {
            id: resetTimer
            interval: 2000
            running: true
            repeat: true
            property int repeats: 0
            onTriggered: {
                shapeSpawner.reset()
                repeats += 1
            }
        }

        Node {
            id: shapeSpawner
            property var instancesCharacters: []
            property var characterComponent: Qt.createComponent("Character.qml")

            function createCharacter(position) {
                let settings = {
                    "position": position,
                    "gravity": world.gravity
                }
                let character = characterComponent.createObject(shapeSpawner, settings)

                if (character === null) {
                    console.log("Error creating object")
                    return
                }
                instancesCharacters.push(character)
            }

            function reset() {
                instancesCharacters.forEach(character => { character.destroy() })
                instancesCharacters = []
            }
        }

        TestCase {
            name: "trigger box"
            when: resetTimer.repeats > 5
            function triggered() {}
        }
    }
}
