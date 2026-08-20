// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: appWindow
    width: 1280
    height: 720
    visible: true
    title: qsTr("PhysX Friction Test")

    PhysicsWorld {
        id: physicsWorld
        gravity: Qt.vector3d(0, -9.81, 0)
        running: false
        typicalLength: typicalLengthSlider.value
        typicalSpeed: typicalSpeedSlider.value
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
        }

        focus: true
        Keys.onSpacePressed: {
            console.log("toggle physics")
            physicsWorld.running = !physicsWorld.running
        }

        Node {
            id: scene1
            PerspectiveCamera {
                id: camera1
                z: 5
                x: -2
                y: 1
                eulerRotation : Qt.vector3d(-20, -20, 0)
                clipFar: 500
                clipNear: 0.01
            }

            DirectionalLight {
                eulerRotation: Qt.vector3d(-45, 45, 0)
                castsShadow: true
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                shadowBias: 0.05
                pcfFactor: 0.005
            }


            PhysicsMaterial {
                id: testMaterial
                staticFriction: staticFrictionSlider.value
                dynamicFriction: dynamicFrictionSlider.value
                restitution: restitutionSlider.value
            }

            DynamicRigidBody {
                id: floor
                isKinematic: true
                eulerRotation: Qt.vector3d(-79, -90, 0)
                kinematicEulerRotation: Qt.vector3d(-79, -90, 0)
                scale: Qt.vector3d(0.3, 0.3, 0.3)
                physicsMaterial: testMaterial
                collisionShapes: PlaneShape {
                    enableDebugDraw: true
                }
                Model {
                    source: "#Rectangle"
                    materials: PrincipledMaterial {
                        baseColor: "green"
                    }
                }
            }

            DynamicRigidBody {
                id: box
                physicsMaterial: testMaterial
                mass: 50

                function init() {
                    console.log("Reinit pos")
                    box.reset(Qt.vector3d(0, 5.9, 0), Qt.vector3d(0, 0, 0));
                }

                y: 0.9
                Model {
                    source: "#Cube"
                    materials: PrincipledMaterial {
                        baseColor: "red"
                    }
                    scale: Qt.vector3d(0.01, 0.01, 0.01)
                }
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(1, 1, 1)
                    enableDebugDraw: true
                }
            }
        } // scene

    } // View3D

    WasdController {
        keysEnabled: true
        property bool controllingUnit: false
        controlledObject: camera1
        speed: 0.02
        shiftSpeed: 0.2

        Keys.onPressed: (event)=> {
            if (keysEnabled) handleKeyPress(event);
            if (event.key === Qt.Key_Space) {
                if (box.isKinematic) {
                    console.log("set box to moving")
                    box.isKinematic = false
                    physicsWorld.running = true
                } else {
                    physicsWorld.running = !physicsWorld.running
                    console.log("Set physics to", physicsWorld.running)
                }
            } else if (event.key === Qt.Key_J) {
                floor.kinematicEulerRotation.x++
            } else if (event.key === Qt.Key_K) {
                floor.kinematicEulerRotation.x--
            } else if (event.key === Qt.Key_I) {
                box.init()
            }
        }
        Keys.onReleased: (event)=> { if (keysEnabled) handleKeyRelease(event) }
    }

    ColumnLayout {

        Label {
            text: "Static friction: " + staticFrictionSlider.value
        }
        Slider {
            id: staticFrictionSlider
            focusPolicy: Qt.NoFocus
            from: 0
            to: 1
            value: 0.1
        }

        Label {
            text: "Dynamic friction: " + dynamicFrictionSlider.value
        }
        Slider {
            id: dynamicFrictionSlider
            focusPolicy: Qt.NoFocus
            from: 0
            to: 1
            value: 0.1
        }

        Label {
            text: "Restitution: " + restitutionSlider.value
        }
        Slider {
            id: restitutionSlider
            focusPolicy: Qt.NoFocus
            from: 0
            to: 1
            value: 0.1
        }

        Label {
            text: "Typical length: " + typicalLengthSlider.value
        }
        Slider {
            id: typicalLengthSlider
            focusPolicy: Qt.NoFocus
            from: 0
            to: 10
            value: 1
        }

        Label {
            text: "Typical speed: " + typicalSpeedSlider.value
        }
        Slider {
            id: typicalSpeedSlider
            focusPolicy: Qt.NoFocus
            from: 0
            to: 10
            value: 1
        }
    }
}
