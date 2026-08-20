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
    title: qsTr("Qt Quick 3D Physics - Mass")

    //! [world]
    PhysicsWorld {
        running: true
        gravity: Qt.vector3d(0, -9.81, 0)
        typicalLength: 1
        typicalSpeed: 10
        scene: viewport.scene
    }
    //! [world]

    //! [scene]
    View3D {
        id: viewport
        anchors.fill: parent

        //! [environment]
        environment: SceneEnvironment {
            clearColor: "lightblue"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 2, 5)
            eulerRotation: Qt.vector3d(-10, 0, 0)
            clipFar: 100
            clipNear: 0.01
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1
            shadowFactor: 50
            shadowBias: 0.1
            pcfFactor: 0.01
        }
        //! [environment]

        //! [plane]
        StaticRigidBody {
            position: Qt.vector3d(0, 0, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
        //! [plane]

        //! [rolypolys]
        RolyPoly {
            position: Qt.vector3d(-2, 0.5, 0)
            color: "blue"

            mass: 0.9
            centerOfMassPosition: Qt.vector3d(0, -0.5, 0)
            inertiaTensor: Qt.vector3d(0.217011, 0.0735887, 0.217011)
            massMode: DynamicRigidBody.MassAndInertiaTensor
        }

        RolyPoly {
            position: Qt.vector3d(0, 0.5, 0)
            color: "purple"

            mass: 0.9
            centerOfMassPosition: Qt.vector3d(0, -0.5, 0)
            inertiaTensor: Qt.vector3d(0.05, 100, 100)
            massMode: DynamicRigidBody.MassAndInertiaTensor
        }

        RolyPoly {
            position: Qt.vector3d(2, 0.5, 0)
            color: "red"

            mass: 0.9
            massMode: DynamicRigidBody.Mass
        }
        //! [rolypolys]

        //! [spawner]
        Node {
            id: shapeSpawner
            property var spheres: []
            property var sphereComponent: Qt.createComponent("Sphere.qml")

            function createBall(position, forward) {
                let diameter = 0.2
                let speed = 20
                let sphere = sphereComponent.createObject(shapeSpawner, {
                                                              "position": position,
                                                              "sphereDiameter": diameter
                                                          })
                sphere.setLinearVelocity(forward.times(speed))

                var pair = {
                    "sphere": sphere,
                    "date": Date.now()
                }

                spheres.push(pair)

                if (sphere === null) {
                    console.log("Error creating object")
                }
            }

            function clean() {
                spheres = spheres.filter(sphere => {
                    let diff = Date.now() - sphere['date'];
                    if (diff > 5000) {
                        sphere['sphere'].destroy();
                        return false;
                    }
                    return true;
                });
            }

            Timer {
                interval: 200
                running: true
                repeat: true
                onTriggered: shapeSpawner.clean()
            }
        }
        //! [spawner]

    }
    //! [scene]


    //! [controller]
    WasdController {
        speed: 0.01
        shiftSpeed: 0.1
        controlledObject: camera
        Keys.onPressed: (event) => {
            handleKeyPress(event);
            if (event.key === Qt.Key_Space) {
                shapeSpawner.createBall(camera.position, camera.forward);
            }
        }
        Keys.onReleased: (event) => { handleKeyRelease(event) }
    }
    //! [controller]
}
