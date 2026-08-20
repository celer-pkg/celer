// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Qt Quick 3D Physics - Simple")

    //! [world]
    PhysicsWorld {
        scene: viewport.scene
    }
    //! [world]

    //! [scene]
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
            castsShadow: true
            brightness: 1
            shadowFactor: 50
        }

        //! [plane]
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
        //! [plane]

        //! [box]
        DynamicRigidBody {
            position: Qt.vector3d(-100, 100, 0)
            collisionShapes: BoxShape {
                id: boxShape
            }
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }
        //! [box]

        //! [sphere]
        DynamicRigidBody {
            position: Qt.vector3d(0, 100, 0)
            collisionShapes: SphereShape {
                id: sphereShape
            }
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
        //! [sphere]

        //! [capsule]
        DynamicRigidBody {
            position: Qt.vector3d(75, 200, 0)
            collisionShapes: CapsuleShape {
                id: capsuleShape
            }

            Model {
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }
        //! [capsule]
    }
    //! [scene]
}
