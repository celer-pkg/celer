// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 800
    height: 600
    visible: true

    PhysicsWorld {
        id: physicsWorld
        running: true
        forceDebugDraw: true
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        focus: true
        property bool debugView: false

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
            isKinematic: false
            position: Qt.vector3d(-200, 0, 0)
            collisionShapes: SphereShape {}

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        DynamicRigidBody {
            id: boxRoot
            isKinematic: true
            kinematicPosition: Qt.vector3d(-50, 50, 0)
            collisionShapes: BoxShape {}

            SequentialAnimation {
                 running: physicsWorld.running
                 loops: Animation.Infinite;
                 NumberAnimation {target: boxRoot; property: "kinematicEulerRotation.x"; from: 0; to:  360; duration: 10000 }
                 NumberAnimation {target: boxRoot; property: "kinematicEulerRotation.x"; from:  360; to: 0; duration: 10000 }
            }

            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }


            DynamicRigidBody {
                isKinematic: true
                kinematicPosition: Qt.vector3d(-50, 50, 0)
                position: Qt.vector3d(-50,50,0)
                collisionShapes: BoxShape {}

                Model {
                    source: "#Cube"
                    materials: PrincipledMaterial {
                        baseColor: "yellow"
                    }
                }

                DynamicRigidBody {
                    id: boxChild
                    isKinematic: true
                    kinematicPosition: Qt.vector3d(-50, 50, 0)
                    collisionShapes: BoxShape {}

                    SequentialAnimation {
                         running: physicsWorld.running
                         loops: Animation.Infinite;
                         NumberAnimation {target: boxChild; property: "kinematicEulerRotation.x"; from: 0; to:  360; duration: 10000 }
                         NumberAnimation {target: boxChild; property: "kinematicEulerRotation.x"; from:  360; to: 0; duration: 10000 }
                    }

                    Model {
                        source: "#Cube"
                        materials: PrincipledMaterial {
                            baseColor: "yellow"
                        }
                    }
                }
            }
        }
    }
}
