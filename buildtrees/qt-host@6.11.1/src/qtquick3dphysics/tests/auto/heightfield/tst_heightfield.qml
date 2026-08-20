// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

// Test loading a heightfield from both 'source' and 'image' property

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
            id: dynamicBoxA
            property bool hit : false
            onBodyContact: () => {
                hit = true
            }
            receiveContactReports: true

            position: Qt.vector3d(-300, 300, 0)
            collisionShapes: BoxShape {}
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        StaticRigidBody {
            collisionShapes: HeightFieldShape {
                image: Image {
                    source: "qrc:/data/hf.png"
                }
                position: Qt.vector3d(-300, -300, 0)
                extents: "400, 200, 400"
            }
            sendContactReports: true
        }

        DynamicRigidBody {
            id: dynamicBoxB
            property bool hit : false
            onBodyContact: () => {
                hit = true
            }
            receiveContactReports: true

            position: Qt.vector3d(300, 300, 0)
            collisionShapes: BoxShape {}
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        StaticRigidBody {
            collisionShapes: HeightFieldShape {
                source: "qrc:/data/hf.png"
                position: Qt.vector3d(300, -300, 0)
                extents: "400, 200, 400"
            }
            sendContactReports: true
        }
    }

    TestCase {
        name: "Box A"
        when: dynamicBoxA.hit
        function triggered() {  }
    }

    TestCase {
        name: "Box B"
        when: dynamicBoxB.hit
        function triggered() {  }
    }
}
