// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import QtQuick.Controls
import QtQuick.Layouts


Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("QtQuick3DPhysics character controller test")


    PhysicsWorld {
        id: physicsWorld
        running: true
        forceDebugDraw: false
        //        enableCCD: true
        scene: viewport.scene
    }

    View3D {
        anchors.fill: parent
        id: viewport



    environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.SkyBox
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            lightProbe: Texture {
                source: "maps/OpenfootageNET_lowerAustria01-512.hdr"
            }
            skyboxBlurAmount: 0.22
            probeExposure: 5
        }

        StaticRigidBody {
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
        }

        PrincipledMaterial {
            id: stairMaterial
            metalness: 0
            roughness: 0.5
            baseColor: "orange"
            alphaMode: PrincipledMaterial.Opaque
        }
        PrincipledMaterial {
            id: wallMaterial
            metalness: 0
            roughness: 0.5
            baseColor: "gray"
            alphaMode: PrincipledMaterial.Opaque
        }

        Node {
            id: scene

            DirectionalLight {
                eulerRotation.x: -25
                eulerRotation.y: 45
                castsShadow: true
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                shadowFactor: 50
            }

            StaticRigidBody {
                collisionShapes: TriangleMeshShape {
                    id: roomShape
                    source: "meshes/room.mesh"
                }

                Model {
                    id: room
                    source: "meshes/room.mesh"
                    materials: [
                    wallMaterial
                    ]

                }
            }
            StaticRigidBody {
                    x: -600
                    y: 0
                    z: 600
                collisionShapes: TriangleMeshShape {
                    id: stairShape
                    source: "meshes/stairs.mesh"
                }

                Model {
                    id: stairs
                    source: "meshes/stairs.mesh"
                    materials: [
                    stairMaterial
                    ]
                }
            }

            Model {
                id: scenery
                source: "#Cube"
                materials: stairMaterial
                z: -5000
                scale: "10, 0.1, 10"
            }

            Model {
                id: scenery2
                source: "#Cube"
                materials: wallMaterial
                z: -7000
                scale: "10, 0.1, 10"
            }

            Model {
                id: scenery3
                source: "#Cube"
                materials: stairMaterial
                z: -15000
                scale: "10, 0.1, 100"
            }

            Repeater3D {
                model: 10
                StaticRigidBody {
                    id: platform
                    x: 300
                    z: -1300 - 600 * index
                    scale: "2, 0.3, 2"
                    y: 500 + 50 * index
                    collisionShapes: BoxShape {}
                    Model {
                        source: "#Cube"
                        materials: stairMaterial
                    }

                }
            }

            CharacterController {
                id: character
                position: "100, 100, 300"
                eulerRotation.y: wasd.cameraRotation.x

                gravity: flying ? Qt.vector3d(0,0,0) : physicsWorld.gravity
                midAirControl: midAirButton.checked
                property bool physicallyCorrect: correctnessButton.checked
                property bool flying: flyingButton.checked

                property bool grounded: (collisions & CharacterController.Down)
                property bool jumpingAllowed : false

                Timer {
                    id: groundednessTimer
                    // Wile E. Coyote mode: how long after you started to fall
                    // can you still jump? (Only in non-physically-correct mode.)
                    interval: 150
                }
                onGroundedChanged: {
                    groundednessTimer.stop()
                    if (grounded) {
                        // Landing is always immediate, but we don't jump in flying mode
                        jumpingAllowed = !flying
                    } else if (!midAirControl || wasd.jump) {
                        // Jumping is always immediate (no double jumping)
                        jumpingAllowed = false
                    } else {
                        // Walking off a ledge in mid-air mode has a grace period
                        groundednessTimer.restart()
                    }
                }

                property string collisionText: ""
                onCollisionsChanged: {
                    if (collisions === CharacterController.None) {
                        collisionText = " No collisions"
                    } else {
                        collisionText = (collisions & CharacterController.Up ? " Above" : "")
                                     + (collisions & CharacterController.Side ? " Side" : "")
                                     + ((collisions & CharacterController.Down) ? " Below" : "")
                    }
                    console.log("Collision state:" + collisionText + " (" + collisions + ")")
                }

                movement.x: wasd.xFactor * 500;
                movement.z: wasd.zFactor * 500;
                movement.y: ((wasd.jump && jumpingAllowed) ? 500 : 0)
                          + wasd.yFactor * 500
                Behavior on movement.z {
                    PropertyAnimation { duration: 200 }
                }
                Behavior on movement.x {
                    PropertyAnimation { duration: 200 }
                }

                collisionShapes:  CapsuleShape {
                    id: capsuleShape
                }

                Model {
                    // not visible from the camera, but casts a shadow
                    eulerRotation.z: 90
                    geometry: CapsuleGeometry {}
                    materials: PrincipledMaterial {
                        baseColor: "red"
                    }
                }

                PerspectiveCamera {
                    id: camera2
                    position: Qt.vector3d(0, capsuleShape.height, 0)
                    eulerRotation.x: wasd.cameraRotation.y
                    clipFar: 10000
                    clipNear: 10
                }
            }
        }
        Wasd {
            id: wasd
            property real sprintFactor: sprintActive ? 3 : 1
            property real xFactor: (moveLeft ? -1 : moveRight ? 1 : 0) * sprintFactor
            property real zFactor: (moveForwards ? -1 : moveBackwards ? 1 : 0) * sprintFactor
            property real yFactor: (character.flying ? (moveUp ? 1 : moveDown ? -1 : 0) : 0) * sprintFactor

            property bool jump: false

            Timer {
                id: preJumpTimer
                interval: 200 // how long before you land can you press jump?
                onTriggered: wasd.jump = false
            }

            function startJump()
            {
                wasd.jump = true
                preJumpTimer.restart()
            }

            Keys.onPressed: (event)=> {
                handleKeyPress(event);
                if (event.key === Qt.Key_Space && !event.isAutoRepeat) {
                    wasd.startJump()
                } else if (event.key === Qt.Key_G) {
                }
            }

            Keys.onReleased: (event) => {
                handleKeyRelease(event)
                if (event.key === Qt.Key_Space) {
                    wasd.jump = false
                }
            }
        }

    } // View3D

    ButtonGroup {
        buttons: column.children
    }

    Column {
        id: column
        RadioButton {
            id: midAirButton
            checked: true
            text: qsTr("Mid-air controls")
            focusPolicy: Qt.NoFocus
        }
        RadioButton {
            id: correctnessButton
            text: qsTr("Physically accurate free fall")
            focusPolicy: Qt.NoFocus
        }
        RadioButton {
            id: flyingButton
            text: qsTr("Flying (no gravity)")
            focusPolicy: Qt.NoFocus
        }

        Button {
            text: "Teleport"
            onClicked: character.teleport(Qt.vector3d(-700, 3000, -6850))
            focusPolicy: Qt.NoFocus
        }
        Text {
            text: "Current position: " + character.position
        }
        Text {
            text: "Collision state:" + character.collisionText
        }
    }
}
