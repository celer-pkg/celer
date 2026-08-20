// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import TileGeometry

// Drops two balls on two tiles. One tile with a hole and one without.
// These tiles' geometries are updated and we check that it is done
// correctly by checking that the balls hit the correct objects.

Item {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        scene: viewport.scene
        minimumTimestep: 15
        maximumTimestep: 15
        forceDebugDraw: false
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            position: Qt.vector3d(-200, 100, 200)
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
            id: ballLeft
            isKinematic: false
            position: Qt.vector3d(-200, 100, 0)
            scale: Qt.vector3d(0.3, 0.3, 0.3)
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
            collisionShapes: SphereShape {}
            sendContactReports: true
        }

        DynamicRigidBody {
            id: ballRight
            isKinematic: false
            position: Qt.vector3d(0, 100, 0)
            scale: Qt.vector3d(0.3, 0.3, 0.3)
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
            collisionShapes: SphereShape {}
            sendContactReports: true
        }

        StaticRigidBody {
            position: Qt.vector3d(-200, 0, 0)
            id: tileLeft
            Model {
                geometry: TileGeometry {
                    id: tileLeftGeometry
                    hasHole: false
                }
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }

            collisionShapes: TriangleMeshShape {
                geometry: tileLeftGeometry
            }

            receiveContactReports: true
            property bool ballHit: false
            onBodyContact: (body, positions, impulses, normals) => {
                if (body === ballLeft) {
                    ballHit = true;
                }
            }
        }

        StaticRigidBody {
            id: tileRight
            Model {
                geometry: TileGeometry {
                    id: tileRightGeometry
                    hasHole: true
                }
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
            collisionShapes: TriangleMeshShape {
                geometry: tileRightGeometry
            }

            receiveContactReports: true
            property bool ballHit: false
            onBodyContact: (body, positions, impulses, normals) => {
                if (body === ballRight) {
                    ballHit = true;
                }
            }
        }

        StaticRigidBody {
            id: floor
            position: Qt.vector3d(0, -100, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(10, 10, 1)
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }

            receiveContactReports: true
            property bool ballHit: false
            onBodyContact: (body, positions, impulses, normals) => {
                if (body === ballLeft) {
                    ballHit = true;
                }
            }
        }
    }


    Timer {
        interval: 100; running: true; repeat: false
        onTriggered: {
            tileLeftGeometry.hasHole = !tileLeftGeometry.hasHole;
            tileRightGeometry.hasHole = !tileRightGeometry.hasHole;
        }
    }

    TestCase {
        name: "scene"
        when: !tileLeft.ballHit && tileRight.ballHit && floor.ballHit
        function triggered() {  }
    }
}
