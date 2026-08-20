// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import Geometry

// This test tries repeatedly adding and removing a geometry from a triangle mesh.
// It should fallback to the source property.

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
            sendContactReports: true

            position: Qt.vector3d(-50, 300, 0)
            collisionShapes: ConvexMeshShape {
                geometry: ExampleTriangleGeometry { }
            }
            Model {
                geometry: ExampleTriangleGeometry { }
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }

            onPositionChanged: frameAnimation.stepFrame()
        }

        StaticRigidBody {
            id: staticBox
            sendContactReports: true
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(1, 1, 1)
            collisionShapes: TriangleMeshShape {
                id: staticBoxShape
                geometry: ExampleTriangleGeometry { }
                source: "qrc:/data/tetrahedron.cooked.tri"
            }
        }
    }

    QtObject {
        id: frameAnimation
        property int frame: -1
        function stepFrame() {
            frame += 1;
            let step = frame % 2;
            if (step == 0) {
                staticBox.scale = Qt.vector3d(100, 100, 100)
                staticBoxShape.geometry = null
            } else if (step == 1) {
                staticBox.scale = Qt.vector3d(1, 1, 1)
                let shape = Qt.createQmlObject(`
                    import QtQuick
                    import QtQuick3D
                    import QtQuick3D.Helpers
                    import QtQuick3D.Physics
                    import Geometry
                    ExampleTriangleGeometry { }
                    `,
                    frameAnimation,
                    "myGeometry"
                );
                staticBoxShape.geometry = shape
            }
        }
    }

    TestCase {
        name: "scene"
        when: dynamicBox.hit
        function triggered() {  }
    }
}
