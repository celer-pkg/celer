// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Qt Quick 3D Physics - Impeller")

    //! [world]
    PhysicsWorld {
        gravity: Qt.vector3d(0, -490, 0)
        scene: viewport.scene
    }
    //! [world]

    //! [scene]
    View3D {
        id: viewport
        anchors.fill: parent

        //! [environment]
        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 200, 1000)
            clipFar: 2000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1
            shadowFactor: 50
        }
        //! [environment]

        //! [plane]
        StaticRigidBody {
            position: Qt.vector3d(0, -100, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(500, 500, 1)
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
        //! [plane]

        //! [sphere]
        DynamicRigidBody {
            id: sphere
            massMode: DynamicRigidBody.CustomDensity
            density: 0.00001
            position: Qt.vector3d(0, 600, 0)
            property bool inArea: false
            sendContactReports: true
            receiveTriggerReports: true

            onEnteredTriggerBody: {
                inArea = true
            }
            onExitedTriggerBody: {
                inArea = false
            }

            collisionShapes: SphereShape {}
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: sphere.inArea ? "yellow" : "red"
                }
            }
        }
        //! [sphere]

        //! [box]
        TriggerBody {
            position: Qt.vector3d(0, 350, 0)
            scale: Qt.vector3d(1, 2, 1)

            collisionShapes: BoxShape {
                id: boxShape
            }
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: Qt.rgba(1, 0, 1, 0.2)
                    alphaMode: PrincipledMaterial.Blend
                }
            }
        }
        //! [box]

        //! [impeller]
        StaticRigidBody {
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(2, 2, 2)
            receiveContactReports: true

            collisionShapes: SphereShape {}

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }

            onBodyContact: (body, positions, impulses, normals) => {
                for (var normal of normals) {
                    let velocity = normal.times(-700)
                    body.setLinearVelocity(velocity)
                }
            }
        }
        //! [impeller]
    }
    //! [scene]
}
