// Copyright (C) 2022 The Qt Company Ltd.
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
        gravity: Qt.vector3d(0, -9.81, 0)
        running: true
        forceDebugDraw: true
        typicalLength: 1
        typicalSpeed: 10
        minimumTimestep: 15
        maximumTimestep: 15
        scene: viewport.scene
    }
    property bool simulationActuallyRunning: false

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#151a3f"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: scene
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

            Node {
                id: fallingBoxNode
                StaticRigidBody {
                    position: Qt.vector3d(0, -1, 0)
                    collisionShapes: BoxShape {
                        extents: Qt.vector3d(2,1,2)
                    }
                    Model {
                        source: "#Cube"
                        scale: Qt.vector3d(2, 1, 2).times(0.01)
                        eulerRotation: Qt.vector3d(0, 90, 0)
                        materials: PrincipledMaterial {
                            baseColor: "green"
                        }
                    }
                }

                TestCube {
                    id: fallingBox
                    y: 1
                    color: "yellow"
                }
            }

            Node {
                id: triggerNode
                x: 5

                TriggerBody {
                    id: triggerBox
                    position: Qt.vector3d(0, 3, 0)
                    collisionShapes: BoxShape {
                        extents: Qt.vector3d(1, 2, 1)
                    }

                    onBodyEntered: (body) => {
                        if (body.hasOwnProperty('inArea'))
                            body.inArea = true;
                    }
                    onBodyExited: (body) => {
                        if (body.hasOwnProperty('inArea'))
                            body.inArea = false;
                    }
                }

                DynamicRigidBody {
                    id: collisionSphere
                    massMode: DynamicRigidBody.CustomDensity
                    density: 1000
                    position: Qt.vector3d(0, 6, 0)
                    sendTriggerReports: true
                    property bool inArea: false

                    collisionShapes: SphereShape {
                        id: sphereShape
                        diameter: 1
                    }
                    Model {
                        source: "#Sphere"
                        materials: PrincipledMaterial {
                            baseColor: collisionSphere.inArea ? "yellow" : "red"
                        }
                        scale: Qt.vector3d(0.01, 0.01, 0.01)
                    }
                }
            }

            Node {
                id: dynamicCreationNode
                x: -5
                StaticRigidBody {
                    position: Qt.vector3d(0, -1, 0)
                    collisionShapes: BoxShape {
                        extents: Qt.vector3d(2,1,2)
                    }
                    Model {
                        source: "#Cube"
                        scale: Qt.vector3d(2, 1, 2).times(0.01)
                        eulerRotation: Qt.vector3d(0, 90, 0)
                        materials: PrincipledMaterial {
                            baseColor: "red"
                        }
                    }
                }

                property int frame: 0;
                property var createdObject: null
                property bool createdObjectIsStable: createdObject ? createdObject.stable : false
                function checkStable() {
                    if (createdObject && frame > 10)
                        createdObject.checkStable()
                    frame += 1
                }
                Component {
                    id: spawnComponent
                    TestCube {
                      color: "cyan"
                      y: 1
                    }
                }

                TestCase {
                    name: "DynamicCreation"
                    when: dynamicCreationNode.createdObjectIsStable
                    function test_created_object_is_simulated() {
                        fuzzyCompare(dynamicCreationNode.createdObject.y, 0, 0.001)
                    }
                }
            }

            Node {
                id: frictionNode
                z: -10
                eulerRotation.z: 20
                PhysicsMaterial {
                    id: frictionMaterial
                    dynamicFriction: 0
                    staticFriction: 0
                }
                StaticRigidBody {
                    position: Qt.vector3d(0, -1, 0)
                    collisionShapes: BoxShape {
                        id: slide
                        extents: Qt.vector3d(10, 1, 2)
                    }
                    physicsMaterial: frictionMaterial

                    Model {
                        source: "#Cube"
                        scale: slide.extents.times(0.01)
                        materials: PrincipledMaterial {
                            baseColor: "orange"
                        }
                    }
                }
                TestCube {
                    id: slidingBox
                    x: 4
                    color: "red"
                    physicsMaterial: frictionMaterial
                    property real brakePoint: -9999
                }
                TestCase {
                    name: "Friction"
                    when: timer.simulationSteps > 20
                    function test_low_friction() {
                        verify(!slidingBox.stable)
                        verify(slidingBox.x < 4)
                    }
                    function test_set_high_friction() {
                        frictionMaterial.dynamicFriction = 0.5
                        frictionMaterial.staticFriction = 0.5
                        slidingBox.brakePoint = slidingBox.x
                    }
                }
                TestCase {
                    name: "Friction2"
                    when: slidingBox.stable
                    function test_high_friction() {
                        fuzzyCompare(slidingBox.y, 0, 0.001)
                        verify(slidingBox.x < slidingBox.brakePoint)
                    }
                }
            }

            Node {
                id: heightFieldNode
                z: -5
                x: -5
                StaticRigidBody {
                    position: "0, 0, 0"
                    collisionShapes: HeightFieldShape {
                        source: "qrc:/data/hf.png"
                        extents: "8, 2, 4"
                    }
                    physicsMaterial: frictionMaterial
                }

                DynamicRigidBody {
                    id: hfBall
                    massMode: DynamicRigidBody.CustomDensity
                    density: 1000
                    scale: "0.5, 0.5, 0.5"
                    position: Qt.vector3d(-3, 1, 0)

                    collisionShapes: SphereShape {
                        diameter: 1
                    }
                    Model {
                        source: "#Sphere"
                        materials: PrincipledMaterial {
                            baseColor: "yellow"
                        }
                        scale: Qt.vector3d(0.01, 0.01, 0.01)
                    }
                    property bool stable: false
                    property vector3d prevPos: "9999,9999,9999"
                    property vector3d prevPos2: "9999,9999,9999"
                    function fuzzyEquals(a, b) {
                        return Math.abs(a.x - b.x) < 0.01 && Math.abs(a.y - b.y) < 0.001 && Math.abs(a.z - b.z) < 0.01
                    }
                    function checkStable() {
                        // Compare against the previous two positions to avoid triggering on the top of a bounce
                        if (fuzzyEquals(position, prevPos) && fuzzyEquals(position, prevPos2)) {
                            stable = true
                        } else {
                            prevPos2 = prevPos
                            prevPos = position
                        }
                    }
                }

                TestCase {
                    name: "HeightField"
                    when: hfBall.stable
                    function test_ball_position() {
                        fuzzyCompare(hfBall.x, 2, 0.5)
                        fuzzyCompare(hfBall.y, -0.75, 0.1)
                        fuzzyCompare(hfBall.z, 0, 0.5)
                    }
                }
            }


            // Trick to get a callback when the physics simulation advances, no matter how slowly the CI machine is running
            DynamicRigidBody {
                id: timer
                position: "-10, 100, -10"
                collisionShapes: SphereShape {
                    diameter: 2
                }
                property int simulationSteps: 0
                onPositionChanged: {
                    // Only do the check(s) every other step, since we have no guarantees on signal emission order
                    if (simulationSteps % 2) {
                        fallingBox.checkStable()
                        dynamicCreationNode.checkStable()
                        slidingBox.checkStable()
                        hfBall.checkStable()
                        simulationActuallyRunning = true
                    }
                    simulationSteps++
                }
            }
        }
    }

    TestCase {
        name: "SceneStart"
        when: simulationActuallyRunning
        function test_collision_sphere() {
            verify(!collisionSphere.inArea)
            compare(triggerEnterSpy.count, 0)
            compare(triggerExitSpy.count, 0)
        }
        function test_falling_box() {
            verify(fallingBox.y > 0)
            compare(fallingBox.isSleeping, false)
        }
        function test_spawn() {
            var obj = spawnComponent.createObject(dynamicCreationNode)
            verify(obj.y === 1)
            dynamicCreationNode.createdObject = obj
        }
    }

    TestCase {
        name: "FallingBox"
        when: fallingBox.stable
        function test_stable_position() {
            fuzzyCompare(fallingBox.y, 0, 0.0001)
            fuzzyCompare(fallingBox.eulerRotation.x, 0, 1)
            fuzzyCompare(fallingBox.eulerRotation.z, 0, 1)
            compare(fallingBox.isSleeping, true)
        }
    }

    TestCase {
        name: "TriggerBody"
        when: collisionSphere.y < 0
        function test_enter_exit() {
            compare(triggerEnterSpy.count, 1)
            compare(triggerExitSpy.count, 1)
        }
        SignalSpy {
            id: triggerEnterSpy
            target: triggerBox
            signalName: "onBodyEntered"
        }
        SignalSpy {
            id: triggerExitSpy
            target: triggerBox
            signalName: "onBodyExited"
        }
    }
}
