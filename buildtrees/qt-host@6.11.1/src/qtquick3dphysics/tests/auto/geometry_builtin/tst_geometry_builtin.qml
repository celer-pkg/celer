// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Physics

Item {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        scene: viewport.scene
        minimumTimestep: 15
        maximumTimestep: 15
        forceDebugDraw: true
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            position: Qt.vector3d(-200, 100, 500)
            eulerRotation: Qt.vector3d(-20, -20, 0)
            clipFar: 5000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: false
            brightness: 1
            shadowFactor: 100
        }

        DynamicRigidBody {
            id: cube
            property bool hit: false
            position: Qt.vector3d(-100, 100, 0)
            receiveContactReports: true
            onBodyContact: (body, positions, impulses, normals) => hit = true
            collisionShapes: ConvexMeshShape {
                source: "#Cube"
            }
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        DynamicRigidBody {
            id: sphere
            property bool hit: false
            position: Qt.vector3d(0, 100, 0)
            receiveContactReports: true
            onBodyContact: (body, positions, impulses, normals) => hit = true
            collisionShapes: ConvexMeshShape {
                source: "#Sphere"
            }
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        DynamicRigidBody {
            id: cone
            property bool hit: false
            position: Qt.vector3d(100, 100, 0)
            receiveContactReports: true
            onBodyContact: (body, positions, impulses, normals) => hit = true
            collisionShapes: ConvexMeshShape {
                source: "#Cone"
            }
            Model {
                source: "#Cone"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        StaticRigidBody {
            position: Qt.vector3d(0, -100, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            sendContactReports: true
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(10, 10, 1)
                materials: DefaultMaterial {
                    diffuseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
    }

    TestCase {
        name: "scene"
        when: cube.hit
        function triggered() {  }
    }

    TestCase {
        name: "scene"
        when: sphere.hit
        function triggered() {  }
    }

    TestCase {
        name: "scene"
        when: cone.hit
        function triggered() {  }
    }
}
