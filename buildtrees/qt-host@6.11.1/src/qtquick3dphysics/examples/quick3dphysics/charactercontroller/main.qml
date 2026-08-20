// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("Qt Quick 3D Physics - Character Controller")
    PhysicsWorld {
        id: physicsWorld
        running: true
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#8090d0"
            backgroundMode: SceneEnvironment.SkyBox
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {}
            }
            specularAAEnabled: true
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: "#666"
            brightness: 0.1
        }

        //! [building]
        Building {
            id: building
            onTeleporterTriggered: character.teleportHome()
        }
        //! [building]

        //! [charactercontroller]
        CharacterController {
            id: character
            //! [position]
            property vector3d startPos: Qt.vector3d(800, 175, -850)
            position: startPos
            function teleportHome() {
                character.teleport(character.startPos)
                wasd.cameraRotation.x = 180
            }
            //! [position]

            //! [capsuleshape]
            collisionShapes: CapsuleShape {
                id: capsuleShape
                diameter: 50
                height: wasd.crouchActive ? 0 : 100
                Behavior on height {
                    NumberAnimation { duration: 300 }
                }
            }
            property real characterHeight: capsuleShape.height + capsuleShape.diameter
            //! [capsuleshape]

            //! [triggerreports]
            sendTriggerReports: true
            //! [triggerreports]

            //! [movement]
            movement: Qt.vector3d(wasd.sideSpeed, 0, wasd.forwardSpeed)
            Behavior on movement {
                PropertyAnimation { duration: 200 }
            }
            //! [movement]

            //! [gravity]
            gravity: building.inGravityField ? Qt.vector3d(0, 100, 0) : physicsWorld.gravity
            //! [gravity]

            //! [camera]
            eulerRotation.y: wasd.cameraRotation.x
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, character.characterHeight / 2 - 10, 0)
                eulerRotation.x: wasd.cameraRotation.y
                clipFar: 10000
                clipNear: 10
            }
            //! [camera]
        }
        //! [charactercontroller]
    }

    //! [wasd]
    Wasd {
        id: wasd
        property real walkingSpeed: 500
        property real speedFactor: sprintActive ? 3 : crouchActive ? 0.3 : 1
        property real sideSpeed: (moveLeft ? -1 : moveRight ? 1 : 0) * walkingSpeed * speedFactor
        property real forwardSpeed: (moveForwards ? -1 : moveBackwards ? 1 : 0) * walkingSpeed * speedFactor
        cameraRotation.x: 180
    }
    //! [wasd]
}
