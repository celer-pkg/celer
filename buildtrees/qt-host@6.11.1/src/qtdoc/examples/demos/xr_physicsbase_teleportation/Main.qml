// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import QtQuick3D.Xr

XrView {
    id: xrView
    XrErrorDialog { id: err }
    onInitializeFailed: (errorString) => err.run("XR physics-base teleportation", errorString)
    referenceSpace: XrView.ReferenceSpaceStage

    environment: ExtendedSceneEnvironment {
        id: extendedSceneEnvironment
        backgroundMode: SceneEnvironment.Color
        probeExposure: 0.7
        lightProbe: Texture {
            source: "media/textures/OpenfootageNET_lowerAustria01-1024.hdr"
        }
        skyboxBlurAmount: 0.0
        specularAAEnabled: true
        clearColor: theFog.color
        exposure: teleporter.screenVisibility

        fog: Fog {
            id: theFog
            color:"gray"
            enabled: true
            depthEnabled: true
            depthNear: 1500
            depthFar: 3000
        }
    }

    Component.onCompleted: {
        ParticleResources.init(xrView)
    }

    xrOrigin: XrOrigin {
        id: xrOrigin

        camera: XrCamera {
            id: trackingCamera
            Prompter {
                id: prompter
            }
        }

        XrController {
            id: xrLeftController
            controller: XrController.ControllerLeft
            poseSpace: XrController.AimPose

            TorchGripper {
                hand: XrInputAction.LeftHand
            }
        }

        XrController {
            id: xrRightController
            controller: XrController.ControllerRight
            poseSpace: XrController.AimPose

            XrInputAction {
                id: thumbstickX
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.ThumbstickX]
            }

            XrInputAction {
                id: thumbstickY
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.ThumbstickY]
            }

            XrInputAction {
                id: trackpadX
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.TrackpadX]
            }

            XrInputAction {
                id: trackpadY
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.TrackpadY]
            }

            XrInputAction {
                id: trackpadPressed
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.TrackpadPressed]
            }

            TorchGripper {
                hand: XrInputAction.RightHand
            }

            property real xValue: trackpadPressed.pressed ? trackpadX.value : thumbstickX.value
            property real yValue: trackpadPressed.pressed ? trackpadY.value : thumbstickY.value
        }
    }

    PhysicsbaseTeleporter {
        id: teleporter
        rayPicker: xrView
        cameraOrigin: xrOrigin
        camera: xrOrigin.camera
        beamHandle: xrRightController
        characterController: character
        xStickValue: xrRightController.xValue
        yStickValue: xrRightController.yValue
        onDoTeleportation: (cameraOriginPosition, characterControllerPosition)=> {
                               xrOrigin.position = cameraOriginPosition
                               character.teleport(characterControllerPosition)
                           }
        onDoRotation: (cameraOriginRotation, cameraOriginPosition)=> {
                          xrOrigin.rotation = cameraOriginRotation
                          xrOrigin.position = cameraOriginPosition
                      }
    }

    PhysicsWorld {
        id: physicsWorld
        running: true
        scene: xrView
        gravity: Qt.vector3d(0, -981, 0)
    }

    CharacterController {
        id: character
        property bool inTheHouse: true
        signal enteredTheHouse
        signal leftTheHouse

        onEnteredTheHouse: {
            inTheHouse = true
        }
        onLeftTheHouse: {
            inTheHouse = false
        }

        property vector3d startPos: Qt.vector3d(0, 200, 0)
        scale: Qt.vector3d(0.5, 1.75, 0.5)
        Component.onCompleted: {
            prompter.prompt = Prompter.DarknessFalling
            prompter.prompt = Prompter.PickupTorch
            teleporter.teleportTo(character.startPos)
        }

        collisionShapes: CapsuleShape {
            id: capsuleShape
        }

        onShapeHit: (body, position, impulse, normal)=>{
                        if (body.objectName === "Ground")
                        leftTheHouse()
                        else if (body.objectName === "HouseFloor")
                        enteredTheHouse()
                    }

        enableShapeHitCallback: true
        sendTriggerReports: true
        gravity: physicsWorld.gravity.times(10) //make it fall faster
    }

    Ground {

    }

    Fence {

    }

    House {
        id: house
    }

    Chair {
        eulerRotation: Qt.vector3d(0, -45, 0)
        position: Qt.vector3d(-200, 200, -300)
    }

    Table {
        position: Qt.vector3d(-30, 200, -300)
    }

    Torch {
        id: torch
        position: Qt.vector3d(0, 350, -300)
        eulerRotation: Qt.vector3d(-90, 0, 0)
        windEnabled: !character.inTheHouse
        houseFire: house.fire
        onAttachedToChanged: {
            if (attachedTo)
                prompter.prompt = Prompter.LightupTorch
        }

        onIsOnChanged: {
            prompter.prompt = Prompter.DefendCrop
        }
    }

    Grass {
        id: field
        z: -1200
    }

    Stands {

    }

    Campfire {
        id: campFire1
        position: Qt.vector3d(1000, 185 ,1000)
        function updateColors(){
            if (campFire1.isOn && campFire2.isOn &&
                    campFire3.isOn && campFire4.isOn) {
                campFire1.fireColor = "white"
                campFire1.lightColor = Qt.lighter("orange", 1.5)
                enemy.run = true
            }else {
                campFire1.fireColor = "orange"
                campFire1.lightColor = "orange"
                enemy.run = false
            }

            campFire2.fireColor = campFire1.fireColor
            campFire2.lightColor = campFire1.lightColor
            campFire3.fireColor = campFire1.fireColor
            campFire3.lightColor = campFire1.lightColor
            campFire4.fireColor = campFire1.fireColor
            campFire4.lightColor = campFire1.lightColor
        }
        onIsOnChanged: {
            campFire1.updateColors()
        }
    }

    Campfire {
        id: campFire2
        position: Qt.vector3d(1000, 185 ,-1000)
        onIsOnChanged: {
            campFire1.updateColors()
        }
    }

    Campfire {
        id: campFire3
        position: Qt.vector3d(-1000, 185 ,1000)
        onIsOnChanged: {
            campFire1.updateColors()
        }
    }

    Campfire {
        id: campFire4
        position: Qt.vector3d(-1000, 185 ,-1000)
        onIsOnChanged: {
            campFire1.updateColors()
        }
    }

    Smoke {
        id: enemy
        readonly property real maxY: 5000
        readonly property real startY: 40000
        property bool run: false
        position: Qt.vector3d(0, startY, 0)
        color: "black"
        colorVariation: 0.0
        size: run ? 6.0 : 4.0

        FrameAnimation {
            running: true
            property real time: 0
            property real fieldEncounterTime: 0
            onTriggered: {
                time += 0.5 * frameTime
                let enemyLinearY = enemy.scenePosition.y / enemy.maxY
                let enemyOffset = enemyLinearY * 5000 + 200
                let targetBase = field.scenePosition
                let enemyMovementSpeed = 20.
                let chaseTheCamera = false

                if (fieldEncounterTime > 0.99 && !character.inTheHouse && !enemy.run) {
                    targetBase = xrOrigin.camera.scenePosition
                    enemyOffset = 20
                    chaseTheCamera = true
                    enemyMovementSpeed = 5
                }

                let target = Qt.vector3d(targetBase.x + Math.cos(time) * enemyOffset ,
                                         targetBase.y + Math.abs(Math.cos(time)) * 100,
                                         targetBase.z + Math.sin(time) * enemyOffset)
                if (torch.isOn) {
                    let enemyToTorchDir =  torch.scenePosition.minus(enemy.position)
                    let enemyToTorchLen = enemyToTorchDir.length()
                    let dis = Math.max(0, 500 - enemyToTorchLen)
                    target.y += dis * dis * dis
                }

                if (enemy.run)
                    target.y += enemy.maxY

                if (!chaseTheCamera) {
                    field.baseColor = ""

                    if (field.scenePosition.minus(enemy.scenePosition).length() < 500)
                        fieldEncounterTime += frameTime * 0.02
                    else
                        fieldEncounterTime -= frameTime * 0.01

                    fieldEncounterTime = Math.min(Math.max(fieldEncounterTime, 0), 1.)

                    field.baseColor = Qt.darker("white", Math.exp(fieldEncounterTime))
                }

                let enemyToTargetDir = target.minus(enemy.scenePosition).normalized().times(enemyMovementSpeed)
                enemy.position = enemy.scenePosition.plus(enemyToTargetDir)
            }
        }
    }
}
