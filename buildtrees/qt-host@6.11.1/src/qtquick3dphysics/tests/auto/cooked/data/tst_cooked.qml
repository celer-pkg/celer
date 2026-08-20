// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// This test simply loads three cooked meshes and drops a ball on each mesh.
// If the balls successfully trigger a collision then the test passes.

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
            collisionShapes: HeightFieldShape {
                source: "qrc:/data/hf.cooked.hf"
                extents: "8, 2, 4"
            }
            sendContactReports: true
        }

        StaticRigidBody {
            position: Qt.vector3d(-6, 0, 0)
            collisionShapes: TriangleMeshShape {
                source: "qrc:/data/tetrahedron.cooked.tri"
            }
            sendContactReports: true
        }

        StaticRigidBody {
            position: Qt.vector3d(6, 0, 0)
            collisionShapes: ConvexMeshShape {
                source: "qrc:/data/tetrahedron.cooked.cvx"
            }
            sendContactReports: true
        }

        // Heightfield ball
        DynamicRigidBody {
            position: Qt.vector3d(2, 6, 0)
            id: hfBall
            collisionShapes: SphereShape {
                diameter: 1
            }
            property bool collided: false
            receiveContactReports: true
            onBodyContact: (body, positions, impulses, normals) => {
                collided = true;
            }
        }

        // Convex ball
        DynamicRigidBody {
            position: Qt.vector3d(-6, 6, 0)
            id: cvxBall
            collisionShapes: SphereShape {
                diameter: 1
            }
            property bool collided: false
            receiveContactReports: true
            onBodyContact: (body, positions, impulses, normals) => {
                collided = true;
            }
        }

        // Triangle ball
        DynamicRigidBody {
            position: Qt.vector3d(6, 6, 0)
            id: triBall
            collisionShapes: SphereShape {
                diameter: 1
            }
            property bool collided: false
            receiveContactReports: true
            onBodyContact: (body, positions, impulses, normals) => {
                collided = true;
            }
        }

        TestCase {
            name: "Heightfield"
            when: hfBall.collided
            function triggered() {}
        }

        TestCase {
            name: "Triangle mesh"
            when: triBall.collided
            function triggered() {}
        }

        TestCase {
            name: "Convex mesh"
            when: cvxBall.collided
            function triggered() {}
        }
    }
}
