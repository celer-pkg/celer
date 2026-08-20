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
        gravity: Qt.vector3d(0, -9.81, 0)
        running: true
        forceDebugDraw: true
        typicalLength: 1
        typicalSpeed: 10
        minimumTimestep: 15
        maximumTimestep: 15
        scene: viewport.scene
        property real elapsedTime: 0
    }

    Connections {
        target: world
        function onFrameDone(timeStep) {
            world.elapsedTime += timeStep
        }
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


        Node {
            x: -5

            TriggerBody {
                position: Qt.vector3d(0, 3, 0)
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(1, 2, 1)
                }

                onBodyEntered: (body) => {
                        body.triggered = true;
                        boxCentre.simulationEnabled = false;
                        boxCentreLow.simulationEnabled = true;
                        collisionSphereLeft.simulationEnabled = false;
                        boxRight.simulationEnabled = true;
                }
            }

            DynamicRigidBody {
                id: collisionSphereLeft
                massMode: DynamicRigidBody.CustomDensity
                density: 1000
                position: Qt.vector3d(0, 6, 0)
                sendTriggerReports: true
                simulationEnabled: false
                property bool triggered: false

                collisionShapes: SphereShape {
                    diameter: 1
                }
                Model {
                    source: "#Sphere"
                    materials: PrincipledMaterial {
                        baseColor: collisionSphereLeft.triggered ? "yellow" : "red"
                    }
                    scale: Qt.vector3d(0.01, 0.01, 0.01)
                }
            }
        }


        Node {
            x: 0
            DynamicRigidBody {
                id: boxCentreLow
                isKinematic: true
                simulationEnabled: false
                position: Qt.vector3d(0, -1, 0)
                kinematicPosition: position
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
                receiveContactReports: true
                sendContactReports: true
                property bool hasCollided: false

                onBodyContact: (body, positions, impulses, normals) => {
                    hasCollided = true
                }
            }

            TriggerBody {
                position: Qt.vector3d(0, 3, 0)
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(1, 2, 1)
                }

                onBodyEntered: (body) => {
                    body.triggered = true;
                    collisionSphereRight.simulationEnabled = true;
                }
            }

            StaticRigidBody {
                id: boxCentre
                position: Qt.vector3d(0, 4.5, 0)
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(2,1,2)
                }
                Model {
                    source: "#Cube"
                    scale: Qt.vector3d(2, 1, 2).times(0.01)
                    eulerRotation: Qt.vector3d(0, 90, 0)
                    materials: PrincipledMaterial {
                        baseColor: "blue"
                    }
                }
            }

            DynamicRigidBody {
                id: collisionSphereCenter
                massMode: DynamicRigidBody.CustomDensity
                density: 1000
                position: Qt.vector3d(0, 6, 0)
                sendTriggerReports: true
                sendContactReports: true
                property bool triggered: false

                collisionShapes: SphereShape {
                    diameter: 1
                }
                Model {
                    source: "#Sphere"
                    materials: PrincipledMaterial {
                        baseColor: collisionSphereCenter.triggered ? "yellow" : "red"
                    }
                    scale: Qt.vector3d(0.01, 0.01, 0.01)
                }
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
                        body.triggered = true;
                        collisionSphereLeft.simulationEnabled = true;
                        collisionSphereRight.simulationEnabled = false;
                }
            }

            DynamicRigidBody {
                id: collisionSphereRight
                massMode: DynamicRigidBody.CustomDensity
                density: 1000
                position: Qt.vector3d(0, 6, 0)
                sendTriggerReports: true
                property bool triggered: false

                collisionShapes: SphereShape {
                    id: sphereShape
                    diameter: 1
                }
                Model {
                    source: "#Sphere"
                    materials: PrincipledMaterial {
                        baseColor: collisionSphereRight.triggered ? "yellow" : "red"
                    }
                    scale: Qt.vector3d(0.01, 0.01, 0.01)
                }
            }

            StaticRigidBody {
                id: boxRight
                simulationEnabled: false
                position: Qt.vector3d(0, 0, 0)
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(2,1,2)
                }
                Model {
                    source: "#Cube"
                    scale: Qt.vector3d(2, 1, 2).times(0.01)
                    eulerRotation: Qt.vector3d(0, 90, 0)
                    materials: PrincipledMaterial {
                        baseColor: "blue"
                    }
                }
            }
        }
    }

    TestCase {
        name: "trigger right"
        when: collisionSphereRight.triggered
        function triggered() {
        }
    }

    TestCase {
        name: "trigger left"
        when: collisionSphereLeft.triggered
        function triggered() {
        }
    }

    TestCase {
        name: "trigger center"
        when: collisionSphereCenter.triggered
        function triggered() {
        }
    }

    TestCase {
        name: "trigger right box"
        when: collisionSphereCenter.triggered
        function triggered() {
        }
    }

    TestCase {
        name: "trigger center low box"
        when: boxCentreLow.hasCollided
        function triggered() {
        }
    }
}
