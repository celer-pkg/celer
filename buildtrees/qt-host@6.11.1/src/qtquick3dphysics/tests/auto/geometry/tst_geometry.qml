// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

import Geometry

Item {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        scene: viewport.scene
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
            position: Qt.vector3d(-200, 300, 500)
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

        StaticRigidBody {
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
        }

        DynamicRigidBody {
            id: dynamicBox
            property bool hit : false
            onBodyContact: () => {
                dynamicBox.hit = true
            }
            receiveContactReports: true
            sendContactReports: false

            position: Qt.vector3d(-50, 400, 0)
            collisionShapes: ConvexMeshShape {
                geometry: ExampleTriangleGeometry { }
            }
            Model {
                geometry: ExampleTriangleGeometry { }
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        StaticRigidBody {
            sendContactReports: true
            position: Qt.vector3d(-175, 250, 0)
            collisionShapes: TriangleMeshShape {
                geometry: CapsuleGeometry {
                    enableNormals: true
                    enableUV: true
                }
            }
            Model {
                geometry: CapsuleGeometry { }
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        DynamicRigidBody {
            id: dynamicCapsule
            property bool hit : false
            onBodyContact: () => {
                dynamicCapsule.hit = true
            }
            receiveContactReports: true
            sendContactReports: false

            position: Qt.vector3d(175, 400, 0)
            collisionShapes: ConvexMeshShape {
                geometry: CapsuleGeometry {
                    enableNormals: true
                    enableUV: true
                }
            }
            Model {
                geometry: CapsuleGeometry { }
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        StaticRigidBody {
            sendContactReports: true
            position: Qt.vector3d(275, 250, 0)
            collisionShapes: TriangleMeshShape {
                geometry: ExampleTriangleGeometry {}
            }
            Model {
                geometry: ExampleTriangleGeometry { }
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }
    }

    TestCase {
        name: "box hit"
        when: dynamicBox.hit
        function triggered() {  }
    }

    TestCase {
        name: "capsule hit"
        when: dynamicCapsule.hit
        function triggered() {  }
    }
}
